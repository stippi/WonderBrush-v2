// BlurStroke.cpp

#include <stdio.h>

#include <Message.h>

#include "bitmap_support.h"

#include "Brush.h"
#include "CommonPropertyIDs.h"
#include "FloatProperty.h"
#include "LanguageManager.h"
#include "PropertyObject.h"
#include "Strings.h"

#include "BlurStroke.h"

// constructor
BlurStroke::BlurStroke(range radius, range hardness,
					   range alpha, range spacing,
					   uint32 flags, rgb_color color,
					   float blurRadius)
	: BrushStroke(radius, hardness, alpha, spacing,
				  flags, color, MODE_BLUR),
	  fBlurRadius(blurRadius)
{
}

// copy constructor
BlurStroke::BlurStroke(const BlurStroke& other)
	: BrushStroke(other),
	  fBlurRadius(other.fBlurRadius)
{
}

// BArchivable constructor
BlurStroke::BlurStroke(BMessage* archive)
	: BrushStroke(archive)
{
	// restore control alpha flag
	if (archive->FindFloat("blur radius", &fBlurRadius) < B_OK) {
		fBlurRadius = 5.0;
	}
}

// destructor
BlurStroke::~BlurStroke()
{
}

// Clone
Stroke*
BlurStroke::Clone() const
{
	return new BlurStroke(*this);
}

// SetTo
bool
BlurStroke::SetTo(const Stroke* from)
{
	const BlurStroke* blurStroke = dynamic_cast<const BlurStroke*>(from);

	AutoNotificationSuspender _(this);

	if (blurStroke && BrushStroke::SetTo(from)) {
		fBlurRadius = blurStroke->fBlurRadius;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
BlurStroke::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "BlurStroke"))
		return new BlurStroke(archive);
	return NULL;
}

// Archive
status_t
BlurStroke::Archive(BMessage* into, bool deep) const
{
	status_t status = BrushStroke::Archive(into, deep);

	// add our data to the message
	if (status >= B_OK)
		// store control alpha flag
		status = into->AddFloat("blur radius", fBlurRadius);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "BlurStroke");

	return status;
}

// ExtendRebuildArea
void
BlurStroke::ExtendRebuildArea(BRect& area, BRect layerBounds) const
{
	BRect bounds = Bounds();
	// this rect contains all the pixels necessary to do
	// the blur effect
	bounds.InsetBy(-fBlurRadius * scale(), -fBlurRadius * scale());
	// if this touches area, include all
	if (area.Intersects(bounds)) {
		area = area | bounds;
	}

// old code, see CloneStroke.cpp for why this is flawed...

/*	BRect bounds = Bounds();
	// extend area by the blur radius
	// we thereby get the region that the pixels
	// in the area influence (concerning our blur radius)
	// if the computed area touches our bounds,
	// the rebuild area needs to also include the common part
	BRect blurInfluenceArea = area;
	blurInfluenceArea.InsetBy(-fBlurRadius, -fBlurRadius);
	if (bounds.Intersects(blurInfluenceArea)) {
		BRect commonArea = bounds & blurInfluenceArea;
		area = area | commonArea;
	}*/
}

// MergeWithBitmap
void
BlurStroke::MergeWithBitmap(BBitmap* dest, BBitmap* strokeBitmap,
						BRect area, uint32 colorSpace) const
{
	blur_bitmap(dest, strokeBitmap, area, fBlurRadius * scale());
}

// MergeWithBitmap
void
BlurStroke::MergeWithBitmap(BBitmap* from, BBitmap* dest,
						BBitmap* strokeBitmap, BRect area, uint32 colorSpace) const
{
	blur_bitmap_copy(from, dest, strokeBitmap, area, fBlurRadius * scale());
}

// MakePropertyObject
PropertyObject*
BlurStroke::MakePropertyObject() const
{
	PropertyObject* object = BrushStroke::MakePropertyObject();
	if (object) {
		object->AddProperty(new FloatProperty("radius",
											  PROPERTY_BLUR_RADIUS,
											  fBlurRadius,
											  0.0, 100.0));
	}
	return object;
}

// SetToPropertyObject
bool
BlurStroke::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	bool ret = BrushStroke::SetToPropertyObject(object);
	if (object) {
		float f = object->FindFloatProperty(PROPERTY_BLUR_RADIUS, fBlurRadius);
		if (f != fBlurRadius)
			ret = true;
		fBlurRadius = f;

		if (ret)
			Notify();
	}
	return ret;
}

// Name
const char*
BlurStroke::Name() const
{
	LanguageManager* manager = LanguageManager::Default();
	const char* name = manager->GetString(BLUR, "Blur");
	return name;
}

// ToolID
int32
BlurStroke::ToolID() const
{
	return TOOL_BLUR;
}
