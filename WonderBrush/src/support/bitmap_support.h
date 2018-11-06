// bitmap_support.h

#ifndef BITMAP_SUPPORT_H
#define BITMAP_SUPPORT_H

#include <GraphicsDefs.h>
#include <Rect.h>

class BBitmap;

// puts the color on top of supplied bitmap according to alphaMap
void overlay_color(BBitmap* into, BBitmap* alphaMap, BRect area,
				   uint8 c1, uint8 c2, uint8 c3);
void overlay_color_copy(BBitmap* from, BBitmap* to,
						BBitmap* alphaMap, BRect area,
						uint8 c1, uint8 c2, uint8 c3);

// erases (removes) opaqueness from pixels according alphaMap
void erase(BBitmap* into,
		   BBitmap* alphaMap, BRect area);
void erase_copy(BBitmap* from, BBitmap* to,
				BBitmap* alphaMap, BRect area);
/*
// restore is the reverse of erasing, in that it restores the alpha channel
void restore(BBitmap* into,
			 BBitmap* alphaMap, BRect area);
void restore_copy(BBitmap* from, BBitmap* to,
				  BBitmap* alphaMap, BRect area);
*/
// clones an area from the supplied bitmap and blends it on top with
// the supplied offset using the alphaMap
void clone_bitmap(BBitmap* into, BBitmap* alphaMap,
				  BRect area, BPoint offset);
void clone_bitmap_copy(BBitmap* from, BBitmap* into,
					   BBitmap* alphaMap, BRect area, BPoint offset);

// computes the average color (non-weighted) of the pixels within area
rgb_color average_color(const BBitmap* bitmap, BRect area);

// creates a blurred copy of the bitmap, and blends it on top using alphaMap
void blur_bitmap(BBitmap* into, BBitmap* alphaMap, BRect area, float blurRadius);
void blur_bitmap_copy(BBitmap* from, BBitmap* into,
					  BBitmap* alphaMap, BRect area, float blurRadius);

// general bitmap stuff
void copy_area(BBitmap* from, BBitmap* into, BRect area);
BBitmap* cropped_bitmap(const BBitmap* from, BRect area,
						bool offsetToOrigin = false,
						bool clipEmptyAlpha = false);

void clear_area(BBitmap* strokeBitmap, BRect area, uint8 value = 0);
void clear_area(BBitmap* bitmap, BRect area,
				uint8 c1, uint8 c2, uint8 c3, uint8 alpha);

// remapping the color components of a bitmap using the provied LUT
void remap_colors(BBitmap* bitmap, BRect area, const uint8 table[256]);

// extend a bitmap with extend pixels on each side
// repeating the last original pixels in the extended area
BBitmap* extended_bitmap(const BBitmap *source, uint32 extend);

void multiply_alpha(BBitmap* bitmap);
void demultiply_alpha(BBitmap* bitmap);

// get_bgra32_bitmap
BBitmap*
get_bgra32_bitmap(const BBitmap* source);

// converting internal colorspaces (RGB, R'G'B' and Lab)
void get_components_for_color(rgb_color color, uint32 colorSpace,
							  uint8& c1, uint8& c2, uint8& c3);

const char* string_for_color_space(color_space format);
void print_color_space(color_space format);

# endif // BITMAP_SUPPORT_H
