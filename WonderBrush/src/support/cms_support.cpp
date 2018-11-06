// cms_support.cpp

#include <stdio.h>

#include <Bitmap.h>

#include <lcms.h>

#include "defines.h"
#include "lab_convert.h"

#include "cms_support.h"

static cmsCIExyYTRIPLE kD50Primaries = (cmsCIExyYTRIPLE){ { D50X, D50Y, D50Z } };

// create_rgb_profile
static cmsHPROFILE
create_rgb_profile()
{
	// create rgb profile
	LPGAMMATABLE gammaTable[3];
	gammaTable[0] = cmsBuildGamma(255, 1.0);
	gammaTable[1] = cmsBuildGamma(255, 1.0);
	gammaTable[2] = cmsBuildGamma(255, 1.0);
	return cmsCreateRGBProfile(cmsD50_xyY(), &kD50Primaries, gammaTable);
}

static cmsHPROFILE kLabProfile = cmsCreateLabProfile(cmsD50_xyY());
static cmsHPROFILE kRGBProfile = create_rgb_profile();

static cmsHTRANSFORM kLabTransform = cmsCreateTransform(kLabProfile,
														(COLORSPACE_SH(PT_Lab)|CHANNELS_SH(3)|BYTES_SH(1)|EXTRA_SH(1)),
														kRGBProfile, TYPE_BGRA_8,
														INTENT_ABSOLUTE_COLORIMETRIC, 0);

static cmsHTRANSFORM kRGBTransform = cmsCreateTransform(kRGBProfile, TYPE_RGB_8,
														kLabProfile, TYPE_Lab_8,
														INTENT_ABSOLUTE_COLORIMETRIC, 0);;

// get_lab_components
void
get_lab_components(rgb_color color, uint8& c1, uint8& c2, uint8& c3)
{
	uint8 rgb[3];
	rgb[0] = color.red;
	rgb[1] = color.green;
	rgb[2] = color.blue;

	uint8 lab[3];

	cmsDoTransform(kRGBTransform, rgb, lab, 1);

rgb2lab(color.red, color.green, color.blue, c1, c2, c3);
printf("rgb2lab: (%d, %d, %d) -> (%d, %d, %d)\n", color.red, color.green, color.blue, c1, c2, c3);
	
	c1 = lab[0];
	c2 = lab[1];
	c3 = lab[2];

printf("get_lab_components: (%d, %d, %d) -> (%d, %d, %d)\n\n", color.red, color.green, color.blue, c1, c2, c3);
}

#define CLAMP(x) ((uint8)((x) < 0 ? 0 : ((x) > 255 ? 255 : (x))))

// Lab2Rgb
void
Lab2Rgb(int iL, int ia, int ib, uint8* pbgr)
{
	double X, Y, Z, Yo, L, a, b;

	L = iL * 100.0 / 255.0;
	a = ia - 128;
	b = ib - 128;

	if (L <= 8) {
		Y = L / 903.3;
		X = Y + (a / 3893.5);
		Z = Y - (b / 1557.4);
	} else {
		Yo = (L + 16.0) / 116.0;
		Y = Yo * Yo * Yo;
		X = a / 500.0 + Yo;
		X = X * X * X;
		Z = Yo - b / 200.0;
		Z = Z * Z * Z;
	}
	X *= 0.3127 / 0.3290;
	Z *= 0.3582 / 0.3290;

	pbgr[0] = CLAMP((int)((0.055648 * X - 0.204043 * Y + 1.057311 * Z) * 255.0 + 0.5)); // Blue
	pbgr[1] = CLAMP((int)((-.969256 * X + 1.875992 * Y + 0.041556 * Z) * 255.0 + 0.5)); // Green
	pbgr[2] = CLAMP((int)((3.240479 * X - 1.537150 * Y - 0.498535 * Z) * 255.0 + 0.5)); // Red
}

// convert_lab_bitmap
void
convert_lab_bitmap(BBitmap* from, BBitmap* into, BRect area)
{
	if (from && into && from->IsValid() && into->IsValid()) {
		// clip area to bitmaps size
		area = area & into->Bounds();
		area = area & from->Bounds();
		if (area.IsValid()) {
			// pointers to data
			uint8* src = (uint8*)from->Bits();
			uint8* dst = (uint8*)into->Bits();
			uint32 srcBPR = from->BytesPerRow();
			uint32 dstBPR = into->BytesPerRow();
			// offset pointer into bitmaps
			src += 4 * (int32)area.left + srcBPR * (int32)area.top;
			dst += 4 * (int32)area.left + dstBPR * (int32)area.top;
			uint32 width = area.IntegerWidth() + 1;
			uint32 height = area.IntegerHeight() + 1;
			for (uint32 y = 0; y < height; y++) {
				// do the lcms transform
				cmsDoTransform(kLabTransform, src, dst, width);
/*				uint8* srcHandle = src;
				uint8* dstHandle = dst;
				for (uint32 x = 0; x < width; x++) {
					Lab2Rgb(srcHandle[0], srcHandle[1], srcHandle[2], dstHandle);
					srcHandle += 4;
					dstHandle += 4;
				}*/
				// next line
				src += srcBPR;
				dst += dstBPR;
			}
		}
	}
}

