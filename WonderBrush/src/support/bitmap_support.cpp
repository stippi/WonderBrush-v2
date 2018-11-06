// bitmap_support.cpp

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <Bitmap.h>

#include "alpha.h"
#include "blending.h"
#include "colorspace.h"
#include "cms_support.h"
#include "defines.h"
#include "lab_convert.h"
#include "support.h"

#include "bitmap_support.h"

#include "ImageProcessingLibrary.h"

// overlay_color
void
overlay_color(BBitmap* into, BBitmap* alphaMap, BRect area,
			  uint8 c1, uint8 c2, uint8 c3)
{
	if (area.IsValid() && area.Intersects(into->Bounds())) {
		// make sure we don't draw out of bounds
		area = into->Bounds() & area;
		// collect data
		uint32 dstBPR = into->BytesPerRow();
		uint32 srcBPR = alphaMap->BytesPerRow();
		uint8* dest = (uint8*)into->Bits();
		uint8* top = (uint8*)alphaMap->Bits();

		int32 left, y, right, bottom;
		rect_to_int(area, left, y, right, bottom);

		top += y * srcBPR + left;

		color_space dstFormat = into->ColorSpace();
		if (dstFormat == B_RGB32 || dstFormat == B_RGBA32) {

			dest += y * dstBPR + 4 * left;
#if GAMMA_BLEND
			// cache gamma converted values
			uint16 gc1 = convert_to_gamma(c1);
			uint16 gc2 = convert_to_gamma(c2);
			uint16 gc3 = convert_to_gamma(c3);
	
			for (; y <= bottom; y++) {
				uint8* destHandle = dest;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					blend_colors(destHandle, *topHandle, c1, c2, c3, gc1, gc2, gc3);
					destHandle += 4;
					topHandle ++;
				}
				dest += dstBPR;
				top += srcBPR;
			}
#else
			for (; y <= bottom; y++) {
				uint8* destHandle = dest;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					blend_colors(destHandle, *topHandle, c1, c2, c3);
					destHandle += 4;
					topHandle ++;
				}
				dest += dstBPR;
				top += srcBPR;
			}
#endif
		} else if (dstFormat == B_GRAY8) {
			dest += y * dstBPR + left;
			for (; y <= bottom; y++) {
				uint8* destHandle = dest;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					if (*topHandle > 0) {
						if (*destHandle == 0 || *topHandle == 255)
							*destHandle = *topHandle;
						else {
							uint32 alphaTemp = (65025 - (255 - *topHandle) * (255 - *destHandle));
							*destHandle = alphaTemp / 255;
						}
					}
					destHandle ++;
					topHandle ++;
				}
				dest += dstBPR;
				top += srcBPR;
			}
		}
	}
}

// overlay_color_copy
void
overlay_color_copy(BBitmap* from, BBitmap* to,
				   BBitmap* alphaMap, BRect area,
				   uint8 c1, uint8 c2, uint8 c3)
{
	if (area.IsValid() && area.Intersects(from->Bounds())) {
		// make sure we don't draw out of bounds
		area = from->Bounds() & area;
		// collect data
		uint32 dstBPR = from->BytesPerRow();
		uint32 srcBPR = alphaMap->BytesPerRow();
		uint8* dest = (uint8*)to->Bits();
		uint8* src = (uint8*)from->Bits();
		uint8* top = (uint8*)alphaMap->Bits();

		int32 left, y, right, bottom;
		rect_to_int(area, left, y, right, bottom);

		top += y * srcBPR + left;

		color_space dstFormat = to->ColorSpace();
		color_space srcFormat = from->ColorSpace();

		if (srcFormat != B_RGB32 && srcFormat != B_RGBA32)
			return;

		if (dstFormat == B_RGB32 || dstFormat == B_RGBA32) {

			int32 offset32 = y * dstBPR + 4 * left;
			src += offset32;
			dest += offset32;

#if GAMMA_BLEND
			// cache gamma converted values
			uint16 gc1 = convert_to_gamma(c1);
			uint16 gc2 = convert_to_gamma(c2);
			uint16 gc3 = convert_to_gamma(c3);
	
			for (; y <= bottom; y++) {
				uint8* srcHandle = src;
				uint8* destHandle = dest;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					blend_colors_copy(srcHandle, *topHandle, destHandle,
									  c1, c2, c3, gc1, gc2, gc3);
					srcHandle += 4;
					destHandle += 4;
					topHandle ++;
				}
				src += dstBPR;
				dest += dstBPR;
				top += srcBPR;
			}
#else
			for (; y <= bottom; y++) {
				uint8* srcHandle = src;
				uint8* destHandle = dest;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					blend_colors_copy(srcHandle, *topHandle, destHandle, c1, c2, c3);
					srcHandle += 4;
					destHandle += 4;
					topHandle ++;
				}
				src += dstBPR;
				dest += dstBPR;
				top += srcBPR;
			}
#endif
		} else if (dstFormat == B_GRAY8) {
			dest += y * dstBPR + left;
			src += y * dstBPR + left;
			for (; y <= bottom; y++) {
				uint8* destHandle = dest;
				uint8* srcHandle = src;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					if (*topHandle > 0) {
						if (*srcHandle == 0 || *topHandle == 255)
							*destHandle = *topHandle;
						else {
							uint32 alphaTemp = (65025 - (255 - *topHandle) * (255 - *srcHandle));
							*destHandle = alphaTemp / 255;
						}
					} else
						*destHandle = *srcHandle;
						
					srcHandle ++;
					destHandle ++;
					topHandle ++;
				}
				dest += dstBPR;
				src += dstBPR;
				top += srcBPR;
			}
		}
	}
}

// erase
void
erase(BBitmap* into, BBitmap* alphaMap, BRect area)
{
	if (area.IsValid() && area.Intersects(into->Bounds())) {
		// make sure we don't draw out of bounds
		area = into->Bounds() & area;
		// collect data
		uint32 dstBPR = into->BytesPerRow();
		uint32 srcBPR = alphaMap->BytesPerRow();
		uint8* dest = (uint8*)into->Bits();
		uint8* top = (uint8*)alphaMap->Bits();

		int32 left, y, right, bottom;
		rect_to_int(area, left, y, right, bottom);

		top += y * srcBPR + left;

		color_space dstFormat = into->ColorSpace();
		if (dstFormat == B_RGB32 || dstFormat == B_RGBA32) {

			dest += y * dstBPR + 4 * left;

			for (; y <= bottom; y++) {
				uint8* destHandle = dest;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					subtract_alpha(destHandle, *topHandle);
					destHandle += 4;
					topHandle ++;
				}
				dest += dstBPR;
				top += srcBPR;
			}
		} else if (dstFormat == B_GRAY8) {

			dest += y * dstBPR + left;

			for (; y <= bottom; y++) {
				uint8* destHandle = dest;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					*destHandle = max_c(0, *destHandle - *topHandle);
					destHandle ++;
					topHandle ++;
				}
				dest += dstBPR;
				top += srcBPR;
			}
		}
	}
}

// erase_copy
void
erase_copy(BBitmap* from, BBitmap* to, BBitmap* alphaMap, BRect area)
{
	if (area.IsValid() && area.Intersects(from->Bounds())) {
		// make sure we don't draw out of bounds
		area = from->Bounds() & area;
		// collect data
		uint32 dstBPR = from->BytesPerRow();
		uint32 srcBPR = alphaMap->BytesPerRow();
		uint8* dest = (uint8*)to->Bits();
		uint8* src = (uint8*)from->Bits();
		uint8* top = (uint8*)alphaMap->Bits();

		int32 left, y, right, bottom;
		rect_to_int(area, left, y, right, bottom);

		top += y * srcBPR + left;

		color_space dstFormat = to->ColorSpace();
		color_space srcFormat = from->ColorSpace();

		if (srcFormat != B_RGB32 && srcFormat != B_RGBA32)
			return;

		if (dstFormat == B_RGB32 || dstFormat == B_RGBA32) {

			int32 offset32 = y * dstBPR + 4 * left;
			src += offset32;
			dest += offset32;
	
			for (; y <= bottom; y++) {
				uint8* srcHandle = src;
				uint8* destHandle = dest;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					subtract_alpha_copy(srcHandle, destHandle, *topHandle);
					srcHandle += 4;
					destHandle += 4;
					topHandle ++;
				}
				src += dstBPR;
				dest += dstBPR;
				top += srcBPR;
			}
		} else if (dstFormat == B_GRAY8) {

			src += y * dstBPR + left;
			dest += y * dstBPR + left;

			for (; y <= bottom; y++) {
				uint8* srcHandle = src;
				uint8* destHandle = dest;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					*destHandle = max_c(0, *srcHandle - *topHandle);
					srcHandle ++;
					destHandle ++;
					topHandle ++;
				}
				src += dstBPR;
				dest += dstBPR;
				top += srcBPR;
			}
		}
	}
}
/*
// restore
void
restore(BBitmap* into, BBitmap* alphaMap, BRect area)
{
	if (area.IsValid() && area.Intersects(into->Bounds())) {
		// make sure we don't draw out of bounds
		area = into->Bounds() & area;
		// collect data
		uint32 dstBPR = into->BytesPerRow();
		uint32 srcBPR = alphaMap->BytesPerRow();
		uint8* dest = (uint8*)into->Bits();
		uint8* top = (uint8*)alphaMap->Bits();

		int32 left, y, right, bottom;
		rect_to_int(area, left, y, right, bottom);

		dest += y * dstBPR + 4 * left;
		top += y * srcBPR + left;

		for (; y <= bottom; y++) {
			uint8* destHandle = dest;
			uint8* topHandle = top;
			for (int32 x = left; x <= right; x++) {
				add_alpha32(destHandle, *topHandle);
				destHandle += 4;
				topHandle ++;
			}
			dest += dstBPR;
			top += srcBPR;
		}
	}
}

// restore_copy
void
restore_copy(BBitmap* from, BBitmap* to, BBitmap* alphaMap, BRect area)
{
	if (area.IsValid() && area.Intersects(from->Bounds())) {
		// make sure we don't draw out of bounds
		area = from->Bounds() & area;
		// collect data
		uint32 dstBPR = from->BytesPerRow();
		uint32 srcBPR = alphaMap->BytesPerRow();
		uint8* dest = (uint8*)to->Bits();
		uint8* src = (uint8*)from->Bits();
		uint8* top = (uint8*)alphaMap->Bits();

		int32 left, y, right, bottom;
		rect_to_int(area, left, y, right, bottom);

		int32 offset32 = y * dstBPR + 4 * left;
		src += offset32;
		dest += offset32;
		top += y * srcBPR + left;

		for (; y <= bottom; y++) {
			uint8* srcHandle = src;
			uint8* destHandle = dest;
			uint8* topHandle = top;
			for (int32 x = left; x <= right; x++) {
				add_alpha32_copy(srcHandle, destHandle, *topHandle);
				srcHandle += 4;
				destHandle += 4;
				topHandle ++;
			}
			src += dstBPR;
			dest += dstBPR;
			top += srcBPR;
		}
	}
}*/

// clone_bitmap
void
clone_bitmap(BBitmap* into, BBitmap* alphaMap, BRect area, BPoint offset)
{
	if (area.IsValid() && area.Intersects(into->Bounds())) {
		// make sure we don't draw out of bounds
		BRect clonableArea = into->Bounds();
		if (offset.x > 0.0)
			clonableArea.left += offset.x;
		else
			clonableArea.right -= -offset.x;
		if (offset.y > 0.0)
			clonableArea.top += offset.y;
		else
			clonableArea.bottom -= -offset.y;
		area = clonableArea & area;
		// collect data
		int32 dstBPR = into->BytesPerRow();
		int32 srcBPR = alphaMap->BytesPerRow();
		uint8* dest = (uint8*)into->Bits();
		uint8* top = (uint8*)alphaMap->Bits();
		uint8* clone = dest;

		int32 left, y, right, bottom;
		rect_to_int(area, left, y, right, bottom);
		// depending on the clone offset, we traverse the bitmaps
		// (x < 0) left to right (x > 0) right to left
		// (y < 0) top to bottom (x > 0) bottom to top
		// this has to be done since we clone the bitmap in-place
		// and we don't want to touch pixels, that we need in their
		// original color at another place in the bitmap
		if (offset.y > 0.0) {
			dest += bottom * dstBPR;
			clone += (bottom - (int32)offset.y) * dstBPR;
			top += bottom * srcBPR;
			// reverse bytes per line for loop
			dstBPR = -dstBPR;
			srcBPR = -srcBPR;
		} else {
			dest += y * dstBPR;
			clone += (y - (int32)offset.y) * dstBPR;
			top += y * srcBPR;
		}

		int32 dstBPP = 4;
		int32 srcBPP = 1;

		color_space dstFormat = into->ColorSpace();
		if (dstFormat == B_GRAY8)
			dstBPP = 1;

		if (offset.x > 0.0) {
			dest += dstBPP * right;
			clone += dstBPP * (right - (int32)offset.x);
			top += right;
			dstBPP = -dstBPP;
			srcBPP = -srcBPP;
		} else {
			dest += dstBPP * left;
			clone += dstBPP * (left - (int32)offset.x);
			top += left;
		}

		if (dstFormat == B_RGB32 || dstFormat == B_RGBA32) {
			for (; y <= bottom; y++) {
				uint8* destHandle = dest;
				uint8* cloneHandle = clone;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					blend_pixels(destHandle, cloneHandle, *topHandle);
					destHandle += dstBPP;
					cloneHandle += dstBPP;
					topHandle += srcBPP;
				}
				dest += dstBPR;
				clone += dstBPR;
				top += srcBPR;
			}
		} else if (dstFormat == B_GRAY8) {
			for (; y <= bottom; y++) {
				uint8* destHandle = dest;
				uint8* cloneHandle = clone;
				uint8* topHandle = top;
				for (int32 x = left; x <= right; x++) {
					if (*topHandle > 0) {
						if (*topHandle == 255)
							*destHandle = *cloneHandle;
						else
							*destHandle = (*destHandle * (255 - *topHandle) + *cloneHandle * *topHandle) / 255;
					}
					destHandle += dstBPP;
					cloneHandle += dstBPP;
					topHandle += srcBPP;
				}
				dest += dstBPR;
				clone += dstBPR;
				top += srcBPR;
			}
		}
	}
}

// clone_bitmap_copy
void
clone_bitmap_copy(BBitmap* from, BBitmap* into,
				  BBitmap* alphaMap, BRect area, BPoint offset)
{
	if (area.IsValid() && area.Intersects(into->Bounds())) {
		// make sure we don't draw out of bounds
		BRect clonableArea = into->Bounds();
		if (offset.x > 0.0)
			clonableArea.left += offset.x;
		else
			clonableArea.right -= -offset.x;
		if (offset.y > 0.0)
			clonableArea.top += offset.y;
		else
			clonableArea.bottom -= -offset.y;
		area = clonableArea & area;
		// collect data
		uint32 dstBPR = into->BytesPerRow();
		uint32 srcBPR = alphaMap->BytesPerRow();
		uint8* dest = (uint8*)into->Bits();
		uint8* alpha = (uint8*)alphaMap->Bits();
		uint8* src = (uint8*)from->Bits();
		uint8* clone = src;

		int32 left, y, right, bottom;
		rect_to_int(area, left, y, right, bottom);

		color_space dstFormat = into->ColorSpace();
		color_space srcFormat = from->ColorSpace();
		if (srcFormat != B_RGB32 && srcFormat != B_RGBA32)
			return;

		if (dstFormat == B_RGB32 || dstFormat == B_RGBA32) {
			src += y * dstBPR + 4 * left;
			dest += y * dstBPR + 4 * left;
			clone += (y - (int32)offset.y) * dstBPR + 4 * (left - (int32)offset.x);
			alpha += y * srcBPR + left;
	
			for (; y <= bottom; y++) {
				uint8* destHandle = dest;
				uint8* cloneHandle = clone;
				uint8* srcHandle = src;
				uint8* alphaHandle = alpha;
				for (int32 x = left; x <= right; x++) {
					blend_pixels_copy(srcHandle,
									  cloneHandle,
									  destHandle,
									  *alphaHandle);
					destHandle += 4;
					cloneHandle += 4;
					srcHandle += 4;
					alphaHandle ++;
				}
				dest += dstBPR;
				clone += dstBPR;
				src += dstBPR;
				alpha += srcBPR;
			}
		} else if (dstFormat == B_GRAY8) {
			src += y * dstBPR + left;
			dest += y * dstBPR + left;
			clone += (y - (int32)offset.y) * dstBPR + (left - (int32)offset.x);
			alpha += y * srcBPR + left;
	
			for (; y <= bottom; y++) {
				uint8* destHandle = dest;
				uint8* cloneHandle = clone;
				uint8* srcHandle = src;
				uint8* alphaHandle = alpha;
				for (int32 x = left; x <= right; x++) {
					blend_pixels_copy(srcHandle,
									  cloneHandle,
									  destHandle,
									  *alphaHandle);
					destHandle ++;
					cloneHandle ++;
					srcHandle ++;
					alphaHandle ++;
				}
				dest += dstBPR;
				clone += dstBPR;
				src += dstBPR;
				alpha += srcBPR;
			}
		}
	}
}

#if GAMMA_BLEND
// add
inline uint32
add(uint32& c1, uint32& c2, uint32& c3, uint32& a, uint8* p)
{
	if (p[3] > 0) {
		c1 += kGammaTable[p[0]];
		c2 += kGammaTable[p[1]];
		c3 += kGammaTable[p[2]];
		a += p[3];
		return 1;
	}
	return 0;
}

// average_nine
inline void
average_nine(uint8* dest, uint8* p1, uint8* p2, uint8* p3,
						  uint8* p4, uint8* p5, uint8* p6,
						  uint8* p7, uint8* p8, uint8* p9)
{
	uint32 c1 = 0;
	uint32 c2 = 0;
	uint32 c3 = 0;
	uint32 a = 0;
	uint32 count = 0;

	count += add(c1, c2, c3, a, p1);
	count += add(c1, c2, c3, a, p2);
	count += add(c1, c2, c3, a, p3);
	count += add(c1, c2, c3, a, p4);
	count += add(c1, c2, c3, a, p5);
	count += add(c1, c2, c3, a, p6);
	count += add(c1, c2, c3, a, p7);
	count += add(c1, c2, c3, a, p8);
	count += add(c1, c2, c3, a, p9);

	if (count > 0) {
		// colors may only be valid if alpha > 0,
		// thus we divide by count, which is the number
		// of neighbor pixels with alpha > 0, however
		// alpha is always valid, that's why we ignore count
		int32 roundingTrick = count / 2;
		dest[0] = kInverseGammaTable[(c1 + roundingTrick) / count];
		dest[1] = kInverseGammaTable[(c2 + roundingTrick) / count];
		dest[2] = kInverseGammaTable[(c3 + roundingTrick) / count];
		dest[3] = (a + 4) / 9;
	} else {
		dest[3] = 0;
	}
}

#else // GAMMA_BLEND

// add
inline uint32
add(uint32& c1, uint32& c2, uint32& c3, uint32& a, uint8* p)
{
	if (p[3] > 0) {
		c1 += p[0];
		c2 += p[1];
		c3 += p[2];
		a += p[3];
		return 1;
	}
	return 0;
}

// average_nine
inline void
average_nine(uint8* dest, uint8* p1, uint8* p2, uint8* p3,
						  uint8* p4, uint8* p5, uint8* p6,
						  uint8* p7, uint8* p8, uint8* p9)
{
	uint32 c1 = 0;
	uint32 c2 = 0;
	uint32 c3 = 0;
	uint32 a = 0;
	uint32 count = 0;

	count += add(c1, c2, c3, a, p1);
	count += add(c1, c2, c3, a, p2);
	count += add(c1, c2, c3, a, p3);
	count += add(c1, c2, c3, a, p4);
	count += add(c1, c2, c3, a, p5);
	count += add(c1, c2, c3, a, p6);
	count += add(c1, c2, c3, a, p7);
	count += add(c1, c2, c3, a, p8);
	count += add(c1, c2, c3, a, p9);

	if (count > 0) {
		int32 roundingTrick = count / 2;
		dest[0] = (c1 + roundingTrick) / count;
		dest[1] = (c2 + roundingTrick) / count;
		dest[2] = (c3 + roundingTrick) / count;
		dest[3] = (a + 4) / 9;
	} else {
		dest[3] = 0;
	}
}

#endif // GAMMA_BLEND

// average_color
rgb_color
average_color(const BBitmap* bitmap, BRect area)
{
	rgb_color c;
	c.red = 0;
	c.green = 0;
	c.blue = 0;
	c.alpha = 0;
	if (bitmap && bitmap->IsValid() && area.IsValid() && bitmap->Bounds().Intersects(area)) {
		area = area & bitmap->Bounds();
		uint8* bits = (uint8*)bitmap->Bits();
		uint32 bpr = bitmap->BytesPerRow();
		uint32 width = area.IntegerWidth() + 1;
		uint32 height = area.IntegerHeight() + 1;
		bits += (int32)area.left * 4 + (int32)area.top * bpr;
		// add up all pixels, count will be number of valid (non-transparent) pixels
		uint32 c1 = 0, c2 = 0, c3 = 0, a = 0;
		uint32 count = 0;
		for (uint32 y = 0; y < height; y++) {
			uint8* bitsHandle = bits;
			for (uint32 x = 0; x < width; x++) {
				count += add(c1, c2, c3, a, bitsHandle);
				bitsHandle += 4;
			}
			bits += bpr;
		}
		// compute the color
		if (count > 0) {
			int32 roundingTrick = count / 2;
#if GAMMA_BLEND
			c.blue = kInverseGammaTable[(c1 + roundingTrick) / count];
			c.green = kInverseGammaTable[(c2 + roundingTrick) / count];
			c.red = kInverseGammaTable[(c3 + roundingTrick) / count];
#else
			c.blue = (c1 + roundingTrick) / count;
			c.green = (c2 + roundingTrick) / count;
			c.red = (c3 + roundingTrick) / count;
#endif
			// compute alpha according to total number of pixels in area
			uint32 pixelCountTotal = (width + 1) * (height + 1);
			roundingTrick = pixelCountTotal / 2;
			c.alpha = (a + roundingTrick) / pixelCountTotal;
		}
	}
	return c;
}

// blur_bitmap
//
// code taken from TranslatorPanel, modified to handle smaller output,
// with bounds laying in input
bool
blur_bitmap(const BBitmap* input, const BBitmap* output)
{
	if (!output || !output->IsValid()) {
		return false;
	}

	color_space inFormat = input->ColorSpace();
	color_space outFormat = output->ColorSpace();
// TODO: is this the correct check?!?
	if (inFormat != outFormat)
		return false;

	BRect r(output->Bounds());
	if (r.IsValid() && input->Bounds().Intersects(r)) {
		// make sure we don't read/write out of bounds
		r = r & input->Bounds();

		uint32 width = r.IntegerWidth() + 1;

		int32 pixelSize = 4;
		if (inFormat == B_GRAY8)
			pixelSize = 1;

		// allocate temporary buffers
		uint8* rows[3] = { NULL, NULL, NULL };
		for (uint32 x = 0; x < 3; x++) {
			rows[x] = (uint8*)malloc((width + 2) * pixelSize);
			if (rows[x] == NULL) {
				fprintf(stderr, "BlurFilter: could not allocate rows\n");
				if (rows[0])
					free(rows[0]);
				if (rows[1])
					free(rows[1]);
				return false;
			}
			if (x == 0)
				memset(rows[x], 0, (width + 2) * pixelSize);
		}

		uint8* src = (uint8*)input->Bits();
		uint8* dst = (uint8*)output->Bits();
	
		uint32 srcBPR = input->BytesPerRow();

		int32 left, top, right, bottom;
		rect_to_int(r, left, top, right, bottom);

		for (int32 y = top; y <= bottom; y++) {

			if (y != top)
				memcpy(rows[0] + pixelSize,
					   src + ((y - 1) * srcBPR) + left * pixelSize,
					   width * pixelSize);

			memcpy(rows[1] + pixelSize,
				   src + (y * srcBPR) + left * pixelSize,
				   width * pixelSize);

			if (y != bottom)
				memcpy(rows[2] + pixelSize,
					   src + ((y + 1) * srcBPR) + left * pixelSize,
					   width * pixelSize);
			else
				memset(rows[2], 0, (width + 2) * pixelSize);

			if (inFormat == B_GRAY8) {
				for (uint32 x = 0; x < width; x++) {
					if (y == top || y == bottom) {
						*dst = 0;
					} else {
						if (x == 0 || x == width - 1) {
							*dst = 0;
						} else {
							uint32 sum = rows[0][x] +
										 rows[0][x + 1] +
										 rows[0][x + 2] +
										 rows[1][x] +
										 rows[1][x + 1] +
										 rows[1][x + 2] +
										 rows[2][x] +
										 rows[2][x + 1] +
										 rows[2][x + 2];
							*dst = (sum + 4) / 9;
						}
					}
					// next pixel
					dst ++;
				}
			} else {
				for (uint32 x = 0; x < width; x++) {
					if (y == top || y == bottom) {
						dst[3] = 0;
					} else {
						if (x == 0 || x == width - 1) {
							dst[3] = 0;
						} else {
							average_nine(dst, rows[0] + x * 4,
											  rows[0] + (x + 1) * 4,
											  rows[0] + (x + 2) * 4,
											  rows[1] + x * 4,
											  rows[1] + (x + 1) * 4,
											  rows[1] + (x + 2) * 4,
											  rows[2] + x * 4,
											  rows[2] + (x + 1) * 4,
											  rows[2] + (x + 2) * 4);
						}
					}
					// next pixel
					dst += 4;
				}
			}
		}
		// free temporary buffers		
		free(rows[0]);
		free(rows[1]);
		free(rows[2]);

		return true;
	}
	return false;
}


// blur_bitmap
// 
// into and alphaMap are expected to have the same bounds
void
blur_bitmap(BBitmap* into, BBitmap* alphaMap, BRect area, float blurRadius)
{
	if (area.IsValid() && area.Intersects(into->Bounds())) {
		// round area to nearest integer boundaries
		area.left = floorf(area.left - blurRadius);
		area.top = floorf(area.top - blurRadius);
		area.right = ceilf(area.right + blurRadius);
		area.bottom = ceilf(area.bottom + blurRadius);
		// make sure we don't copy out of bounds
		area = into->Bounds() & area;
#if 1
		BBitmap* temp = cropped_bitmap(into, area);
		if (temp && temp->IsValid()) {
			ImageProcessingLibrary ip;
			ip.gaussian_blur(temp, blurRadius);
#else
		// make temporary bitmap for blurring
		BBitmap* temp = new BBitmap(area, 0, into->ColorSpace());
		if (blur_bitmap(into, temp)) {
#endif
			uint32 srcBPR = temp->BytesPerRow();
			uint32 dstBPR = into->BytesPerRow();
			uint32 alphaBPR = alphaMap->BytesPerRow();
			
			uint8* src = (uint8*)temp->Bits();
			uint8* dst = (uint8*)into->Bits();
			uint8* alpha = (uint8*)alphaMap->Bits();

			BRect r = temp->Bounds();
			area = r;
//			area.InsetBy(1.0, 1.0);

			int32 left, y, right, bottom;
			rect_to_int(area, left, y, right, bottom);

			// source already has the correct offset
			// offset into othe bitmaps
			alpha += y * alphaBPR + left;

			color_space dstFormat = into->ColorSpace();
			if (dstFormat == B_RGB32 || dstFormat == B_RGBA32) {
				// skip first row and pixel of source
				src += (y - (uint32)r.top) * srcBPR + (left - (uint32)r.left) * 4;
				dst += y * dstBPR + left * 4;
	
				for (; y <= bottom; y++) {
					uint8* srcHandle = src;
					uint8* dstHandle = dst;
					uint8* alphaHandle = alpha;
					for (int32 x = left; x <= right; x++) {
//						blend_pixels_overlay(dstHandle, srcHandle, *alphaHandle);
						blend_pixels(dstHandle, srcHandle, *alphaHandle);
						srcHandle += 4;
						dstHandle += 4;
						alphaHandle += 1;
					}
					src += srcBPR;
					dst += dstBPR;
					alpha += alphaBPR;
				}
			} else if (dstFormat == B_GRAY8) {
				// skip first row and pixel of source
				src += (y - (uint32)r.top) * srcBPR + (left - (uint32)r.left);
				dst += y * dstBPR + left;
	
				for (; y <= bottom; y++) {
					uint8* srcHandle = src;
					uint8* dstHandle = dst;
					uint8* alphaHandle = alpha;
					for (int32 x = left; x <= right; x++) {
						uint8 alpha = (*srcHandle * *alphaHandle) / 255;
						if (alpha > 0) {
							if (alpha == 255)
								*dstHandle = *srcHandle;
							else
								*dstHandle = (*dstHandle * (255 - alpha) + *srcHandle * alpha) / 255;
						}
						srcHandle ++;
						dstHandle ++;
						alphaHandle ++;
					}
					src += srcBPR;
					dst += dstBPR;
					alpha += alphaBPR;
				}
			}
		}
		delete temp;
	}
}

// blur_bitmap_copy
// 
// from, into and alphaMap are expected to have the same bounds!
void
blur_bitmap_copy(BBitmap* from, BBitmap* into, BBitmap* alphaMap, BRect area, float blurRadius)
{
	if (area.IsValid() && area.Intersects(from->Bounds())) {
		// round area to nearest integer boundaries
		area.left = floorf(area.left - blurRadius);
		area.top = floorf(area.top - blurRadius);
		area.right = ceilf(area.right + blurRadius);
		area.bottom = ceilf(area.bottom + blurRadius);
		// make sure we don't copy out of bounds
		area = from->Bounds() & area;
#if 1
		BBitmap* temp = cropped_bitmap(from, area);
		if (temp && temp->IsValid()) {
			ImageProcessingLibrary ip;
			ip.gaussian_blur(temp, blurRadius);
#else
		// make temporary bitmap for blurring
		BBitmap* temp = new BBitmap(area, 0, into->ColorSpace());
		if (blur_bitmap(into, temp)) {
#endif
			uint32 srcBPR = temp->BytesPerRow();
			uint32 btmBPR = from->BytesPerRow();
			uint32 dstBPR = into->BytesPerRow();
			uint32 alphaBPR = alphaMap->BytesPerRow();
			
			uint8* src = (uint8*)temp->Bits();
			uint8* btm = (uint8*)from->Bits();
			uint8* dst = (uint8*)into->Bits();
			uint8* alpha = (uint8*)alphaMap->Bits();

			BRect r = temp->Bounds();
			area = r;
//			area.InsetBy(1.0, 1.0);

			int32 left, y, right, bottom;
			rect_to_int(area, left, y, right, bottom);

			color_space dstFormat = into->ColorSpace();
			color_space srcFormat = from->ColorSpace();
			if (srcFormat != B_RGB32 && srcFormat != B_RGBA32)
				return;

			alpha += y * alphaBPR + left;

			if (dstFormat == B_RGB32 || dstFormat == B_RGBA32) {
				// skip first row and pixel of source
				src += (y - (uint32)r.top) * srcBPR + (left - (uint32)r.left) * 4;
				btm += y * btmBPR + left * 4;
				dst += y * dstBPR + left * 4;
	
				for (; y <= bottom; y++) {
					uint8* srcHandle = src;
					uint8* btmHandle = btm;
					uint8* dstHandle = dst;
					uint8* alphaHandle = alpha;
					for (int32 x = left; x <= right; x++) {
//						blend_pixels_overlay_copy(btmHandle, srcHandle, dstHandle, *alphaHandle);
						blend_pixels_copy(btmHandle, srcHandle, dstHandle, *alphaHandle);
						srcHandle += 4;
						btmHandle += 4;
						dstHandle += 4;
						alphaHandle += 1;
					}
					src += srcBPR;
					btm += btmBPR;
					dst += dstBPR;
					alpha += alphaBPR;
				}
			} else if (dstFormat == B_GRAY8) {
				// skip first row and pixel of source
				src += (y - (uint32)r.top) * srcBPR + (left - (uint32)r.left);
				btm += y * btmBPR + left;
				dst += y * dstBPR + left;
	
				for (; y <= bottom; y++) {
					uint8* srcHandle = src;
					uint8* btmHandle = btm;
					uint8* dstHandle = dst;
					uint8* alphaHandle = alpha;
					for (int32 x = left; x <= right; x++) {
						uint8 alpha = (*srcHandle * *alphaHandle) / 255;
						if (alpha > 0) {
							if (alpha == 255)
								*dstHandle = *srcHandle;
							else
								*dstHandle = (*btmHandle * (255 - alpha) + *srcHandle * alpha) / 255;
						} else
							*dstHandle = *btmHandle;
						srcHandle ++;
						btmHandle ++;
						dstHandle ++;
						alphaHandle ++;
					}
					src += srcBPR;
					btm += btmBPR;
					dst += dstBPR;
					alpha += alphaBPR;
				}
			}
		}
		delete temp;
	}
}

// copy_area
//
// handes B_RGBA32, B_RGB32 and GRAY8 bitmaps
void
copy_area(BBitmap* from, BBitmap* into, BRect area)
{
	if (from && from->IsValid() && into && into->IsValid()
		&& area.IsValid() && area.Intersects(from->Bounds())) {
		// make sure the bitmaps have the same colorspace
		color_space srcFormat = from->ColorSpace();
		color_space dstFormat = into->ColorSpace();
		if (srcFormat == dstFormat) {
			// make sure we don't copy out of bounds
			area = from->Bounds() & area;
			area = into->Bounds() & area;
			uint32 srcBPR = from->BytesPerRow();
			uint32 destBPR = into->BytesPerRow();
			uint8* dest = (uint8*)into->Bits();
			uint8* src = (uint8*)from->Bits();
	
			int32 x, y, right, bottom;
			rect_to_int(area, x, y, right, bottom);

			int32 pixelSize = 0;
			if (dstFormat == B_RGBA32 || dstFormat == B_RGB32) {
				pixelSize = 4;
			} else if (dstFormat == B_GRAY8) {
				pixelSize = 1;
			}

			dest += y * destBPR + x * pixelSize;
			src += y * srcBPR + x * pixelSize;
			int32 bytes = (right - x + 1) * pixelSize;
			for (; y <= bottom; y++) {
				memcpy(dest, src, bytes);
				dest += destBPR;
				src += srcBPR;
			}
		}
	}
}

// cropped_bitmap
//
// handes B_RGBA32, B_RGB32 and GRAY8 bitmaps
BBitmap*
cropped_bitmap(const BBitmap* from, BRect area, bool offsetToOrigin, bool clipEmptyAlpha)
{
	BBitmap* result = NULL;
	if (from && from->IsValid() && area.IsValid() && area.Intersects(from->Bounds())) {

		color_space srcFormat = from->ColorSpace();
		int32 pixelSize = 0;
		int32 alphaOffset = 0;
		if (srcFormat == B_RGBA32 || srcFormat == B_RGB32) {
			pixelSize = 4;
			alphaOffset = 3;
		} else if (srcFormat == B_GRAY8) {
			pixelSize = 1;
			alphaOffset = 0;
		}

		// make sure we don't copy out of bounds
		area = from->Bounds() & area;

		uint8* src = (uint8*)from->Bits();
		uint32 srcBPR = from->BytesPerRow();

		int32 left, y, right, bottom;
		rect_to_int(area, left, y, right, bottom);

		if (clipEmptyAlpha) {
			// search for contents in alpha channel
			uint8* srcAlpha = src + y * srcBPR + left * pixelSize + alphaOffset;

			int32 minX = right;
			int32 maxX = left;
			int32 minY = bottom;
			int32 maxY = y;

			for (; y <= bottom; y++) {
				uint8* handle = srcAlpha;
				for (int32 x = left; x <= right; x++) {
					if (*handle) {
						if (minX > x)
							minX = x;
						if (maxX < x)
							maxX = x;

						if (minY > y)
							minY = y;
						if (maxY < y)
							maxY = y;
					}
					handle += pixelSize;
				}
				srcAlpha += srcBPR;
			}

			area.left = minX;
			area.right = maxX;
			area.top = minY;
			area.bottom = maxY;
		}
		if (area.IsValid()) {
			rect_to_int(area, left, y, right, bottom);

			BRect r(left, y, right, bottom);
			if (offsetToOrigin)
				r.OffsetTo(0.0, 0.0);
			result = new BBitmap(r, 0, srcFormat);

			if (result && result->IsValid()) {
		
				uint8* dst = (uint8*)result->Bits();
				uint32 dstBPR = result->BytesPerRow();
		
				src += y * srcBPR + left * pixelSize;
				int32 bytes = (right - left + 1) * pixelSize;

				if (bytes > 0) {
					for (; y <= bottom; y++) {
						memcpy(dst, src, bytes);
						dst += dstBPR;
						src += srcBPR;
					}
				}
			} else {
				delete result;
				result = NULL;
			}
		}
	}
	return result;
}


// clear_area
void
clear_area(BBitmap* strokeBitmap, BRect area, uint8 value)
{
	if (area.IsValid() && area.Intersects(strokeBitmap->Bounds())) {
		// make sure we don't clear out of bitmap bounds
		area = strokeBitmap->Bounds() & area;
	
		uint32 bpr = strokeBitmap->BytesPerRow();
		uint8* bits = (uint8*)strokeBitmap->Bits();

		int32 x, y, right, bottom;
		rect_to_int(area, x, y, right, bottom);

		bits += y * bpr + x;
		uint32 bytes = right - x + 1;
		// clear aera to 0
		for (; y <= bottom; y++) {
			memset(bits, value, bytes);
			bits += bpr;
		}
	}
}

// clear_area
void
clear_area(BBitmap* bitmap, BRect area,
		   uint8 c1, uint8 c2, uint8 c3, uint8 alpha)
{
	if (area.IsValid() && area.Intersects(bitmap->Bounds())) {
		// make sure we don't clear out of bitmap bounds
		area = bitmap->Bounds() & area;
	
		uint32 bpr = bitmap->BytesPerRow();
		uint8* bits = (uint8*)bitmap->Bits();

		int32 left, y, right, bottom;
		rect_to_int(area, left, y, right, bottom);

		// clear aera to provided color
		for (; y <= bottom; y++) {
			for (int32 x = left; x <= right; x++) {
				uint32 offset = x * 4 + y * bpr;
				bits[offset + 0] = c1;
				bits[offset + 1] = c2;
				bits[offset + 2] = c3;
				bits[offset + 3] = alpha;
			}
		}
	}
}

// remap_colors
void
remap_colors(BBitmap* bitmap, BRect area, const uint8 table[256])
{
	if (area.IsValid() && area.Intersects(bitmap->Bounds())) {
		// make sure we don't work out of bitmap bounds
		area = bitmap->Bounds() & area;
	
		int32 left, y, right, bottom;
		rect_to_int(area, left, y, right, bottom);

		uint32 bpr = bitmap->BytesPerRow();
		uint8* bits = (uint8*)bitmap->Bits();
		color_space format = bitmap->ColorSpace();


		// remap colors according to table
		if (format == B_RGBA32 || format == B_RGB32) {

			bits += y * bpr + left * 4;

			for (; y <= bottom; y++) {
				uint8* handle = bits;
				for (int32 x = left; x <= right; x++) {
					handle[0] = table[handle[0]];
					handle[1] = table[handle[1]];
					handle[2] = table[handle[2]];
					handle += 4;
				}
				bits += bpr;
			}
		} else if (format == B_GRAY8) {

			bits += y * bpr + left;

			for (; y <= bottom; y++) {
				uint8* handle = bits;
				for (int32 x = left; x <= right; x++) {
					handle[0] = table[handle[0]];
					handle ++;
				}
				bits += bpr;
			}
		}
	}
}

// copy_small_to_extended
void
copy_small_to_extended(const BBitmap* small, BBitmap* extended, uint32 extend)
{
	// code orgininates from ArtPaint MotionBlur add-on
	uint32* dst = (uint32*)extended->Bits();
	uint32* src = (uint32*)small->Bits();
	uint32 srcBPR = small->BytesPerRow() / 4;
	uint32 srcHeight = small->Bounds().IntegerHeight()/*TODO: + 1?!?*/;
/*	uint8* dst = (uint8*)extended->Bits();
	uint8* src = (uint8*)small->Bits();
	uint32 dstBPR = extended->BytesPerRow();
	uint32 srcBPR = small->BytesPerRow();
	uint32 srcWidth = small->Bounds().IntegerWidth() + 1;
	uint32 srcHeight = small->Bounds().IntegerHeight() + 1;

	// first duplicate the first row for extend times
	for (uint32 y = 0; y < extend; y++) {
		uint8* srcHandle = src;
		uint8* dstHandle = dst;

		// duplicate the first column extend times
		for (uint32 x = 0; x < extend; x++) {
			*dstHandle++ = srcHandle[0];
			*dstHandle++ = srcHandle[1];
			*dstHandle++ = srcHandle[2];
			*dstHandle++ = srcHandle[3];
		}

		// duplicate the actual row
		memcpy(dstHandle, srcHandle, srcWidth * 4);
		dstHandle += srcWidth * 4;
		srcHandle += (srcWidth - 1) * 4;

		// duplicate the last column extend times
		for (uint32 x = 0; x < extend; x++) {
			*dstHandle++ = srcHandle[0];
			*dstHandle++ = srcHandle[1];
			*dstHandle++ = srcHandle[2];
			*dstHandle++ = srcHandle[3];
		}
		src += srcBPR;
	}
	
	// then copy the rows and extend the first and last columns	
	for (uint32 y = 0; y < srcHeight; y++) {
		uint8* srcHandle = src;
		uint8* dstHandle = dst;

		// duplicate the first column extend times
		for (uint32 x = 0; x < extend; x++) {
			*dstHandle++ = srcHandle[0];
			*dstHandle++ = srcHandle[1];
			*dstHandle++ = srcHandle[2];
			*dstHandle++ = srcHandle[3];
		}

		// duplicate the actual row
		memcpy(dstHandle, srcHandle, srcWidth * 4);
		dstHandle += srcWidth * 4;
		srcHandle += (srcWidth - 1) * 4;

		// duplicate the last column extend times
		for (uint32 x = 0; x < extend; x++) {
			*dstHandle++ = srcHandle[0];
			*dstHandle++ = srcHandle[1];
			*dstHandle++ = srcHandle[2];
			*dstHandle++ = srcHandle[3];
		}

		dst += dstBPR;
		src += srcBPR;
	}

	// finally duplicate the last row for extend times
	// first duplicate the first row for extend times
	for (uint32 y = 0; y < extend; y++) {
		uint8* srcHandle = src;
		uint8* dstHandle = dst;

		// duplicate the first column extend times
		for (uint32 x = 0; x < extend; x++) {
			*dstHandle++ = srcHandle[0];
			*dstHandle++ = srcHandle[1];
			*dstHandle++ = srcHandle[2];
			*dstHandle++ = srcHandle[3];
		}

		// duplicate the actual row
		memcpy(dstHandle, srcHandle, srcWidth * 4);
		dstHandle += srcWidth * 4;
		srcHandle += (srcWidth - 1) * 4;

		// duplicate the last column extend times
		for (uint32 x = 0; x < extend; x++) {
			*dstHandle++ = srcHandle[0];
			*dstHandle++ = srcHandle[1];
			*dstHandle++ = srcHandle[2];
			*dstHandle++ = srcHandle[3];
		}
		src += srcBPR;
	}*/


	// first duplicate the first row for extend times
	for (uint32 y = 0; y < extend; y++) {
		// duplicate the first column extend times
		for (uint32 x = 0; x < extend; x++) {
			*dst++ = *src;
		}
		// duplicate the actual row
		for (uint32 x = 0; x < srcBPR; x++) {
			*dst++ = *src++;	
		}
		src--;
		// duplicate the last column extend times
		for (uint32 x = 0; x < extend; x++) {
			*dst++ = *src;
		}
		src -= (srcBPR - 1);
	}

	// then copy the rows and extend the first and last columns	
	for (uint32 y = 0; y < srcHeight; y++) {
		// duplicate the first column extend times
		for (uint32 x = 0; x < extend; x++) {
			*dst++ = *src;
		}
		// duplicate the actual row
		for (uint32 x = 0; x < srcBPR; x++) {
			*dst++ = *src++;	
		}
		src--;
		// duplicate the last column extend times
		for (uint32 x = 0; x < extend; x++) {
			*dst++ = *src;
		}
		src++;
	}
	src -= srcBPR;
	
	// finally duplicate the last row for extend times
	for (uint32 y = 0; y < extend; y++) {
		// duplicate the first column extend times
		for (uint32 x =0 ; x < extend; x++) {
			*dst++ = *src;
		}
		// duplicate the actual row
		for (uint32 x = 0; x < srcBPR; x++) {
			*dst++ = *src++;	
		}
		src--;
		// duplicate the last column extend times
		for (uint32 x = 0; x < extend; x++) {
			*dst++ = *src;
		}
		src -= (srcBPR - 1);
	}
}

// extended_bitmap
BBitmap*
extended_bitmap(const BBitmap *source, uint32 extend)
{
	// code orgininates from ArtPaint MotionBlur add-on
	BBitmap* extended = NULL;
	if (source && source->IsValid()) {
		BRect bounds = source->Bounds();

		bounds.right += 2 * extend;
		bounds.bottom += 2 * extend;
		
		extended = new BBitmap(bounds, 0, B_RGBA32);

		if (extended && extended->IsValid()) {
			copy_small_to_extended(source, extended, extend);
		} else {
			delete extended;
			extended = NULL;
		}
	}
	return extended;
}

// multiply_alpha
void
multiply_alpha(BBitmap* bitmap)
{
	if (bitmap && bitmap->IsValid() && bitmap->ColorSpace() == B_RGBA32) {
		uint8* bits = (uint8*)bitmap->Bits();
		uint32 bpr = bitmap->BytesPerRow();
		uint32 height = bitmap->Bounds().IntegerHeight() + 1;
		uint32 width = bitmap->Bounds().IntegerWidth() + 1;
	
		for (uint32 y = 0; y < height; y++) {
			uint8* handle = bits;
			for (uint32 x = 0; x < width; x++) {
				handle[0] = handle[0] * handle[3] / 255;
				handle[1] = handle[1] * handle[3] / 255;
				handle[2] = handle[2] * handle[3] / 255;
				handle += 4;
			}
			bits += bpr;
		}
	}
}

// demultiply_alpha
void
demultiply_alpha(BBitmap* bitmap)
{
	if (bitmap && bitmap->IsValid() && bitmap->ColorSpace() == B_RGBA32) {
		uint8* bits = (uint8*)bitmap->Bits();
		uint32 bpr = bitmap->BytesPerRow();
		uint32 height = bitmap->Bounds().IntegerHeight() + 1;
		uint32 width = bitmap->Bounds().IntegerWidth() + 1;
	
		for (uint32 y = 0; y < height; y++) {
			uint8* handle = bits;
			for (uint32 x = 0; x < width; x++) {
				if (handle[3]) {
					handle[0] = handle[0] * 255 / handle[3];
					handle[1] = handle[1] * 255 / handle[3];
					handle[2] = handle[2] * 255 / handle[3];
				}
				handle += 4;
			}
			bits += bpr;
		}
	}
}

// get_bgra32_bitmap
BBitmap*
get_bgra32_bitmap(const BBitmap* source)
{
	BBitmap* dest = NULL;
	if (source && source->InitCheck() >= B_OK) {
		BRect bounds = source->Bounds();
		bounds.OffsetTo(0.0, 0.0);
		dest = new BBitmap(bounds, 0, B_RGBA32);
		if (dest->InitCheck() >= B_OK) {
			// fetch some variables
			uint8* src = (uint8*)source->Bits();
			uint8* dst = (uint8*)dest->Bits();
			uint32 srcBPR = source->BytesPerRow();
			uint32 dstBPR = dest->BytesPerRow();
			uint32 height = bounds.IntegerHeight() + 1;
			color_space sourceFormat = source->ColorSpace();
			// the actual conversion loop
			int32 bytes = min_c(srcBPR, dstBPR);
			for (uint32 y = 0; y < height; y++) {
				expand_data(sourceFormat, src, bytes, dst);
				src += srcBPR;
				dst += dstBPR;
			}
		} else {
			delete dest;
			dest = NULL;
		}
	}
	return dest;
}

// get_components_for_color
void
get_components_for_color(rgb_color color, uint32 colorSpace,
						 uint8& c1, uint8& c2, uint8& c3)
{
	switch (colorSpace) {
		case COLOR_SPACE_NONLINEAR_RGB:
			c1 = color.blue;
			c2 = color.green;
			c3 = color.red;
			break;
		case COLOR_SPACE_LINEAR_RGB:
			c1 = (uint8)(pow((double)color.blue / 255.0, 2.5) * 255.0);
			c2 = (uint8)(pow((double)color.green / 255.0, 2.5) * 255.0);
			c3 = (uint8)(pow((double)color.red / 255.0, 2.5) * 255.0);
			break;
		case COLOR_SPACE_LAB:
#if USE_LCMS
			get_lab_components(color, c1, c2, c3);
#else
			rgb2lab(color.red, color.green, color.blue, c1, c2, c3);
#endif
			break;
	}
}

// string_for_color_space
const char*
string_for_color_space(color_space format)
{
	const char* name = "<unkown format>";
	switch (format) {
		case B_RGB32:
			name = "B_RGB32";
			break;
		case B_RGBA32:
			name = "B_RGBA32";
			break;
		case B_RGB32_BIG:
			name = "B_RGB32_BIG";
			break;
		case B_RGBA32_BIG:
			name = "B_RGBA32_BIG";
			break;
		case B_RGB24:
			name = "B_RGB24";
			break;
		case B_RGB24_BIG:
			name = "B_RGB24_BIG";
			break;
		case B_CMAP8:
			name = "B_CMAP8";
			break;
		case B_GRAY8:
			name = "B_GRAY8";
			break;
		case B_GRAY1:
			name = "B_GRAY1";
			break;
		default:
			break;
	}
	return name;
}

// print_color_space
void
print_color_space(color_space format)
{
	printf("%s\n", string_for_color_space(format));
}
