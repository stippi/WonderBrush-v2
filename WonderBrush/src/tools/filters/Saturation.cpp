// Saturation.cpp

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

#include "Saturation.h"

// constructor
Saturation::Saturation()
	: FilterObject(FILTER_SATURATION),
	  fSaturation(1.0)
{
}

// copy constructor
Saturation::Saturation(const Saturation& other)
	: FilterObject(other),
	  fSaturation(other.fSaturation)
{
}

// BArchivable constructor
Saturation::Saturation(BMessage* archive)
	: FilterObject(archive),
	  fSaturation(1.0)
{
	if (!archive || archive->FindFloat("saturation", &fSaturation) < B_OK) {
		fSaturation = 1.0;
		SetFilterID(FILTER_SATURATION);
	}
}

// destructor
Saturation::~Saturation()
{
}

// Clone
Stroke*
Saturation::Clone() const
{
	return new Saturation(*this);
}

// SetTo
bool
Saturation::SetTo(const Stroke* from)
{
	const Saturation* simpleBrightness = dynamic_cast<const Saturation*>(from);

	AutoNotificationSuspender _(this);

	if (simpleBrightness && FilterObject::SetTo(from)) {
		fSaturation = simpleBrightness->fSaturation;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
Saturation::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "Saturation"))
		return new Saturation(archive);
	return NULL;
}

// Archive
status_t
Saturation::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddFloat("saturation", fSaturation);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "Saturation");

	return status;
}

// ProcessBitmap
void
Saturation::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	// Code taken from ArtPaints Saturation add-on, with huge speed increase added
	// This function interpolates the image with a degenerate version,
	// which in this case is the luminance image. The luminance image
	// is in bitmap luminance_image, which is in B_CMAP8 color-space.  
	if (dest && dest->IsValid() && area.IsValid() && dest->Bounds().Intersects(area)) {
		area = dest->Bounds() & area;

//bigtime_t lum = system_time();
		if (BBitmap* luminance = _LuminanceBitmap(dest)) {
//bigtime_t now = system_time();
			uint8 *bits = (uint8*)dest->Bits();
			uint32 bpr = dest->BytesPerRow();
		
			uint8 *lumBits = (uint8*)luminance->Bits();
			uint32 lumBPR = luminance->BytesPerRow();
			
			int32 left, right, y, bottom;
			rect_to_int(area, left, y, right, bottom);
		
			bits += left * 4 + bpr * y;
			lumBits += left + lumBPR * y;
		
			float coeff = fSaturation;	
			float oneMinusCoeff = 1.0 - coeff;

			uint32 width = right - left + 1;
			uint32 height = bottom - y + 1;

			if (width * height > 80000) {
//printf("LUT version\n");
				uint8 table[256][256];
				for (int32 i = 0; i < 256; i++) {
					for (int32 j = 0; j < 256; j++) {
						table[i][j] = constrain_int32_0_255((int32)(i * coeff + j * oneMinusCoeff));
					}
				}
				for (; y <= bottom; y++) {
					uint8* bitsHandle = bits;
					uint8* lumHandle = lumBits;
	
					for (int32 x = left; x <= right; x++) {
						bitsHandle[0] = table[bitsHandle[0]][*lumHandle];
						bitsHandle[1] = table[bitsHandle[1]][*lumHandle];
						bitsHandle[2] = table[bitsHandle[2]][*lumHandle];
						bitsHandle += 4;
						lumHandle ++;
					}
					bits += bpr;
					lumBits += lumBPR;
				}
			} else {
				for (; y <= bottom; y++) {
					uint8* bitsHandle = bits;
					uint8* lumHandle = lumBits;
	
					for (int32 x = left; x <= right; x++) {
						bitsHandle[0] = constrain_int32_0_255((int32)(bitsHandle[0] * coeff
															+ *lumHandle * oneMinusCoeff));
						bitsHandle[1] = constrain_int32_0_255((int32)(bitsHandle[1] * coeff
															+ *lumHandle * oneMinusCoeff));
						bitsHandle[2] = constrain_int32_0_255((int32)(bitsHandle[2] * coeff
															+ *lumHandle * oneMinusCoeff));
	
						bitsHandle += 4;
						lumHandle ++;
					}
					bits += bpr;
					lumBits += lumBPR;
				}
			}
//printf("luminance: %lld µsecs\n", now - lum);
//printf("saturation: %lld µsecs\n", system_time() - now);
			delete luminance;
		}
	}
}

// MakePropertyObject
PropertyObject*
Saturation::MakePropertyObject() const
{
	PropertyObject* object = new PropertyObject();
	object->AddProperty(new FloatProperty("saturation",
										  PROPERTY_SATURATION,
										  fSaturation,
										  0.0, 4.0));
	return object;
}

// SetToPropertyObject
bool
Saturation::SetToPropertyObject(PropertyObject* object)
{
	bool ret = false;
	if (object) {
		float f = object->FindFloatProperty(PROPERTY_SATURATION, fSaturation);
		if (f != fSaturation)
			ret = true;
		fSaturation = f;
		if (ret) {
			SaveSettings();
			Notify();
		}
	}
	return ret;
}

// _LuminanceBitmap
BBitmap*
Saturation::_LuminanceBitmap(const BBitmap* bitmap) const
{
	// code taken from ArtPaint "Saturation" add-on, but reworded
	BBitmap* luminance = NULL;
	if (bitmap && bitmap->IsValid()) {

		luminance = new BBitmap(bitmap->Bounds(), 0, B_GRAY8);
		if (luminance && luminance->IsValid()) {

			color_space format = bitmap->ColorSpace();
	
			uint8* bits = (uint8*)bitmap->Bits();
			uint32 bpr = bitmap->BytesPerRow();
			
			uint32 width = bitmap->Bounds().IntegerWidth() + 1;
			uint32 height = bitmap->Bounds().IntegerHeight() + 1;

			uint8* lumBits = (uint8*)luminance->Bits();
			uint32 lumBPR = luminance->BytesPerRow();

			if (format == B_RGBA32 || format == B_RGB32) {

//				float lum;
				int32 lum;
				for (uint32 y = 0; y < height; y++) {
					uint8* handle = bits;
					uint8* lumHandle = lumBits;
					for (uint32 x = 0; x < width; x++) {
//						lum =  0.144 * handle[0];	// B
//						lum += 0.587 * handle[1];	// G
//						lum += 0.299 * handle[2];	// R
/*						lum =  11 * handle[0];	// B
						lum += 59 * handle[1];	// G
						lum += 30 * handle[2];	// R
						*lumHandle = (uint8)(lum / 100);*/
						lum =  28 * handle[0];	// B
						lum += 151 * handle[1];	// G
						lum += 77 * handle[2];	// R
						*lumHandle = (uint8)(lum >> 8);
						handle += 4;
						lumHandle ++;
					}
					bits += bpr;
					lumBits += lumBPR;
				}
			} else if (format == B_GRAY8) {
	
				for (uint32 y = 0; y < height; y++) {
					uint8* handle = bits;
					uint8* lumHandle = lumBits;
					for (uint32 x = 0; x < width; x++) {
						*lumHandle = *handle;
						handle ++;
						lumHandle ++;
					}
					bits += bpr;
					lumBits += lumBPR;
				}
			}
		} else {
			delete luminance;
			luminance = NULL;
		}
	}
	return luminance;
}


