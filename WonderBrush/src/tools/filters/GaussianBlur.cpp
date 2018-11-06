// GaussianBlur.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Message.h>

#include "bitmap_support.h"

#include "CommonPropertyIDs.h"
#include "FilterFactory.h"
#include "FloatProperty.h"
#include "ImageProcessingLibrary.h"
#include "GaussFilter.h"
#include "LanguageManager.h"
#include "PropertyObject.h"
#include "StackBlurFilter.h"

#include "GaussianBlur.h"

// constructor
GaussianBlur::GaussianBlur(float radius)
	: FilterObject(FILTER_GAUSSIAN_BLUR),
	  fRadius(radius)
{
}

// copy constructor
GaussianBlur::GaussianBlur(const GaussianBlur& other)
	: FilterObject(other),
	  fRadius(other.fRadius)
{
}

// BArchivable constructor
GaussianBlur::GaussianBlur(BMessage* archive)
	: FilterObject(archive),
	  fRadius(5.0)
{
	if (archive) {
		if (archive->FindFloat("blur radius", &fRadius) < B_OK)
			fRadius = 5.0;
	} else {
		SetFilterID(FILTER_GAUSSIAN_BLUR);
	}
}

// destructor
GaussianBlur::~GaussianBlur()
{
}

// Clone
Stroke*
GaussianBlur::Clone() const
{
	return new GaussianBlur(*this);
}

// SetTo
bool
GaussianBlur::SetTo(const Stroke* from)
{
	const GaussianBlur* gaussianBlur = dynamic_cast<const GaussianBlur*>(from);

	AutoNotificationSuspender _(this);

	if (gaussianBlur && FilterObject::SetTo(from)) {
		fRadius = gaussianBlur->fRadius;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
GaussianBlur::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "GaussianBlur"))
		return new GaussianBlur(archive);
	return NULL;
}

// Archive
status_t
GaussianBlur::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddFloat("blur radius", fRadius);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "GaussianBlur");

	return status;
}

// ExtendRebuildArea
void
GaussianBlur::ExtendRebuildArea(BRect& area, BRect layerBounds) const
{
	area = layerBounds;
}

// ProcessBitmap
void
GaussianBlur::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	if (dest && dest->IsValid()
		&& area.IsValid() && area.Intersects(dest->Bounds())) {
		// constrain rect to passed bitmap bounds
		area = area & dest->Bounds();

		float blurRadius = fRadius * scale();

bigtime_t now = system_time();
#if 1

// premultiplying alpha unfortunately necessary
multiply_alpha(dest);
//		GaussFilter filter;
		StackBlurFilter filter;
		filter.Filter(dest, blurRadius);

demultiply_alpha(dest);
#else
		ImageProcessingLibrary ip;
		ip.gaussian_blur(dest, blurRadius);
#endif
printf("gaussian blur (%.2f): %lld\n", blurRadius, system_time() - now);
	}
}

// MakePropertyObject
PropertyObject*
GaussianBlur::MakePropertyObject() const
{
	PropertyObject* object = new PropertyObject();
	object->AddProperty(new FloatProperty("radius",
										  PROPERTY_BLUR_RADIUS,
										  fRadius,
										  0.0, 1000.0));
	return object;
}

// SetToPropertyObject
bool
GaussianBlur::SetToPropertyObject(PropertyObject* object)
{
	bool ret = false;
	if (object) {
		float f = object->FindFloatProperty(PROPERTY_BLUR_RADIUS, fRadius);
		if (f != fRadius)
			ret = true;
		fRadius = f;
		if (ret) {
			SaveSettings();
			Notify();
		}
	}
	return ret;
}

