// SolidColor.cpp

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
#include "IntProperty.h"
#include "PropertyObject.h"

#include "SolidColor.h"

// constructor
SolidColor::SolidColor()
	: FilterObject(FILTER_SOLID_COLOR),
	  fAlpha(255)
{
	SetMode(MODE_BRUSH);
}

// copy constructor
SolidColor::SolidColor(const SolidColor& other)
	: FilterObject(other),
	  fAlpha(other.fAlpha)
{
}

// BArchivable constructor
SolidColor::SolidColor(BMessage* archive)
	: FilterObject(archive),
	  fAlpha(255)
{
	SetMode(MODE_BRUSH);
	if (!archive || archive->FindInt8("opacity", (int8*)&fAlpha) < B_OK) {
		SetFilterID(FILTER_SOLID_COLOR);
		fAlpha = 255;
	}
}

// destructor
SolidColor::~SolidColor()
{
}

// Clone
Stroke*
SolidColor::Clone() const
{
	return new SolidColor(*this);
}

// SetTo
bool
SolidColor::SetTo(const Stroke* from)
{
	const SolidColor* solidColor = dynamic_cast<const SolidColor*>(from);

	AutoNotificationSuspender _(this);

	if (solidColor && FilterObject::SetTo(from)) {
		if (fAlpha != solidColor->fAlpha) {
			fAlpha = solidColor->fAlpha;
			Notify();
		}
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
SolidColor::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "SolidColor"))
		return new SolidColor(archive);
	return NULL;
}

// Archive
status_t
SolidColor::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddInt8("opacity", fAlpha);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "SolidColor");

	return status;
}

// LayerBoundsChanged
void
SolidColor::LayerBoundsChanged(const BRect& layerBounds)
{
	fLayerBounds = layerBounds;
	fLayerBounds.right++;
	fLayerBounds.bottom++;
}

// Bounds
BRect
SolidColor::Bounds() const
{
	if (fLayerBounds.IsValid())
		return fLayerBounds;
	return Stroke::Bounds();
}

// ExtendRebuildArea
void
SolidColor::ExtendRebuildArea(BRect& area, BRect layerBounds) const
{
	fLayerBounds = layerBounds;
	fLayerBounds.right++;
	fLayerBounds.bottom++;
}

// Draw
void
SolidColor::Draw(BBitmap* bitmap)
{
	Draw(bitmap, bitmap->Bounds());
}

// Draw
void
SolidColor::Draw(BBitmap* bitmap, BRect constrainRect)
{
	clear_area(bitmap, constrainRect, fAlpha);
}

// ProcessBitmap
void
SolidColor::ProcessBitmap(BBitmap* bitmap, BBitmap* alphaBitmap, BRect constrainRect) const
{
	// not used
}

// MergeWithBitmap
void
SolidColor::MergeWithBitmap(BBitmap* dest, BBitmap* strokeBitmap,
							BRect area, uint32 colorSpace) const
{
	Stroke::MergeWithBitmap(dest, strokeBitmap, area, colorSpace);
}

// MergeWithBitmap
void
SolidColor::MergeWithBitmap(BBitmap* from, BBitmap* dest,
							BBitmap* strokeBitmap, BRect area, uint32 colorSpace) const
{
	Stroke::MergeWithBitmap(from, dest, strokeBitmap, area, colorSpace);
}

// MakePropertyObject
PropertyObject*
SolidColor::MakePropertyObject() const
{
	PropertyObject* object = Stroke::MakePropertyObject();
	if (object) {
		// opacity
		object->AddProperty(new IntProperty("opacity",
											PROPERTY_OPACITY,
											fAlpha));
	}
	return object;
}

// SetToPropertyObject
bool
SolidColor::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	bool ret = Stroke::SetToPropertyObject(object);
	if (object) {
		// opacity
		int32 i = object->FindIntProperty(PROPERTY_OPACITY, fAlpha);
		if (i != fAlpha)
			ret = true;
		fAlpha = i;
	}

	return ret;
}



