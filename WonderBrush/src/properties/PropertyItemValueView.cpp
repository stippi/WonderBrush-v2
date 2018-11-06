// PropertyItemValueView.cpp

#include <stdio.h>

#include "Property.h"
#include "PropertyItemView.h"

#include "PropertyItemValueView.h"

// constructor
PropertyItemValueView::PropertyItemValueView(Property* property)
	: BView(BRect(0.0, 0.0, 10.0, 10.0), "property item",
			B_FOLLOW_NONE, B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE),
	  fParent(NULL),
	  fProperty(property),
	  fSelected(false)
{
}

// destructor
PropertyItemValueView::~PropertyItemValueView()
{
}

// Draw
void
PropertyItemValueView::Draw(BRect updateRect)
{
	// just draw background
	FillRect(Bounds(), B_SOLID_LOW);
}

// MouseDown
void
PropertyItemValueView::MouseDown(BPoint where)
{
	if (fParent) {
		// forward click
		fParent->MouseDown(ConvertToParent(where));
	}
}

// MouseUp
void
PropertyItemValueView::MouseUp(BPoint where)
{
	if (fParent) {
		// forward click
		fParent->MouseUp(ConvertToParent(where));
	}
}

// MouseMoved
void
PropertyItemValueView::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	if (fParent) {
		// forward click
		fParent->MouseMoved(ConvertToParent(where), transit, dragMessage);
	}
}

// PreferredHeight
float
PropertyItemValueView::PreferredHeight() const
{
	font_height fh;
	GetFontHeight(&fh);

	float height = floorf(4.0 + fh.ascent + fh.descent);

	return height;
}

// SetSelected
void
PropertyItemValueView::SetSelected(bool selected)
{
	fSelected = selected;
}

// SetItemView
void
PropertyItemValueView::SetItemView(PropertyItemView* parent)
{
	fParent = parent;
	if (fParent) {
		BFont font;
		fParent->GetFont(&font);
		SetFont(&font);
		SetLowColor(fParent->LowColor());
	}
}

// ValueChanged
void
PropertyItemValueView::ValueChanged()
{
	if (fParent)
		fParent->UpdateObject();
}


