// PenStroke.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Message.h>

#include "alpha.h"
#include "blending.h"

#include "Brush.h"
#include "RLEBuffer.h"
#include "BoolProperty.h"
#include "CommonPropertyIDs.h"
#include "IntProperty.h"
#include "LanguageManager.h"
#include "PropertyObject.h"
#include "Strings.h"

#include "PenStroke.h"

// constructor
PenStroke::PenStroke(rgb_color color, range alpha,
					 uint32 flags, uint32 mode)
	: Stroke(color, mode),
	  fAlpha(alpha),
	  fFlags(flags),
	  fDistLeftOver(1.0)
{
}

// copy constructor
PenStroke::PenStroke(const PenStroke& other)
	: Stroke(other),
	  fAlpha(other.fAlpha),
	  fFlags(other.fFlags),
	  fDistLeftOver(other.fDistLeftOver)
{
}

// BArchivable constructor
PenStroke::PenStroke(BMessage* archive)
	: Stroke(archive),
	  fDistLeftOver(1.0)
{
	// restore control alpha flag
	if (archive->FindInt32("flags", (int32*)&fFlags) != B_OK)
		fFlags = FLAG_PRESSURE_CONTROLS_APHLA;
	// restore alpha range
	if (archive->FindFloat("min alpha", &fAlpha.min) != B_OK)
		fAlpha.min = 0.0;
	if (archive->FindFloat("max alpha", &fAlpha.max) != B_OK)
		fAlpha.min = 1.0;
}

// destructor
PenStroke::~PenStroke()
{
}

// Clone
Stroke*
PenStroke::Clone() const
{
	return new PenStroke(*this);
}

// SetTo
bool
PenStroke::SetTo(const Stroke* from)
{
	const PenStroke* penStroke = dynamic_cast<const PenStroke*>(from);

	AutoNotificationSuspender _(this);

	if (penStroke && Stroke::SetTo(from)) {
		fAlpha = penStroke->fAlpha;
		fFlags = penStroke->fFlags;
		fDistLeftOver = penStroke->fDistLeftOver;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
PenStroke::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "PenStroke"))
		return new PenStroke(archive);
	return NULL;
}

// Archive
status_t
PenStroke::Archive(BMessage* into, bool deep) const
{
	status_t status = Stroke::Archive(into, deep);

	// add our data to the message
	if (status >= B_OK)
		// store control alpha flag
		status = into->AddInt32("flags", fFlags);
	// store alpha range
	if (status >= B_OK)
		status = into->AddFloat("min alpha", fAlpha.min);
	// store bounds
	if (status >= B_OK)
		status = into->AddFloat("max alpha", fAlpha.max);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "PenStroke");

	return status;
}


// DrawLastLine
bool
PenStroke::DrawLastLine(BBitmap* bitmap, BRect& updateRect)
{
	int32 count = fTrackPoints.CountItems();
	Point* last = (Point*)fTrackPoints.ItemAt(count - 1);
	Point* previous = (Point*)fTrackPoints.ItemAt(count - 2);
	bool success = false;
	if (last) {
		if (!previous)
			previous = last;
		uint8* dest = (uint8*)bitmap->Bits();
		uint32 bpr = bitmap->BytesPerRow();
		_StrokeLine(previous, last, dest, bpr, updateRect);
		// calculate update rect (and give some room)
		if (last->point.x < previous->point.x) {
			updateRect.left = last->point.x - 1.0;
			updateRect.right = previous->point.x + 1.0;
		} else {
			updateRect.left = previous->point.x - 1.0;
			updateRect.right = last->point.x + 1.0;
		}
		if (last->point.y < previous->point.y) {
			updateRect.top = last->point.y - 1.0;
			updateRect.bottom = previous->point.y + 1.0;
		} else {
			updateRect.top = previous->point.y - 1.0;
			updateRect.bottom = last->point.y + 1.0;
		}
		// update our bounds
		Touch(updateRect);
		success = true;
	} else {
		// make rect invalid
		updateRect.left = 0.0;
		updateRect.top = 0.0;
		updateRect.right = -1.0;
		updateRect.bottom = -1.0;
	}
	return success;
}

// Draw
void
PenStroke::Draw(BBitmap* bitmap)
{
	Draw(bitmap, bitmap->Bounds());
}

// Draw
void
PenStroke::Draw(BBitmap* bitmap, BRect constrainRect)
{
	constrainRect = constrainRect & bitmap->Bounds();
	if (IsCacheValid()) {
		BRect bounds = Bounds();
		BPoint lt = Bounds().LeftTop();

		fCache->Fill(bitmap, (int32)lt.x, (int32)lt.y,
					 constrainRect);
		BRect cacheRect(fCache->Bounds());
		cacheRect.OffsetBy((int32)bounds.left, (int32)bounds.top);
		bounds.Set(floorf(bounds.left),
				   floorf(bounds.top),
				   ceilf(bounds.right),
				   ceilf(bounds.bottom));
		if (cacheRect != bounds) {
			// draw the regions that are missing from the cache
			constrainRect.left = floorf(constrainRect.left);
			constrainRect.top = floorf(constrainRect.top);
			constrainRect.right = ceilf(constrainRect.right);
			constrainRect.bottom = ceilf(constrainRect.bottom);
			BRect r(constrainRect);
			// left side of cacheRect
			r.right = cacheRect.left - 1.0;
			if (r.IsValid())
				_Draw(bitmap, r);
			// right side of cacheRect
			r.right = constrainRect.right;
			r.left = cacheRect.right + 1.0;
			if (r.IsValid())
				_Draw(bitmap, r);
			// top side of cacheRect
			r.top = constrainRect.top;
			r.bottom = cacheRect.top - 1.0;
			r.left = constrainRect.left > cacheRect.left ? constrainRect.left : cacheRect.left;
			r.right = constrainRect.right < cacheRect.right ? constrainRect.right : cacheRect.right;
			if (r.IsValid())
				_Draw(bitmap, r);
			// bottom side of cacheRect
			r.top = cacheRect.bottom + 1.0;
			r.bottom = constrainRect.bottom;
			if (r.IsValid())
				_Draw(bitmap, r);
		}
	} else {
		_Draw(bitmap, constrainRect);
	}
}

// Reset
void
PenStroke::Reset()
{
	fDistLeftOver = 1.0;
	Stroke::Reset();
}

// Name
const char*
PenStroke::Name() const
{
	LanguageManager* manager = LanguageManager::Default();
	const char* name = NULL;
	if (fMode == MODE_ERASER || fMode == MODE_PEN_ERASER)
		name = manager->GetString(PEN_ERASER, "Eraser Pen");
	else
		name = manager->GetString(PEN, "Pen");
	return name;
}

// ToolID
int32
PenStroke::ToolID() const
{
	return fMode == MODE_PEN_ERASER ? TOOL_PEN_ERASER : TOOL_PEN;
}

// MakePropertyObject
PropertyObject*
PenStroke::MakePropertyObject() const
{
	PropertyObject* object = Stroke::MakePropertyObject();
	if (object) {
		// opacity
		object->AddProperty(new IntProperty("min opac.",
											PROPERTY_MIN_OPACITY,
											(int32)(fAlpha.min * 255)));
		object->AddProperty(new IntProperty("opacity",
											PROPERTY_OPACITY,
											(int32)(fAlpha.max * 255)));
		object->AddProperty(new BoolProperty("dyn. opac.",
											  PROPERTY_PRESSURE_CONTROLS_OPACITY,
											  fFlags & FLAG_PRESSURE_CONTROLS_APHLA));

		// solid
		object->AddProperty(new BoolProperty("solid",
											  PROPERTY_NO_ANTIALIASING,
											  fFlags & FLAG_SOLID));

	}
	return object;
}

// SetToPropertyObject
bool
PenStroke::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);
	
	bool ret = Stroke::SetToPropertyObject(object);
	if (object) {
		// opacity
		int32 maxAlpha = (int32)(fAlpha.max * 255);
		int32 i = object->FindIntProperty(PROPERTY_OPACITY, maxAlpha);
		if (i != maxAlpha)
			ret = true;
		maxAlpha = i;

		int32 minAlpha = (int32)(fAlpha.min * 255);
		i = object->FindIntProperty(PROPERTY_MIN_OPACITY, minAlpha);
		if (i != minAlpha)
			ret = true;
		minAlpha = i;

		fAlpha.min = minAlpha / 255.0;
		fAlpha.max = maxAlpha / 255.0;

		bool b = object->FindBoolProperty(PROPERTY_PRESSURE_CONTROLS_OPACITY,
										  fFlags & FLAG_PRESSURE_CONTROLS_APHLA);
		if (b != (fFlags & FLAG_PRESSURE_CONTROLS_APHLA))
			ret = true;
		fFlags = b ? fFlags | FLAG_PRESSURE_CONTROLS_APHLA
				   : fFlags & ~FLAG_PRESSURE_CONTROLS_APHLA;

		// solid
		b = object->FindBoolProperty(PROPERTY_NO_ANTIALIASING,
									 fFlags & FLAG_SOLID);
		if (b != (fFlags & FLAG_SOLID))
			ret = true;
		fFlags = b ? fFlags | FLAG_SOLID
				   : fFlags & ~FLAG_SOLID;

		// cache is toast
		if (ret) {
			delete fCache;
			fCache = NULL;
			Notify();
		}
	}
	return ret;
}

// _Draw
void
PenStroke::_Draw(BBitmap* bitmap, BRect constrainRect)
{
	uint8* dest = (uint8*)bitmap->Bits();
	uint32 bpr = bitmap->BytesPerRow();
	// traverse lines
	fDistLeftOver = 1.0;
	Point* previous = (Point*)fTrackPoints.ItemAt(0);
	if (fTrackPoints.CountItems() > 1) {
		for (int32 i = 1; Point* current = (Point*)fTrackPoints.ItemAt(i); i++) {
			_StrokeLine(previous, current, dest, bpr, constrainRect);
			previous = current;
		}
	} else if (previous)
		_StrokeLine(previous, previous, dest, bpr, constrainRect);
}

// _StrokeLine
void
PenStroke::_StrokeLine(Point* a, Point* b, uint8* dest, uint32 bpr,
					   BRect& constrainRect)
{
	BPoint pA = a->point;
	BPoint pB = b->point;

	Transform(&pA);
	Transform(&pB);

	pA = pA - BPoint(0.5, 0.5);
	pB = pB - BPoint(0.5, 0.5);

	BPoint vector = pB - pA;
	float dist = sqrtf(vector.x * vector.x + vector.y * vector.y);
	float pressureDiff = b->pressure - a->pressure;
	float currentStepDist = 1.0;
	float p = currentStepDist - fDistLeftOver;
	if (p >= 0.0 && p <= dist) {
		for (; p <= dist; p += currentStepDist) {
			float scale = dist > 0.0 ? p / dist : 0.0;
			double centerX = pA.x + vector.x * scale;
			double centerY = pA.y + vector.y * scale;
			_StrokePoint(centerX, centerY,
						 a->pressure + pressureDiff * scale,
						 dest, bpr, constrainRect);
		}
		fDistLeftOver = dist - (p - currentStepDist);
	} else
		fDistLeftOver += dist;
}

// _StrokePoint
void
PenStroke::_StrokePoint(float x, float y, float pressure, uint8* bits, uint32 bpr,
						BRect& constrainRect) const
{
	if (fFlags & FLAG_PRESSURE_CONTROLS_APHLA)
		pressure = fAlpha.min + (fAlpha.max - fAlpha.min) * pressure;
	else
		pressure = fAlpha.max;
	uint8 maxAlpha = (uint8)(255.0 * pressure);

	if (fFlags & FLAG_SOLID) {
		BPoint p(floorf(x + 0.5), floorf(y + 0.5));
		if (constrainRect.Contains(p)) {
			uint8* dest = bits + (int32)p.x;
			dest += bpr * (int32)p.y;
			add_alpha(dest, pressure, maxAlpha);
		}
	} else {
		float floorX = floorf(x);
		float floorY = floorf(y);
		float leftX = x - floorX;	// part not covered by pixel
		float leftY = y - floorY;
		float coverX = 1.0 - leftX;	// part covered by pixel
		float coverY = 1.0 - leftY;
		uint8* dest = bits + (int32)floorX;
		dest += bpr * (int32)floorY;
		if (constrainRect.Contains(BPoint(floorX, floorY))) {
			// offset into top left pixel
			add_alpha(dest, pressure * (coverX * coverY), maxAlpha);
		}
		dest ++;
		if (constrainRect.Contains(BPoint(floorX + 1.0, floorY))) {
			// offset into top right pixel
			add_alpha(dest, pressure * (leftX * coverY), maxAlpha);
		}
		dest += bpr - 1;
		if (constrainRect.Contains(BPoint(floorX, floorY + 1.0))) {
			// offset into bottom left pixel
			add_alpha(dest, pressure * (coverX * leftY), maxAlpha);
		}
		dest ++;
		if (constrainRect.Contains(BPoint(floorX + 1.0, floorY + 1.0))) {
			// offset into bottom right pixel
			add_alpha(dest, pressure * (leftX * leftY), maxAlpha);
		}
	}
}

