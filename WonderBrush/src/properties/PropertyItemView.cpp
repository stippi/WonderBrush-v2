// PropertyItemView.cpp

#include <stdio.h>

#include <Message.h>
#include <Window.h>

#include "Property.h"
#include "PropertyItemValueView.h"
#include "PropertyListView.h"

#include "PropertyItemView.h"

// constructor
PropertyItemView::PropertyItemView(Property* property)
	: BView(BRect(0.0, 0.0, 10.0, 10.0), "property item",
			B_FOLLOW_NONE, B_NAVIGABLE | B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE),
	  fParent(NULL),
	  fValueView(property ? property->Editor() : NULL),
	  fSelected(false),
	  fLabelWidth(0.0)
{
	if (fValueView) {
		AddChild(fValueView);
		fValueView->SetItemView(this);
	}
}

// destructor
PropertyItemView::~PropertyItemView()
{
}

// Draw
void
PropertyItemView::Draw(BRect updateRect)
{
	const Property* property = GetProperty();
	if (property && fParent) {
		BRect b(Bounds());

		// just draw background and label
		SetHighColor(0, 0, 0, 255);
		BFont font;
		GetFont(&font);
		
		BString truncated(property->Name());
		font.TruncateString(&truncated, B_TRUNCATE_MIDDLE, fLabelWidth - 10.0);

		font_height fh;
		font.GetHeight(&fh);

		FillRect(BRect(b.left, b.top, b.left + fLabelWidth, b.bottom), B_SOLID_LOW);
		DrawString(truncated.String(), BPoint(b.left + 5.0,
											  floorf(b.top + b.Height() / 2.0
												  		   + fh.ascent / 2.0)));

		// draw a "separator" line behind the label
		SetHighColor(tint_color(LowColor(), B_DARKEN_1_TINT));
		StrokeLine(BPoint(b.left + fLabelWidth - 1.0, b.top),
				   BPoint(b.left + fLabelWidth - 1.0, b.bottom), B_SOLID_HIGH);
	}
}

// FrameResized
void
PropertyItemView::FrameResized(float width, float height)
{
	if (fValueView) {
		fValueView->MoveTo(fLabelWidth, 0.0);
		fValueView->ResizeTo(width - fLabelWidth, height);
		fValueView->FrameResized(fValueView->Bounds().Width(),
								 fValueView->Bounds().Height());
	}
}

// MakeFocus
void
PropertyItemView::MakeFocus(bool focused)
{
	if (fValueView)
		fValueView->MakeFocus(focused);
}

// MouseDown
void
PropertyItemView::MouseDown(BPoint where)
{
	if (fParent) {
		// select ourself
		fParent->Select(this);
		if (fValueView)
			fValueView->MakeFocus(true);

		if (BMessage* message = Window()->CurrentMessage()) {
			int32 clicks;
			if (message->FindInt32("clicks", &clicks) >= B_OK)
				if (clicks >= 2)
					fParent->DoubleClicked(this);
				else
					fParent->Clicked(this);
		}
	}
}

// MouseUp
void
PropertyItemView::MouseUp(BPoint where)
{
}

// MouseMoved
void
PropertyItemView::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
}

// PreferredHeight
float
PropertyItemView::PreferredHeight() const
{
	font_height fh;
	GetFontHeight(&fh);

	float height = floorf(4.0 + fh.ascent + fh.descent);
	if (fValueView)
		height = max_c(height, fValueView->PreferredHeight());

	return height;
}

// PreferredLabelWidth
float
PropertyItemView::PreferredLabelWidth() const
{
	float width = 0.0;
	if (const Property* property = GetProperty())
		width = ceilf(StringWidth(property->Name()) + 10.0);
	return width;
}

// SetLabelWidth
void
PropertyItemView::SetLabelWidth(float width)
{
	if (width < 0.0)
		width = 0.0;
/*	if (fValueView && width > Bounds().Width() - fValueView->Bounds().Width())
		width = Bounds().Width() - fValueView->Bounds().Width();
	else if (width > Bounds().Width())
		width = Bounds().Width();*/

	fLabelWidth = width;
}

// SetSelected
void
PropertyItemView::SetSelected(bool selected)
{
	fSelected = selected;
	SetListView(fParent);
	Invalidate();
}

// IsFocused
bool
PropertyItemView::IsFocused() const
{
	if (fValueView)
		return fValueView->IsFocused();
	return false;
}

// GetProperty
const Property*
PropertyItemView::GetProperty() const
{
	if (fValueView)
		return fValueView->GetProperty();
	return NULL;
}

// SetProperty
bool
PropertyItemView::SetProperty(const Property* property)
{
	if (fValueView)
		return fValueView->SetToProperty(property);
	return false;
}

// SetListView
void
PropertyItemView::SetListView(PropertyListView* parent)
{
	fParent = parent;
	if (fParent) {
		if (fSelected)
			SetLowColor(tint_color(fParent->LowColor(), B_DARKEN_2_TINT));
		else
			SetLowColor(fParent->LowColor());
	}
}

// UpdateObject
void
PropertyItemView::UpdateObject()
{
	if (fParent)
		fParent->UpdateObject();
}



