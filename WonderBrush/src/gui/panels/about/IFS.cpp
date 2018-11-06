// IFS.cpp
//
// changed into a C++ class, modified to use the Be API
// Stephan Aßmus, <stippi@yellowbites.com>
//
// original copyright header follows:
//
// P.S. I added the ya_random* functions because I had linker problems
// I didn't understand and know how to solve...
//
/* ifs --- modified iterated functions system */

/*-
 * Copyright (c) 1997 by Massimino Pascal <Pascal.Massimon@ens.fr>
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 * If this mode is weird and you have an old MetroX server, it is buggy.
 * There is a free SuSE-enhanced MetroX X server that is fine.
 *
 * When shown ifs, Diana Rose (4 years old) said, "It looks like dancing."
 *
 * Revision History:
 * 25-Jan-2004: Ported to BScreenSaver API by Stephan Aßmus
 * 01-Nov-2000: Allocation checks
 * 10-May-1997: jwz@jwz.org: turned into a standalone program.
 *              Made it render into an offscreen bitmap and then copy
 *              that onto the screen, to reduce flicker.
 */

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <OS.h>
#include <Screen.h>
#include <View.h>

#include <unistd.h>  // for getpid()
#include <sys/time.h> // for gettimeofday()

#include "IFS.h"

#define HALF 0

//#include "yarandom.h"


// The following 'random' numbers are taken from CRC, 18th Edition, page 622.
// Each array element was taken from the corresponding line in the table,
// except that a[0] was from line 100. 8s and 9s in the table were simply
// skipped. The high order digit was taken mod 4.

#define VectorSize 55
static unsigned int a[VectorSize] = {
 035340171546, 010401501101, 022364657325, 024130436022, 002167303062, //  5
 037570375137, 037210607110, 016272055420, 023011770546, 017143426366, // 10
 014753657433, 021657231332, 023553406142, 004236526362, 010365611275, // 14
 007117336710, 011051276551, 002362132524, 001011540233, 012162531646, // 20
 007056762337, 006631245521, 014164542224, 032633236305, 023342700176, // 25
 002433062234, 015257225043, 026762051606, 000742573230, 005366042132, // 30
 012126416411, 000520471171, 000725646277, 020116577576, 025765742604, // 35
 007633473735, 015674255275, 017555634041, 006503154145, 021576344247, // 40
 014577627653, 002707523333, 034146376720, 030060227734, 013765414060, // 45
 036072251540, 007255221037, 024364674123, 006200353166, 010126373326, // 50
 015664104320, 016401041535, 016215305520, 033115351014, 017411670323  // 55
};

static int i1, i2;

// ya_random
unsigned int
ya_random (void)
{
  register int ret = a[i1] + a[i2];
  a[i1] = ret;
  if (++i1 >= VectorSize) i1 = 0;
  if (++i2 >= VectorSize) i2 = 0;
  return ret;
}

// ya_rand_init
void
ya_rand_init(unsigned int seed)
{
  int i;
  if (seed == 0)
    {
      struct timeval tp;
      struct timezone tzp;
      gettimeofday(&tp, &tzp);
      /* ignore overflow */
      seed = (999*tp.tv_sec) + (1001*tp.tv_usec) + (1003 * getpid());
    }

  a[0] += seed;
  for (i = 1; i < VectorSize; i++)
    {
      seed = a[i-1]*1001 + seed*999;
      a[i] += seed;
    }

  i1 = a[0] % VectorSize;
  i2 = (i1 + 024) % VectorSize;
}

#define random()   ya_random()
#define RAND_MAX   0xFFFFFFFF

#define FLOAT_TO_INT(x)  (int32)( (float)(UNIT)*(x) )

#define LRAND()			((long) (random() & 0x7fffffff))
#define NRAND(n)		((int) (LRAND() % (n)))
#define MAXRAND			(2147483648.0) // unsigned 1<<31 as a float
#define SRAND(n)		// already seeded by screenhack.c TODO: ?!? is it?


// gauss_rand
static float
gauss_rand(float c, float A, float S)
{
	float         y;

	y = (float) LRAND() / MAXRAND;
	y = A * (1.0 - exp(-y * y * S)) / (1.0 - exp(-S));
	if (NRAND(2))
		return (c + y);
	return (c - y);
}

// half_gauss_rand
static float
half_gauss_rand(float c, float A, float S)
{
	float         y;

	y = (float) LRAND() / MAXRAND;
	y = A * (1.0 - exp(-y * y * S)) / (1.0 - exp(-S));
	return (c + y);
}

// transform
inline void
transform(SIMI* Simi, int32 xo, int32 yo, int32* x, int32* y)
{
	int32        xx, yy;

	xo = xo - Simi->Cx;
	xo = (xo * Simi->R) / UNIT;
	yo = yo - Simi->Cy;
	yo = (yo * Simi->R) / UNIT;

	xx = xo - Simi->Cx;
	xx = (xx * Simi->R2) / UNIT;
	yy = -yo - Simi->Cy;
	yy = (yy * Simi->R2) / UNIT;

	*x = ((xo * Simi->Ct - yo * Simi->St + xx * Simi->Ct2 - yy * Simi->St2) / UNIT) + Simi->Cx;
	*y = ((xo * Simi->St + yo * Simi->Ct + xx * Simi->St2 + yy * Simi->Ct2) / UNIT) + Simi->Cy;
}


// constructor
IFS::IFS(BRect bounds)
	: fRoot(NULL),
	  fAdditive(true)
{
	ya_rand_init(system_time());

	int         i;
	FRACTAL    *Fractal;

	if (fRoot == NULL) {
		fRoot = (FRACTAL*) calloc(1, sizeof (FRACTAL));
		if (fRoot == NULL)
			return;
	}
	Fractal = fRoot;

	_FreeBuffers(Fractal);
	i = (NRAND(4)) + 2;	// Number of centers
	switch (i) {
		case 3:
			Fractal->Depth = MAX_DEPTH_3;
			Fractal->r_mean = .6;
			Fractal->dr_mean = .4;
			Fractal->dr2_mean = .3;
			break;

		case 4:
			Fractal->Depth = MAX_DEPTH_4;
			Fractal->r_mean = .5;
			Fractal->dr_mean = .4;
			Fractal->dr2_mean = .3;
			break;

		case 5:
			Fractal->Depth = MAX_DEPTH_5;
			Fractal->r_mean = .5;
			Fractal->dr_mean = .4;
			Fractal->dr2_mean = .3;
			break;

		default:
		case 2:
			Fractal->Depth = MAX_DEPTH_2;
			Fractal->r_mean = .7;
			Fractal->dr_mean = .3;
			Fractal->dr2_mean = .4;
			break;
	}
	// fprintf( stderr, "N=%d\n", i );
	Fractal->Nb_Simi = i;
	Fractal->Max_Pt = Fractal->Nb_Simi - 1;
	for (i = 0; i <= Fractal->Depth + 2; ++i)
		Fractal->Max_Pt *= Fractal->Nb_Simi;

	if ((Fractal->buffer1 = (Point *) calloc(Fractal->Max_Pt,
			sizeof (Point))) == NULL) {
		_FreeIFS(Fractal);
		return;
	}
	if ((Fractal->buffer2 = (Point *) calloc(Fractal->Max_Pt,
			sizeof (Point))) == NULL) {
		_FreeIFS(Fractal);
		return;
	}
	Fractal->Speed = 6;
#if HALF
	Fractal->Width = bounds.IntegerWidth() / 2 + 1;
	Fractal->Height = bounds.IntegerHeight() / 2 + 1;
#else
	Fractal->Width = bounds.IntegerWidth() + 1;
	Fractal->Height = bounds.IntegerHeight() + 1;
#endif
	Fractal->Cur_Pt = 0;
	Fractal->Count = 0;
	Fractal->Lx = (Fractal->Width - 1) / 2;
	Fractal->Ly = (Fractal->Height - 1) / 2;
	Fractal->Col = NRAND(Fractal->Width * Fractal->Height - 1) + 1;

	_RandomSimis(Fractal, Fractal->Components, 5 * MAX_SIMI);

	if (Fractal->bitmap != NULL)
		delete Fractal->bitmap;
	Fractal->bitmap = new BBitmap(BRect(0.0, 0.0,
										Fractal->Width - 1,
										Fractal->Height - 1),
										0,//B_BITMAP_IS_OFFSCREEN,
										B_RGB32);
	// Allocation checked
	if (Fractal->bitmap != NULL && Fractal->bitmap->IsValid()) {
		memset(Fractal->bitmap->Bits(), 0, Fractal->bitmap->BitsLength());
	} else {
		delete Fractal->bitmap;
		Fractal->bitmap = NULL;
	}
}

// destructor
IFS::~IFS()
{
	if (fRoot != NULL) {
		_FreeIFS(fRoot);
		free((void*) fRoot);
	}
}

// Draw
void
IFS::Draw(const BBitmap* bitmap)
{
//bigtime_t now = system_time();
	int         i;
	float         u, uu, v, vv, u0, u1, u2, u3;
	SIMI       *S, *S1, *S2, *S3, *S4;
	FRACTAL    *F;

	if (fRoot == NULL)
		return;
	F = fRoot;
	if (F->buffer1 == NULL)
		return;


	u = (float) (F->Count) * (float) (F->Speed) / 1000.0;
	uu = u * u;
	v = 1.0 - u;
	vv = v * v;
	u0 = vv * v;
	u1 = 3.0 * vv * u;
	u2 = 3.0 * v * uu;
	u3 = u * uu;

	S = F->Components;
	S1 = S + F->Nb_Simi;
	S2 = S1 + F->Nb_Simi;
	S3 = S2 + F->Nb_Simi;
	S4 = S3 + F->Nb_Simi;

	for (i = F->Nb_Simi; i; --i, S++, S1++, S2++, S3++, S4++) {
		S->c_x = u0 * S1->c_x + u1 * S2->c_x + u2 * S3->c_x + u3 * S4->c_x;
		S->c_y = u0 * S1->c_y + u1 * S2->c_y + u2 * S3->c_y + u3 * S4->c_y;
		S->r = u0 * S1->r + u1 * S2->r + u2 * S3->r + u3 * S4->r;
		S->r2 = u0 * S1->r2 + u1 * S2->r2 + u2 * S3->r2 + u3 * S4->r2;
		S->A = u0 * S1->A + u1 * S2->A + u2 * S3->A + u3 * S4->A;
		S->A2 = u0 * S1->A2 + u1 * S2->A2 + u2 * S3->A2 + u3 * S4->A2;
	}

//bigtime_t beforeDraw = system_time();
	_DrawFractal(bitmap);

//bigtime_t draw = system_time() - beforeDraw;

	if (F->Count >= 1000 / F->Speed) {
		S = F->Components;
		S1 = S + F->Nb_Simi;
		S2 = S1 + F->Nb_Simi;
		S3 = S2 + F->Nb_Simi;
		S4 = S3 + F->Nb_Simi;

		for (i = F->Nb_Simi; i; --i, S++, S1++, S2++, S3++, S4++) {
			S2->c_x = 2.0 * S4->c_x - S3->c_x;
			S2->c_y = 2.0 * S4->c_y - S3->c_y;
			S2->r = 2.0 * S4->r - S3->r;
			S2->r2 = 2.0 * S4->r2 - S3->r2;
			S2->A = 2.0 * S4->A - S3->A;
			S2->A2 = 2.0 * S4->A2 - S3->A2;

			*S1 = *S4;
		}
		_RandomSimis(F, F->Components + 3 * F->Nb_Simi, F->Nb_Simi);

		_RandomSimis(F, F->Components + 4 * F->Nb_Simi, F->Nb_Simi);

		F->Count = 0;
	} else
		F->Count++;
//		F->Col++;
//bigtime_t finish = (system_time() - now) - draw;
//if (info)
//printf("draw: %lld\nnon-draw: %lld\n\n", draw, finish);

}

// SetAdditive
void
IFS::SetAdditive(bool additive)
{
	fAdditive = additive;
}

// SetSpeed
void
IFS::SetSpeed(int32 speed)
{
	if (fRoot && speed > 0 && speed <= 12)
		fRoot->Speed = speed;
}

// _DrawFractal
void
IFS::_DrawFractal(const BBitmap* bitmap)
{
	FRACTAL    *F = fRoot;
	int         i, j;
	int32        x, y, xo, yo;
	SIMI       *Cur, *Simi;

	for (Cur = F->Components, i = F->Nb_Simi; i; --i, Cur++) {
		Cur->Cx = FLOAT_TO_INT(Cur->c_x);
		Cur->Cy = FLOAT_TO_INT(Cur->c_y);

		Cur->Ct = FLOAT_TO_INT(cos(Cur->A));
		Cur->St = FLOAT_TO_INT(sin(Cur->A));
		Cur->Ct2 = FLOAT_TO_INT(cos(Cur->A2));
		Cur->St2 = FLOAT_TO_INT(sin(Cur->A2));

		Cur->R = FLOAT_TO_INT(Cur->r);
		Cur->R2 = FLOAT_TO_INT(Cur->r2);
	}


	fCurrentPoint = 0;
	fCurrentFractal = F;
	fPointBuffer = F->buffer2;
	for (Cur = F->Components, i = F->Nb_Simi; i; --i, Cur++) {
		xo = Cur->Cx;
		yo = Cur->Cy;
		for (Simi = F->Components, j = F->Nb_Simi; j; --j, Simi++) {
			if (Simi == Cur)
				continue;
			transform(Simi, xo, yo, &x, &y);
			_Trace(F, x, y);
		}
	}

	if (F->bitmap != NULL) {
		uint8* bits = (uint8*)F->bitmap->Bits();
		uint32 bpr = F->bitmap->BytesPerRow();
		// draw the new dots into the bitmap
		if (fCurrentPoint) {
			for (int32 i = 0; i <  fCurrentPoint; i++) {
				Point p = F->buffer2[i];
				if (p.x >= 0 && p.x < F->Width
					&& p.y >= 0 && p.y < F->Height) {
					int32 offset = bpr * p.y + p.x * 4;
					if (fAdditive) {
						if (bits[offset + 2] < 245) {
//							bits[offset + 0] += 51;
							bits[offset + 1] += 2;
							bits[offset + 2] += 10;
						}
					} else {
						*(uint32*)&bits[offset] = 0x0000ffff;
					}
				}
			}
		}
		if (bitmap && bitmap->IsValid()) {
	
			uint8* screenBits = (uint8*)bitmap->Bits();
			uint32 screenBPR = bitmap->BytesPerRow();
	
			int32 screenWidth = bitmap->Bounds().IntegerWidth() + 1;
			int32 screenHeight = bitmap->Bounds().IntegerHeight() + 1;

			// redraw the previous points on screen
			// with the contents of the current bitmap
			//
			// draw the new points, erasing the bitmap as we go
			int32 maxPoints = max_c(F->Cur_Pt, fCurrentPoint);
			if (maxPoints > 0) {				

				for (int32 i = 0; i < maxPoints; i++) {
					if (i < F->Cur_Pt) {
						Point p = F->buffer1[i];
						if (p.x >= 0 && p.x < F->Width && p.x < screenWidth
							&& p.y >= 0 && p.y < F->Height && p.y < screenHeight) {
							int32 offset = bpr * p.y + p.x * 4;
							int32 screenOffset = screenBPR * p.y + p.x * 4;
							*(uint32*)&screenBits[screenOffset] = *(uint32*)&bits[offset];
						}
					}
					if (i < fCurrentPoint) {
						Point p = F->buffer2[i];
						if (p.x >= 0 && p.x < F->Width && p.x < screenWidth
							&& p.y >= 0 && p.y < F->Height && p.y < screenHeight) {
							int32 offset = bpr * p.y + p.x * 4;
							if (bits[offset]) {
								int32 screenOffset = screenBPR * p.y + p.x * 4;
//								uint32* src = (uint32*)&bits[offset];
								// copy the pixel to the screen
								*(uint32*)&screenBits[screenOffset] = *(uint32*)&bits[offset];
								// set the src to zero already
//								*src = 0;
							} // else it means the pixel has been copied already
						}
					}
				}
			}
			memset(bits, 0, bitmap->BitsLength());
		}
	}

	// flip buffers
	F->Cur_Pt = fCurrentPoint;
	fPointBuffer = F->buffer1;
	F->buffer1 = F->buffer2;
	F->buffer2 = fPointBuffer;
}

// _Trace
void
IFS::_Trace(FRACTAL* F, int32 xo, int32 yo)
{
	int32 x, y, i;
	SIMI* Cur;

	Cur = fCurrentFractal->Components;
	for (i = fCurrentFractal->Nb_Simi; i; --i, Cur++) {
		transform(Cur, xo, yo, &x, &y);
		// fPointBuffer->x = F->Lx + (x * F->Lx / (UNIT * 2));
		// fPointBuffer->y = F->Ly - (y * F->Ly / (UNIT * 2));
		fPointBuffer->x = (UNIT * 2 + x) * F->Lx / (UNIT * 2);
		fPointBuffer->y = (UNIT * 2 - y) * F->Ly / (UNIT * 2);
		fPointBuffer++;
		fCurrentPoint++;

		if (F->Depth && ((x - xo) >> 4) && ((y - yo) >> 4)) {
			F->Depth--;
			_Trace(F, x, y);
			F->Depth++;
		}
	}
}

// _RandomSimis
void
IFS::_RandomSimis(FRACTAL* f, SIMI* cur, int i) const
{
	while (i--) {
		cur->c_x = gauss_rand(0.0, .8, 4.0);
		cur->c_y = gauss_rand(0.0, .8, 4.0);
		cur->r = gauss_rand(f->r_mean, f->dr_mean, 3.0);
		cur->r2 = half_gauss_rand(0.0,f->dr2_mean, 2.0);
		cur->A = gauss_rand(0.0, 360.0, 4.0) * (M_PI / 180.0);
		cur->A2 = gauss_rand(0.0, 360.0, 4.0) * (M_PI / 180.0);
		cur++;
	}
}

// _FreeBuffers
void
IFS::_FreeBuffers(FRACTAL *f)
{
	if (f->buffer1) {
		free((void*) f->buffer1);
		f->buffer1 = (Point*)NULL;
	}
	if (f->buffer2) {
		free((void*) f->buffer2);
		f->buffer2 = (Point*)NULL;
	}
}

// _FreeIFS
void
IFS::_FreeIFS(FRACTAL* f)
{
	_FreeBuffers(f);
	if (f->bitmap != NULL) {
		delete f->bitmap;
		f->bitmap = NULL;
	}
}



