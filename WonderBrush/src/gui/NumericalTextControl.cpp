// NumericalTextControl.cpp

#include <stdio.h>
#include <stdlib.h>

#include <String.h>

#include "NumericalTextControl.h"

// constructor
NumericalTextControl::NumericalTextControl(char* label, char* text)
	: MTextControl(label, text),
	  fFloatMode(true),
	  fIntValueCache(0),
	  fFloatValueCache(0.0)
{
	_Init();
}

// constructor
NumericalTextControl::NumericalTextControl(char* label, char* text,
										   BMessage* message)
	: MTextControl(label, text, message),
	  fFloatMode(true),
	  fIntValueCache(0),
	  fFloatValueCache(0.0)
{
	_Init();
}

// destructor
NumericalTextControl::~NumericalTextControl()
{
}

/*
// KeyDown
void
NumericalTextControl::KeyDown(const char* bytes, int32 numBytes)
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

// Invoke
status_t
NumericalTextControl::Invoke(BMessage* message)
{
	if (!message)
		message = Message();

	if (message) {
		BMessage copy(*message);
		copy.AddInt64("when", system_time());
		copy.AddPointer("source", (BView*)this);
		copy.AddInt32("be:value", IntValue());
		copy.AddFloat("float value", FloatValue());
		BInvoker::Invoke(&copy);
	}
}
*/
// SetFloatMode
void
NumericalTextControl::SetFloatMode(bool floatingPoint)
{
	fFloatMode = floatingPoint;
	if (floatingPoint)
		TextView()->AllowChar('.');
	else
		TextView()->DisallowChar('.');
}

// SetValue
void
NumericalTextControl::SetValue(int32 value)
{
	BString helper;
	helper << value;
	SetText(helper.String());

	// update caches
	IntValue();
	FloatValue();

	if (TextView()->IsFocus())
		TextView()->SelectAll();
}

// SetValue
void
NumericalTextControl::SetValue(float value)
{
	BString helper;
	helper << value;
	SetText(helper.String());

	// update caches
	IntValue();
	FloatValue();

	if (TextView()->IsFocus())
		TextView()->SelectAll();
}

// SetValue
void
NumericalTextControl::SetValue(double value)
{
	SetValue((float)value);
}

// IntValue
int32
NumericalTextControl::IntValue() const
{
	fIntValueCache = atoi(Text());
	return fIntValueCache;
}

// FloatValue
float
NumericalTextControl::FloatValue() const
{
	fFloatValueCache = atof(Text());
	return fFloatValueCache;
}
/*
// InsertText
void
NumericalTextControl::InsertText(const char* inText, int32 inLength, int32 inOffset,
							   const text_run_array* inRuns)
{
	BTextView::InsertText(inText, inLength, inOffset, inRuns);

	_CheckMinusAllowed();
	_CheckDotAllowed();
}

// DeleteText
void
NumericalTextControl::DeleteText(int32 fromOffset, int32 toOffset)
{
	BTextView::DeleteText(fromOffset, toOffset);

	_CheckMinusAllowed();
	_CheckDotAllowed();
}

// _ToggleAllowChar
void
NumericalTextControl::_ToggleAllowChar(char c)
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
NumericalTextControl::_CheckMinusAllowed()
{
	_ToggleAllowChar('-');
}

// _CheckDotAllowed
void
NumericalTextControl::_CheckDotAllowed()
{
	if (fFloatMode) {
		_ToggleAllowChar('.');
	}
}

// _ValueChanged
void
NumericalTextControl::_ValueChanged()
{
	int32 i = atoi(Text());
	float f = atof(Text());

	if ((fFloatMode && f != fFloatValueCache) ||
		(!fFloatMode && i != fIntValueCache)) {
		Invoke();
	}
}
*/

// _Init
void
NumericalTextControl::_Init()
{
	for (uint32 i = 0; i < '0'; i++) {
		TextView()->DisallowChar(i);
	}
	for (uint32 i = '9' + 1; i < 255; i++) {
		TextView()->DisallowChar(i);
	}
	TextView()->AllowChar('-');
	TextView()->AllowChar('.');

	SetAlignment(B_ALIGN_RIGHT, B_ALIGN_LEFT);
}
