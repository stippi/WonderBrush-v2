#include "MStatusBar.h"

#include <stdio.h>

// constructor
MStatusBar::MStatusBar(const char* label, const char* trailingLabel)
	: BStatusBar(BRect(0.0f, 0.0f, 10.0f, 10.0f), label, trailingLabel)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetResizingMode(B_FOLLOW_NONE);
}

// destructor
MStatusBar::~MStatusBar()
{
}

// layoutprefs
minimax
MStatusBar::layoutprefs()
{
	GetPreferredSize(&mpm.mini.x, &mpm.mini.y);
	mpm.maxi.x = 10000.0f;
	mpm.maxi.y = mpm.mini.y;
	mpm.weight = 0.01f;
	return mpm;
}

// layout
BRect
MStatusBar::layout(BRect frame)
{
	ResizeTo(frame.Width(), frame.Height());
	MoveTo(frame.LeftTop());
	Invalidate();
	return Frame();
}
