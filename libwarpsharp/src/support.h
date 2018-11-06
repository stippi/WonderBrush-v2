// support.h

#ifndef SUPPORT_H
#define SUPPORT_H

#include <SupportDefs.h>

#include "complex.h"

class BBitmap;
class BFile;

typedef complex point;

const double TINY = 1.0e-20;
const double HUGE = 1.0e20;
const double SQRT2 =1.4142135623730950488;

enum {
	FORWARD		= 1,
	BACKWARD	= 0,

	X_DIR		= 0,
	Y_DIR		= 1,

	IMAGINARY	= 1,
	REAL		= 0,

	RED			= 0,
	GREEN		= 1,
	BLUE		= 2,
};

#define sqr(x) ((x)*(x))
#define max(a,b)   ((a)<(b) ? (b) : (a))
#define min(a,b)   ((a)>(b) ? (b) : (a))
#define max3(a,b,c)   (max((max(a,b)), (c)))
#define min3(a,b,c)   (min((min(a,b)), (c)))
#define med3(a,b,c) ((((a) < (b))  && ((a) > (c))) ? (a) : \
                       (((b) < (a))  && ((b) > (c))) ? (b) : (c))

const point I = point(0, 1);

// is_legal
inline bool
is_legal(int32 value, int32 max)
{
	return ((value >= 0) && (value < max));
}

// clip
inline uint8
clip(float value) 
{
	uint8 clipped = 0;
	if (value > 255.0)
		clipped =  255;
	else if (value > 0)
		clipped = (uint8)value;
	return clipped;
}

// intclip
inline uint8
intclip(int32 value)
{
	uint8 clipped = 0;
	if (value > 255)
		clipped =  255;
	else if (value > 0)
		clipped = (uint8)value;
	return clipped;
}

double**	new_dmatrix(uint32 entries, uint32 entrySize);

double**	clear_dmatrix(uint32 entries, uint32 entrySize);

void		free_dmatrix(double** matrix, uint32 entries);

float*		copy(float* src, uint32 size);

int32*		copy(int32* src, uint32 size);

float**		new_fmatrix(int32 n, int32 m);

void		free_fmatrix(float** fmatrix, int32 n);

int32**		new_imatrix(int32 n, int32 m);

void		free_imatrix(int32** imatrix, int32 n);

void		complex2float(complex* z, float* x, float* y, uint32 size);

void		min_max(float* data, float& min, float& max, uint32 size);

float		fmax(float* area, uint32 size);

int32		imax(int32* area, uint32 size);

float*		clear_float(uint32 size);

status_t	set_file_type(BFile* file, int32 translator, uint32 type);

bool		set_translator(const char* name = NULL);

void		save_bitmap(BBitmap* bitmap, const char* fileName);

// mult
inline complex
mult(complex a, complex b)
{
	return complex(a.real() * b.real(), a.imag() * b.imag());
}

// div
inline complex
div(complex a, complex b) 
{
	if((fabs(b.real()) >= TINY) && (fabs(b.imag()) >= TINY))
		return complex(a.real() / b.real(), a.imag() / b.imag());
	return complex(0.0, 0.0);
}

// realdiv
inline complex
realdiv(float a, complex b) 
{
	if ((fabs(b.real()) >= TINY) && (fabs(b.imag()) >= TINY))
		return complex(a / b.real(), a / b.imag());
	return complex(0.0, 0.0);
}

// norm
inline float
norm(complex *x, int n)
{
   float output = 0;
   for(int i = 0; i< n; i++) output += norm(x[i]);
   return output;
}

// gaussian
inline double
gaussian (double x)
{
	return exp(-(sqr(x)));
}

// Dgaussian
inline double
Dgaussian(double x)
{
	return -2 * x * gaussian(x);
}

#endif // SUPPORT_H







