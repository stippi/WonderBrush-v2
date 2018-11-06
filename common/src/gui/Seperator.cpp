// Seperator.cpp

#include <stdio.h>
#include <math.h>

#include <StringView.h>

#include "Seperator.h"

// constructor
Seperator::Seperator(const char *label, orientation direction)
	: BView(BRect(0.0, 0.0, 10.0, 10.0), "seperator", B_FOLLOW_NONE,
			B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	  fLabelView(NULL),
	  fLabel(label),
	  fOrient(direction),
	  fLabelWidth(10.0),
	  fLabelHeight(10.0)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	BFont font(be_bold_font);
	if (fOrient == B_VERTICAL)
		font.SetRotation(90.0);
	SetFont(&font);
}

// constructor
Seperator::Seperator(MView *label, orientation direction)
	: BView(BRect(0.0, 0.0, 10.0, 10.0), "seperator", B_FOLLOW_NONE,
			B_WILL_DRAW),
	  fLabelView(label),
	  fLabel(""),
	  fOrient(direction),
	  fLabelWidth(10.0),
	  fLabelHeight(10.0)
{
	if (BView* view = dynamic_cast<BView*>(fLabelView)) {
		AddChild(view);
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
		SetLowColor(ViewColor());
	} else {
		fLabelView = NULL;
		SetViewColor(B_TRANSPARENT_32_BIT);
	}
}

// constructor
Seperator::Seperator(orientation direction)
	: BView(BRect(0.0, 0.0, 10.0, 10.0), "seperator", B_FOLLOW_NONE,
			B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	  fLabelView(NULL),
	  fLabel(NULL),
	  fOrient(direction),
	  fLabelWidth(10.0),
	  fLabelHeight(10.0)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	BFont font(be_bold_font);
	if (fOrient == B_VERTICAL)
		font.SetRotation(90.0);
	SetFont(&font);
}

// destructor
Seperator::~Seperator()
{
}

// Draw
void
Seperator::Draw(BRect update)
{
	rgb_color lowColor;
	if (BView* parent = Parent())
		lowColor = parent->ViewColor();
	else
		lowColor = ui_color(B_PANEL_BACKGROUND_COLOR);
	if (!fLabelView) {
		SetLowColor(lowColor);
		FillRect(update, B_SOLID_LOW);
	}
	float xm = (Bounds().left + Bounds().right) / 2.0;
	float ym = (Bounds().top + Bounds().bottom) / 2.0;
	
	if (fLabel.CountChars() > 0) {
		rgb_color shadow = tint_color(LowColor(), B_DARKEN_2_TINT);
		rgb_color light = tint_color(LowColor(), B_LIGHTEN_MAX_TINT);
	
		// don't touch label!
		BRect rect(Bounds());
		float tooMuchX = rect.Width() - fLabelWidth;
		float tooMuchY = rect.Height() - fLabelHeight;
		rect.left = tooMuchX / 2.0;
		rect.top = tooMuchY / 2.0;
		rect.right = rect.left + fLabelWidth;
		rect.bottom = rect.top + fLabelHeight;

		if (fOrient == B_HORIZONTAL) {
			SetHighColor(0, 0, 0, 255);
			DrawString(fLabel.String(), BPoint(rect.left, rect.top + fAscent));
			SetHighColor(shadow);
			StrokeLine(BPoint(1.0, ym - 1.0),
					   BPoint(rect.left - 4.0, ym - 1.0));
			StrokeLine(BPoint(rect.right + 4.0, ym - 1.0),
					   BPoint(Bounds().right - 1.0, ym - 1.0));
			SetHighColor(light);
			StrokeLine(BPoint(1.0, ym),
					   BPoint(rect.left - 4.0, ym));
			StrokeLine(BPoint(rect.right + 4.0, ym),
					   BPoint(Bounds().right - 1.0, ym));
		} else {
			SetHighColor(0, 0, 0, 255);
			DrawString(fLabel.String(), BPoint(rect.left + fAscent, rect.bottom));
			SetHighColor(shadow);
			StrokeLine(BPoint(xm, 1.0),
					   BPoint(xm, rect.top - 4.0));
			StrokeLine(BPoint(xm, rect.bottom + 4.0),
					   BPoint(xm, Bounds().bottom - 1.0));
			SetHighColor(light);
			StrokeLine(BPoint(xm + 1.0, 1.0),
					   BPoint(xm + 1.0, rect.top - 4.0));
			StrokeLine(BPoint(xm + 1.0, rect.bottom + 4.0),
					   BPoint(xm + 1.0, Bounds().bottom - 1.0));
		}
	} else {
		rgb_color shadow = tint_color(LowColor(), B_DARKEN_1_TINT);
		rgb_color light = tint_color(LowColor(), B_LIGHTEN_2_TINT);
	
		// draw underneath view if we have one at all
		SetHighColor(shadow);
		if (fOrient == B_HORIZONTAL)
			StrokeLine(BPoint(update.left, ym - 1.0),
					   BPoint(update.right, ym - 1.0));
		else 
			StrokeLine(BPoint(xm, update.top),
					   BPoint(xm, update.bottom));
		SetHighColor(light);
		if (fOrient == B_HORIZONTAL)
			StrokeLine(BPoint(update.left, ym),
					   BPoint(update.right, ym));
		else 
			StrokeLine(BPoint(xm + 1.0, update.top),
					   BPoint(xm + 1.0, update.bottom));
	}
}

// layoutprefs
minimax
Seperator::layoutprefs()
{
	mpm.weight = 1.0;
	if (fLabelView) {
		minimax childPrefs = fLabelView->layoutprefs();
		if (fOrient == B_HORIZONTAL) {
			fLabelWidth = childPrefs.mini.x;
			fLabelHeight = childPrefs.mini.y;
		} else {
			fLabelWidth = childPrefs.mini.y;
			fLabelHeight = childPrefs.mini.x;
		}
	} else if (fLabel.CountChars() > 0) {
		fLabelWidth = StringWidth(fLabel.String());
		font_height fontHeight;
		GetFontHeight(&fontHeight);
		fLabelHeight = ceilf(fontHeight.ascent + fontHeight.descent) + 3.0;
		fAscent = ceilf(fontHeight.ascent);
	}
	if (fOrient == B_HORIZONTAL) {
		mpm.mini.x = fLabelWidth + 10.0;
		mpm.mini.y = fLabelHeight;
		mpm.maxi.x = 10000.0;
		mpm.maxi.y = fLabelHeight;
	} else {
		mpm.mini.x = fLabelHeight;
		mpm.mini.y = fLabelWidth + 10.0;
		mpm.maxi.x = fLabelHeight;
		mpm.maxi.y = 10000.0;
		// swap width and height
		float temp = fLabelHeight;
		fLabelHeight = fLabelWidth;
		fLabelWidth = temp;
	}
	mpm.weight = 1.0;
	return mpm;
}

// layout
BRect
Seperator::layout(BRect rect)
{
	MoveTo(rect.LeftTop());
	ResizeTo(rect.Width(), rect.Height());
	rect = Bounds();
	if (fLabelView) {
		float tooMuchX = rect.Width() - fLabelWidth;
		float tooMuchY = rect.Height() - fLabelHeight;
		rect.left = tooMuchX / 2.0;
		rect.top = tooMuchY / 2.0;
		rect.right = rect.left + fLabelWidth;
		rect.bottom = rect.top + fLabelHeight;
		fLabelView->layout(rect);
	}
	return Frame();
}

// SetLabel
void
Seperator::SetLabel(const char* label)
{
	fLabel.SetTo(label);
	Invalidate();
}
