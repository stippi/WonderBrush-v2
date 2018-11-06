// Noise.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#include <Bitmap.h>
#include <Message.h>

#include "support.h"

#include "CommonPropertyIDs.h"
#include "BoolProperty.h"
#include "FilterFactory.h"
#include "IntProperty.h"
#include "LanguageManager.h"
#include "PropertyObject.h"
#include "RandomNumberGenerator.h"

#include "Noise.h"

// constructor
Noise::Noise(float radius)
	: FilterObject(FILTER_NOISE),
	  fLuminanceOnly(true),
	  fStrength(20),
	  fRandomSeed(rand())
{
}

// copy constructor
Noise::Noise(const Noise& other)
	: FilterObject(other),
	  fLuminanceOnly(other.fLuminanceOnly),
	  fStrength(other.fStrength),
	  fRandomSeed(other.fRandomSeed)
{
}

// BArchivable constructor
Noise::Noise(BMessage* archive)
	: FilterObject(archive),
	  fLuminanceOnly(true),
	  fStrength(20),
	  fRandomSeed(rand())
{
	if (archive) {
		if (archive->FindBool("luminance", &fLuminanceOnly) < B_OK)
			fLuminanceOnly = true;

		if (archive->FindInt32("strength", &fStrength) < B_OK)
			fStrength = 20;

		if (archive->FindInt32("random seed", &fRandomSeed) < B_OK)
			fRandomSeed = rand();
	} else {
		SetFilterID(FILTER_NOISE);
	}
}

// destructor
Noise::~Noise()
{
}

// Clone
Stroke*
Noise::Clone() const
{
	return new Noise(*this);
}

// SetTo
bool
Noise::SetTo(const Stroke* from)
{
	const Noise* dispersion = dynamic_cast<const Noise*>(from);

	AutoNotificationSuspender _(this);

	if (dispersion && FilterObject::SetTo(from)) {
		fLuminanceOnly = dispersion->fLuminanceOnly;
		fStrength = dispersion->fStrength;
		fRandomSeed = dispersion->fRandomSeed;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
Noise::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "Noise"))
		return new Noise(archive);
	return NULL;
}

// Archive
status_t
Noise::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddBool("luminance", fLuminanceOnly);

	if (status >= B_OK)
		status = into->AddInt32("strength", fStrength);

	if (status >= B_OK)
		status = into->AddInt32("random seed", fRandomSeed);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "Noise");

	return status;
}

// ProcessBitmap
void
Noise::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	if (dest && dest->IsValid()
		&& area.IsValid() && area.Intersects(dest->Bounds())) {
		// constrain rect to passed bitmap bounds
		area = area & dest->Bounds();

		int32 left, top, right, bottom;
		rect_to_int(area, left, top, right, bottom);

		uint32 width = dest->Bounds().IntegerWidth() + 1;

		uint8 *bits = (uint8*)dest->Bits();
		uint32 bpr = dest->BytesPerRow();
	
		bits += top * bpr + left * 4;

		RandomNumberGenerator g(fRandomSeed, 4096);

		if (fLuminanceOnly) {
			int8 d;
			g.SkipIntegers(top * width);
			for (int32 y = top; y <= bottom; y++) {
				g.SkipIntegers(left);
				uint8* handle = bits;
				for (int32 x = left; x <= right; x++) {
	
					d = g.IntegerUniformDistribution(-fStrength, fStrength);
	
					handle[0] = constrain_int32_0_255(handle[0] + d);
					handle[1] = constrain_int32_0_255(handle[1] + d);
					handle[2] = constrain_int32_0_255(handle[2] + d);
	
					handle += 4;
				}
				g.SkipIntegers(width - (right + 1));
				bits += bpr;
			}
		} else {
			g.SkipIntegers((top * width) * 3);
			for (int32 y = top; y <= bottom; y++) {
				uint8* handle = bits;
				g.SkipIntegers(left * 3);
				for (int32 x = left; x <= right; x++) {
					handle[0] = constrain_int32_0_255(handle[0] + g.IntegerUniformDistribution(-fStrength, fStrength));
					handle[1] = constrain_int32_0_255(handle[1] + g.IntegerUniformDistribution(-fStrength, fStrength));
					handle[2] = constrain_int32_0_255(handle[2] + g.IntegerUniformDistribution(-fStrength, fStrength));
	
					handle += 4;
				}
				g.SkipIntegers((width - (right + 1)) * 3);
				bits += bpr;
			}
		}
	}
}

// MakePropertyObject
PropertyObject*
Noise::MakePropertyObject() const
{
	PropertyObject* object = new PropertyObject();

	object->AddProperty(new IntProperty("Strength",
										PROPERTY_STRENGTH,
										fStrength,
										0, 255));

	object->AddProperty(new BoolProperty("luminance",
										  PROPERTY_LUMINANCE_ONLY,
										  fLuminanceOnly));
	return object;
}

// SetToPropertyObject
bool
Noise::SetToPropertyObject(PropertyObject* object)
{
	bool ret = false;
	if (object) {
		int32 i = object->FindIntProperty(PROPERTY_STRENGTH, fStrength);
		if (i != fStrength)
			ret = true;
		fStrength = i;

		bool b = object->FindBoolProperty(PROPERTY_LUMINANCE_ONLY, fLuminanceOnly);
		if (b != fLuminanceOnly)
			ret = true;
		fLuminanceOnly = b;

		if (ret) {
			SaveSettings();
			Notify();
		}
	}
	return ret;
}

// Randomize
void
Noise::Randomize()
{
	fRandomSeed = rand();
}

