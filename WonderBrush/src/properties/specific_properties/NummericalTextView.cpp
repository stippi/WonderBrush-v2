// NummericalTextView.cpp

#include <stdio.h>
#include <stdlib.h>

#include <String.h>

#include "NummericalTextView.h"

// constructor
NummericalTextView::NummericalTextView(BRect frame, const char* name,
									   BRect textRect,
									   uint32 resizingMode,
									   uint32 flags)
	: BTextView(frame, name, textRect, resizingMode, flags),
	  fFloatMode(false),
	  fIntValueCache(0),
	  fFloatValueCache(0.0)
{
	for (uint32 i = 0; i < '0'; i++) {
		DisallowChar(i);
	}
	for (uint32 i = '9' + 1; i < 255; i++) {
		DisallowChar(i);
	}
	AllowChar('-');
	SetWordWrap(false);
}

// destructor
NummericalTextView::~NummericalTextView()
{
}

// MouseDown
void
NummericalTextView::MouseDown(BPoint where)
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
NummericalTextView::MouseUp(BPoint where)
{
	// enforce the behaviour of a typical BTextControl
	// only let the BTextView handle mouse up/down when
	// it already had focus
	if (fWasFocus)
		BTextView::MouseUp(where);
}

// KeyDown
void
NummericalTextView::KeyDown(const char* bytes, int32 numBytes)
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

// MakeFocus
void
NummericalTextView::MakeFocus(bool focus)
{
	if (focus != IsFocus()) {
		if (BView* view = Parent())
			view->Invalidate();
		BTextView::MakeFocus(focus);
		if (focus)
			SelectAll();
		_ValueChanged();
	}
}

// Invoke
status_t
NummericalTextView::Invoke(BMessage* message)
{
	if (!message)
		message = Message();

	if (message) {
		BMessage copy(*message);
		copy.AddInt64("when", system_time());
		copy.AddPointer("source", (BView*)this);
		copy.AddInt32("be:value", IntValue());
		copy.AddFloat("float value", FloatValue());
		return BInvoker::Invoke(&copy);
	}
	return B_BAD_VALUE;
}

// SetFloatMode
void
NummericalTextView::SetFloatMode(bool floatingPoint)
{
	fFloatMode = floatingPoint;
	if (floatingPoint)
		AllowChar('.');
	else
		DisallowChar('.');
}

// SetValue
void
NummericalTextView::SetValue(int32 value)
{
	BString helper;
	helper << value;
	SetText(helper.String());

	// update caches
	IntValue();
	FloatValue();

	if (IsFocus())
		SelectAll();
}

// SetValue
void
NummericalTextView::SetValue(float value)
{
	BString helper;
	helper << value;
	SetText(helper.String());

	// update caches
	IntValue();
	FloatValue();

	if (IsFocus())
		SelectAll();
}

// IntValue
int32
NummericalTextView::IntValue() const
{
	fIntValueCache = atoi(Text());
	return fIntValueCache;
}

// FloatValue
float
NummericalTextView::FloatValue() const
{
	fFloatValueCache = atof(Text());
	return fFloatValueCache;
}

// InsertText
void
NummericalTextView::InsertText(const char* inText, int32 inLength, int32 inOffset,
							   const text_run_array* inRuns)
{
	BTextView::InsertText(inText, inLength, inOffset, inRuns);

	_CheckMinusAllowed();
	_CheckDotAllowed();
}

// DeleteText
void
NummericalTextView::DeleteText(int32 fromOffset, int32 toOffset)
{
	BTextView::DeleteText(fromOffset, toOffset);

	_CheckMinusAllowed();
	_CheckDotAllowed();
}

// _ToggleAllowChar
void
NummericalTextView::_ToggleAllowChar(char c)
{
	const char* text = Text();
	if (text) {
		bool found = false;
		while (*text) {
			if (*text == c) {
				found = true;
				break;
			}
			text++;
		}
		if (found)
			DisallowChar(c);
		else
			AllowChar(c);
	}
}

// _CheckMinusAllowed
void
NummericalTextView::_CheckMinusAllowed()
{
	_ToggleAllowChar('-');
}

// _CheckDotAllowed
void
NummericalTextView::_CheckDotAllowed()
{
	if (fFloatMode) {
		_ToggleAllowChar('.');
	}
}

// _ValueChanged
void
NummericalTextView::_ValueChanged()
{
	int32 i = atoi(Text());
	float f = atof(Text());

	if ((fFloatMode && f != fFloatValueCache) ||
		(!fFloatMode && i != fIntValueCache)) {
		Invoke();
	}
}



