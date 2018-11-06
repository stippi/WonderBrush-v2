// DualSlider.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <Looper.h>
#include <Message.h>
#include <Window.h>

#include "support.h"

#include "BubbleHelper.h"

#include "DualSlider.h"

enum {
	MIN_ENABLED		= 0x01,
	MAX_ENABLED		= 0x02,
	DRAGGING_MIN	= 0x04,
	DRAGGING_MAX	= 0x08,
	PRESSURE_INSIDE	= 0x10,
	TILT_INSIDE		= 0x20,
	DISABLED		= 0x40,
};

#define LABEL_SPACING 2.0

// constructor
DualSlider::DualSlider(const char* name, const char* label,
					   BMessage* valueMessage,
					   BMessage* controlMessage, BHandler* target,
					   float minValue, float maxValue)
	: BView(BRect(0.0, 0.0, 23.0, 17.0), name,
			B_FOLLOW_NONE, B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	  fMinValue(minValue),
	  fMaxValue(maxValue),
	  fLastFactor(0.0),
	  fFlags(MIN_ENABLED | MAX_ENABLED),
	  fValueMessage(valueMessage),
	  fControlMessage(controlMessage),
	  fTarget(target),
	  fLabel(label),
	  fPressureControlTip("Enables control by pen pressure.")
{
	SetViewColor(B_TRANSPARENT_32_BIT);
	SetLowColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	SetHighColor(0, 0, 0, 255);
}

// destructor
DualSlider::~DualSlider()
{
	delete fValueMessage;
}

// layoutprefs
minimax
DualSlider::layoutprefs()
{
	mpm.mini.x = 0.0;
	mpm.maxi.x = 10000.0;
	mpm.mini.y = 18.0;
	if (fLabel.CountChars() > 0) {
		font_height fh;
		GetFontHeight(&fh);
		mpm.mini.y += ceilf(fh.ascent + fh.descent) + LABEL_SPACING;
		mpm.mini.x = StringWidth(fLabel.String());
		if (fControlMessage) {
			mpm.mini.x += 20.0;
		}
	}
	if (mpm.mini.x == 0.0)
		mpm.mini.x = 60.0;
	mpm.maxi.y = mpm.mini.y;
	mpm.weight = 1.0;
	return mpm;
}

// layout
BRect
DualSlider::layout(BRect frame)
{
	MoveTo(frame.LeftTop());
	ResizeTo(frame.Width(), frame.Height());
	return Frame();
}

// Draw
void
DualSlider::Draw(BRect updateRect)
{
	rgb_color background;
	rgb_color shadow;
	rgb_color light;
	rgb_color blue;
	rgb_color softLight;
	rgb_color darkShadow;
	rgb_color darkestShadow;
	rgb_color softBlack;
	rgb_color lightShadow;
	rgb_color black;
	if (IsEnabled()) {
		background = ui_color(B_PANEL_BACKGROUND_COLOR);
		shadow = tint_color(background, B_DARKEN_2_TINT);
		light = tint_color(background, B_LIGHTEN_MAX_TINT);
		blue = ui_color(B_KEYBOARD_NAVIGATION_COLOR);
		softLight = tint_color(background, B_LIGHTEN_2_TINT);
		darkShadow = tint_color(background, B_DARKEN_3_TINT);
		darkestShadow = tint_color(background, 1.7);
		softBlack = tint_color(background, B_DARKEN_4_TINT);
		lightShadow = tint_color(background, B_DARKEN_1_TINT);
		black = tint_color(background, B_DARKEN_MAX_TINT);
	} else {
		background = ui_color(B_PANEL_BACKGROUND_COLOR);
		shadow = tint_color(background, B_DARKEN_1_TINT);
		light = tint_color(background, B_LIGHTEN_2_TINT);
		blue = tint_color(ui_color(B_KEYBOARD_NAVIGATION_COLOR),
						  B_LIGHTEN_2_TINT);
		softLight = background;//tint_color(background, B_LIGHTEN_2_TINT);
		darkShadow = tint_color(background, B_DARKEN_1_TINT);
		softBlack = tint_color(background, B_DARKEN_2_TINT);
		darkestShadow = softBlack;
		lightShadow = background;//tint_color(background, B_DARKEN_1_TINT);
		black = tint_color(background, B_DARKEN_3_TINT);
	}
	BRect r(Bounds());
	// label
	float labelHeight = _LabelHeight();
	if (labelHeight > 0.0) {
		r.bottom = r.top + labelHeight;
		FillRect(r, B_SOLID_LOW);
//		r.left += 5.0;
		font_height fh;
		GetFontHeight(&fh);
		SetHighColor(black);
		DrawString(fLabel.String(), BPoint(r.left,
										   r.top + fh.ascent + 1.0));
	}
	// pressure check mark
	r = _PressureBoxFrame();
	if (updateRect.Intersects(r) && fControlMessage) {
		BPoint start(r.left + 3.0, r.top + 3.0);
		// frame & background
		_StrokeRect(r, lightShadow, light);
		r.InsetBy(1.0, 1.0);
		_StrokeRect(r, softBlack, background);
		r.InsetBy(1.0, 1.0);
		SetHighColor(light);
		FillRect(r);
		// arrow
		if (IsMinEnabled() || fFlags & PRESSURE_INSIDE) {
			if (!IsMinEnabled())
				blue = tint_color(blue, B_LIGHTEN_2_TINT);
			SetHighColor(blue);
			FillRect(BRect(start.x + 2.0, start.y,
						   start.x + 4.0, start.y + 2.0));
	
			BPoint arrow[3];
			arrow[0].x = start.x;
			arrow[0].y = start.y + 3.0;
			arrow[1].x = start.x + 6.0;
			arrow[1].y = start.y + 3.0;
			arrow[2].x = start.x + 3.0;
			arrow[2].y = start.y + 6.0;
			FillPolygon(arrow, 3);
		}
/*		if (IsMinEnabled()) {
			_StrokeRect(r, shadow, light);
			r.InsetBy(1.0, 1.0);
			_StrokeRect(r, softBlack, background);
			r.left++;
			r.top++;
			SetHighColor(lightShadow);
			FillRect(r);
			// offset starting point of arrow
			start.x++;
			start.y++;
		} else if (fFlags & PRESSURE_INSIDE) {
			_StrokeRect(r, shadow, softBlack);
			r.InsetBy(1.0, 1.0);
			_StrokeRect(r, light, lightShadow);
			r.InsetBy(1.0, 1.0);
			SetHighColor(softBackground);
			FillRect(r);
		}
		// arrow
		SetHighColor(black);
		FillRect(BRect(start.x + 2.0, start.y,
					   start.x + 4.0, start.y + 2.0));

		BPoint arrow[3];
		arrow[0].x = start.x;
		arrow[0].y = start.y + 3.0;
		arrow[1].x = start.x + 6.0;
		arrow[1].y = start.y + 3.0;
		arrow[2].x = start.x + 3.0;
		arrow[2].y = start.y + 6.0;
		FillPolygon(arrow, 3);*/
	}
	// slider background
	r = _SliderFrame();
	BRect barFrame = _BarFrame();
	BRect top(r.left, r.top, r.right, barFrame.top - 1.0);
	BRect left(r.left, barFrame.top,
			   barFrame.left - 1.0, barFrame.bottom);
	BRect right(barFrame.right + 1.0, barFrame.top,
				r.right, barFrame.bottom);
	BRect bottom(r.left, barFrame.bottom + 1.0,
				 r.right, r.bottom);
	FillRect(top, B_SOLID_LOW);
	FillRect(left, B_SOLID_LOW);
	FillRect(right, B_SOLID_LOW);
	FillRect(bottom, B_SOLID_LOW);
	// slider bar
	// frame
	_StrokeRect(barFrame, shadow, light);
	barFrame.InsetBy(1.0, 1.0);
	_StrokeRect(barFrame, black, shadow);
	barFrame.left++;
	barFrame.top++;
	// inside
	rgb_color leftBarShadow;
	if (fMinValue == 0.0 || fMaxValue == 0.0)
		leftBarShadow = softBlack;
	else
		leftBarShadow = darkShadow;
	SetHighColor(leftBarShadow);
	StrokeLine(barFrame.LeftTop(), barFrame.LeftBottom());
	barFrame.left++;
	float minPos = floorf(barFrame.left - 1.0
						  + (barFrame.Width() + 1.0) * fMinValue
						  + 0.5);
	float maxPos = floorf(barFrame.left - 1.0
						  + (barFrame.Width() + 1.0) * fMaxValue
						  + 0.5);
	float lines = barFrame.Height() + 1.0;
	float darkBegin = minPos < maxPos ? minPos : maxPos;
	float darkEnd = minPos < maxPos ? maxPos : minPos;
	for (float y = 0.0; y < lines; y++) {
		float top = barFrame.top + y;
		rgb_color barNormal;
		rgb_color barRange;
		if (y == 0.0) {
			barNormal = darkShadow;
			barRange = IsMinEnabled() ? darkestShadow : barNormal;
		} else {
			barNormal = shadow;
			barRange = IsMinEnabled() ? softBlack : barNormal;
		}
		if (fMinValue > 0.0 && fMaxValue > 0.0) {
			SetHighColor(barNormal);
			StrokeLine(BPoint(barFrame.left, top),
					   BPoint(darkBegin - 1.0, top));
		}
		SetHighColor(barRange);
		StrokeLine(BPoint(darkBegin, top),
				   BPoint(darkEnd, top));
		if (fMinValue < 1.0 && fMaxValue < 1.0) {
			SetHighColor(barNormal);
			StrokeLine(BPoint(darkEnd + 1.0, top),
					   BPoint(barFrame.right, top));
		}
	}
	// arrows
	BRect a(minPos - 5.0, barFrame.top - 5.0,
			minPos + 5.0, barFrame.top);
	BPoint arrow[3];
	// min (little) arrow
	if (IsMinEnabled()) {
		BeginLineArray(7);
			AddLine(BPoint(a.left, a.top - 1.0),
					BPoint(a.right, a.top - 1.0), darkShadow);
			AddLine(BPoint(a.left + 1.0, a.top),
					BPoint(a.right - 2.0, a.top), softLight);
			AddLine(BPoint(a.right, a.top),
					BPoint(minPos, a.bottom), black);
			AddLine(BPoint(minPos - 1.0, a.bottom - 1.0),
					BPoint(a.left, a.top), black);
			AddLine(BPoint(a.left + 3.0, a.top + 1.0),
					BPoint(a.right - 3.0, a.top + 1.0), lightShadow);
			AddLine(BPoint(a.right - 1.0, a.top),
					BPoint(minPos, a.bottom - 1.0), shadow);
			AddLine(BPoint(minPos - 1.0, a.bottom - 2.0),
					BPoint(a.left + 2.0, a.top + 1.0), background);
		EndLineArray();
		// filling
		SetHighColor(lightShadow);
		arrow[0].x = a.left + 4.0;
		arrow[0].y = a.top + 2.0;
		arrow[1].x = a.right - 4.0;
		arrow[1].y = a.top + 2.0;
		arrow[2].x = minPos;
		arrow[2].y = a.bottom - 2.0;
		FillPolygon(arrow, 3);
	}
	// max (big) arrow
	a.Set(maxPos - 6.0, barFrame.bottom - 1.0,
		  maxPos + 6.0, barFrame.bottom + 5.0);
	BeginLineArray(7);
		AddLine(BPoint(a.left + 1.0, a.bottom),
				BPoint(a.right - 1.0, a.bottom), shadow);
		AddLine(BPoint(a.right, a.bottom),
				BPoint(maxPos, a.top), black);
		AddLine(BPoint(maxPos - 1.0, a.top + 1.0),
				BPoint(a.left, a.bottom), darkShadow);
		AddLine(BPoint(a.left + 3.0, a.bottom - 1.0),
				BPoint(a.right - 3.0, a.bottom - 1.0), background);
		AddLine(BPoint(a.right - 2.0, a.bottom - 1.0),
				BPoint(maxPos, a.top + 1.0), lightShadow);
		AddLine(BPoint(maxPos - 1.0, a.top + 2.0),
				BPoint(a.left + 2.0, a.bottom - 1.0), light);
		AddLine(BPoint(a.left, a.bottom + 1.0),
				BPoint(a.right, a.bottom + 1.0), black);
	EndLineArray();
	// filling
	SetHighColor(background);
	arrow[0].x = a.left + 4.0;
	arrow[0].y = a.bottom - 2.0;
	arrow[1].x = a.right - 4.0;
	arrow[1].y = a.bottom - 2.0;
	arrow[2].x = maxPos;
	arrow[2].y = a.top + 2.0;
	FillPolygon(arrow, 3);
}

// MouseDown
void
DualSlider::MouseDown(BPoint where)
{
	if (IsEnabled()) {
		BRect r(_SliderFrame());
		BRect minFrame(r);
		if (IsMaxEnabled() && IsMinEnabled())
			minFrame.bottom = r.top + floorf(r.Height() / 2.0);
		BRect maxFrame(r);
		if (IsMaxEnabled() && IsMinEnabled())
			maxFrame.top = minFrame.bottom + 1.0;
		float value = _ValueFor(where);
		if (IsMinEnabled() && minFrame.Contains(where)) {
			fFlags |= DRAGGING_MIN;
			fFlags &= ~DRAGGING_MAX;
			fLastFactor = fMaxValue / fMinValue;
			SetMinValue(value);
			if (fLastFactor < 1.0)
				SetMaxValue(fLastFactor * fMinValue);
			_Invoke(fValueMessage, MOUSE_DOWN);
			SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
		} else if (IsMaxEnabled() && maxFrame.Contains(where)) {
			fFlags |= DRAGGING_MAX;
			fFlags &= ~DRAGGING_MIN;
			fLastFactor = fMinValue / fMaxValue;
			SetMaxValue(value);
			if (fLastFactor < 1.0)
				SetMinValue(fLastFactor * fMaxValue);
			_Invoke(fValueMessage, MOUSE_DOWN);
			SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
		}
		if (fControlMessage && _PressureBoxFrame().Contains(where))
			SetMinEnabled(!IsMinEnabled(), true);
	}
}

// MouseUp
void
DualSlider::MouseUp(BPoint where)
{
	if (fFlags & DRAGGING_MIN || fFlags & DRAGGING_MAX)
		_Invoke(fValueMessage, MOUSE_UP);

	fFlags &= ~DRAGGING_MIN;
	fFlags &= ~DRAGGING_MAX;
}

// MouseMoved
void
DualSlider::MouseMoved(BPoint where, uint32 transit,
					   const BMessage* dragMessage)
{
	if (IsEnabled()) {
		float value = _ValueFor(where);
//		float oldMin = fMinValue;
//		float oldMax = fMaxValue;
		if (fFlags & DRAGGING_MIN) {
			SetMinValue(value);
			if (fLastFactor < 1.0)
				SetMaxValue(fLastFactor * fMinValue);
			_Invoke(fValueMessage);
		} else if (fFlags & DRAGGING_MAX) {
			SetMaxValue(value);
			if (fLastFactor < 1.0)
				SetMinValue(fLastFactor * fMaxValue);
			_Invoke(fValueMessage);
		}
//		if (oldMin != fMinValue || oldMax != fMaxValue) {
//			_Invoke(fValueMessage);
//		}
	}
	BRect r(_PressureBoxFrame());
	if (r.Contains(where)) {
		if (!(fFlags & PRESSURE_INSIDE)) {
			fFlags |= PRESSURE_INSIDE;
			if (!IsMinEnabled())
				Invalidate(r);
			// set tool tip text
			BubbleHelper::Default()->SetHelp(this, fPressureControlTip.String());
		}
	} else {
		if (fFlags & PRESSURE_INSIDE) {
			fFlags &= ~PRESSURE_INSIDE;
			if (!IsMinEnabled())
				Invalidate(r);
			// set tool tip text
			BubbleHelper::Default()->SetHelp(this, NULL);
		}
	}
}

// SetValues
void
DualSlider::SetLabel(const char* label)
{
	if (label) {
		fLabel.SetTo(label);
		Invalidate();
	}
}

// SetValues
void
DualSlider::SetValues(float min, float max)
{
	SetMinValue(min);
	SetMaxValue(max);
}

// SetMinValue
void
DualSlider::SetMinValue(float value)
{
	if (value < 0.0)
		value = 0.0;
	if (value > 1.0)
		value = 1.0;
	if (value != fMinValue) {
		fMinValue = value;
		_Invalidate();
	}
}

// SetMaxValue
void
DualSlider::SetMaxValue(float value)
{
	if (value < 0.0)
		value = 0.0;
	if (value > 1.0)
		value = 1.0;
	if (value != fMaxValue) {
		fMaxValue = value;
		_Invalidate();
	}
}

// SetEnabled
void
DualSlider::SetEnabled(bool enable)
{
	if (enable != IsEnabled()) {
		if (enable)
			fFlags &= ~DISABLED;
		else
			fFlags |= DISABLED;
		Invalidate();
	}
}

// IsEnabled
bool
DualSlider::IsEnabled() const
{
	return !(fFlags & DISABLED);
}

// SetMinEnabled
void
DualSlider::SetMinEnabled(bool enable, bool sendMessage)
{
	if (enable) {
		if (!(fFlags & MIN_ENABLED)) {
			fFlags |= MIN_ENABLED;
			Invalidate(_PressureBoxFrame());
			_Invalidate();
			if (sendMessage)
				_Invoke(fControlMessage);
		}
	} else {
		if (fFlags & MIN_ENABLED) {
			fFlags &= ~MIN_ENABLED;
			Invalidate(_PressureBoxFrame());
			_Invalidate();
			if (sendMessage)
				_Invoke(fControlMessage);
		}
	}
}

// IsMinEnabled
bool
DualSlider::IsMinEnabled() const
{
	return fFlags & MIN_ENABLED;
}

// SetMaxEnabled
void
DualSlider::SetMaxEnabled(bool enable)
{
	if (enable) {
		if (!(fFlags & MAX_ENABLED)) {
			fFlags |= MAX_ENABLED;
			_Invalidate();
		}
	} else {
		if (fFlags & MAX_ENABLED) {
			fFlags &= ~MAX_ENABLED;
			_Invalidate();
		}
	}
}

// IsMaxEnabled
bool
DualSlider::IsMaxEnabled() const
{
	return fFlags & MAX_ENABLED;
}

// SetPressureControlTip
void
DualSlider::SetPressureControlTip(const char* text)
{
	fPressureControlTip.SetTo(text);
}

// _Invoke
void
DualSlider::_Invoke(BMessage* fromMessage, uint32 mode)
{
	if (!fromMessage)
		fromMessage = fValueMessage;
	if (fromMessage) {
		BHandler* target = fTarget ? fTarget : Window();
		BLooper* looper;
		if (target && (looper = target->Looper())) {
			BMessage message(*fromMessage);
			message.AddPointer("be:source", (void*)this);
			message.AddInt64("be:when", system_time());
			if (fromMessage == fValueMessage) {
				message.AddFloat("min value", fMinValue);
				message.AddFloat("max value", fMaxValue);
				if (mode == MOUSE_DOWN)
					message.AddBool("begin", true);
				else if (mode == MOUSE_UP)
					message.AddBool("end", true);
			}
			if (fromMessage == fControlMessage)
				message.AddInt32("be:value", (int32)IsMinEnabled());
			looper->PostMessage(&message, target);
		}
	}
}

// _Invalidate
void
DualSlider::_Invalidate()
{
	if (LockLooper()) {
		Invalidate(_SliderFrame());
		UnlockLooper();
	}
}

// _StrokeRect
void
DualSlider::_StrokeRect(BRect r, rgb_color leftTop,
					   rgb_color rightBottom)
{
	BeginLineArray(4);
		AddLine(BPoint(r.left, r.bottom),
				BPoint(r.left, r.top), leftTop);
		AddLine(BPoint(r.left + 1.0, r.top),
				BPoint(r.right, r.top), leftTop);
		AddLine(BPoint(r.right, r.top + 1.0),
				BPoint(r.right, r.bottom), rightBottom);
		AddLine(BPoint(r.right - 1.0, r.bottom),
				BPoint(r.left + 1.0, r.bottom), rightBottom);
	EndLineArray();
}

// _ValueFor
float
DualSlider::_ValueFor(BPoint where) const
{
	BRect r(_BarFrame());
	return (where.x - r.left) / r.Width();
}


// _BarFrame
BRect
DualSlider::_BarFrame() const
{
	BRect r(_SliderFrame());
	r.InsetBy(5.0, 5.0);
	r.bottom = r.top + 6.0;
	return r;
}

// _SliderFrame
BRect
DualSlider::_SliderFrame() const
{
	BRect r(Bounds());
	r.top += _LabelHeight();
	return r;
}

// _PressureBoxFrame
BRect
DualSlider::_PressureBoxFrame() const
{
	BRect r(Bounds());
	r.right -= 5.0;
	r.left = r.right - 12.0;
	r.bottom = r.top + 12.0;
	return r;
}

// _LabelHeight
float
DualSlider::_LabelHeight() const
{
	float height = 0.0;
	if (fLabel.CountChars() > 0) {
		font_height fh;
		GetFontHeight(&fh);
		height = ceilf(fh.ascent + fh.descent) + LABEL_SPACING;
	}
	return height;
}

