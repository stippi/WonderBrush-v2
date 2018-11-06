// BoolValueView.cpp

#include <stdio.h>

#include "BoolProperty.h"

#include "BoolValueView.h"

// constructor
BoolValueView::BoolValueView(BoolProperty* property)
	: PropertyItemValueView(property),
	  fProperty(property),
	  fCheckBoxRect(0.0, 0.0, -1.0, -1.0)
{
}

// destructor
BoolValueView::~BoolValueView()
{
}

// Draw
void
BoolValueView::Draw(BRect updateRect)
{
	BRect b(Bounds());
	// focus indication
	if (IsFocus()) {
		SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		StrokeRect(b);
		b.InsetBy(1.0, 1.0);
	}
	// background
	FillRect(b, B_SOLID_LOW);

	// checkmark box
	SetHighColor(0, 0, 0, 255);
	b = fCheckBoxRect;
	StrokeRect(b);

	// checkmark
	if (fProperty && fProperty->Value()) {
		SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		b.InsetBy(3.0, 3.0);
		SetPenSize(2.0);
		StrokeLine(b.LeftTop(), b.RightBottom());
		StrokeLine(b.LeftBottom(), b.RightTop());
	}
}

// FrameResized
void
BoolValueView::FrameResized(float width, float height)
{
	float radius = ceilf((height - 6.0) / 2.0);
	float centerX = floorf(Bounds().left + width / 2.0);
	float centerY = floorf(Bounds().top + height / 2.0);
	fCheckBoxRect.Set(centerX - radius, centerY - radius,
					  centerX + radius, centerY + radius);
}

// MakeFocus
void
BoolValueView::MakeFocus(bool focused)
{
	PropertyItemValueView::MakeFocus(focused);
	Invalidate();
}

// MouseDown
void
BoolValueView::MouseDown(BPoint where)
{
	MakeFocus(true);	
	if (fCheckBoxRect.Contains(where)) {
		_ToggleValue();
	}
	// careful, when this function returns, the object will in fact have been deleted
}

// KeyDown
void
BoolValueView::KeyDown(const char* bytes, int32 numBytes)
{
	bool handled = true;
	if (numBytes > 0) {
		switch (bytes[0]) {
			case B_RETURN:
			case B_SPACE:
			case B_UP_ARROW:
			case B_DOWN_ARROW:
			case B_LEFT_ARROW:
			case B_RIGHT_ARROW:
				_ToggleValue();
				break;
			default:
				handled = false;
				break;
		}
	}
	if (!handled)
		PropertyItemValueView::KeyDown(bytes, numBytes);
}

// _ToggleValue
void
BoolValueView::_ToggleValue()
{
	if (fProperty) {
		fProperty->SetValue(!fProperty->Value());
		BRect b(fCheckBoxRect);
		b.InsetBy(1.0, 1.0);
		Invalidate(b);
		ValueChanged();
	}
}

// SetToProperty
bool
BoolValueView::SetToProperty(const Property* property)
{
	const BoolProperty* p = dynamic_cast<const BoolProperty*>(property);
	if (p) {
		if (p->Value() != fProperty->Value()) {
			fProperty->SetValue(p->Value());
			BRect b(fCheckBoxRect);
			b.InsetBy(1.0, 1.0);
			Invalidate(b);
		}
		return true;
	}
	return false;
}

