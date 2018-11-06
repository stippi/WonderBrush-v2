// MotionBlur.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Message.h>

#include "bitmap_compression.h"
#include "bitmap_support.h"
#include "blending.h"
#include "defines.h"
#include "support.h"

#include "BoolProperty.h"
#include "CommonPropertyIDs.h"
#include "FilterFactory.h"
#include "FloatProperty.h"
#include "PropertyObject.h"

#include "MotionBlur.h"

// constructor
MotionBlur::MotionBlur()
	: FilterObject(FILTER_MOTION_BLUR),
	  fDirection(0.0),
	  fBlurRadius(5.0),
	  fBlurAlpha(true)
{
}

// copy constructor
MotionBlur::MotionBlur(const MotionBlur& other)
	: FilterObject(other),
	  fDirection(other.fDirection),
	  fBlurRadius(other.fBlurRadius),
	  fBlurAlpha(other.fBlurAlpha)
{
}

// BArchivable constructor
MotionBlur::MotionBlur(BMessage* archive)
	: FilterObject(archive),
	  fDirection(0.0),
	  fBlurRadius(5.0),
	  fBlurAlpha(true)
{
	if (archive) {
		if (archive->FindFloat("angle", &fDirection) < B_OK)
			fDirection = 0.0;
		if (archive->FindFloat("blur radius", &fBlurRadius) < B_OK)
			fBlurRadius = 5.0;
		if (archive->FindBool("blur alpha", &fBlurAlpha) < B_OK)
			fBlurAlpha = true;
	} else {
		SetFilterID(FILTER_MOTION_BLUR);
	}
}

// destructor
MotionBlur::~MotionBlur()
{
}

// Clone
Stroke*
MotionBlur::Clone() const
{
	return new MotionBlur(*this);
}

// SetTo
bool
MotionBlur::SetTo(const Stroke* from)
{
	const MotionBlur* motionBlur = dynamic_cast<const MotionBlur*>(from);

	AutoNotificationSuspender _(this);

	if (motionBlur && FilterObject::SetTo(from)) {
		fDirection = motionBlur->fDirection;
		fBlurRadius = motionBlur->fBlurRadius;
		fBlurAlpha = motionBlur->fBlurAlpha;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
MotionBlur::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "MotionBlur"))
		return new MotionBlur(archive);
	return NULL;
}

// Archive
status_t
MotionBlur::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddFloat("angle", fDirection);

	if (status >= B_OK)
		status = into->AddFloat("blur radius", fBlurRadius);

	if (status >= B_OK)
		status = into->AddBool("blur alpha", fBlurAlpha);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "MotionBlur");

	return status;
}

// ExtendRebuildArea
void
MotionBlur::ExtendRebuildArea(BRect& area, BRect layerBounds) const
{
	area = layerBounds;
}

// ProcessBitmap
void
MotionBlur::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	// TODO: this function is broken if area != dest->Bounds()
	// (but it wouldn't make sense if we didn't blur the entire bitmap)
	if (dest && dest->IsValid()) {

		uint32 count;
		blur_delta* deltas = _CalculateDeltas(&count);

		uint32 extend = count / 2 + 1;
		if (BBitmap* extended = extended_bitmap(dest, extend)) {

			uint8* src = (uint8*)extended->Bits();
			uint32 srcBPR = extended->BytesPerRow();

			uint8* dst = (uint8*)dest->Bits();
			uint32 dstBPR = dest->BytesPerRow();

			uint32 pixels = dest->Bounds().IntegerWidth() + 1;
			uint32 lines = dest->Bounds().IntegerHeight() + 1;

			multiply_alpha(extended);
			_Blur(dst, src, dstBPR, srcBPR, extend, lines, pixels,
				  deltas, count, fBlurAlpha);
			demultiply_alpha(dest);

			delete extended;
		}
	}
}

// MakePropertyObject
PropertyObject*
MotionBlur::MakePropertyObject() const
{
	PropertyObject* object = new PropertyObject();

	object->AddProperty(new FloatProperty("angla",
										  PROPERTY_ANGLE,
										  fDirection,
										  0.0, 360.0));
	object->AddProperty(new FloatProperty("blur radius",
										  PROPERTY_BLUR_RADIUS,
										  fBlurRadius,
										  0.0, 100.0));
/*	object->AddProperty(new BoolProperty("blur alpha",
										  PROPERTY_BLUR_ALPHA,
										  fBlurAlpha));*/
	return object;
}

// SetToPropertyObject
bool
MotionBlur::SetToPropertyObject(PropertyObject* object)
{
	bool ret = false;
	if (object) {
		float f = object->FindFloatProperty(PROPERTY_ANGLE, fDirection);
		if (f != fDirection)
			ret = true;
		fDirection = f;

		f = object->FindFloatProperty(PROPERTY_BLUR_RADIUS, fBlurRadius);
		if (f != fBlurRadius)
			ret = true;
		fBlurRadius = f;

/*		bool b = object->FindBoolProperty(PROPERTY_BLUR_ALPHA, fBlurAlpha);
		if (b != fBlurAlpha)
			ret = true;
		fBlurAlpha = b;*/


		if (ret) {
			SaveSettings();
			Notify();
		}
	}
	return ret;
}

// TransformationChanged
void
MotionBlur::TransformationChanged()
{
	// undo any translation, we're only interested in rotation and scale
	tx = 0.0;
	ty = 0.0;

	// undo any perspective influence
	w0 = 0.0;
	w1 = 0.0;
	w2 = 1.0;

	Notify();
}

// _CalculateDeltas
blur_delta*
MotionBlur::_CalculateDeltas(uint32* count) const
{
	blur_delta* deltas;
	// code orginates from ArtPaint, but is heavily modified,
	// so that the blur is centered;
	// "coeff" is not used anymore (as in a real motion blur)

	// This uses a DDA-algorithm to calculate the pixels to be modified
	// We could also calculate pixel coverage and put coefficients
	// according to that.
	BPoint start_point(0.0, -(fBlurRadius / 2.0));
	BPoint end_point(0.0, fBlurRadius / 2.0);

	Transformable transform(*this);
	transform.RotateBy(BPoint(0.0, 0.0), fDirection);
	transform.Transform(&start_point);
	transform.Transform(&end_point);

	// use DDA-algorithm to calculate line between the two points		
	// first check whether the line is longer in x direction than y
	bool increase_x = fabs(start_point.x - end_point.x) >= fabs(start_point.y - end_point.y);
	// check which direction the line is going
	float sign_x;
	float sign_y;

	if ((end_point.x-start_point.x) != 0)
		sign_x = (end_point.x-start_point.x)/fabs(end_point.x-start_point.x);
	else
		sign_x = 0;

	if ((end_point.y-start_point.y) != 0)
		sign_y = (end_point.y-start_point.y)/fabs(end_point.y-start_point.y);		
	else
		sign_y = 0;

	if (increase_x) {
		*count = (uint32)fabs(start_point.x - end_point.x) + 1;
		deltas = new blur_delta[*count];

		float y_add = ((float)fabs(start_point.y - end_point.y)) /
					  ((float)fabs(start_point.x - end_point.x));

		for (uint32 i = 0; i < *count; i++) {
			deltas[i].dx = (int32)start_point.x;
			deltas[i].dy = (int32)start_point.y;
			
			start_point.x += sign_x;
			start_point.y += sign_y * y_add;	
		}				
	} else {
		*count = (uint32)fabs(start_point.y - end_point.y) + 1;
		deltas = new blur_delta[*count];

		float x_add = ((float)fabs(start_point.x - end_point.x)) /
					  ((float)fabs(start_point.y - end_point.y));

		for (uint32 i = 0; i < *count; i++) {
			deltas[i].dx = (int32)start_point.x;
			deltas[i].dy = (int32)start_point.y;
			
			start_point.y += sign_y;
			start_point.x += sign_x * x_add;	
		}				
	}
	return deltas;
}

// _Blur
//
// input bitmap data needs to be premultiplied!
// output bitmap data is also premultiplied
void
MotionBlur::_Blur(uint8 *dst,
				  uint8 *src,
				  uint32 dstBPR,
				  uint32 srcBPR,
				  uint32 extend,
				  uint32 lines,
				  uint32 pixels,
				  blur_delta* deltas,
				  uint32 count,
				  bool blurAlpha) const
{
	// code orginates from ArtPaint, but is heavily modified to behave better with real alpha channel

	src += extend * srcBPR;

	int32 red, green, blue, alpha;
	
 	for (uint32 y = 0 ; y < lines; y++) {

		uint8* srcHandle = src + extend * 4;
		uint8* dstHandle = dst;

 		for (uint32 x = 0; x < pixels; x++) {

			red = green = blue = alpha = 0;
			for (uint32 i = 0; i < count; i++) {
				int32 offset = deltas[i].dx * 4 + deltas[i].dy * srcBPR;
				blue	+= srcHandle[0 + offset];
				green	+= srcHandle[1 + offset];
				red		+= srcHandle[2 + offset];
				alpha	+= srcHandle[3 + offset];
			}
 			dstHandle[0] = (uint8)(blue / count);
 			dstHandle[1] = (uint8)(green / count);
 			dstHandle[2] = (uint8)(red / count);
 			dstHandle[3] = (uint8)(alpha / count);

			srcHandle += 4;
			dstHandle += 4;
 		}
		src += srcBPR;
		dst += dstBPR;
 	}		
}


