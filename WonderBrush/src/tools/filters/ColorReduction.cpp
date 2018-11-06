// ColorReduction.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Message.h>
#include <Screen.h>

#include "bitmap_compression.h"
#include "bitmap_support.h"
#include "blending.h"
#include "defines.h"
#include "support.h"

//#include "CommonPropertyIDs.h"
#include "FilterFactory.h"
//#include "FloatProperty.h"
//#include "PropertyObject.h"

#include "ColorReduction.h"

// constructor
ColorReduction::ColorReduction()
	: FilterObject(FILTER_COLOR_REDUCTION)
{
}

// copy constructor
ColorReduction::ColorReduction(const ColorReduction& other)
	: FilterObject(other)
{
}

// BArchivable constructor
ColorReduction::ColorReduction(BMessage* archive)
	: FilterObject(archive)
{
	if (!archive) {
		SetFilterID(FILTER_COLOR_REDUCTION);
	}
}

// destructor
ColorReduction::~ColorReduction()
{
}

// Clone
Stroke*
ColorReduction::Clone() const
{
	return new ColorReduction(*this);
}

// SetTo
bool
ColorReduction::SetTo(const Stroke* from)
{
	const ColorReduction* cr = dynamic_cast<const ColorReduction*>(from);

	AutoNotificationSuspender _(this);

	if (cr && FilterObject::SetTo(from)) {
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
ColorReduction::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "ColorReduction"))
		return new ColorReduction(archive);
	return NULL;
}

// Archive
status_t
ColorReduction::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "ColorReduction");

	return status;
}

// ProcessBitmap
void
ColorReduction::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	if (!area.IsValid() || !area.Intersects(dest->Bounds()))
		return;

	area = area & dest->Bounds();

	int32 left, y, right, bottom;
	rect_to_int(area, left, y, right, bottom);

	uint8* bits = (uint8*)dest->Bits();
	uint32 bpr = dest->BytesPerRow();

	bits += y * bpr + left * 4;

	BScreen screen;

	for (; y <= bottom; y++) {
		uint8* d = bits;
		for (int32 x = left; x <= right; x++) {
			int32 index = screen.IndexForColor(d[2], d[1], d[0]);
			rgb_color c = screen.ColorForIndex(index);
			d[2] = c.red;
			d[1] = c.green;
			d[0] = c.blue;
			d += 4;
		}
		bits += bpr;
	}
}

//// MakePropertyObject
//PropertyObject*
//ColorReduction::MakePropertyObject() const
//{
//	PropertyObject* object = new PropertyObject();
//	object->AddProperty(new FloatProperty("contrast",
//										  PROPERTY_CONTRAST,
//										  fContrast,
//										  0.0, 255.0));
//	return object;
//}
//
//// SetToPropertyObject
//bool
//ColorReduction::SetToPropertyObject(PropertyObject* object)
//{
//	bool ret = false;
//	if (object) {
//		float f = object->FindFloatProperty(PROPERTY_CONTRAST, fContrast);
//		if (f != fContrast)
//			ret = true;
//		fContrast = f;
//		if (ret) {
//			SaveSettings();
//			Notify();
//		}
//	}
//	return ret;
//}

