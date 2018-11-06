// BrushConfigView.cpp

#include <stdio.h>

#include <Message.h>
#include <Window.h>

#include <MCheckBox.h>

#include "BrushView.h"

#include "BrushConfigView.h"

// constructor
BrushConfigView::BrushConfigView(float borderInset,
								 BrushView* brushView,
								 MView* group1,
								 MView* group2)
	: BView(BRect(0.0, 0.0, 10.0, 10.0), "brush config view", B_FOLLOW_NONE,
			B_FULL_UPDATE_ON_RESIZE | B_WILL_DRAW),
	  fBorderInset(borderInset),
	  fBrushView(brushView),
	  fSliderGroup(group1),
	  fCheckBoxGroup(group2)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	if (fBorderInset < 0.0)
		fBorderInset = 0.0;
	AddChild(fBrushView);
	AddChild(dynamic_cast<BView*>(fSliderGroup));
	AddChild(dynamic_cast<BView*>(fCheckBoxGroup));
}

// destructor
BrushConfigView::~BrushConfigView()
{
}

// AllAttached
void
BrushConfigView::AllAttached()
{
	BView::AllAttached();
	SetViewColor(B_TRANSPARENT_32_BIT);
}

// Draw
void
BrushConfigView::Draw(BRect updateRect)
{
	rgb_color light = tint_color(LowColor(), B_LIGHTEN_MAX_TINT);
	rgb_color shadow = tint_color(LowColor(), B_DARKEN_2_TINT);
	BRect r(Bounds());
	BeginLineArray(4);
		AddLine(BPoint(r.left, r.bottom),
				BPoint(r.left, r.top), light);
		AddLine(BPoint(r.left + 1.0, r.top),
				BPoint(r.right, r.top), light);
		AddLine(BPoint(r.right, r.top + 1.0),
				BPoint(r.right, r.bottom), shadow);
		AddLine(BPoint(r.right - 1.0, r.bottom),
				BPoint(r.left + 1.0, r.bottom), shadow);
	EndLineArray();
	r.InsetBy(1.0, 1.0);
	FillRect(r, B_SOLID_LOW);
}

// MessageReceived
void
BrushConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		default:
			BView::MessageReceived(message);
	}
}

// layoutprefs
minimax
BrushConfigView::layoutprefs()
{
	mpm.mini.x = 2.0 + fBorderInset;
	mpm.mini.x += fBrushView->layoutprefs().mini.x + fBorderInset;
	mpm.mini.x += fSliderGroup->layoutprefs().mini.x + fBorderInset;
	mpm.mini.x += fCheckBoxGroup->layoutprefs().mini.x + fBorderInset;
	// since we might be asked to layout in a frame heigher that maxi.y
	// give some room to allow for brush view to still be square and cover the whole height
	mpm.mini.x += 15.0;

	mpm.maxi.x = mpm.mini.x + 10000.0;

	mpm.mini.y = max_c(fBrushView->mpm.mini.y, fSliderGroup->mpm.mini.y) + 2.0 * fBorderInset;
	mpm.maxi.y = mpm.mini.y;
	mpm.weight = 1.0;

	return mpm;
}

// layout
BRect
BrushConfigView::layout(BRect rect)
{
	MoveTo(rect.LeftTop());
	ResizeTo(rect.Width(), rect.Height());
	// layout children
	// brush view
	rect = ConvertFromParent(rect);
	rect.InsetBy(fBorderInset, fBorderInset);
	BRect r(rect);
	r.right = r.left + r.Height(); // layout brush preview with 1:1 aspect
	_LayoutView(fBrushView, r);
	// subpixel checkbox
	r.right = rect.right;
	r.left = r.right - fCheckBoxGroup->mpm.mini.x;
	_LayoutView(fCheckBoxGroup, r);
	// group
	r.right = r.left - fBorderInset;
	r.left = fBrushView->Frame().right + fBorderInset;
	_LayoutView(fSliderGroup, r);

	return Frame();
}

// _LayoutView
void
BrushConfigView::_LayoutView(MView* view, BRect frame)
{
	float tooWide = frame.Width() - view->mpm.maxi.x;
	if (tooWide > 0.0)
		frame.InsetBy(tooWide / 2.0, 0.0);

	float tooTall = frame.Height() - view->mpm.maxi.y;
	if (tooTall > 0.0)
		frame.InsetBy(0.0, tooTall / 2.0);
	view->layout(frame);
}


