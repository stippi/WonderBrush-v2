// BetterSpinButton.cpp

#include <stdio.h>

#include "BetterSpinButton.h"

// constructor
BetterSpinButton::BetterSpinButton(const char* label,
								   spinmode mode,
								   BHandler* target)
	: SpinButton(label, mode, target)
{
	if (BTextView* tv = TextView()) {
		if (mode == SPIN_INTEGER)
			tv->DisallowChar('.');
	}
}

// destructor
BetterSpinButton::~BetterSpinButton()
{
}

// SetLabel
void
BetterSpinButton::SetLabel(const char* label)
{
	BControl::SetLabel(label);
	// enhance the original version to force relayouting later on
	if (labelwidth > 0.0)
		labelwidth = -1.0;
}

// SetEnabled
void
BetterSpinButton::SetEnabled(bool enable)
{
	if (IsEnabled() != enable) {
		SpinButton::SetEnabled(enable);
		if (BView* textView = TextView())
			textView->Invalidate();
	}
}

// TextView
BTextView*
BetterSpinButton::TextView()
{
	return dynamic_cast<BTextView*>(ChildAt(0));
}
