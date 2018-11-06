// Warpsharp.cpp

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

// libwarpsharp includes
#include <MapImage.h>
#include <RGB8Image.h>
#include <WarpManager.h>

#include "Warpsharp.h"

// constructor
Warpsharp::Warpsharp()
	: FilterObject(FILTER_WARPSHARP),
	  fLambda(10.0),
	  fMu(0.01),
	  fNonMaximalSuppression(false)
{
}

// copy constructor
Warpsharp::Warpsharp(const Warpsharp& other)
	: FilterObject(other),
	  fLambda(other.fLambda),
	  fMu(other.fMu),
	  fNonMaximalSuppression(other.fNonMaximalSuppression)
{
}

// BArchivable constructor
Warpsharp::Warpsharp(BMessage* archive)
	: FilterObject(archive),
	  fLambda(10.0),
	  fMu(0.01),
	  fNonMaximalSuppression(false)
{
	if (archive) {
		if (archive->FindFloat("lambda", &fLambda) < B_OK)
			fLambda = 10.0;
		if (archive->FindFloat("mu", &fMu) < B_OK)
			fMu = 0.01;
		if (archive->FindBool("nms", &fNonMaximalSuppression) < B_OK)
			fNonMaximalSuppression = false;
	} else {
		SetFilterID(FILTER_WARPSHARP);
	}
}

// destructor
Warpsharp::~Warpsharp()
{
}

// Clone
Stroke*
Warpsharp::Clone() const
{
	return new Warpsharp(*this);
}

// SetTo
bool
Warpsharp::SetTo(const Stroke* from)
{
	const Warpsharp* warpsharp = dynamic_cast<const Warpsharp*>(from);

	AutoNotificationSuspender _(this);

	if (warpsharp && FilterObject::SetTo(from)) {
		fLambda = warpsharp->fLambda;
		fMu = warpsharp->fMu;
		fNonMaximalSuppression = warpsharp->fNonMaximalSuppression;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
Warpsharp::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "Warpsharp"))
		return new Warpsharp(archive);
	return NULL;
}

// Archive
status_t
Warpsharp::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddFloat("lambda", fLambda);

	if (status >= B_OK)
		status = into->AddFloat("mu", fMu);

	if (status >= B_OK)
		status = into->AddBool("nms", fNonMaximalSuppression);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "Warpsharp");

	return status;
}

// ExtendRebuildArea
void
Warpsharp::ExtendRebuildArea(BRect& area, BRect layerBounds) const
{
	area = layerBounds;
}

// ProcessBitmap
void
Warpsharp::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	// TODO: this function is broken if area != dest->Bounds()
	// (but it wouldn't make sense if we didn't blur the entire bitmap)
	if (dest && dest->IsValid()) {

/*		uint32 count;
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
		*/
		// don't try to sharp a homogen area
		bool homogenArea = true;
		uint8* bits = (uint8*)dest->Bits();
		uint32 pixel = *(uint32*)bits | 0xffffff00;
		uint32 width = dest->Bounds().IntegerWidth() + 1;
		uint32 height = dest->Bounds().IntegerHeight() + 1;
		uint32 bpr = dest->BytesPerRow();
		for (uint32 y = 0; homogenArea && y < height; y++) {
			uint32* b = (uint32*)bits;
			for (uint32 x = 0; x < width; x++) {
				if (pixel != (*b | 0xffffff00)) {
					homogenArea = false;
					break;
				}
				b++;
			}
			bits += bpr;
		}
		if (homogenArea)
			return;

		// convert dest to planar image
		RGB8Image input(dest);
		if (input.IsValid()) {
			WarpManager wm;

			// configure warpsharping
			wm.SetLambda(fLambda);
			wm.SetMu(fMu);
			wm.SetUseNonMaximalSupression(fNonMaximalSuppression);

			wm.SetInputImage(&input);

			RGB8Image* result = wm.GetWarpedImage();
			if (result && result->IsValid())
				result->CopyIntoBitmap(dest);

			delete result;
		}
	}
}

// MakePropertyObject
PropertyObject*
Warpsharp::MakePropertyObject() const
{
	PropertyObject* object = new PropertyObject();

	object->AddProperty(new FloatProperty("lambda",
										  PROPERTY_WARPSHARP_LAMBDA,
										  fLambda,
										  0.0, 30.0));
	object->AddProperty(new FloatProperty("mu",
										  PROPERTY_WARPSHARP_MU,
										  fMu,
										  0.0, 5.0));
	object->AddProperty(new BoolProperty("non-max suppression",
										  PROPERTY_WARPSHARP_NON_MAX_SUPR,
										  fNonMaximalSuppression));
	return object;
}

// SetToPropertyObject
bool
Warpsharp::SetToPropertyObject(PropertyObject* object)
{
	bool ret = false;
	if (object) {
		float f = object->FindFloatProperty(PROPERTY_WARPSHARP_LAMBDA, fLambda);
		if (f != fLambda)
			ret = true;
		fLambda = f;

		f = object->FindFloatProperty(PROPERTY_WARPSHARP_MU, fMu);
		if (f != fMu)
			ret = true;
		fMu = f;

		bool b = object->FindBoolProperty(PROPERTY_WARPSHARP_NON_MAX_SUPR, fNonMaximalSuppression);
		if (b != fNonMaximalSuppression)
			ret = true;
		fNonMaximalSuppression = b;


		if (ret) {
			SaveSettings();
			Notify();
		}
	}
	return ret;
}

// TransformationChanged
void
Warpsharp::TransformationChanged()
{
	// TODO: ...

	// undo any translation, we're only interested in rotation and scale
	tx = 0.0;
	ty = 0.0;

	// undo any perspective influence
	w0 = 0.0;
	w1 = 0.0;
	w2 = 1.0;

	Notify();
}

