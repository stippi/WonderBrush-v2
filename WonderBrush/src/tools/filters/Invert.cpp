// Invert.cpp

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

#include "LanguageManager.h"
#include "FilterFactory.h"

#include "Invert.h"

// constructor
Invert::Invert()
	: FilterObject(FILTER_INVERT)
{
}

// copy constructor
Invert::Invert(const Invert& other)
	: FilterObject(other)
{
}

// BArchivable constructor
Invert::Invert(BMessage* archive)
	: FilterObject(archive)
{
}

// destructor
Invert::~Invert()
{
}

// Clone
Stroke*
Invert::Clone() const
{
	return new Invert(*this);
}

// SetTo
bool
Invert::SetTo(const Stroke* from)
{
	return FilterObject::SetTo(from);
}

// Instantiate
BArchivable*
Invert::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "Invert"))
		return new Invert(archive);
	return NULL;
}

// Archive
status_t
Invert::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "Invert");

	return status;
}

// ProcessBitmap
void
Invert::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	if (dest && dest->IsValid()
		&& area.IsValid() && area.Intersects(dest->Bounds())) {
		// constrain rect to passed bitmap bounds
		area = area & dest->Bounds();

		// handles and byte lengths
		uint8* dst = (uint8*)dest->Bits();
		uint32 dstBPR = dest->BytesPerRow();

		int32 left, top, right, bottom;
		rect_to_int(area, left, top, right, bottom);

		// offsets into bitmaps
		dst += left * 4 + top * dstBPR;

		// invert the color channels of the bitmap
		for (int32 y = top; y <= bottom; y++) {

			uint8* dstHandle = dst;

			for (int32 x = left; x <= right; x++) {

				dstHandle[0] = 255 - dstHandle[0];
				dstHandle[1] = 255 - dstHandle[1];
				dstHandle[2] = 255 - dstHandle[2];

				dstHandle += 4;
			}
			dst += dstBPR;
		}
	}
}


