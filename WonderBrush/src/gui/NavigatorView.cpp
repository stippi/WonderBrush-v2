// NavigatorView.cpp

#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Cursor.h>
#include <Message.h>
#include <Region.h>
#include <Window.h>

#include "cursors.h"
#include "defines.h"
#include "support.h"

#include "NavigatorView.h"

#define SIZE 32.0

// constructor
NavigatorView::NavigatorView(const char* name)
	: BView(BRect(0.0, 0.0, 10.0, 10.0),
			name, B_FOLLOW_NONE, B_WILL_DRAW | B_FRAME_EVENTS
				  | B_FULL_UPDATE_ON_RESIZE),
	  fBitmap(NULL),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fTracking(false),
	  fTarget(NULL)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
}

// destructor
NavigatorView::~NavigatorView()
{
}

// layoutprefs
minimax
NavigatorView::layoutprefs()
{
/*	mpm.mini.x = 4.0;
	mpm.maxi.x = 10000.0 + 4.0;
	mpm.mini.y = 104.0;
	mpm.maxi.y = 104.0;*/
	mpm.weight = 1.0;
	return mpm;
}

// layout
BRect
NavigatorView::layout(BRect frame)
{
	MoveTo(frame.LeftTop());
	ResizeTo(frame.Width(), frame.Height());
	return Frame();
}

// Draw
void
NavigatorView::Draw(BRect updateRect)
{
	rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color shadow = tint_color(background, B_DARKEN_3_TINT);
	rgb_color lightShadow = tint_color(background, B_DARKEN_1_TINT);
	rgb_color light = tint_color(background, B_LIGHTEN_MAX_TINT);
	BRect r = Bounds();
	stroke_frame(this, r, lightShadow, lightShadow, light, light);
	r.InsetBy(1.0, 1.0);
	stroke_frame(this, r, shadow, shadow, background, background);
	r.InsetBy(1.0, 1.0);
	if (fBitmap) {
		BRect bitmapFrame = fBitmap->Bounds();
		float xScale = r.Width() / bitmapFrame.Width();
		float yScale = r.Height() / bitmapFrame.Height();
		float scale = xScale > yScale ? yScale : xScale;
		if (scale > 1.0)
			scale = 1.0;
		bitmapFrame.right = floorf(bitmapFrame.right * scale + 0.5);
		bitmapFrame.bottom = floorf(bitmapFrame.bottom * scale + 0.5);
		bitmapFrame.OffsetTo(BPoint(
			floorf((r.left + r.right) / 2.0 - bitmapFrame.Width() / 2.0),
			floorf((r.top + r.bottom) / 2.0 - bitmapFrame.Height() / 2.0)));

		BRect visibleFrame = fBounds;
		visibleFrame.left = floorf(visibleFrame.left * scale + 0.5);
		visibleFrame.top = floorf(visibleFrame.top * scale + 0.5);
		visibleFrame.right = floorf(visibleFrame.right * scale + 0.5);
		visibleFrame.bottom = floorf(visibleFrame.bottom * scale + 0.5);
		visibleFrame.OffsetBy(bitmapFrame.LeftTop());
		DrawBitmap(fBitmap, fBitmap->Bounds(), bitmapFrame);

		// constrain clipping region to inner frame
		BRegion region;
		region.Set(r);
		ConstrainClippingRegion(&region);
		if (visibleFrame != bitmapFrame) {
			SetHighColor(0, 0, 0, 255);
			SetLowColor(255, 255, 255, 255);
			StrokeRect(visibleFrame, kDottedBigger);
		}
		// constrain clipping region to just the inner part outside the bitmap
		region.Exclude(bitmapFrame);
		ConstrainClippingRegion(&region);
	}
	SetLowColor(112, 112, 112, 255);
	SetHighColor(104, 104, 104, 255);
	FillRect(r, kStripes);
}

// MouseDown
void
NavigatorView::MouseDown(BPoint where)
{
	fTracking = true;
	SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
	BCursor cursor(kGrabCursor);
	SetViewCursor(&cursor, true);
	_CenterCanvas(where);
}

// MouseUp
void
NavigatorView::MouseUp(BPoint where)
{
	BCursor cursor(kHandCursor);
	SetViewCursor(&cursor, true);
	fTracking = false;
}

// MouseMoved
void
NavigatorView::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	if (transit == B_ENTERED_VIEW)  {
		BCursor cursor(fTracking ? kGrabCursor : kHandCursor);
		SetViewCursor(&cursor, true);
	}
	if (fTracking) {
		uint32 buttons = 0;
		if (Window()->CurrentMessage()->FindInt32("buttons", (int32*)&buttons) >= B_OK) {
			if (buttons == 0) {
				MouseUp(where);
			} else {
				// send off message
				_CenterCanvas(where);
			}
		}
	}
}

// Update
void
NavigatorView::Update(const BBitmap* bitmap, BRect bounds)
{
	fBitmap = bitmap;
	fBounds = bounds;
	if (fBitmap) {
		float height = ceilf((fBitmap->Bounds().Height() + 1.0) / ((fBitmap->Bounds().Width() + 1.0) / mpm.mini.x));
		height++;
		if (height > mpm.mini.x)
			height = mpm.mini.x;
		if (height != mpm.mini.y) {
			mpm.mini.y = mpm.maxi.y = height;
			if (BWindow* window = Window())
				window->PostMessage(M_RECALCULATE_SIZE);
		}
	}
	if (Window())
		Invalidate();
}

// SetTarget
void
NavigatorView::SetTarget(BHandler* target)
{
	fTarget = target;
}

// _CenterCanvas
void
NavigatorView::_CenterCanvas(BPoint where)
{
	if (fTarget) {
		if (BLooper* looper = fTarget->Looper()) {
			// translate point to point in canvas
			if (fBitmap) {
				BRect r = Bounds();
				r.InsetBy(2.0, 2.0);
				BRect bitmapFrame = fBitmap->Bounds();
				float xScale = r.Width() / bitmapFrame.Width();
				float yScale = r.Height() / bitmapFrame.Height();
				float scale = xScale > yScale ? yScale : xScale;
				if (scale > 1.0)
					scale = 1.0;
				bitmapFrame.right = floorf(bitmapFrame.right * scale + 0.5);
				bitmapFrame.bottom = floorf(bitmapFrame.bottom * scale + 0.5);
				bitmapFrame.OffsetTo(BPoint(floorf((r.left + r.right) / 2.0 - bitmapFrame.Width() / 2.0),
											floorf((r.top + r.bottom) / 2.0 - bitmapFrame.Height() / 2.0)));

				// account for offset of bitmapFrame
				where -= bitmapFrame.LeftTop();
				// scale to bitmap size
				where.x /= scale;
				where.y /= scale;
				// send of message
				BMessage message(MSG_CENTER_CANVAS);
				message.AddPoint("center", where);
				looper->PostMessage(&message, fTarget);
			}
		}
	}
}

