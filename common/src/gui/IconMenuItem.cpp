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

// menu items with small icons.

#include <Bitmap.h>
#include <Debug.h>
#include <Menu.h>
#include <NodeInfo.h>

#include "IconMenuItem.h"


// constructor
IconMenuItem::IconMenuItem(const char *label, BMessage *message, BBitmap *icon)
	: BMenuItem(label, message),
	  fDeviceIcon(icon)
{
	// IconMenuItem is used in synchronously invoked menus, make sure
	// we invoke with a timeout
	SetTimeout(kSynchMenuInvokeTimeout);
}

// constructor
IconMenuItem::IconMenuItem(const char* label, BMessage *message,
						   const BNodeInfo* nodeInfo, icon_size which)
	: BMenuItem(label, message),
	  fDeviceIcon(NULL)
{
	if (nodeInfo) {
		fDeviceIcon = new BBitmap(BRect(0, 0, which - 1, which - 1), B_COLOR_8_BIT);
		if (nodeInfo->GetTrackerIcon(fDeviceIcon, which)) {
			delete fDeviceIcon;
			fDeviceIcon = NULL;
		}
	}
	
	// IconMenuItem is used in synchronously invoked menus, make sure
	// we invoke with a timeout
	SetTimeout(kSynchMenuInvokeTimeout);
}

// constructor
IconMenuItem::IconMenuItem(const char* label, BMessage* message,
						   const char* iconType, icon_size which)
	: BMenuItem(label, message),
	  fDeviceIcon(NULL)
{
	BMimeType mime(iconType);
	fDeviceIcon = new BBitmap(BRect(0, 0, which - 1, which - 1), B_COLOR_8_BIT);

	if (mime.GetIcon(fDeviceIcon, which) != B_OK) {
		delete fDeviceIcon;
		fDeviceIcon = NULL;
	}
	
	// IconMenuItem is used in synchronously invoked menus, make sure
	// we invoke with a timeout
	SetTimeout(kSynchMenuInvokeTimeout);
}

// constructor
IconMenuItem::IconMenuItem(BMenu* submenu, BMessage* message,
						   const char* iconType, icon_size which)
	: BMenuItem(submenu, message),
	  fDeviceIcon(NULL)
{
	BMimeType mime(iconType);
	fDeviceIcon = new BBitmap(BRect(0, 0, which - 1, which - 1), B_COLOR_8_BIT);

	if (mime.GetIcon(fDeviceIcon, which) != B_OK) {
		delete fDeviceIcon;
		fDeviceIcon = NULL;
	}
	
	// IconMenuItem is used in synchronously invoked menus, make sure
	// we invoke with a timeout
	SetTimeout(kSynchMenuInvokeTimeout);
}

// destructor
IconMenuItem::~IconMenuItem()
{
	delete fDeviceIcon;
}

// GetContentSize
void
IconMenuItem::GetContentSize(float* width, float* height)
{
	_inherited::GetContentSize(width, height);
	if (fDeviceIcon) {
		*width += fDeviceIcon->Bounds().Width() + 5.0;
		*height += fDeviceIcon->Bounds().Height() - 12.0;
	}
}

// DrawContent
void
IconMenuItem::DrawContent()
{
	BPoint drawPoint(ContentLocation());
	drawPoint.x += 20;
	Menu()->MovePenTo(drawPoint);
	_inherited::DrawContent();
	
	BPoint where(ContentLocation());
	where.y = Frame().top;
	
	if (fDeviceIcon) {
		if (IsEnabled())
			Menu()->SetDrawingMode(B_OP_OVER);
		else
			Menu()->SetDrawingMode(B_OP_BLEND);	
		
		Menu()->DrawBitmapAsync(fDeviceIcon, where);
	}
}

