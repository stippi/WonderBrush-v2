// SimpleBrightness.cpp

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

#include "CommonPropertyIDs.h"
#include "FilterFactory.h"
#include "FloatProperty.h"
#include "PropertyObject.h"

#include "SimpleBrightness.h"

// constructor
SimpleBrightness::SimpleBrightness()
	: FilterObject(FILTER_SIMPLE_BRIGHTNESS),
	  fBrightness(1.0)
{
}

// copy constructor
SimpleBrightness::SimpleBrightness(const SimpleBrightness& other)
	: FilterObject(other),
	  fBrightness(other.fBrightness)
{
}

// BArchivable constructor
SimpleBrightness::SimpleBrightness(BMessage* archive)
	: FilterObject(archive),
	  fBrightness(1.0)
{
	if (!archive || archive->FindFloat("brightness", &fBrightness) < B_OK) {
		fBrightness = 1.0;
		SetFilterID(FILTER_SIMPLE_BRIGHTNESS);
	}
}

// destructor
SimpleBrightness::~SimpleBrightness()
{
}

// Clone
Stroke*
SimpleBrightness::Clone() const
{
	return new SimpleBrightness(*this);
}

// SetTo
bool
SimpleBrightness::SetTo(const Stroke* from)
{
	const SimpleBrightness* simpleBrightness = dynamic_cast<const SimpleBrightness*>(from);

	AutoNotificationSuspender _(this);

	if (simpleBrightness && FilterObject::SetTo(from)) {
		fBrightness = simpleBrightness->fBrightness;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
SimpleBrightness::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "SimpleBrightness"))
		return new SimpleBrightness(archive);
	return NULL;
}

// Archive
status_t
SimpleBrightness::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddFloat("brightness", fBrightness);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "SimpleBrightness");

	return status;
}

// ProcessBitmap
void
SimpleBrightness::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	uint8 table[256];
	for (int32 i = 0; i < 256; i++) {
		table[i] = min_c(255, (uint32)floorf(i * fBrightness + 0.5));
	}
	remap_colors(dest, area, table);
}

// MakePropertyObject
PropertyObject*
SimpleBrightness::MakePropertyObject() const
{
	PropertyObject* object = new PropertyObject();
	object->AddProperty(new FloatProperty("brightness",
										  PROPERTY_BRIGHTNESS,
										  fBrightness,
										  0.0, 255.0));
	return object;
}

// SetToPropertyObject
bool
SimpleBrightness::SetToPropertyObject(PropertyObject* object)
{
	bool ret = false;
	if (object) {
		float f = object->FindFloatProperty(PROPERTY_BRIGHTNESS, fBrightness);
		if (f != fBrightness)
			ret = true;
		fBrightness = f;
		if (ret) {
			SaveSettings();
			Notify();
		}
	}
	return ret;
}



