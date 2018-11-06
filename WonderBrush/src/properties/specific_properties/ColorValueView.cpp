// ColorValueView.cpp

#include <stdio.h>

#include <Message.h>
#include <String.h>
#include <Window.h>

#include "support.h"

#include "CanvasView.h"
#include "ColorProperty.h"
#include "PropertyItemView.h"
#include "SwatchValueView.h"

#include "ColorValueView.h"

enum {
	MSG_VALUE_CHANGED	= 'vchd',
};

// constructor
ColorValueView::ColorValueView(ColorProperty* property)
	: PropertyItemValueView(property),
	  fProperty(property)
{
	fSwatchView = new SwatchValueView("swatch property view",
									  new BMessage(MSG_SET_COLOR), this,
									  fProperty->Color());
	fSwatchView->SetDroppedMessage(new BMessage(MSG_VALUE_CHANGED));
	AddChild(fSwatchView);
}

// destructor
ColorValueView::~ColorValueView()
{
}

// Draw
void
ColorValueView::Draw(BRect updateRect)
{
	BRect b(Bounds());
	if (fSwatchView->IsFocus()) {
		SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		StrokeRect(b);
		b.InsetBy(1.0, 1.0);
		updateRect = updateRect & b;
	}
	FillRect(b, B_SOLID_LOW);
}

// FrameResized
void
ColorValueView::FrameResized(float width, float height)
{
	BRect b(Bounds());
	b.InsetBy(2.0, 2.0);
	b.left = floorf(b.left + (b.Width() / 2.0) - b.Height() / 2.0);
	b.right = b.left + b.Height();
	
	fSwatchView->MoveTo(b.LeftTop());
	fSwatchView->ResizeTo(b.Width(), b.Height());
}

// MakeFocus
void
ColorValueView::MakeFocus(bool focused)
{
	fSwatchView->MakeFocus(focused);
}

// MessageReceived
void
ColorValueView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_PASTE:
			fSwatchView->MessageReceived(message);
			break;
		case MSG_SET_COLOR:
			if (BWindow* window = Window())
				window->PostMessage(message, window);
			break;
		case MSG_VALUE_CHANGED: {
			rgb_color c;
			if (restore_color_from_message(message, c) >= B_OK) {
				fProperty->SetColor(c);
				ValueChanged();
			}
			break;
		}
		default:
			PropertyItemValueView::MessageReceived(message);
	}
}

// IsFocused
bool
ColorValueView::IsFocused() const
{
	return fSwatchView->IsFocus();
}


// SetToProperty
bool
ColorValueView::SetToProperty(const Property* property)
{
	const ColorProperty* p = dynamic_cast<const ColorProperty*>(property);
	if (p) {
		rgb_color ownColor = fProperty->Color();
		rgb_color color = p->Color();
		if (ownColor.red != color.red ||
			ownColor.green != color.green ||
			ownColor.blue != color.blue ||
			ownColor.alpha != color.alpha) {
			fProperty->SetColor(color);
			fSwatchView->SetColor(color);
		}
		return true;
	}
	return false;
}
