// NummericalInputValueView.cpp

#include <stdio.h>

#include <Message.h>
#include <String.h>

#include "NummericalTextView.h"
#include "PropertyItemView.h"

#include "NummericalInputValueView.h"

enum {
	MSG_VALUE_CHANGED	= 'vchd',
};

// constructor
NummericalInputValueView::NummericalInputValueView(Property* property)
	: PropertyItemValueView(property)
{
	BRect b = Bounds();
	fTextView = new NummericalTextView(b, "nummerical input", b,
									   B_FOLLOW_NONE, B_WILL_DRAW);

	AddChild(fTextView);
}

// destructor
NummericalInputValueView::~NummericalInputValueView()
{
}

// AttachedToWindow
void
NummericalInputValueView::AttachedToWindow()
{
	fTextView->SetMessage(new BMessage(MSG_VALUE_CHANGED));
	fTextView->SetTarget(this);
}

// Draw
void
NummericalInputValueView::Draw(BRect updateRect)
{
	BRect b(Bounds());
	if (fTextView->IsFocus())
		SetLowColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
	StrokeRect(b, B_SOLID_LOW);
}

// FrameResized
void
NummericalInputValueView::FrameResized(float width, float height)
{
	BRect b(Bounds());
	b.InsetBy(1.0, 1.0);
	fTextView->MoveTo(b.LeftTop());
	fTextView->ResizeTo(b.Width(), b.Height());
	BRect tr(fTextView->Bounds());
	tr.InsetBy(4.0, 1.0);
	fTextView->SetTextRect(tr);
}

// MakeFocus
void
NummericalInputValueView::MakeFocus(bool focused)
{
	fTextView->MakeFocus(focused);
}

// MessageReceived
void
NummericalInputValueView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_VALUE_CHANGED:
			ValueChanged();
			break;
		default:
			PropertyItemValueView::MessageReceived(message);
	}
}

// IsFocused
bool
NummericalInputValueView::IsFocused() const
{
	return fTextView->IsFocus();
}

// SetFloatMode
void
NummericalInputValueView::SetFloatMode(bool floatingPoint)
{
	fTextView->SetFloatMode(floatingPoint);
}


