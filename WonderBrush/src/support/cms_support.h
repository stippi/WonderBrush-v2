// cms_support.h

#ifndef CMS_SUPPORT_H
#define CMS_SUPPORT_H

#include <Rect.h>

class BBitmap;

void get_lab_components(rgb_color color,
						uint8& c1, uint8& c2, uint8& c3);

void convert_lab_bitmap(BBitmap* from, BBitmap* into,
						BRect area);

void Lab2Rgb(int iL, int ia, int ib, uint8* pbgr);

#endif // CMS_SUPPORT_H
