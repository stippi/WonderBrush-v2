// blending.cpp

#include <math.h>

#include "blending.h"

#if GAMMA_BLEND

// speed tests done on Pentium M, 1450 MHz
// blending two 800x600 bitmaps with "50" on each component (including alpha): 1500000 usecs
// -"-														  using gamma LUT:  651000 usecs
// -"-													   no use of floorf():  572000 usecs

// -"-												   uint16 integer version:   72000 usecs
// -"-																   inline:   60200 usecs

// for comparison:
// -"-								inline only, no LUTs, no gamma correction:   44000 usecs
// -"-							   + premultiplied alpha (less MULs, no DIVs):   16500 usecs


const float kGamma = 2.2;
const float kInverseGamma = 1.0 / kGamma;

/*
static float* kGammaTable = NULL;

// blend
void
blend(uint8 b1, uint8 b2, uint8 b3, uint8 ba,		// bottom components
	  uint8 t1, uint8 t2, uint8 t3, uint8 ta,		// top components
	  uint8* d1, uint8* d2, uint8* d3, uint8* da)	// dest components
{
	// convert to linear rgb
	float ft1 = kGammaTable[t1];
	float ft2 = kGammaTable[t2];
	float ft3 = kGammaTable[t3];

	float fb1 = kGammaTable[b1];
	float fb2 = kGammaTable[b2];
	float fb3 = kGammaTable[b3];

	float sAlpha = (float)ta / 255.0;
	float dAlpha = 1.0 - sAlpha;

	if (ba == 255) {
		*d1 = (uint8)(powf(fb1 * dAlpha + ft1 * sAlpha, kInverseGamma) * 255.0 + 0.5);
		*d2 = (uint8)(powf(fb2 * dAlpha + ft2 * sAlpha, kInverseGamma) * 255.0 + 0.5);
		*d3 = (uint8)(powf(fb3 * dAlpha + ft3 * sAlpha, kInverseGamma) * 255.0 + 0.5);
		*da = 255;
	} else {
		float srcAlpha = ta / 255.0;
		float dstAlpha = ba / 255.0;
		float rltAlpha = (1.0 - (1.0 - srcAlpha) * (1.0 - dstAlpha));
		float dAlpha = dstAlpha * (1.0 - srcAlpha);
		*d1 = (uint8)(powf((dAlpha * fb1 + srcAlpha * ft1) / rltAlpha, kInverseGamma) * 255 + 0.5);
		*d2 = (uint8)(powf((dAlpha * fb2 + srcAlpha * ft2) / rltAlpha, kInverseGamma) * 255 + 0.5);
		*d3 = (uint8)(powf((dAlpha * fb3 + srcAlpha * ft3) / rltAlpha, kInverseGamma) * 255 + 0.5);
		*da = (uint8)(255.0 * rltAlpha);
	}
}

// init_gamma_blending
void
init_gamma_blending()
{
	if (!kGammaTable)
		kGammaTable = new float[256];
	for (uint32 i = 0; i < 256; i++)
		kGammaTable[i] = powf((float)i / 255.0, kGamma);
}*/



uint16* kGammaTable = NULL;
uint8* kInverseGammaTable = NULL;


// convert_to_gamma
uint16
convert_to_gamma(uint8 value)
{
	return kGammaTable[value];
}

// init_gamma_blending
void
init_gamma_blending()
{
	// init LUT R'G'B' [0...255] -> RGB [0...25500]
	if (!kGammaTable)
		kGammaTable = new uint16[256];
	for (uint32 i = 0; i < 256; i++)
		kGammaTable[i] = (uint16)(powf((float)i / 255.0, kGamma) * 25500.0);

	// init LUT RGB [0...25500] -> R'G'B' [0...255] 
	if (!kInverseGammaTable)
		kInverseGammaTable = new uint8[25501];
	for (uint32 i = 0; i < 25501; i++)
		kInverseGammaTable[i] = (uint8)(powf((float)i / 25500.0, kInverseGamma) * 255.0);
}


#endif // GAMMA_BLEND
