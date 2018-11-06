// MenuBar.cpp

#include <stdio.h>

#include "MenuBar.h"

// constructor
MenuBar::MenuBar(const char* name)
/*	: BMenuBar(BRect(0.0, 0.0, 10.0, 10.0),
			   name, B_FOLLOW_NONE, B_ITEMS_IN_ROW, false)*/
	: MMenuBar(B_ITEMS_IN_ROW, false)
{
	SetName(name);
	SetBorder(B_BORDER_FRAME);
	ResizeToPreferred();
}

// destructor
MenuBar::~MenuBar()
{
}

// layoutprefs
minimax
MenuBar::layoutprefs()
{
	float width, height;
	GetPreferredSize(&width, &height);

	mpm.mini.x = width + 1.0;
	mpm.maxi.x = mpm.mini.x + 10000.0;
	mpm.mini.y = mpm.maxi.y = height + 1.0;
	mpm.weight = 1.0;
	ct_mpm = mpm;
	return mpm;
}

// layout
BRect
MenuBar::layout(BRect rect)
{
	MoveTo(rect.LeftTop());
	ResizeTo(rect.Width(), rect.Height());
	return Frame();
}
