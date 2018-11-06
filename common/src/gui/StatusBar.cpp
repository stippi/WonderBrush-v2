// StatusBar.cpp

#include <math.h>
#include <new>
#include <stdio.h>

#include <Message.h>
#include <MessageRunner.h>
#include <Bitmap.h>
#include <Font.h>
#include <ScrollBar.h>
#include <Window.h>

#include "StatusBar.h"

const bigtime_t TICK_PERIOD				= 40000;
const bigtime_t TEXT_DECAY_DELAY		= 10 * 1000 * 1000;
const bigtime_t TEXT_DECAY_TIME			= 3 * 1000 * 1000;
const int32		BUSY_STRIPE_THICKNESS	= 10;

enum {
	MSG_FADE_PULSE		= 'plsf',
	MSG_BUSY_PULSE		= 'plsb',
};

// constructor
StatusBar::StatusBar(const char *defaultMessage,
					 bool useDoubleBuffering,
					 border_style border,
					 bool fixedHeight,
					 BScrollBar *scrollBar)
	: BView(BRect(0.0, 0.0, 10.0, 10.0), "status bar", B_FOLLOW_NONE,
				 B_FRAME_EVENTS | B_WILL_DRAW),
	  fDefaultMessage(defaultMessage),
	  fFullMessage(""),
	  fTruncatedMessage(""),
	  fTextColor((rgb_color){ 0, 0, 0, 255 }),
	  fBaseColor(ui_color(B_PANEL_BACKGROUND_COLOR)),
	  fFadedColor(fBaseColor),
	  fBorderStyle(border),
	  fFixedHeight(fixedHeight),
	  fScrollBar(scrollBar),
	  fOpacity(0.0),
	  fFadePulse(NULL),
	  fBusyPulse(NULL),
	  fBusyStep(-1),
	  fDragging(false),
	  fHadJobMessage(false),
	  fJobProgress(-1.0),
	  fBackBitmap(NULL),
	  fBackView(NULL),
	  fDirty(true)
{
	SetViewColor(B_TRANSPARENT_32_BIT);

	if (useDoubleBuffering)
		_AllocBackBitmap(Bounds().Width(), Bounds().Height());
}

// destructor
StatusBar::~StatusBar()
{
	delete fFadePulse;
	delete fBusyPulse;
	_FreeBackBitmap();
}

// Draw
void
StatusBar::Draw(BRect updateRect)
{
	if (!fBackView) {
		SetLowColor(fFadedColor);
		_DrawInto(this, updateRect);
	} else {
		if (fDirty) {
			if (fBackBitmap->Lock()) {
				fBackView->SetLowColor(fFadedColor);
				_DrawInto(fBackView, updateRect);
				fBackView->Sync();
				fBackBitmap->Unlock();
			}
			fDirty = false;
		}
		SetDrawingMode(B_OP_COPY);
		DrawBitmap(fBackBitmap, updateRect, updateRect);
	}
}

// AttachedToWindow
void
StatusBar::AttachedToWindow()
{
	BFont font;
	GetFont(&font);
	if (font.Size() > 10.0)
		font.SetSize(10.0);
	SetFont(&font);
	// restart message runners if they were created before we have been attached
	if (fFadePulse && fFadePulse->InitCheck() != B_OK) {
		delete fFadePulse;
		BMessage message(MSG_FADE_PULSE);
		fFadePulse = new BMessageRunner(BMessenger(this), &message, TICK_PERIOD);
	}
	if (fBusyPulse && fBusyPulse->InitCheck() != B_OK) {
		delete fBusyPulse;
		BMessage message(MSG_BUSY_PULSE);
		fBusyPulse = new BMessageRunner(BMessenger(this), &message, TICK_PERIOD);
	}
}

// FrameResized
void
StatusBar::FrameResized(float width, float height)
{
	if (fBackView)
		_AllocBackBitmap(width, height);

	// re-truncate the string if necessary
	fTruncatedMessage = fFullMessage;
	_TruncateString(&fTruncatedMessage, B_TRUNCATE_END,
					_WidthForMessage());

	fTruncatedDefaultMessage = fDefaultMessage;
	_TruncateString(&fTruncatedDefaultMessage, B_TRUNCATE_END,
					_WidthForMessage());
/*
	float minWidth, maxWidth, minHeight, maxHeight;
	Window()->GetSizeLimits(&minWidth, &maxWidth, &minHeight, &maxHeight);
	minWidth = width + 6 * B_V_SCROLL_BAR_WIDTH;
	Window()->SetSizeLimits(minWidth, maxWidth, minHeight, maxHeight);
*/
	fDirty = true;
	Invalidate();
}

// MessageReceived
void
StatusBar::MessageReceived(BMessage *msg)
{
	switch (msg->what) {
		case MSG_FADE_PULSE:
			_FadeTick();
			break;
		case MSG_BUSY_PULSE:
			_BusyTick();
			break;
		default:
			BView::MessageReceived(msg);
	}
}

// MouseDown
void
StatusBar::MouseDown(BPoint point)
{
	if (fScrollBar) {
		// drag rect
		BRect dragRect(Bounds());
		dragRect.left = dragRect.right - 10.0;
		if (dragRect.Contains(point)) {
			// resize
			fDragging = true;
			SetMouseEventMask(B_POINTER_EVENTS,
							  B_LOCK_WINDOW_FOCUS | B_NO_POINTER_HISTORY);
		}
	}
}

// MouseUp
void
StatusBar::MouseUp(BPoint point)
{
	fDragging = false;
}

// MouseMoved
void
StatusBar::MouseMoved(BPoint point, uint32 transit, const BMessage *msg)
{
	if (fDragging) {
		float x = point.x - (Bounds().right - 5.0);
		if ((Bounds().Width() + x) <= 16.0) {
			return;
		}
		if (fScrollBar
		 && ((fScrollBar->Bounds().Width() - x) <= (6 * B_V_SCROLL_BAR_WIDTH))) {
			return;
		}
		ResizeBy(x, 0.0);
		BRect r(Bounds());
		r.left = r.right - 10.0;
		fDirty = true;
		Invalidate(r);
		if (fScrollBar) {
			fScrollBar->ResizeBy(-x, 0.0);
			fScrollBar->MoveBy(x, 0.0);
		}
	}
}

// layoutprefs
minimax
StatusBar::layoutprefs()
{
	font_height fh;
	GetFontHeight(&fh);
	float labelHeight = ceil(fh.ascent + fh.descent);
	float borderWidth = 0.0;
	if (fScrollBar || fBorderStyle == B_FANCY_BORDER)
		borderWidth = 2.0;
	if (fBorderStyle == B_PLAIN_BORDER)
		borderWidth = 1.0;
	
//	mpm.mini.x = borderWidth + 10.0 + ceil(StringWidth(fDefaultMessage.String())) + 10.0 + borderWidth;
	mpm.mini.x = borderWidth + 10.0 + 25.0 + 10.0 + borderWidth;
	mpm.maxi.x = 10000.0;
	mpm.mini.y = borderWidth + 2.0 + labelHeight + 2.0 + borderWidth;
	if (fFixedHeight)
		mpm.maxi.y = mpm.mini.y;
	else
		mpm.maxi.y = 10000.0;
	mpm.weight = 0.01;
	return mpm;
}

// layout
BRect
StatusBar::layout(BRect rect)
{
	MoveTo(rect.LeftTop());
	ResizeTo(rect.Width(), rect.Height());
	return Frame();
}

// reloadfont
void
StatusBar::reloadfont(BFont* font[])
{
}

// SetStatus
void
StatusBar::SetStatus(const char *message, status_type type)
{
	if (LockLooper()) {
		fFullMessage.SetTo(message);
		fTruncatedMessage.SetTo(message);
		_TruncateString(&fTruncatedMessage, B_TRUNCATE_END,
						_WidthForMessage());
		switch (type) {
			case STATUS_WARNING:
				fBaseColor = (rgb_color){ 255, 217, 121, 255 };
				break;
			case STATUS_ERROR:
				fBaseColor = (rgb_color){ 255, 120, 97, 255 };
				break;
			case STATUS_INFO:
			default:
				fBaseColor = ui_color(B_PANEL_BACKGROUND_COLOR);
				break;
		}
		fFadedColor = fBaseColor;
	
		fDirty = true;
		_StartFade();
		Invalidate();
		Window()->UpdateIfNeeded();
		UnlockLooper();
	}
}

// SetDefaultMessage
void
StatusBar::SetDefaultMessage(const char* message)
{
	fDefaultMessage.SetTo(message);
	fTruncatedDefaultMessage.SetTo(message);
	_TruncateString(&fTruncatedDefaultMessage, B_TRUNCATE_END,
					_WidthForMessage());
	fDirty = true;
	Invalidate();
}

// SetTextColor
void
StatusBar::SetTextColor(const rgb_color& color)
{
	fTextColor = color;
	fDirty = true;
	Invalidate();
}

// JobStarted
void
StatusBar::JobStarted(const char* message)
{
	if (LockLooper()) {
		// set message
		if (message) {
			fHadJobMessage = true;
			fFullMessage.SetTo(message);
			fOpacity = 1.0;
			fBaseColor = fFadedColor = ui_color(B_PANEL_BACKGROUND_COLOR);
		}
		fJobProgress = 0.0;
		SetIndicateBusy(false);
		// message might need to get tuncated
		fTruncatedMessage.SetTo(fFullMessage.String());
		_TruncateString(&fTruncatedMessage, B_TRUNCATE_END,
						_WidthForMessage());
		fDirty = true;
		Invalidate();
		Window()->UpdateIfNeeded();
		UnlockLooper();
	}
}

// JobProgress
void
StatusBar::JobProgress(float percent, const char* message)
{
	if (LockLooper()) {
		if (message) {
			fHadJobMessage = true;
			fFullMessage.SetTo(message);
			fTruncatedMessage.SetTo(message);
			_TruncateString(&fTruncatedMessage, B_TRUNCATE_END,
							_WidthForMessage());
			fOpacity = 1.0;
			fBaseColor = fFadedColor = ui_color(B_PANEL_BACKGROUND_COLOR);
		}
		// sanity check
		if (percent > 100.0)
			percent = 100.0;
		if (percent < 0.0)
			percent = 0.0;
		
		fJobProgress = percent;
		fDirty = true;
		Invalidate();
		Window()->UpdateIfNeeded();
		UnlockLooper();
	}
}

// JobDone
void
StatusBar::JobDone()
{
	if (LockLooper()) {
		fJobProgress = -1.0;
		SetIndicateBusy(false);
		// returncate message
		fTruncatedMessage.SetTo(fFullMessage.String());
		_TruncateString(&fTruncatedMessage, B_TRUNCATE_END,
						Bounds().Width() - 25.0);
		// return to default message
		fDirty = true;
		if (fHadJobMessage)
			_StartFade();
		Invalidate();
		Window()->UpdateIfNeeded();
		UnlockLooper();
	}
}

// IsIndicatingProgress
bool
StatusBar::IsIndicatingProgress() const
{
	return fJobProgress >= 0.0;
}

// SetIndicateBusy
void
StatusBar::SetIndicateBusy(bool busy)
{
	if (busy) {
		if (!fBusyPulse && fJobProgress < 0.0) {
			BMessage message(MSG_BUSY_PULSE);
			fBusyPulse = new BMessageRunner(BMessenger(this), &message, TICK_PERIOD);
			fBusyStep = 0;
		}
	} else {
		fBusyStep = -1;
		delete fBusyPulse;
		fBusyPulse = NULL;
		fDirty = true;
		Invalidate();
	}
}

// IsIndicatingBusy
bool
StatusBar::IsIndicatingBusy() const
{
	return fBusyPulse != NULL;
}

// _DrawInto
void 
StatusBar::_DrawInto(BView *v, BRect updateRect)
{
	BRect b(Bounds()); // not v->Bounds()!
	rgb_color background = v->LowColor();
	rgb_color lightShadow = tint_color(background, B_DARKEN_1_TINT);
	rgb_color shadow = tint_color(background, B_DARKEN_2_TINT);
	rgb_color darkShadow = tint_color(background, B_DARKEN_4_TINT);
	rgb_color light = tint_color(background, B_LIGHTEN_MAX_TINT);

	if (fScrollBar) {
		// draw border (minus right edge, which the scrollbar draws)
		v->BeginLineArray(7);
			v->AddLine(BPoint(b.right, b.bottom), BPoint(b.left, b.bottom), shadow);
			v->AddLine(BPoint(b.left, b.bottom - 1.0), BPoint(b.left, b.top), shadow);
			v->AddLine(BPoint(b.left + 1.0, b.top), BPoint(b.right, b.top), shadow);
			v->AddLine(BPoint(b.left, b.bottom), BPoint(b.left, b.top), light);
			v->AddLine(BPoint(b.left + 1.0, b.top), BPoint(b.right, b.top), light);
			v->AddLine(BPoint(b.right, b.top + 1.0), BPoint(b.right, b.bottom), lightShadow);
			v->AddLine(BPoint(b.right - 1.0, b.bottom), BPoint(b.left + 1.0, b.bottom), lightShadow);
		v->EndLineArray();
	} else {
		switch (fBorderStyle) {
			case B_PLAIN_BORDER: {
				v->BeginLineArray(4);
					v->AddLine(BPoint(b.left, b.bottom), BPoint(b.left, b.top), light);
					v->AddLine(BPoint(b.left + 1.0, b.top), BPoint(b.right, b.top), light);
					v->AddLine(BPoint(b.right, b.top + 1.0), BPoint(b.right, b.bottom), shadow);
					v->AddLine(BPoint(b.right - 1.0, b.bottom), BPoint(b.left + 1.0, b.bottom), shadow);
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
			default: {
			}
		}
	}
	v->FillRect(b, B_SOLID_LOW);

	// draw text
	font_height fh;
	v->GetFontHeight(&fh);
	BPoint p(b.left + 10.0, (b.top + b.bottom) / 2.0 + fh.descent + 1.0);
	rgb_color tc = fTextColor;
	if (fOpacity == 1.0) {
		v->SetHighColor(tc.red, tc.green, tc.blue, 255);
		v->DrawString(fTruncatedMessage.String(), p);
	} else if (fOpacity == 0.0 && fTruncatedDefaultMessage.CountChars() > 0) {
		v->SetHighColor(tc.red, tc.green, tc.blue, 255);
		v->DrawString(fTruncatedDefaultMessage.String(), p);
	} else {
		v->SetDrawingMode(B_OP_ALPHA);
		v->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);
		if (fTruncatedDefaultMessage.CountChars() > 0) {
			v->SetHighColor(tc.red, tc.green, tc.blue,
				uint8(255 * (1.0 - fOpacity)));
			v->DrawString(fTruncatedDefaultMessage.String(), p);
		}
		v->SetHighColor(tc.red, tc.green, tc.blue, uint8(255 * fOpacity));
		v->DrawString(fTruncatedMessage.String(), p);
		v->SetDrawingMode(B_OP_COPY);
	}

	if (fScrollBar) {
		// draw resize dragger
		v->SetDrawingMode(B_OP_OVER);
		b = Bounds();
		b.right -= 2.0;
		b.left = b.right - 2.0;
		b.InsetBy(0.0, 3.0);
		b.top += 1.0;
		for (int32 i = 0; i < b.IntegerHeight(); i += 3) {
			BPoint p = b.LeftTop() + BPoint(0.0, i);
			v->SetHighColor(shadow);
			v->StrokeLine(p, p, B_SOLID_HIGH);
			p += BPoint(1.0, 1.0);
			v->SetHighColor(light);
			v->StrokeLine(p, p, B_SOLID_HIGH);
		}
	}

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
	}
}

// _StartFade
void
StatusBar::_StartFade()
{
	fOpacity = 1.0;
	fDecayDelay = TEXT_DECAY_DELAY;

	delete fFadePulse;
	BMessage message(MSG_FADE_PULSE);
	fFadePulse = new BMessageRunner(BMessenger(this), &message, TICK_PERIOD);
}

// _FadeTick
void
StatusBar::_FadeTick()
{
	if (fOpacity > 0.0) {
		if (fDecayDelay > 0) {
			fDecayDelay -= TICK_PERIOD;
			return;
		}
		float steps = (float)TEXT_DECAY_TIME / (float)TICK_PERIOD;
		fOpacity -= (1.0 / steps);
		if (fOpacity < 0.001)
			fOpacity = 0.0;
		// calculate base color
		rgb_color normal = ui_color(B_PANEL_BACKGROUND_COLOR);
		fFadedColor.red = (uint8)(fOpacity * fBaseColor.red + (1.0 - fOpacity) * normal.red);
		fFadedColor.green = (uint8)(fOpacity * fBaseColor.green + (1.0 - fOpacity) * normal.green);
		fFadedColor.blue = (uint8)(fOpacity * fBaseColor.blue + (1.0 - fOpacity) * normal.blue);
		fFadedColor.alpha = 255;
		fDirty = true;
		Invalidate();
	} else if (fFadePulse) {
		fBaseColor = fFadedColor = ui_color(B_PANEL_BACKGROUND_COLOR);
		delete fFadePulse;
		fFadePulse = NULL;
	}
}

// _BusyTick
void
StatusBar::_BusyTick()
{
	if (fBusyPulse) {
		fBusyStep++;
		if (fBusyStep >= BUSY_STRIPE_THICKNESS * 2)
			fBusyStep = 0;
		fDirty = true;
		Invalidate();
	}
}

// _AllocBackBitmap
void
StatusBar::_AllocBackBitmap(float width, float height)
{
	// sanity check
	if (width <= 0.0 || height <= 0.0)
		return;

	if (fBackBitmap) {
		// see if the bitmap needs to be expanded
		BRect b = fBackBitmap->Bounds();
		if (b.Width() >= width && b.Height() >= height)
			return;

		// it does; clean up:
		_FreeBackBitmap();
	}

	BRect b(0.0, 0.0, width, height);
	fBackBitmap = new(nothrow) BBitmap(b, B_RGB32, true);
	if (!fBackBitmap) {
		fprintf(stderr, "StatusBar::_AllocBackBitmap(): failed to allocate\n");
		return;
	}
	if (fBackBitmap->IsValid()) {
		fBackView = new BView(b, 0, B_FOLLOW_NONE, B_WILL_DRAW);
		BFont font;
		GetFont(&font);
		fBackView->SetFont(&font);
		fBackBitmap->AddChild(fBackView);
		fDirty = true;
	} else {
		_FreeBackBitmap();
		fprintf(stderr, "StatusBar::_AllocBackBitmap(): bitmap invalid\n");
	}
}

// _FreeBackBitmap
void
StatusBar::_FreeBackBitmap()
{
	if (fBackBitmap) {
		delete fBackBitmap;
		fBackBitmap = NULL;
		fBackView = NULL;
	}
}

// _ProgressBarFrame
BRect
StatusBar::_ProgressBarFrame() const
{
	BRect frame(Bounds());
	frame.left = frame.right - frame.Width() / 3.0;
	frame.InsetBy(2.0, 2.0);
	font_height fh;
	GetFontHeight(&fh);
	float maxHeight = fh.ascent + fh.descent;
	if (frame.Height() > maxHeight)
		frame.InsetBy(0.0, floorf((frame.Height() - maxHeight) / 2.0));
	if (fScrollBar)
		frame.OffsetBy(-5.0, 0.0);
	return frame;
}

// _WidthForMessage
float
StatusBar::_WidthForMessage() const
{
	float width = Bounds().Width() - 25.0;
	if (fJobProgress >= 0.0)
		width -= _ProgressBarFrame().Width();
	return width;
}

// _TruncateString
void
StatusBar::_TruncateString(BString* inOutString,
						   uint32 mode, float maxWidth)
{
	BFont font;
	GetFont(&font);
	font.TruncateString(inOutString, mode, maxWidth);
}

