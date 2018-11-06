//	FramedIconButton.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <Region.h>
#include <Window.h>

#include "FramedIconButton.h"

// constructor
FramedIconButton::FramedIconButton(const char* name, uint32 id, const char* label,
								   BMessage* message, BHandler* target)
	: IconButton(name, id, label, message, target)
{
}

// destructor
FramedIconButton::~FramedIconButton()
{
}

// layoutprefs
minimax
FramedIconButton::layoutprefs()
{
	float minWidth = 0.0;
	float minHeight = 0.0;
	if (IsValid()) {
		minWidth += fNormalBitmap->Bounds().IntegerWidth() + 1.0;
		minHeight += fNormalBitmap->Bounds().IntegerHeight() + 1.0;
	} else {
		minWidth += 2.0;
		minHeight += 2.0;
	}

	mpm.mini.x = minWidth + 4.0;
	mpm.maxi.x = 10000.0 + 4.0;
	mpm.mini.y = minHeight + 4.0;
	mpm.maxi.y = 10000.0 + 4.0;
	mpm.weight = 0.0;
	return mpm;
}

// DrawBorder
bool
FramedIconButton::DrawBorder() const
{
	return true;
}

// DrawNormalBorder
void
FramedIconButton::DrawNormalBorder(BRect r, rgb_color background,
							 rgb_color shadow, rgb_color darkShadow,
							 rgb_color lightShadow, rgb_color light)
{
	_DrawFrame(r, light, darkShadow, background, lightShadow);
}

// DrawPressedBorder
void
FramedIconButton::DrawPressedBorder(BRect r, rgb_color background,
							rgb_color shadow, rgb_color darkShadow,
							rgb_color lightShadow, rgb_color light)
{
	_DrawFrame(r, darkShadow, background, background, background);
}
