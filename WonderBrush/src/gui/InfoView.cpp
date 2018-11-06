// InfoView.cpp

#include <stdio.h>
#include <math.h>

#include <Message.h>
#include <MessageRunner.h>
#include <Bitmap.h>
#include <Font.h>
#include <ScrollBar.h>
#include <Window.h>

#include "CanvasView.h"

#include "InfoView.h"

const bigtime_t TICK_PERIOD				= 40000;
const int32		BUSY_STRIPE_THICKNESS	= 10;

enum {
	MSG_BUSY_PULSE		= 'plsb',
	MSG_CURSOR_PULSE	= 'plsc',
};

// constructor
InfoView::InfoView(const char* name, border_style border)
	: BView(BRect(0.0, 0.0, 10.0, 10.0), name, B_FOLLOW_NONE,
			B_FULL_UPDATE_ON_RESIZE | B_WILL_DRAW),
	  fInfoText(""),
	  fBorderStyle(border),
	  fBusyPulse(NULL),
	  fBusyStep(-1),
	  fJobProgress(-1.0),
	  fCanvasView(NULL),
	  fCursorPulse(NULL),
	  fCursorIdle(0)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	BFont font(be_plain_font);
	font.SetSize(10.0);
	SetFont(&font);
}

// destructor
InfoView::~InfoView()
{
	delete fBusyPulse;
	delete fCursorPulse;
}

// Draw
void
InfoView::Draw(BRect updateRect)
{
	_DrawInto(this, updateRect);
}

// AttachedToWindow
void
InfoView::AttachedToWindow()
{
	if (fBusyPulse && fBusyPulse->InitCheck() != B_OK) {
		delete fBusyPulse;
		BMessage message(MSG_BUSY_PULSE);
		fBusyPulse = new BMessageRunner(BMessenger(this), &message, TICK_PERIOD);
	}
}

// MessageReceived
void
InfoView::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case MSG_BUSY_PULSE:
			_BusyTick();
			break;
		case MSG_CURSOR_PULSE:
			_CursorTick();
			break;
		default:
			BView::MessageReceived(msg);
	}
}

// layoutprefs
minimax
InfoView::layoutprefs()
{
	float borderWidth = 0.0;
	if (fBorderStyle == B_FANCY_BORDER)
		borderWidth = 2.0;
	if (fBorderStyle == B_PLAIN_BORDER)
		borderWidth = 1.0;
	
	mpm.mini.x = borderWidth + 10.0 + ceil(StringWidth("00000 x 00000, 0000%")) + 10.0 + borderWidth;
	mpm.maxi.x = 10000.0;
	mpm.mini.y = mpm.maxi.y = B_H_SCROLL_BAR_HEIGHT - 1.0;
	mpm.weight = 1.0;
	return mpm;
}

// layout
BRect
InfoView::layout(BRect rect)
{
	MoveTo(rect.LeftTop());
	ResizeTo(rect.Width(), rect.Height());
	return Frame();
}

// SetInfo
void
InfoView::SetInfo(const char *info)
{
	if (info && strcmp(info, fInfoText.String()) != 0) {
		fInfoText.SetTo(info);
		if (LockLooper()) {
			Invalidate();
			Window()->UpdateIfNeeded();
			UnlockLooper();
		}
	}
}

// SetCanvasView
void
InfoView::SetCanvasView(CanvasView* view)
{
	fCanvasView = view;
	delete fCursorPulse;
	fCursorPulse = NULL;
	if (fCanvasView) {
		BMessage message(MSG_CURSOR_PULSE);
		fCursorPulse = new BMessageRunner(BMessenger(this), &message, TICK_PERIOD);
	}
}

// JobStarted
void
InfoView::JobStarted()
{
	fJobProgress = 0.0;
	if (LockLooper()) {
		Invalidate();
		Window()->UpdateIfNeeded();
		UnlockLooper();
	}
}

// JobProgress
void
InfoView::JobProgress(float percent)
{
	// sanity check
	if (percent > 100.0)
		percent = 100.0;
	if (percent < 0.0)
		percent = 0.0;

	fJobProgress = percent;

	if (LockLooper()) {
		Invalidate();
		Window()->UpdateIfNeeded();
		UnlockLooper();
	}
}

// JobDone
void
InfoView::JobDone()
{
	fJobProgress = -1.0;
	if (LockLooper()) {
		Invalidate();
		Window()->UpdateIfNeeded();
		UnlockLooper();
	}
}

// IsIndicatingProgress
bool
InfoView::IsIndicatingProgress() const
{
	return fJobProgress >= 0.0;
}

// SetIndicateBusy
void
InfoView::SetIndicateBusy(bool busy)
{
	if (busy && !fBusyPulse) {
		BMessage message(MSG_BUSY_PULSE);
		fBusyPulse = new BMessageRunner(BMessenger(this), &message, TICK_PERIOD);
		fBusyStep = 0;
	} else {
		delete fBusyPulse;
		fBusyPulse = NULL;
	}
}

// IsIndicatingBusy
bool
InfoView::IsIndicatingBusy() const
{
	return fBusyPulse != NULL;
}

// _DrawInto
void 
InfoView::_DrawInto(BView *v, BRect updateRect)
{
	BRect b(Bounds()); // not v->Bounds()!
	rgb_color background = v->LowColor();
	rgb_color lightShadow = tint_color(background, B_DARKEN_1_TINT);
	rgb_color shadow = tint_color(background, B_DARKEN_2_TINT);
	rgb_color darkShadow = tint_color(background, B_DARKEN_4_TINT);
	rgb_color light = tint_color(background, B_LIGHTEN_MAX_TINT);

	switch (fBorderStyle) {
		case B_PLAIN_BORDER: {
			v->BeginLineArray(4);
				v->AddLine(BPoint(b.left, b.bottom), BPoint(b.left, b.top), light);
				v->AddLine(BPoint(b.left + 1.0, b.top), BPoint(b.right - 1.0, b.top), light);
				v->AddLine(BPoint(b.right, b.top), BPoint(b.right, b.bottom), shadow);
				v->AddLine(BPoint(b.right - 1.0, b.bottom), BPoint(b.left + 1.0, b.bottom), lightShadow);
			v->EndLineArray();
			b.InsetBy(1.0, 1.0);
			break;
		}
		case B_FANCY_BORDER: {
			v->BeginLineArray(8);
				v->AddLine(BPoint(b.left, b.bottom), BPoint(b.left, b.top), shadow);
				v->AddLine(BPoint(b.left + 1.0, b.top), BPoint(b.right, b.top), shadow);
				v->AddLine(BPoint(b.right, b.top + 1.0), BPoint(b.right, b.bottom), light);
				v->AddLine(BPoint(b.right - 1.0, b.bottom), BPoint(b.left + 1.0, b.bottom), light);
				b.InsetBy(1.0, 1.0);
				v->AddLine(BPoint(b.left, b.bottom), BPoint(b.left, b.top), darkShadow);
				v->AddLine(BPoint(b.left + 1.0, b.top), BPoint(b.right, b.top), darkShadow);
				v->AddLine(BPoint(b.right, b.top + 1.0), BPoint(b.right, b.bottom), lightShadow);
				v->AddLine(BPoint(b.right - 1.0, b.bottom), BPoint(b.left + 1.0, b.bottom), lightShadow);
			v->EndLineArray();
			b.InsetBy(1.0, 1.0);
			break;
		}
		case B_NO_BORDER:
		default:
			break;
	}
	v->FillRect(b, B_SOLID_LOW);

	if (fJobProgress >= 0.0 || fBusyStep >= 0) {
		// draw progress bar
		b = _ProgressBarFrame();
		v->BeginLineArray(8);
			v->AddLine(BPoint(b.left, b.bottom), BPoint(b.left, b.top), lightShadow);
			v->AddLine(BPoint(b.left + 1.0, b.top), BPoint(b.right, b.top), lightShadow);
			v->AddLine(BPoint(b.right, b.top + 1.0), BPoint(b.right, b.bottom), light);
			v->AddLine(BPoint(b.right - 1.0, b.bottom), BPoint(b.left + 1.0, b.bottom), light);
			b.InsetBy(1.0, 1.0);
			v->AddLine(BPoint(b.left, b.bottom), BPoint(b.left, b.top), darkShadow);
			v->AddLine(BPoint(b.left + 1.0, b.top), BPoint(b.right, b.top), darkShadow);
			v->AddLine(BPoint(b.right, b.top + 1.0), BPoint(b.right, b.bottom), background);
			v->AddLine(BPoint(b.right - 1.0, b.bottom), BPoint(b.left + 1.0, b.bottom), background);
		v->EndLineArray();
		// white background
		b.InsetBy(1.0, 1.0);
		v->SetLowColor(255, 255, 255, 255);
		v->FillRect(b, B_SOLID_LOW);
		rgb_color barColor = (rgb_color){ 50, 150, 255, 255 };
		if (fJobProgress > 0.0) {
			// draw progress bar (if progress more than 0 %)
			b.right = b.left + b.Width() * (fJobProgress / 100.0);
			rgb_color barLight = tint_color(barColor, B_LIGHTEN_1_TINT);
			rgb_color barShadow = tint_color(barColor, B_DARKEN_3_TINT);
			v->BeginLineArray(4);
				v->AddLine(BPoint(b.left, b.bottom - 1.0), BPoint(b.left, b.top), barLight);
				v->AddLine(BPoint(b.left + 1.0, b.top), BPoint(b.right - 1.0, b.top), barLight);
				v->AddLine(BPoint(b.right, b.top), BPoint(b.right, b.bottom), barShadow);
				v->AddLine(BPoint(b.right - 1.0, b.bottom), BPoint(b.left, b.bottom), barShadow);
			v->EndLineArray();
			b.InsetBy(1.0, 1.0);
			v->SetHighColor(barColor);
			v->FillRect(b, B_SOLID_HIGH);
		} else if (fBusyStep >= 0) {
			// draw busy indication
			v->SetHighColor(barColor);
			int32 line = 0;
			for (float y = b.top; y <= b.bottom; y++) {
				float x = b.left + (float)fBusyStep - line++;
				for (int32 stripe = -1; stripe * BUSY_STRIPE_THICKNESS <= b.Width() + 1; stripe++) {
					// x coordinates for stripe
					float x1 = x + stripe * BUSY_STRIPE_THICKNESS * 2;
					float x2 = x + stripe * BUSY_STRIPE_THICKNESS * 2
							   + BUSY_STRIPE_THICKNESS - 1;
					// check bounds
					if (x1 < b.left) {
						x1 = b.left;
						if (x2 < x1)
							continue;
					}
					if (x2 > b.right) {
						x2 = b.right;
						if (x1 > x2 + 1.0)
							break;
					}
					// draw blue line
					v->StrokeLine(BPoint(x1, y), BPoint(x2, y), B_SOLID_HIGH);
				}
			}
		}
	} else {
		font_height fh;
		v->GetFontHeight(&fh);
		const char* text = fInfoText.String();
		BString helper("");
		if (fCursorIdle < 50) {
			// draw cursor position
			helper << fCursorPos.x << ", " << fCursorPos.y;
			text = helper.String();
		}
		// draw text
		float width = v->StringWidth(text);
		BPoint p(((b.left + b.right) - width) / 2.0, b.bottom - fh.descent + 1.0);
		v->SetHighColor(0, 0, 0, 255);
		v->DrawString(text, p);
	}
}

// _BusyTick
void
InfoView::_BusyTick()
{
	fBusyStep++;
	if (fBusyStep >= BUSY_STRIPE_THICKNESS * 2)
		fBusyStep = 0;
	Invalidate();
}

// _CursorTick
void
InfoView::_CursorTick()
{
	if (fCanvasView && !fBusyPulse && fJobProgress < 0.0) {
		BPoint cursorPos;
		if (fCanvasView->GetCursorPos(&cursorPos)) {
			if (cursorPos != fCursorPos) {
				fCursorIdle = 0;
				fCursorPos = cursorPos;
				Invalidate();
			} else {
				fCursorIdle++;
				if (fCursorIdle == 50)
					Invalidate();
			}
		} else {
			if (fCursorIdle < 50) {
				fCursorIdle = 50;
				Invalidate();
			}
		}
	}
}

// _ProgressBarFrame
BRect
InfoView::_ProgressBarFrame() const
{
	BRect frame(Bounds());
	frame.InsetBy(2.0, 2.0);
	return frame;
}


