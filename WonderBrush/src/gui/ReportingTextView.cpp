// ReportingTextView.cpp

#include <stdio.h>
#include <stdlib.h>

#include <String.h>
#include <ScrollView.h>

#include "ReportingTextView.h"

// constructor
ReportingTextView::ReportingTextView(minimax size)
	: MTextView(size),
	  fTextChangedMessage(NULL),
	  fWasFocus(false)
{
//	SetWordWrap(false);
}

// destructor
ReportingTextView::~ReportingTextView()
{
}

void
ReportingTextView::AttachedToWindow()
{
	MTextView::AttachedToWindow();
	SetViewColor(255, 255, 255, 255);
	SetLowColor(ViewColor());
}

/*
// MouseDown
void
ReportingTextView::MouseDown(BPoint where)
{
	// enforce the behaviour of a typical BTextControl
	// only let the BTextView handle mouse up/down when
	// it already had focus
	fWasFocus = IsFocus();
	if (fWasFocus) {
		BTextView::MouseDown(where);
	} else {
		// forward click
		if (BView* view = Parent()) {
			view->MouseDown(ConvertToParent(where));
		}
	}
}

// MouseUp
void
ReportingTextView::MouseUp(BPoint where)
{
	// enforce the behaviour of a typical BTextControl
	// only let the BTextView handle mouse up/down when
	// it already had focus
	if (fWasFocus)
		BTextView::MouseUp(where);
}

// KeyDown
void
ReportingTextView::KeyDown(const char* bytes, int32 numBytes)
{
	bool handled = true;
	if (numBytes > 0) {
		switch (bytes[0]) {
			case B_ESCAPE:
				// revert any typing changes
				if (fFloatMode)
					SetValue(fFloatValueCache);
				else
					SetValue(fIntValueCache);
				break;
			case B_TAB:
				// skip BTextView implementation
				BView::KeyDown(bytes, numBytes);
				// fall through
			case B_RETURN:
				_ValueChanged();
				break;
			default:
				handled = false;
				break;
		}
	}
	if (!handled)
		BTextView::KeyDown(bytes, numBytes);
}
*/
// MakeFocus
void
ReportingTextView::MakeFocus(bool focus)
{
	if (focus != IsFocus()) {
		BTextView::MakeFocus(focus);
		if (BScrollView* view = dynamic_cast<BScrollView*>(Parent()))
			view->SetBorderHighlighted(focus);
	}
}

// WindowActivated
void
ReportingTextView::WindowActivated(bool active)
{
	if (BView* view = Parent())
		view->Invalidate();
}

// Invoke
status_t
ReportingTextView::Invoke(BMessage* message)
{
	if (!message)
		message = Message();

	if (message) {
		BMessage copy(*message);
		copy.AddInt64("when", system_time());
		copy.AddPointer("source", (BView*)this);
//		copy.AddString("text", Text());
		return BInvoker::Invoke(&copy);
	}
	return B_BAD_VALUE;
}

// SetModificationMessage
void
ReportingTextView::SetModificationMessage(BMessage* message)
{
	delete fTextChangedMessage;
	fTextChangedMessage = message;
}

// SetEnabled
void
ReportingTextView::SetEnabled(bool enable)
{
	if (fEnabled != enable) {
		fEnabled = enable;
		if (fEnabled) {
			SetViewColor(255, 255, 255, 255);
			rgb_color textColor = (rgb_color){ 0, 0, 0, 255 };
			SetFontAndColor(be_plain_font, B_FONT_ALL, &textColor);
			MakeEditable(true);
			MakeSelectable(true);
		} else {
			SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
			rgb_color textColor = tint_color(ViewColor(), B_DISABLED_LABEL_TINT);
			SetFontAndColor(be_plain_font, B_FONT_ALL, &textColor);
			MakeEditable(false);
			MakeSelectable(false);
		}
		Invalidate();
	}
}

// InsertText
void
ReportingTextView::InsertText(const char* inText, int32 inLength, int32 inOffset,
							   const text_run_array* inRuns)
{
	BTextView::InsertText(inText, inLength, inOffset, inRuns);

	if (fTextChangedMessage)
		Invoke(fTextChangedMessage);
}

// DeleteText
void
ReportingTextView::DeleteText(int32 fromOffset, int32 toOffset)
{
	BTextView::DeleteText(fromOffset, toOffset);

	if (fTextChangedMessage)
		Invoke(fTextChangedMessage);
}

