// BrushView.cpp

#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <String.h>

#include "support.h"

#include "Brush.h"

#include "BrushView.h"

#define SIZE 32.0

// constructor
BrushView::BrushView(const char* name,
					 BMessage* message, const BHandler* target)
	: BView(BRect(0.0, 0.0, SIZE + 4.0, SIZE + 4.0),
			name, B_FOLLOW_NONE, B_WILL_DRAW | B_FRAME_EVENTS
				  | B_FULL_UPDATE_ON_RESIZE),
	  BInvoker(message, target),
	  fAlphaBitmap(new BBitmap(BRect(0.0, 0.0, SIZE, SIZE), B_GRAY8)),
	  fDisplayBitmap(new BBitmap(BRect(0.0, 0.0, SIZE, SIZE), B_RGB32)),
	  fBrush(new Brush((range){ 15.0, 15.0 }, (range){ 1.0, 1.0 })),
	  fFlags(0),
	  fEnabled(true)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	Update(fBrush, fFlags);
}

// destructor
BrushView::~BrushView()
{
	delete fAlphaBitmap;
	delete fDisplayBitmap;
	delete fBrush;
}

// layoutprefs
minimax
BrushView::layoutprefs()
{
	mpm.mini.x = SIZE + 4.0 + 1.0;
	mpm.maxi.x = 4.0 * SIZE + 4.0 + 1.0;
	mpm.mini.y = SIZE + 4.0 + 2.0;
	mpm.maxi.y = 4.0 * SIZE + 4.0 + 2.0;
	mpm.weight = 1.0;
	return mpm;
}

// layout
BRect
BrushView::layout(BRect frame)
{
	MoveTo(frame.LeftTop());
	float size = frame.Width() > frame.Height() ? frame.Height() : frame.Width();
	ResizeTo(size, size);
	return Frame();
}

// FrameResized
void
BrushView::FrameResized(float width, float height)
{
	delete fAlphaBitmap;
	delete fDisplayBitmap;
	// compensate frame
	width -= 4.0;
	height -= 4.0;
	fAlphaBitmap = new BBitmap(BRect(0.0, 0.0, width, height), B_GRAY8);
	fDisplayBitmap = new BBitmap(BRect(0.0, 0.0, width, height), B_RGB32);
	Update();
}


// Draw
void
BrushView::Draw(BRect updateRect)
{
	rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color shadow = tint_color(background, B_DARKEN_3_TINT);
	rgb_color lightShadow = tint_color(background, B_DARKEN_1_TINT);
	rgb_color light = tint_color(background, B_LIGHTEN_MAX_TINT);
	BRect r = Bounds();
	stroke_frame(this, r, lightShadow, lightShadow, light, light);
	r.InsetBy(1.0, 1.0);
	stroke_frame(this, r, shadow, shadow, background, background);
	r.InsetBy(1.0, 1.0);
	if (fEnabled) {
		DrawBitmap(fDisplayBitmap, BPoint(2.0, 2.0));
		if ( fRadiusCache != fBrush->Radius().max ) {
			float factor = fRadiusCache / fBrush->Radius().max;

			// inform the user that the brush is scaled down
			BPoint top = r.LeftTop() + BPoint(3.0, 2.0);
			BPoint bottom = r.LeftBottom() + BPoint(3.0, -2.0);
			float scaleHeight = bottom.y - (bottom.y - top.y) / factor;
			rgb_color rulerColor = tint_color(light, B_DARKEN_2_TINT);
			rgb_color fillColor = (rgb_color){ 255, 0, 0, 255 };
			BeginLineArray(5);
				AddLine(BPoint(top.x - 1.0, top.y),
						BPoint(top.x + 1.0, top.y), rulerColor);
				AddLine(BPoint(top.x, top.y + 1.0),
						BPoint(top.x, scaleHeight - 1.0), rulerColor);
				AddLine(BPoint(top.x - 1.0, scaleHeight),
						BPoint(top.x + 1.0, scaleHeight), fillColor);
				AddLine(BPoint(top.x, scaleHeight + 1.0),
						BPoint(bottom.x, bottom.y - 1.0), fillColor);
				AddLine(BPoint(bottom.x - 1.0, bottom.y),
						BPoint(bottom.x + 1.0, bottom.y), fillColor);
			EndLineArray();
/*			BString helper("x");
			helper << factor;
			font_height fh;
			GetFontHeight(&fh);
			BPoint textPoint;
			textPoint.x = r.right - StringWidth(helper.String()) - 1.0;
			textPoint.y = r.bottom - fh.descent;
			SetHighColor(255, 0, 0, 255);
			SetDrawingMode(B_OP_ALPHA);
			DrawString(helper.String(), textPoint);*/
		}
	} else {
		SetHighColor(light);
		FillRect(r);
		SetHighColor(0, 0, 0, 255);
		BPoint middle((r.left + r.right) / 2.0, (r.top + r.bottom) / 2.0);
		StrokeLine(middle, middle);
	}
}

// MouseDown
void
BrushView::MouseDown(BPoint where)
{
	if (fEnabled) {
		InvokeNotify(Message());
	}
}

// Update
void
BrushView::Update(const Brush* brush, uint32 flags)
{
	if (brush) {
		fRadiusCache = brush->Radius().max;
		fBrush->SetRadius(brush->Radius());
		fBrush->SetHardness(brush->Hardness());
	}
	if (brush || fFlags != flags) {
		fFlags = flags;
		Update();
	}
}

// Update
void
BrushView::Update()
{
	BPoint center;
	BRect r(fAlphaBitmap->Bounds());
	center.x = r.left + r.Width() / 2.0;
	center.y = r.top + r.Height() / 2.0;
	uint8* src = (uint8*)fAlphaBitmap->Bits();
	uint8* dst = (uint8*)fDisplayBitmap->Bits();
	uint32 bpr8 = fAlphaBitmap->BytesPerRow();
	uint32 bpr32 = fDisplayBitmap->BytesPerRow();
	// scale brush down to fit preview
	range radiusRange = { fRadiusCache, fRadiusCache };
	fBrush->SetRadius(radiusRange);
	float radius = fBrush->Radius().max;
	while (radius > center.x || radius > center.y)
		radius /= 2.0;
	radiusRange.min = radius;
	radiusRange.max = radius;
	range alphaRange = { 1.0, 1.0 };
	fBrush->SetRadius(radiusRange);
	// put brush into alpha bitmap
	memset(src, 0, fAlphaBitmap->BitsLength());
	fBrush->Draw(center, 1.0, alphaRange, fFlags, src, bpr8, r);
	// translate alpha bitmap into display bitmap
	uint32 width = r.IntegerWidth() + 1;
	uint32 height = r.IntegerHeight() + 1;
	for (uint32 y = 0; y < height; y++) {
		uint8* srcHandle = src;
		uint8* dstHandle = dst;
		for (uint32 x = 0; x < width; x++) {
			uint8 alpha = 255 - *srcHandle;
			dstHandle[0] = alpha;
			dstHandle[1] = alpha; 
			dstHandle[2] = alpha;
			// next pixel
			srcHandle += 1;
			dstHandle += 4;
		}
		src += bpr8;
		dst += bpr32;
	}
	Invalidate();
}

// SetEnabled
void
BrushView::SetEnabled(bool enable)
{
	if (enable != fEnabled) {
		fEnabled = enable;
		Invalidate();
	}
}

