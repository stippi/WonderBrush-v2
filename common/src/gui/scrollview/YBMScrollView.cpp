// YBMScrollView.cpp

#include "YBMScrollView.h"

// constructor
YBMScrollView::YBMScrollView(BView* child, uint32 scrollingFlags, const char *name)
	: ScrollView(child, scrollingFlags, BRect(0.0, 0.0, 100.0, 100.0),
				 name, B_FOLLOW_NONE, B_WILL_DRAW | B_FRAME_EVENTS)
{
}

// layoutprefs
minimax
YBMScrollView::layoutprefs()
{
	mpm.mini.x = 100.0f;
	mpm.maxi.x = 2000.0f;
	mpm.mini.y = 70.0f;
	mpm.maxi.y = 2000.0f;
	mpm.weight = 1.0f;
	return mpm;
}

// layout
BRect
YBMScrollView::layout(BRect rect)
{
	ResizeTo(rect.Width(), rect.Height());
//	FrameResized(rect.Width(), rect.Height());
	MoveTo(rect.LeftTop());
	return Frame();
}
