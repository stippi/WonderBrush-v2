// BetterMScrollView.cpp

#include <stdio.h>

#include "BetterMScrollView.h"

// constructor
BetterMScrollView::BetterMScrollView(MView* target, bool horizontal,
									 bool vertical,
									 border_style border,
									 minimax size)
#ifdef TARGET_PLATFORM_ZETA	
	: MScrollView(target, horizontal, vertical, border, size),
	  fWindowActive(false)
#else
	: MScrollView(target, horizontal, vertical, border, size)
#endif // TARGET_PLATFORM_ZETA
{
	SetFlags(Flags() | B_FULL_UPDATE_ON_RESIZE);
}

// AttachedToWindow
void
BetterMScrollView::AttachedToWindow()
{
	MScrollView::AttachedToWindow();
	SetViewColor(B_TRANSPARENT_32_BIT);

#ifdef TARGET_PLATFORM_ZETA
	// relayout scrollbars as in R5
	// TODO: make this complete for all border_styles
	// and the horizontal scrollbar (but for now it is sufficient)
	BRect bounds = Bounds();
	switch (Border()) {
		case B_FANCY_BORDER: {
			BScrollBar* vScrollBar = ScrollBar(B_VERTICAL);
			if (vScrollBar) {
				vScrollBar->MoveTo(bounds.right - (B_V_SCROLL_BAR_WIDTH + 1),
								   bounds.top + 1);
				vScrollBar->ResizeTo(vScrollBar->Frame().Width(),
									 bounds.Height() - 2);
			}

			BScrollBar* hScrollBar = ScrollBar(B_HORIZONTAL);
			if (hScrollBar) {
				hScrollBar->MoveTo(bounds.left + 1,
								   bounds.bottom - (B_H_SCROLL_BAR_HEIGHT + 1));
				hScrollBar->ResizeTo(bounds.Width() - 2,
									 hScrollBar->Frame().Height());
			}
			
			if (BView* target = Target()) {
				target->MoveTo(2.0, 2.0);
				if (vScrollBar && hScrollBar) {
					target->ResizeTo(bounds.Width() - (B_V_SCROLL_BAR_WIDTH + 4),
									 bounds.Height() - (B_H_SCROLL_BAR_HEIGHT + 4));
				} else if (vScrollBar) {
					target->ResizeTo(bounds.Width() - (B_V_SCROLL_BAR_WIDTH + 4),
									 bounds.Height() - 4);
				} else if (hScrollBar) {
					target->ResizeTo(bounds.Width() - 4,
									 bounds.Height() - (B_H_SCROLL_BAR_HEIGHT + 4));
				} else {
					target->ResizeTo(bounds.Width() - 4, bounds.Height() - 4);
				}
			}
			break;
		}
		default:
			break;
	}
#endif // TARGET_PLATFORM_ZETA
}

#ifdef TARGET_PLATFORM_ZETA

// WindowActivated
void
BetterMScrollView::WindowActivated(bool activated)
{
	fWindowActive = activated;
	if (Target() && Target()->IsFocus())
		Invalidate();
}

// Draw
void
BetterMScrollView::Draw(BRect updateRect)
{
	rgb_color keyboardFocus = keyboard_navigation_color();
	rgb_color light = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
								 B_LIGHTEN_MAX_TINT);
	rgb_color shadow = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
								  B_DARKEN_1_TINT);
	rgb_color darkShadow = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
								  B_DARKEN_2_TINT);
	rgb_color darkerShadow = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),
								  B_DARKEN_3_TINT);
	BRect bounds = Bounds();
	if (fWindowActive && Target() && Target()->IsFocus()) {
		BeginLineArray(4);
		AddLine(BPoint(bounds.left, bounds.bottom),
				BPoint(bounds.left, bounds.top), keyboardFocus);
		AddLine(BPoint(bounds.left + 1.0, bounds.top),
				BPoint(bounds.right, bounds.top), keyboardFocus);
		AddLine(BPoint(bounds.right, bounds.top + 1.0),
				BPoint(bounds.right, bounds.bottom), keyboardFocus);
		AddLine(BPoint(bounds.right - 1.0, bounds.bottom),
				BPoint(bounds.left + 1.0, bounds.bottom), keyboardFocus);
		EndLineArray();
	} else {
		BeginLineArray(4);
		AddLine(BPoint(bounds.left, bounds.bottom),
				BPoint(bounds.left, bounds.top), shadow);
		AddLine(BPoint(bounds.left + 1.0, bounds.top),
				BPoint(bounds.right, bounds.top), shadow);
		AddLine(BPoint(bounds.right, bounds.top + 1.0),
				BPoint(bounds.right, bounds.bottom), light);
		AddLine(BPoint(bounds.right - 1.0, bounds.bottom),
				BPoint(bounds.left + 1.0, bounds.bottom), light);
		EndLineArray();
	}
	bounds.InsetBy(1.0, 1.0);
	BeginLineArray(4);
	AddLine(BPoint(bounds.left, bounds.bottom),
			BPoint(bounds.left, bounds.top), darkerShadow);
	AddLine(BPoint(bounds.left + 1.0, bounds.top),
			BPoint(bounds.right, bounds.top), darkShadow);
	AddLine(BPoint(bounds.right, bounds.top + 1.0),
			BPoint(bounds.right, bounds.bottom), darkShadow);
	AddLine(BPoint(bounds.right - 1.0, bounds.bottom),
			BPoint(bounds.left + 1.0, bounds.bottom), darkShadow);
	EndLineArray();
}

#endif // TARGET_PLATFORM_ZETA
