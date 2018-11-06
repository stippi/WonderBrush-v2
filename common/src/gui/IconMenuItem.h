/*
Open Tracker License

Terms and Conditions

Copyright (c) 1991-2000, Be Incorporated. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice applies to all licensees
and shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE, MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
BE INCORPORATED BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of Be Incorporated shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
this Software without prior written authorization from Be Incorporated.

Tracker(TM), Be(R), BeOS(R), and BeIA(TM) are trademarks or registered trademarks
of Be Incorporated in the United States and other countries. Other brand product
names are registered trademarks or trademarks of their respective holders.
All rights reserved.
*/

// Menu item class with small icons.

#ifndef	ICON_MENU_ITEM_H
#define ICON_MENU_ITEM_H

#include <MenuItem.h>
#include <Mime.h>

class BNodeInfo;

const bigtime_t kSynchMenuInvokeTimeout = 5000000;

class IconMenuItem : public BMenuItem {
 public:
								IconMenuItem(const char* label,
											 BMessage* model, BBitmap* icon);
								IconMenuItem(const char* label,
											 BMessage* model,
											 const char* mimeType,
											 icon_size which = B_MINI_ICON);
								IconMenuItem(const char* label,
											 BMessage* model,
											 const BNodeInfo* nodeInfo,
											 icon_size which = B_MINI_ICON);
								IconMenuItem(BMenu* subMenu, BMessage* model,
											 const char* mimeType,
											 icon_size which = B_MINI_ICON);
	virtual						~IconMenuItem();

	virtual	void				GetContentSize(float *width, float *height);	
	virtual	void				DrawContent();

 private:
		BBitmap*				fDeviceIcon;

		typedef BMenuItem _inherited;
};

#endif // ICON_MENU_ITEM_H
