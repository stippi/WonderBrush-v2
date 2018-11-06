// CanvasTabView.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <Cursor.h>
#include <Looper.h>
#include <Message.h>
#include <TypeConstants.h>
#include <Window.h>

#include "cursors.h"
#include "support.h"

#include "Canvas.h"

#include "CanvasTabView.h"

#define DRAG_INIT_DIST 10.0

// constructor
CanvasTabView::CanvasTabView(const char* name, BMessage* message, BHandler* target)
	: BView(BRect(0.0, 0.0, 23.0, 17.0), name,
			B_FOLLOW_NONE, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
//	  fMouseDown(false),
	  fMessage(message),
	  fTarget(target),
	  fCanvasTabs(4),
	  fCurrentCanvas(NULL)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
}

// destructor
CanvasTabView::~CanvasTabView()
{
	delete fMessage;
}

// layoutprefs
minimax
CanvasTabView::layoutprefs()
{
	mpm.mini.x = 2.0;
	mpm.maxi.x = 10000.0;

	int32 count = fCanvasTabs.CountItems();

	if (count > 1) {
		font_height fh;
		GetFontHeight(&fh);
		mpm.mini.y = (fh.ascent + fh.descent + 6.0) * count + 4.0;
	} else {
		mpm.mini.y = 0.0;
	}

	mpm.maxi.y = mpm.mini.y;
	mpm.weight = 1.0;

	return mpm;
}

// layout
BRect
CanvasTabView::layout(BRect frame)
{
	MoveTo(frame.LeftTop());
	ResizeTo(frame.Width(), frame.Height());
	return Frame();
}

// Draw
void
CanvasTabView::Draw(BRect updateRect)
{
	rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);

	rgb_color shadow = tint_color(background, B_DARKEN_2_TINT);
	rgb_color light = tint_color(background, B_LIGHTEN_MAX_TINT);
	rgb_color darkShadow = tint_color(background, B_DARKEN_3_TINT);
	rgb_color lightShadow = tint_color(background, B_DARKEN_1_TINT);

	BRect r(Bounds());
//	_StrokeRect(r, light, shadow);
//	r.InsetBy(1.0, 1.0);

	int32 count = fCanvasTabs.CountItems();

	float tabHeight = (r.Height() - 4.0 + 1.0) / count;

	for (int32 i = 0; i < count; i++) {
		BRect tabRect(r.left, r.top + i * tabHeight,
					  r.right, r.top + (i + 1) * tabHeight - 1.0);
		rgb_color leftTop;
		rgb_color rightBottom;
		rgb_color bg;

		Canvas* canvas = (Canvas*)fCanvasTabs.ItemAt(i);

		// colors depend on "active" status of the tab item
		if (canvas == fCurrentCanvas) {
			leftTop = darkShadow;
			rightBottom = light;
			bg = lightShadow;
		} else {
			leftTop = background;
			rightBottom = background;
			bg = background;
		}
		// frame
		_StrokeRect(tabRect, leftTop, rightBottom);
		// background
		tabRect.InsetBy(1.0, 1.0);
		SetLowColor(bg);
		FillRect(tabRect, B_SOLID_LOW);
		SetHighColor(0, 0, 0, 255);
		BString string(canvas->Name());
		// label
		BFont font;
		GetFont(&font);
		font_height fh;
		font.GetHeight(&fh);

		font.TruncateString(&string, B_TRUNCATE_MIDDLE, tabRect.Width() - 6.0);

		DrawString(string.String(), BPoint(tabRect.left + 3.0,
										   (tabRect.top + tabRect.bottom) / 2.0
											   + fh.descent + 1.0));
	}
	SetLowColor(background);
	FillRect(BRect(r.left, r.bottom - 3.0, r.right, r.bottom), B_SOLID_LOW);
}

// MessageReceived
void
CanvasTabView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		// TODO: support dropping files, probably automatic
		// anyways (window handles drop message)
		default:
			BView::MessageReceived(message);
			break;
	}
}

// MouseDown
void
CanvasTabView::MouseDown(BPoint where)
{
	if (fMessage && Bounds().Contains(where)) {
		int32 count = fCanvasTabs.CountItems();
		int32 index = (int32)floorf(where.y / Bounds().Height() * count);
		BMessage message(*fMessage);
		message.AddPointer("canvas", (Canvas*)fCanvasTabs.ItemAt(index));
		_Invoke(&message);
//		fMouseDown = true;
	}
}
/*
// MouseUp
void
CanvasTabView::MouseUp(BPoint where)
{
	if (Bounds().Contains(where)
		&& Bounds().Contains(fTrackingStart))
		_Invoke(fMessage);
	fTrackingStart.x = -1.0;
	fTrackingStart.y = -1.0;
}

// MouseMoved
void
CanvasTabView::MouseMoved(BPoint where, uint32 transit,
					   const BMessage* dragMessage)
{
	if (transit == B_ENTERED_VIEW) {
		BCursor cursor(kDropperCursor);
		SetViewCursor(&cursor, true);
	}
	if (Bounds().Contains(fTrackingStart)) {
		if (dist(where, fTrackingStart) > DRAG_INIT_DIST
			|| transit == B_EXITED_VIEW) {
			_DragColor();
			fTrackingStart.x = -1.0;
			fTrackingStart.y = -1.0;
		}
	}
}
*/

// AddCanvas
bool
CanvasTabView::AddCanvas(Canvas* canvas)
{
	bool success = false;
	if (canvas)
		 success = fCanvasTabs.AddItem((void*)canvas);
	return success;
}

// RemoveCanvas
bool
CanvasTabView::RemoveCanvas(Canvas* canvas)
{
	return fCanvasTabs.RemoveItem((void*)canvas);;
}

// SetCanvas
void
CanvasTabView::SetCanvas(Canvas* canvas)
{
	int32 oldIndex = fCanvasTabs.IndexOf((void*)fCurrentCanvas);
	int32 newIndex = fCanvasTabs.IndexOf((void*)canvas);
	if (oldIndex != newIndex) {
		Invalidate(_TabFrame(oldIndex));
		Invalidate(_TabFrame(newIndex));
	}
	fCurrentCanvas = canvas;
}

// _Invoke
void
CanvasTabView::_Invoke(const BMessage* _message)
{
	if (_message) {
		BHandler* target = fTarget ? fTarget
							: dynamic_cast<BHandler*>(Window());
		BLooper* looper;
		if (target && (looper = target->Looper())) {
			BMessage message(*_message);
			message.AddPointer("be:source", (void*)this);
			message.AddInt64("be:when", system_time());
			looper->PostMessage(&message, target);
		}
	}
}

// _StrokeRect
void
CanvasTabView::_StrokeRect(BRect r, rgb_color leftTop,
					   rgb_color rightBottom)
{
	BeginLineArray(4);
		AddLine(BPoint(r.left, r.bottom),
				BPoint(r.left, r.top), leftTop);
		AddLine(BPoint(r.left + 1.0, r.top),
				BPoint(r.right, r.top), leftTop);
		AddLine(BPoint(r.right, r.top + 1.0),
				BPoint(r.right, r.bottom), rightBottom);
		AddLine(BPoint(r.right - 1.0, r.bottom),
				BPoint(r.left + 1.0, r.bottom), rightBottom);
	EndLineArray();
}

// _TabFrame
BRect
CanvasTabView::_TabFrame(int32 index)
{
	int32 count = fCanvasTabs.CountItems();
	BRect r(Bounds());
//	r.InsetBy(1.0, 1.0);
	float tabHeight = (r.Height() - 4.0 + 1.0) / count;
	BRect tabRect(r.left, r.top + index * tabHeight,
				  r.right, r.top + (index + 1) * tabHeight - 1.0);
	return tabRect;
}
