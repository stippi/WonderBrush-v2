// Dispersion.cpp

#include <math.h>
#include <new>
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include <Bitmap.h>
#include <Message.h>

#include "support.h"

#include "CommonPropertyIDs.h"
#include "FilterFactory.h"
#include "FloatProperty.h"
#include "ImageProcessingLibrary.h"
#include "LanguageManager.h"
#include "PropertyObject.h"
#include "RandomNumberGenerator.h"

#include "Dispersion.h"

// constructor
Dispersion::Dispersion(float radius)
	: FilterObject(FILTER_DISPERSION),
	  fRadius(radius),
	  fRandomSeed(rand())
{
}

// copy constructor
Dispersion::Dispersion(const Dispersion& other)
	: FilterObject(other),
	  fRadius(other.fRadius),
	  fRandomSeed(other.fRandomSeed)
{
}

// BArchivable constructor
Dispersion::Dispersion(BMessage* archive)
	: FilterObject(archive),
	  fRadius(5.0),
	  fRandomSeed(rand())
{
	if (archive) {
		if (archive->FindFloat("blur radius", &fRadius) < B_OK)
			fRadius = 5.0;
		if (archive->FindInt32("random seed", &fRandomSeed) < B_OK)
			fRandomSeed = rand();
	} else {
		SetFilterID(FILTER_DISPERSION);
	}
}

// destructor
Dispersion::~Dispersion()
{
}

// Clone
Stroke*
Dispersion::Clone() const
{
	return new Dispersion(*this);
}

// SetTo
bool
Dispersion::SetTo(const Stroke* from)
{
	const Dispersion* dispersion = dynamic_cast<const Dispersion*>(from);

	AutoNotificationSuspender _(this);

	if (dispersion && FilterObject::SetTo(from)) {
		fRadius = dispersion->fRadius;
		fRandomSeed = dispersion->fRandomSeed;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
Dispersion::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "Dispersion"))
		return new Dispersion(archive);
	return NULL;
}

// Archive
status_t
Dispersion::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddFloat("blur radius", fRadius);

	if (status >= B_OK)
		status = into->AddInt32("random seed", fRandomSeed);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "Dispersion");

	return status;
}

// ExtendRebuildArea
void
Dispersion::ExtendRebuildArea(BRect& area, BRect layerBounds) const
{
	area = layerBounds;
}

// ProcessBitmap
void
Dispersion::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	if (dest && dest->IsValid()
		&& area.IsValid() && area.Intersects(dest->Bounds())) {
		// constrain rect to passed bitmap bounds
		area = area & dest->Bounds();

		BBitmap* original = new(std::nothrow) BBitmap(dest);
		if (original && original->IsValid()) {

			int32 blurRadius = (int32)(fRadius * scale());

			int32 left, top, right, bottom;
			rect_to_int(area, left, top, right, bottom);

			uint8 *dst = (uint8*)dest->Bits();
			uint8 *src = (uint8*)original->Bits();

			uint32 bpr = original->BytesPerRow();

			int32 dx, dy;

			src += top * bpr + left * 4;

			RandomNumberGenerator g(fRandomSeed, 0);

			for (int32 y = top; y <= bottom; y++) {
				uint32* srcHandle = (uint32*)src;
				for (int32 x = left; x <= right; x++) {

					dx = x + g.IntegerUniformDistribution(-blurRadius, blurRadius);
					dy = y + g.IntegerUniformDistribution(-blurRadius, blurRadius);

					if (dx >= left && dx <= right && dy >= top && dy <= bottom)
						(*(uint32*)&dst[dx * 4 + dy * bpr]) = *srcHandle;
					srcHandle++;
				}
				src += bpr;
			}
		} else {
fprintf(stderr, "Dispersion::ProcessBitmap() - not enough memory to create temporary bitmap!");
		}
		delete original;
	}
}

// MakePropertyObject
PropertyObject*
Dispersion::MakePropertyObject() const
{
	PropertyObject* object = new PropertyObject();
	object->AddProperty(new FloatProperty("radius",
										  PROPERTY_BLUR_RADIUS,
										  fRadius,
										  0.0, 100.0));
	return object;
}

// SetToPropertyObject
bool
Dispersion::SetToPropertyObject(PropertyObject* object)
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

// Randomize
void
Dispersion::Randomize()
{
	fRandomSeed = rand();
}

