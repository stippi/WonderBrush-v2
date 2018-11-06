// Contrast.cpp

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

#include "Contrast.h"

// constructor
Contrast::Contrast()
	: FilterObject(FILTER_CONTRAST),
	  fContrast(1.0)
{
}

// copy constructor
Contrast::Contrast(const Contrast& other)
	: FilterObject(other),
	  fContrast(other.fContrast)
{
}

// BArchivable constructor
Contrast::Contrast(BMessage* archive)
	: FilterObject(archive),
	  fContrast(1.0)
{
	if (!archive || archive->FindFloat("contrast", &fContrast) < B_OK) {
		fContrast = 1.0;
		SetFilterID(FILTER_CONTRAST);
	}
}

// destructor
Contrast::~Contrast()
{
}

// Clone
Stroke*
Contrast::Clone() const
{
	return new Contrast(*this);
}

// SetTo
bool
Contrast::SetTo(const Stroke* from)
{
	const Contrast* simpleBrightness = dynamic_cast<const Contrast*>(from);

	AutoNotificationSuspender _(this);

	if (simpleBrightness && FilterObject::SetTo(from)) {
		fContrast = simpleBrightness->fContrast;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
Contrast::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "Contrast"))
		return new Contrast(archive);
	return NULL;
}

// Archive
status_t
Contrast::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddFloat("contrast", fContrast);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "Contrast");

	return status;
}

// ProcessBitmap
void
Contrast::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	// Code taken from ArtPaint, but highly simplified; original comment:
	// This function interpolates the image with a degenerate version,
	// which in this case is the average luminance. The luminance image is not actually
	// used, but only implied. This function does not touch the alpha-channel.

	float coeff = fContrast;	
	float averageLuminance = _AverageLuminance(dest);
	int32 luminanceFactor = (int32)(averageLuminance * (1.0 - coeff));

	uint8 luminanceValues[256];
	for (int32 i = 0; i < 256; i++) {
		luminanceValues[i] = max_c(0, min_c(255, (int32)(i * coeff) + luminanceFactor));
	}

	remap_colors(dest, area, luminanceValues);
}

// MakePropertyObject
PropertyObject*
Contrast::MakePropertyObject() const
{
	PropertyObject* object = new PropertyObject();
	object->AddProperty(new FloatProperty("contrast",
										  PROPERTY_CONTRAST,
										  fContrast,
										  0.0, 255.0));
	return object;
}

// SetToPropertyObject
bool
Contrast::SetToPropertyObject(PropertyObject* object)
{
	bool ret = false;
	if (object) {
		float f = object->FindFloatProperty(PROPERTY_CONTRAST, fContrast);
		if (f != fContrast)
			ret = true;
		fContrast = f;
		if (ret) {
			SaveSettings();
			Notify();
		}
	}
	return ret;
}

// _AverageLuminance
uint8
Contrast::_AverageLuminance(const BBitmap* bitmap) const
{
	// code taken from ArtPaint "Contrast" add-on, but reworded and fixed
	// (it had the luminance coefficients swapped for red and blue ?!?)
	// also works for gray bitmaps now
	// TODO: maybe it is worth the effort to ignor pixels with alpha = 0
	// in this case, pixelCount is taken from the pixels actually considered
	uint8 average = 0;
	if (bitmap && bitmap->IsValid()) {

		color_space format = bitmap->ColorSpace();

		uint8* bits = (uint8*)bitmap->Bits();
		uint32 bpr = bitmap->BytesPerRow();
		
//		double luminanceSum = 0.0;
		int64 luminanceSum = 0;

		uint32 width = bitmap->Bounds().IntegerWidth() + 1;
		uint32 height = bitmap->Bounds().IntegerHeight() + 1;
		uint32 pixelCount = width * height;

		if (format == B_RGBA32 || format == B_RGB32) {

			for (uint32 y = 0; y < height; y++) {
				uint8* handle = bits;
				for (uint32 x = 0; x < width; x++) {
/*					luminanceSum += 0.144 * handle[0];	// B
					luminanceSum += 0.587 * handle[1];	// G
					luminanceSum += 0.299 * handle[2];	// R*/
					luminanceSum += 28 * handle[0];		// B
					luminanceSum += 151 * handle[1];	// G
					luminanceSum += 77 * handle[2];		// R
					handle += 4;
				}
				bits += bpr;
			}
		} else if (format == B_GRAY8) {

			for (uint32 y = 0; y < height; y++) {
				uint8* handle = bits;
				for (uint32 x = 0; x < width; x++) {
//					luminanceSum += handle[0];
					luminanceSum += handle[0] << 8;
					handle ++;
				}
				bits += bpr;
			}
		}

//		luminanceSum /= pixelCount;
		luminanceSum = (luminanceSum >> 8) / pixelCount;
	
		average = (uint8)luminanceSum;
	}
	return average;
}


