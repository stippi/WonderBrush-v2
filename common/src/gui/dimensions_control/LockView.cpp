// LockView.cpp

#include <stdio.h>
#include <string.h>

#include <Bitmap.h>

#include "LockBitmaps1.h"

#include "LockView.h"

// constructor
LockView::LockView()
	: BView(BRect(0.0, 0.0, 10.0, 10.0), "lock", B_FOLLOW_NONE, B_WILL_DRAW),
	  fEnabled(true),
	  fLocked(true),
	  fOpenEnabledBitmap(NULL),
	  fOpenDisabledBitmap(NULL),
	  fClosedEnabledBitmap(NULL),
	  fClosedDisabledBitmap(NULL)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	BRect bitmapRect(0.0, 0.0, kImageWidth - 1, kImageHeight - 1);
	fOpenEnabledBitmap = new BBitmap(bitmapRect, kColorSpace);
	fOpenDisabledBitmap = new BBitmap(bitmapRect, kColorSpace);
	fClosedEnabledBitmap = new BBitmap(bitmapRect, kColorSpace);
	fClosedDisabledBitmap = new BBitmap(bitmapRect, kColorSpace);
	memcpy(fOpenEnabledBitmap->Bits(), kOpenEnabledBits, fOpenEnabledBitmap->BitsLength());
	memcpy(fOpenDisabledBitmap->Bits(), kOpenDisabledBits, fOpenDisabledBitmap->BitsLength());
	memcpy(fClosedEnabledBitmap->Bits(), kClosedEnabledBits, fClosedEnabledBitmap->BitsLength());
	memcpy(fClosedDisabledBitmap->Bits(), kClosedDisabledBits, fClosedDisabledBitmap->BitsLength());
}

// destructor
LockView::~LockView()
{
	delete fOpenEnabledBitmap;
	delete fOpenDisabledBitmap;
	delete fClosedEnabledBitmap;
	delete fClosedDisabledBitmap;
}

// Draw
void
LockView::Draw(BRect update)
{
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	rgb_color shadow = tint_color(LowColor(), B_DARKEN_2_TINT);
	rgb_color light = tint_color(LowColor(), B_LIGHTEN_MAX_TINT);
	// fill background
	FillRect(update, B_SOLID_LOW);
	// draw lines
	float room = 3.0;
	float top = (Bounds().top + Bounds().Height() / 4.0) - room;
	float bottom = (Bounds().bottom - Bounds().Height() / 4.0) + room;
	float right = Bounds().right - kImageWidth / 2.0;
	float middle = (Bounds().top + Bounds().bottom) / 2.0;
	float left = Bounds().left + room;
	BeginLineArray(8);
		AddLine(BPoint(left, top),
				BPoint(right, top), shadow);
		AddLine(BPoint(right, top + 1.0),
				BPoint(right, middle - (kImageWidth / 2.0 + room)), light);
		AddLine(BPoint(right, middle + (kImageWidth / 2.0 + room)),
				BPoint(right, bottom - 1.0), light);
		AddLine(BPoint(right, bottom),
				BPoint(left, bottom), light);
		AddLine(BPoint(left, top + 1.0),
				BPoint(right - 1.0, top + 1.0), light);
		AddLine(BPoint(right - 1.0, top + 2.0),
				BPoint(right - 1.0, middle - (kImageWidth / 2.0 + room)), shadow);
		AddLine(BPoint(right - 1.0, middle + (kImageWidth / 2.0 + room)),
				BPoint(right - 1.0, bottom - 2.0), shadow);
		AddLine(BPoint(right - 1.0, bottom - 1.0),
				BPoint(left, bottom - 1.0), shadow);
	EndLineArray();
	// draw lock
	BPoint lockPoint(Bounds().right - kImageWidth,
					 Bounds().Height() / 2.0 - kImageHeight / 2.0);
	if (fEnabled) {
		if (fLocked)
			DrawBitmap(fClosedEnabledBitmap, lockPoint);
		else
			DrawBitmap(fOpenEnabledBitmap, lockPoint);
	} else {
		if (fLocked)
			DrawBitmap(fClosedDisabledBitmap, lockPoint);
		else
			DrawBitmap(fOpenDisabledBitmap, lockPoint);
	}
}

// MouseDown
void
LockView::MouseDown(BPoint where)
{
	if (fEnabled && Bounds().Contains(where)) {
		SetLocked(!fLocked);
	}
}

// layoutprefs
minimax
LockView::layoutprefs()
{
	mpm.mini.x = kImageWidth;
	mpm.maxi.x = kImageWidth + 10000.0;
	mpm.mini.y = kImageHeight;
	mpm.maxi.y = kImageHeight + 10000.0;
//	mpm.mini.x = mpm.maxi.x = kImageWidth;
//	mpm.mini.y = mpm.maxi.y = kImageHeight;
	mpm.weight = 0.2;
	return mpm;
}

// layout
BRect
LockView::layout(BRect rect)
{
	MoveTo(rect.LeftTop());
	ResizeTo(rect.Width(), rect.Height());
	return Frame();
}

// SetEnabled
void
LockView::SetEnabled(bool enabled)
{
	if (enabled != fEnabled) {
		fEnabled = enabled;
		Invalidate();
	}
}

// IsEnabled
bool
LockView::IsEnabled()
{
	return fEnabled;
}

// SetLocked
void
LockView::SetLocked(bool locked)
{
	if (locked != fLocked) {
		fLocked = locked;
		Invalidate();
	}
}

// IsLocked
bool
LockView::IsLocked()
{
	return fLocked;
}

