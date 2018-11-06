// BrushStroke.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Message.h>

#include "Brush.h"
#include "RLEBuffer.h"

#include "BoolProperty.h"
#include "CommonPropertyIDs.h"
#include "FloatProperty.h"
#include "IntProperty.h"
#include "LanguageManager.h"
#include "PropertyObject.h"
#include "Strings.h"

#include "BrushStroke.h"

// constructor
BrushStroke::BrushStroke(range radius, range hardness,
						 range alpha, range spacing,
						 uint32 flags,
						 rgb_color color, uint32 mode)
	: Stroke(color, mode),
	  fBrush(new Brush(radius, hardness)),
	  fFlags(flags),
	  fAlpha(alpha),
	  fSpacing(spacing),
	  fDistLeftOver(0.0)
{
}

// copy constructor
BrushStroke::BrushStroke(const BrushStroke& other)
	: Stroke(other),
	  fBrush(new Brush(other.fBrush->Radius(), other.fBrush->Hardness())),
	  fFlags(other.fFlags),
	  fAlpha(other.fAlpha),
	  fSpacing(other.fSpacing),
	  fDistLeftOver(other.fDistLeftOver)
{
}

// BArchivable constructor
BrushStroke::BrushStroke(BMessage* archive)
	: Stroke(archive),
	  fDistLeftOver(0.0)
{
	// restore flags
	if (archive->FindInt32("brush flags", (int32*)&fFlags) < B_OK)
		fFlags = FLAG_PRESSURE_CONTROLS_APHLA
				 | FLAG_PRESSURE_CONTROLS_RADIUS | FLAG_PRESSURE_CONTROLS_HARDNESS;

	// restore alpha range
	if (archive->FindFloat("min alpha", &fAlpha.min) < B_OK)
		fAlpha.min = 0.0;
	if (archive->FindFloat("max alpha", &fAlpha.max) < B_OK)
		fAlpha.min = 1.0;
	// restore spacing range
	if (archive->FindFloat("min spacing", &fSpacing.min) < B_OK)
		fSpacing.min = 0.1;
	if (archive->FindFloat("max spacing", &fSpacing.max) < B_OK)
		fSpacing.min = 0.1;

	// restore brush
	range radius;
	range hardness;
	// restore spacing range
	if (archive->FindFloat("min radius", &radius.min) < B_OK)
		radius.min = 15.0;
	if (archive->FindFloat("max radius", &radius.max) < B_OK)
		radius.min = 15.0;
	// restore hardness range
	if (archive->FindFloat("min hardness", &hardness.min) < B_OK)
		hardness.min = 1.0;
	if (archive->FindFloat("max hardness", &hardness.max) < B_OK)
		hardness.min = 1.0;
	fBrush = new Brush(radius, hardness);
}

// destructor
BrushStroke::~BrushStroke()
{
	delete fBrush;
}

// Clone
Stroke*
BrushStroke::Clone() const
{
	return new BrushStroke(*this);
}

// SetTo
bool
BrushStroke::SetTo(const Stroke* from)
{
	const BrushStroke* brushStroke = dynamic_cast<const BrushStroke*>(from);

	AutoNotificationSuspender _(this);

	if (brushStroke && Stroke::SetTo(from)) {
		fBrush = new Brush(brushStroke->fBrush->Radius(),
						   brushStroke->fBrush->Hardness());
		fFlags = brushStroke->fFlags;
		fAlpha = brushStroke->fAlpha;
		fSpacing = brushStroke->fSpacing;
		fDistLeftOver = brushStroke->fDistLeftOver;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
BrushStroke::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "BrushStroke"))
		return new BrushStroke(archive);
	return NULL;
}

// Archive
status_t
BrushStroke::Archive(BMessage* into, bool deep) const
{
	status_t status = Stroke::Archive(into, deep);

	// add our data to the message

	// store flags
	if (status >= B_OK)
		status = into->AddInt32("brush flags", (int32)fFlags);
	// store alpha range
	if (status >= B_OK)
		status = into->AddFloat("min alpha", fAlpha.min);
	if (status >= B_OK)
		status = into->AddFloat("max alpha", fAlpha.max);
	// store spacing range
	if (status >= B_OK)
		status = into->AddFloat("min spacing", fSpacing.min);
	if (status >= B_OK)
		status = into->AddFloat("max spacing", fSpacing.max);
	// store brush
	// store radius range
	if (status >= B_OK)
		status = into->AddFloat("min radius", fBrush->Radius().min);
	if (status >= B_OK)
		status = into->AddFloat("max radius", fBrush->Radius().max);
	// store hardness range
	if (status >= B_OK)
		status = into->AddFloat("min hardness", fBrush->Hardness().min);
	if (status >= B_OK)
		status = into->AddFloat("max hardness", fBrush->Hardness().max);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "BrushStroke");

	return status;
}


// DrawLastLine
bool
BrushStroke::DrawLastLine(BBitmap* bitmap, BRect& updateRect)
{
	int32 count = fTrackPoints.CountItems();
	Point* last = (Point*)fTrackPoints.ItemAt(count - 1);
	Point* previous = (Point*)fTrackPoints.ItemAt(count - 2);
	bool success = false;
	if (last) {
		if (!previous)
			previous = last;
		success = _StrokeLine(previous, last,
							  (uint8*)bitmap->Bits(), bitmap->BytesPerRow(),
							  bitmap->Bounds());
		// calculate update rect (and give some room)
		if (success) {
/*			if (fFlags & FLAG_PRESSURE_CONTROLS_RADIUS) {
				float previousRadius = previous->pressure * fBrush->Radius().max + 1.0;
				float lastRadius = last->pressure * fBrush->Radius().max + 1.0;
				if (last->point.x < previous->point.x) {
					updateRect.left = last->point.x - lastRadius;
					updateRect.right = previous->point.x + previousRadius;
				} else {
					updateRect.left = previous->point.x - previousRadius;
					updateRect.right = last->point.x + lastRadius;
				}
				if (last->point.y < previous->point.y) {
					updateRect.top = last->point.y - lastRadius;
					updateRect.bottom = previous->point.y + previousRadius;
				} else {
					updateRect.top = previous->point.y - previousRadius;
					updateRect.bottom = last->point.y + lastRadius;
				}
			} else {*/
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
				float inset = fBrush->Radius().max > fBrush->Radius().min ?
							  fBrush->Radius().max : fBrush->Radius().min;
				inset = -ceilf(inset);
				updateRect.InsetBy(inset, inset);
//			}
			// update our bounds
			Touch(updateRect);
		}
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
BrushStroke::Draw(BBitmap* bitmap)
{
	Draw(bitmap, bitmap->Bounds());
}

// Draw
void
BrushStroke::Draw(BBitmap* bitmap, BRect constrainRect)
{
	constrainRect = constrainRect & bitmap->Bounds();
	if (IsCacheValid()) {
//printf("drawing using cache\n");
		BRect bounds = Bounds();
		BPoint lt = bounds.LeftTop();
		
		fCache->Fill(bitmap, (int32)lt.x, (int32)lt.y,
					 constrainRect);

		BRect cacheRect(fCache->Bounds());
		cacheRect.OffsetBy((int32)lt.x, (int32)lt.y);
		bounds.Set(floorf(bounds.left),
				   floorf(bounds.top),
				   ceilf(bounds.right),
				   ceilf(bounds.bottom));
		if (cacheRect != bounds) {
//printf("drawing missing regions\n");
//cacheRect.PrintToStream();
//bounds.PrintToStream();
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
BrushStroke::Reset()
{
	fDistLeftOver = 0.0;
	Stroke::Reset();
}

// Name
const char*
BrushStroke::Name() const
{
	LanguageManager* manager = LanguageManager::Default();
	const char* name = NULL;
	if (fMode == MODE_ERASER)
		name = manager->GetString(ERASER, "Eraser");
	else
		name = manager->GetString(BRUSH, "Brush");
	return name;
}

// ToolID
int32
BrushStroke::ToolID() const
{
	return fMode == MODE_ERASER ? TOOL_ERASER : TOOL_BRUSH;
}

// MakePropertyObject
PropertyObject*
BrushStroke::MakePropertyObject() const
{
	PropertyObject* object = Stroke::MakePropertyObject();
	if (object) {
		// opacity
		object->AddProperty(new BoolProperty("dyn. opac.",
											  PROPERTY_PRESSURE_CONTROLS_OPACITY,
											  fFlags & FLAG_PRESSURE_CONTROLS_APHLA));
		if (fFlags & FLAG_PRESSURE_CONTROLS_APHLA) {
			object->AddProperty(new IntProperty("min opac.",
												PROPERTY_MIN_OPACITY,
												(int32)(fAlpha.min * 255)));
		}
		object->AddProperty(new IntProperty("opacity",
											PROPERTY_OPACITY,
											(int32)(fAlpha.max * 255)));

		// radius
		object->AddProperty(new BoolProperty("dyn. radius",
											  PROPERTY_PRESSURE_CONTROLS_RADIUS,
											  fFlags & FLAG_PRESSURE_CONTROLS_RADIUS));
		if (fFlags & FLAG_PRESSURE_CONTROLS_RADIUS) {
			object->AddProperty(new FloatProperty("min radius",
												  PROPERTY_MIN_RADIUS,
												  fBrush->Radius().min,
												  0.0, 500.0));
		}
		object->AddProperty(new FloatProperty("radius",
											  PROPERTY_RADIUS,
											  fBrush->Radius().max,
											  0.0, 500.0));

		// hardness
		object->AddProperty(new BoolProperty("dyn. hardn.",
											  PROPERTY_PRESSURE_CONTROLS_HARDNESS,
											  fFlags & FLAG_PRESSURE_CONTROLS_HARDNESS));
		if (fFlags & FLAG_PRESSURE_CONTROLS_HARDNESS) {
			object->AddProperty(new FloatProperty("min hardn.",
												  PROPERTY_MIN_HARDNESS,
												  fBrush->Hardness().min * 100.0,
												  0.0, 100.0));
		}
		object->AddProperty(new FloatProperty("hardness",
											  PROPERTY_HARDNESS,
											  fBrush->Hardness().max * 100.0,
											  0.0, 100.0));
		// spacing
		object->AddProperty(new FloatProperty("spacing",
											  PROPERTY_SPACING,
											  fSpacing.max * 100.0,
											  0.0, 100.0));
		// solid
		object->AddProperty(new BoolProperty("solid",
											  PROPERTY_NO_ANTIALIASING,
											  fFlags & FLAG_SOLID));
		// tilt
		object->AddProperty(new BoolProperty("tilt",
											  PROPERTY_TILT,
											  fFlags & FLAG_TILT_CONTROLS_SHAPE));

	}
	return object;
}

// SetToPropertyObject
bool
BrushStroke::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);
	
	bool ret = Stroke::SetToPropertyObject(object);
	bool ret2 = false;
	if (object) {
		// radius
		range radius = fBrush->Radius();
		float f = object->FindFloatProperty(PROPERTY_MIN_RADIUS, radius.min);
		if (f != radius.min)
			ret2 = true;
		radius.min = f;
		f = object->FindFloatProperty(PROPERTY_RADIUS, radius.max);
		if (f != radius.max)
			ret2 = true;
		radius.max = f;
		fBrush->SetRadius(radius);
		bool b = object->FindBoolProperty(PROPERTY_PRESSURE_CONTROLS_RADIUS,
										  fFlags & FLAG_PRESSURE_CONTROLS_RADIUS);
		if (b != (fFlags & FLAG_PRESSURE_CONTROLS_RADIUS))
			ret2 = true;
		fFlags = b ? fFlags | FLAG_PRESSURE_CONTROLS_RADIUS
				   : fFlags & ~FLAG_PRESSURE_CONTROLS_RADIUS;

		// hardness
		range hardness = fBrush->Hardness();
		f = object->FindFloatProperty(PROPERTY_MIN_HARDNESS, hardness.min * 100.0) / 100.0;
		if (f != hardness.min)
			ret2 = true;
		hardness.min = f;
		f = object->FindFloatProperty(PROPERTY_HARDNESS, hardness.max * 100.0) / 100.0;
		if (f != hardness.max)
			ret2 = true;
		hardness.max = f;
		fBrush->SetHardness(hardness);
		b = object->FindBoolProperty(PROPERTY_PRESSURE_CONTROLS_HARDNESS,
									 fFlags & FLAG_PRESSURE_CONTROLS_HARDNESS);
		if (b != (fFlags & FLAG_PRESSURE_CONTROLS_HARDNESS))
			ret2 = true;
		fFlags = b ? fFlags | FLAG_PRESSURE_CONTROLS_HARDNESS
				   : fFlags & ~FLAG_PRESSURE_CONTROLS_HARDNESS;

		// opacity
		int32 maxAlpha = (int32)(fAlpha.max * 255);
		int32 i = object->FindIntProperty(PROPERTY_OPACITY, maxAlpha);
		if (i != maxAlpha)
			ret2 = true;
		maxAlpha = i;

		int32 minAlpha = (int32)(fAlpha.min * 255);
		i = object->FindIntProperty(PROPERTY_MIN_OPACITY, minAlpha);
		if (i != minAlpha)
			ret2 = true;
		minAlpha = i;

		fAlpha.min = minAlpha / 255.0;
		fAlpha.max = maxAlpha / 255.0;

		b = object->FindBoolProperty(PROPERTY_PRESSURE_CONTROLS_OPACITY,
									 fFlags & FLAG_PRESSURE_CONTROLS_APHLA);
		if (b != (fFlags & FLAG_PRESSURE_CONTROLS_APHLA))
			ret = true;
		fFlags = b ? fFlags | FLAG_PRESSURE_CONTROLS_APHLA
				   : fFlags & ~FLAG_PRESSURE_CONTROLS_APHLA;

		// spacing
		f = object->FindFloatProperty(PROPERTY_SPACING, fSpacing.max * 100.0) / 100.0;
		if (f != fSpacing.max)
			ret2 = true;
		fSpacing.max = f;

		// solid
		b = object->FindBoolProperty(PROPERTY_NO_ANTIALIASING,
									 fFlags & FLAG_SOLID);
		if (b != (fFlags & FLAG_SOLID))
			ret2 = true;
		fFlags = b ? fFlags | FLAG_SOLID
				   : fFlags & ~FLAG_SOLID;

		// tilt
		b = object->FindBoolProperty(PROPERTY_TILT,
									 fFlags & FLAG_TILT_CONTROLS_SHAPE);
		if (b != (fFlags & FLAG_TILT_CONTROLS_SHAPE))
			ret2 = true;
		fFlags = b ? fFlags | FLAG_TILT_CONTROLS_SHAPE
				   : fFlags & ~FLAG_TILT_CONTROLS_SHAPE;

		// cache is toast
		if (ret2) {
			delete fCache;
			fCache = NULL;
			_RecalcBounds();
			Notify();
		}
	}
	return ret || ret2;
}

// _Draw
void
BrushStroke::_Draw(BBitmap* bitmap, BRect constrainRect)
{
	if (!Transformable::IsValid())
		return;

	uint8* dest = (uint8*)bitmap->Bits();
	uint32 bpr = bitmap->BytesPerRow();
	// traverse lines
	fDistLeftOver = 0.0;
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
bool
BrushStroke::_StrokeLine(const Point* a, const Point* b,
						 uint8* dest, uint32 bpr, BRect constrainRect)
{
	bool drawnAnything = false;
	if (a == b) {
		BPoint p = a->point;
		fBrush->Draw(p, a->pressure, a->tiltX, a->tiltY,
					 fAlpha, fFlags,
					 dest, bpr, *this, constrainRect);
		drawnAnything = true;
	} else {
		BPoint pA = a->point;
		BPoint pB = b->point;
			
		BPoint vector = pB - pA;
		float dist = sqrtf(vector.x * vector.x + vector.y * vector.y);
		float pressureDiff = b->pressure - a->pressure;
		float tiltXDiff = b->tiltX - a->tiltX;
		float tiltYDiff = b->tiltY - a->tiltY;
		float stepDist = _StepDist();
		float currentStepDist = stepDist;
		float minStepDist = scale() != 0.0 ? 1.0 / scale() : 1.0;
		if (fFlags & FLAG_PRESSURE_CONTROLS_RADIUS)
			currentStepDist = max_c(minStepDist, stepDist * a->pressure);
//		float p = currentStepDist - fDistLeftOver;
		float p = fDistLeftOver != 0.0 ? currentStepDist - fDistLeftOver : 0.0;
		if (p < dist) {
			for (; p < dist; p += currentStepDist) {
//		if (p <= dist) {
//			for (; p <= dist; p += currentStepDist) {
				float scale = p / dist;
//				float scale = dist > 0.0 ? p / dist : 0.0;
				float currentPressure = a->pressure + pressureDiff * scale;
				float currentTiltX = a->tiltX + tiltXDiff * scale;
				float currentTiltY = a->tiltY + tiltYDiff * scale;
				if (fFlags & FLAG_PRESSURE_CONTROLS_RADIUS)
					currentStepDist = max_c(minStepDist, stepDist * currentPressure);
				BPoint center(pA.x + vector.x * scale,
							  pA.y + vector.y * scale);
				fBrush->Draw(center,
							 currentPressure, currentTiltX, currentTiltY,
							 fAlpha, fFlags,
							 dest, bpr, *this, constrainRect);
				drawnAnything = true;
			}
			fDistLeftOver = dist - (p - currentStepDist);
		} else
			fDistLeftOver += dist;
	}
	return drawnAnything;
}

// _StepDist
float
BrushStroke::_StepDist() const
{
	float step = fBrush->Radius().max * 2.0 * fSpacing.max;
	float minStep = scale() != 0.0 ? 1.0 / scale() : 1.0;
	if (step < minStep)
		step = minStep;
	return step;
}

// _RecalcBounds
void
BrushStroke::_RecalcBounds()
{
	Reset();

	range radius = fBrush->Radius();
	// traverse lines
	for (int32 i = 0; Point* current = (Point*)fTrackPoints.ItemAt(i); i++) {

		BRect b(current->point, current->point);
		float r = ceilf(radius.min + (current->pressure * (radius.max - radius.min))) + 1.0;
		b.InsetBy(-r , -r);

		Touch(b);
	}
}
