// LabelViewBox.cpp

#include <stdio.h>

#include <Window.h>

#include <MBorder.h>
#include <MStringView.h>

#include "LabelViewBox.h"

#include "Debug.h"
//#define ldebug	debug
#define ldebug	nodebug

//constructor
LabelViewBox::LabelViewBox(uint32 borderType, float innerSpacing, const char* name,
						   MView* labelView, MView* innerView, bool refuseLayout,
						   float layoutWeight)
	: MGroup(),
	  BView(BRect(0.0, 0.0, 10.0, 10.0), name, B_FOLLOW_NONE,
			B_WILL_DRAW | B_FRAME_EVENTS |B_NAVIGABLE_JUMP),
	  fBorderType(borderType),
	  fInnerSpacing(innerSpacing),
	  fLabelHeight(0.0),
	  fLayoutWeight(layoutWeight),
	  fLabelView(labelView),
	  fInnerView(innerView),
	  fLabelViewRect(0.0, 0.0, 0.0, 0.0),
	  fRefuseLayout(refuseLayout),
	  fLabelString("")
{
	if (BView* view = dynamic_cast<BView*>(fLabelView))
		AddChild(view);
	if (BView* view = dynamic_cast<BView*>(fInnerView))
		AddChild(view);

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

//constructor
LabelViewBox::LabelViewBox(uint32 borderType, float innerSpacing, const char* name,
						   const char* label, MView* innerView, bool refuseLayout,
						   float layoutWeight)
	: MGroup(),
	  BView(BRect(0.0, 0.0, 10.0, 10.0), name, B_FOLLOW_NONE,
			B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP),
	  fBorderType(borderType),
	  fInnerSpacing(innerSpacing),
	  fLabelHeight(0.0),
	  fLayoutWeight(layoutWeight),
	  fLabelView(NULL),
	  fInnerView(innerView),
	  fLabelViewRect(0.0, 0.0, 0.0, 0.0),
	  fRefuseLayout(refuseLayout),
	  fLabelString(label)
{
	if (BView* view = dynamic_cast<BView*>(fInnerView))
		AddChild(view);

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetLowColor(ViewColor());
	SetHighColor(0, 0, 0, 255);
	SetFont(be_bold_font);
}

// destructor
LabelViewBox::~LabelViewBox()
{
}

// Draw
void
LabelViewBox::Draw(BRect)
{
	rgb_color high = tint_color(ViewColor(), B_LIGHTEN_MAX_TINT);
	rgb_color low = tint_color(ViewColor(), B_DARKEN_2_TINT);
	BRect frame(Bounds());
	frame.top += fLabelHeight / 2.0;
	switch (fBorderType) {
		case M_RAISED_BORDER:
			_StrokeFrame(frame, low, high);
			break;
		case M_DEPRESSED_BORDER:
			_StrokeFrame(frame, high, low);
			break;
		case M_LABELED_BORDER:
		case M_ETCHED_BORDER:
			_StrokeFrame(frame, low, high);
			frame.InsetBy(1.0, 1.0);
			_StrokeFrame(frame, high, low);
			break;
		case M_NO_BORDER:
		default:
			break;
	}
	// draw label
	if (fLabelString.CountChars() > 0) {
		font_height fh;
		GetFontHeight(&fh);
		DrawString(fLabelString.String(), BPoint(fLabelViewRect.left + 4.0,
												 fLabelViewRect.top + 1.0 + fh.ascent));
	}
}

// FrameResized
void
LabelViewBox::FrameResized(float width, float height)
{
	// TODO:
	// could be smarter (remember old Bounds() and invalidate
	// old right and bottom border)
	Invalidate();
}

// AttachedToWindow
void
LabelViewBox::AttachedToWindow()
{
	// set label rect properly
	if (fLabelView) {
		minimax labelPrefs = fLabelView->layoutprefs();
		fLabelViewRect.Set(0.0, 0.0, labelPrefs.mini.x - 1.0,
						   labelPrefs.mini.y - 1.0);
		fLabelHeight = labelPrefs.mini.y;
	} else if (fLabelString.CountChars() > 0) {
		// figure out label width
		float width = be_bold_font->StringWidth(fLabelString.String()) + 8.0;
		// figure out font height -> label height
		font_height fh;
		be_bold_font->GetHeight(&fh);
		fLabelHeight = fh.ascent + fh.descent + 2.0;
		fLabelViewRect.Set(0.0, 0.0, width - 1.0, fLabelHeight - 1.0);
	}
}

// layoutprefs
minimax
LabelViewBox::layoutprefs()
{
	float minWidth = 0.0;
	float minHeight = 0.0;
	float maxWidth = 10000.0;
	float maxHeight = 10000.0;
	// calculate minimal and maximal width and height
	if (fLabelView) {
		minimax labelPrefs = fLabelView->layoutprefs();
		// set minimum to label for now
		minWidth = labelPrefs.mini.x;
		// label height will allways be the minimum height of the label view
		fLabelHeight = labelPrefs.mini.y;
	} else if (fLabelString.CountChars() > 0) {
		// figure out label width
		minWidth = be_bold_font->StringWidth(fLabelString.String()) + 8.0;
		// figure out font height -> label height
		font_height fh;
		be_bold_font->GetHeight(&fh);
		fLabelHeight = fh.ascent + fh.descent + 2.0;
	}
	fLabelViewRect.Set(0.0, 0.0, minWidth - 1.0, fLabelHeight - 1.0);
	if (fInnerView) {
		minimax innerPrefs = fInnerView->layoutprefs();
		// shrink or enlarge values
		minWidth = innerPrefs.mini.x > minWidth ? innerPrefs.mini.x : minWidth;
		minHeight = innerPrefs.mini.y;
	}
	// set our own prefs accordingly
	mpm.mini.x = 2.0 * (_BorderSize() + fInnerSpacing); // left and right border
	mpm.maxi.x = maxWidth + mpm.mini.x; // border plus maximum inner width
	mpm.mini.x += minWidth; // border plus minimum inner width
	float labelHeight = fLabelHeight / 2.0 + fInnerSpacing;
	if (labelHeight == fInnerSpacing)
		labelHeight += _BorderSize();
	if (labelHeight <= fLabelHeight)
		labelHeight = fLabelHeight + 2.0;
	mpm.mini.y = labelHeight + _BorderSize() + fInnerSpacing; // label height plus bottom border
	mpm.maxi.y = maxHeight + mpm.mini.y; // borders plus maximum height
	mpm.mini.y += minHeight; // borders plus minimum height
	mpm.weight = fLayoutWeight;
	// sanity checks
	if (mpm.maxi.x < mpm.mini.x)
		mpm.maxi.x = mpm.mini.x;
	if (mpm.maxi.y < mpm.mini.y)
		mpm.maxi.y = mpm.mini.y;
	if (mpm.weight < 0.0)
		mpm.weight = 1.0;

	return mpm;
}

// layout
BRect
LabelViewBox::layout(BRect rect)
{
	// workarround
	BWindow *win = Window();
	if (fRefuseLayout && (!win || win->IsHidden()))
		return rect;
	// layout self
	ResizeTo(rect.Width(), rect.Height());
	MoveTo(rect.LeftTop());

	if (fInnerView) {
		// calculate inner view rect
		BRect innerViewRect(0.0, 0.0, Bounds().Width(), Bounds().Height());
		innerViewRect.left += fInnerSpacing + _BorderSize();
		float labelHeight = fLabelHeight / 2.0 + fInnerSpacing;
		if (labelHeight == fInnerSpacing)
			labelHeight += _BorderSize();
		if (labelHeight <= fLabelHeight)
			labelHeight = fLabelHeight + 2.0;
		innerViewRect.top += labelHeight;
		innerViewRect.right -= fInnerSpacing + _BorderSize();
		innerViewRect.bottom -= fInnerSpacing + _BorderSize();
		if (!(fInnerView->flags & M_USE_FULL_SIZE)) {
			// shrink inner rect so that it's not too large
			minimax innerPrefs = fInnerView->layoutprefs();
			float tooLarge = innerViewRect.Width() - innerPrefs.maxi.x;
			if (tooLarge > 0.0)
				innerViewRect.InsetBy(ceilf(tooLarge / 2.0), 0.0);
			tooLarge = innerViewRect.Height() - innerPrefs.maxi.y;
			if (tooLarge > 0.0)
				innerViewRect.InsetBy(0.0, ceilf(tooLarge / 2.0));
		}
		// layout inner view
		fInnerView->layout(innerViewRect);
	}
	// center label view rect
	fLabelViewRect.OffsetTo(ceilf(rect.Width() / 2.0 - (fLabelViewRect.Width() / 2.0) + 1.0),
							fLabelViewRect.top);
	if (fLabelView)
		// layout label view
		fLabelView->layout(fLabelViewRect);

	return Frame();
}

// SetLabel
void
LabelViewBox::SetLabel(const char* label)
{
	fLabelString.SetTo(label);
	BRect invalid = fLabelViewRect;

	float width = StringWidth(fLabelString.String()) + 8.0;
	fLabelViewRect.Set(0.0, 0.0, width - 1.0, fLabelHeight - 1.0);
	// center label view rect
	fLabelViewRect.OffsetTo(ceilf(Bounds().Width() / 2.0 - (fLabelViewRect.Width() / 2.0) + 1.0),
							fLabelViewRect.top);

	invalid = invalid | fLabelViewRect;

	Invalidate(invalid);
}

// _StrokeFrame
void
LabelViewBox::_StrokeFrame(BRect r, rgb_color high, rgb_color low)
{
	BeginLineArray(5);
		AddLine(BPoint(r.left, r.bottom - 1.0),
				BPoint(r.left, r.top), high);
		AddLine(BPoint(r.left + 1.0, r.top),
				BPoint(fLabelViewRect.left - 1.0, r.top), high);
		AddLine(BPoint(fLabelViewRect.right + 1.0, r.top),
				BPoint(r.right - 1.0, r.top), high);
		AddLine(BPoint(r.right, r.top),
				BPoint(r.right, r.bottom), low);
		AddLine(BPoint(r.right - 1.0, r.bottom),
				BPoint(r.left, r.bottom), low);
	EndLineArray();
}

// _BorderSize
float
LabelViewBox::_BorderSize() const
{
	float size = 0.0;
	switch (fBorderType) {
		case M_RAISED_BORDER:
		case M_DEPRESSED_BORDER:
			size = 1.0;
			break;
		case M_LABELED_BORDER:
		case M_ETCHED_BORDER:
			size = 2.0;
			break;
		case M_NO_BORDER:
		default:
			break;
	}
	return size;
}
