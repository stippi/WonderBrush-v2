// LabelCheckBox.cpp

#include <stdio.h>

#include "LabelCheckBox.h"

// constructor
LabelCheckBox::LabelCheckBox(const char *label, BMessage *msg,
							 BHandler *target, bool active)
		  : BCheckBox(BRect(0.0f, 0.0f, 10.0f, 10.0f),
					   label, label, msg,
					   B_FOLLOW_NONE, B_WILL_DRAW | B_NAVIGABLE)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetValue(active);
	SetTarget(target);
	SetFont(be_bold_font);
	ResizeToPreferred();
}

// destructor
LabelCheckBox::~LabelCheckBox()
{
}

// layoutprefs
minimax LabelCheckBox::layoutprefs()
{
	GetPreferredSize(&mpm.mini.x, &mpm.mini.y);
	mpm.maxi.x = mpm.mini.x;
	mpm.maxi.y = mpm.mini.y;
	mpm.weight = 0.0;
	return mpm;
}

// layout
BRect LabelCheckBox::layout(BRect rect)
{
	ResizeTo(rect.Width(), rect.Height());
	MoveTo(rect.LeftTop());
	return rect;
}
