// BetterMCheckBox.cpp

#include <stdio.h>

#include "BetterMCheckBox.h"

// constructor
BetterMCheckBox::BetterMCheckBox(const char* label,
								 ulong id, bool state)
	: MCheckBox(label, id, state)
{
}

// constructor
BetterMCheckBox::BetterMCheckBox(const char* label,
								 BMessage* message,
								 BHandler* handler,
								 bool state)
	: MCheckBox(label, message, handler, state)
{
}

// constructor
BetterMCheckBox::BetterMCheckBox(BMessage* archive)
	: MCheckBox(archive)
{
}

// layoutprefs
minimax
BetterMCheckBox::layoutprefs()
{
	mpm = MCheckBox::layoutprefs();
	mpm.maxi.x = mpm.mini.x;
	return mpm;
}
