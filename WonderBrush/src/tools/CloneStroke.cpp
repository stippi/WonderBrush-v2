// CloneStroke.cpp

#include <stdio.h>

#include <Message.h>

#include "bitmap_support.h"

#include "Brush.h"
#include "CommonPropertyIDs.h"
#include "FloatProperty.h"
#include "LanguageManager.h"
#include "PropertyObject.h"
#include "Strings.h"

#include "CloneStroke.h"

// constructor
CloneStroke::CloneStroke(range radius, range hardness,
						 range alpha, range spacing,
						 uint32 flags, rgb_color color,
						 uint32 mode, BPoint offset)
	: BrushStroke(radius, hardness, alpha, spacing,
				  flags, color, mode),
	  fCloneOffset(offset)
{
}

// copy constructor
CloneStroke::CloneStroke(const CloneStroke& other)
	: BrushStroke(other),
	  fCloneOffset(other.fCloneOffset)
{
}

// BArchivable constructor
CloneStroke::CloneStroke(BMessage* archive)
	: BrushStroke(archive)
{
	// restore control alpha flag
	if (archive->FindPoint("clone offset", &fCloneOffset) != B_OK) {
		fCloneOffset.x = 0.0;
		fCloneOffset.y = 0.0;
	}
}

// destructor
CloneStroke::~CloneStroke()
{
}

// Clone
Stroke*
CloneStroke::Clone() const
{
	return new CloneStroke(*this);
}

// SetTo
bool
CloneStroke::SetTo(const Stroke* from)
{
	const CloneStroke* cloneStroke = dynamic_cast<const CloneStroke*>(from);

	AutoNotificationSuspender _(this);

	if (cloneStroke && BrushStroke::SetTo(from)) {
		fCloneOffset = cloneStroke->fCloneOffset;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
CloneStroke::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "CloneStroke"))
		return new CloneStroke(archive);
	return NULL;
}

// Archive
status_t
CloneStroke::Archive(BMessage* into, bool deep) const
{
	status_t status = BrushStroke::Archive(into, deep);

	// add our data to the message
	if (status >= B_OK)
		// store control alpha flag
		status = into->AddPoint("clone offset", fCloneOffset);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "CloneStroke");

	return status;
}

// ExtendRebuildArea
void
CloneStroke::ExtendRebuildArea(BRect& area, BRect layerBounds) const
{
// TODO: this is brute force method

	BRect bounds = Bounds();

	BPoint offset(fCloneOffset);
	Transform(&offset);
	BPoint p(0.0, 0.0);
	Transform(&p);
	offset.x -= p.x;
	offset.y -= p.y;

	// this is the rect from which containes the cloned pixels
	BRect source(bounds);
	source.OffsetBy(-offset.x, -offset.y);
	// this is the rect that includes everything
	BRect complete = source | bounds;
	// if this area somehow touches the rebuild area, include all
	if (area.Intersects(complete)) {
		area = area | complete;
	}

// old code:
// the problem here is that once the rebuilding process
// is started, there is no knowledge of the area originally
// needing the rebuild. The area being rebuild might contain
// all pixels necessary to rebuild the original rebuild area,
// but the tools draw everything, for which in in turn other
// pixels might be needed.

/*	BRect bounds = Bounds();
	BPoint offset = fCloneOffset;
	// constrain area to part that we touch
	BRect touched = area & bounds;
	// area touches our bounds (where we are visible in the canvas)
	// extend it to include the area that we clone
	touched.OffsetBy(-offset.x, -offset.y);
	// move area about our offset
	BRect r = area.OffsetByCopy(offset.x, offset.y);
	// if the offset area touches our bounds...
	if (r.Intersects(bounds)) {
		// cut off parts from offset area that we don't touch
		r = r & bounds;
		// modify original area to include the parts we do touch
		area = r | area;
	}
	if (touched.IsValid()) {
		area = touched | area;
	}*/
}

// MergeWithBitmap
void
CloneStroke::MergeWithBitmap(BBitmap* dest, BBitmap* strokeBitmap,
						BRect area, uint32 colorSpace) const
{
	BPoint offset(fCloneOffset);
	Transform(&offset);
	BPoint p(0.0, 0.0);
	Transform(&p);
	offset.x -= p.x;
	offset.y -= p.y;
	clone_bitmap(dest, strokeBitmap, area, offset);
}

// MergeWithBitmap
void
CloneStroke::MergeWithBitmap(BBitmap* from, BBitmap* dest,
						BBitmap* strokeBitmap, BRect area, uint32 colorSpace) const
{
	BPoint offset(fCloneOffset);
	Transform(&offset);
	BPoint p(0.0, 0.0);
	Transform(&p);
	offset.x -= p.x;
	offset.y -= p.y;
	clone_bitmap_copy(from, dest, strokeBitmap, area, offset);
}

// MakePropertyObject
PropertyObject*
CloneStroke::MakePropertyObject() const
{
	PropertyObject* object = BrushStroke::MakePropertyObject();
	if (object) {
		object->AddProperty(new FloatProperty("x offset",
											  PROPERTY_X_OFFSET,
											  fCloneOffset.x,
											  -10000.0, 10000.0));
		object->AddProperty(new FloatProperty("x offset",
											  PROPERTY_Y_OFFSET,
											  fCloneOffset.y,
											  -10000.0, 10000.0));
	}
	return object;
}

// SetToPropertyObject
bool
CloneStroke::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	bool ret = BrushStroke::SetToPropertyObject(object);
	if (object) {
		float f = object->FindFloatProperty(PROPERTY_X_OFFSET, fCloneOffset.x);
		if (f != fCloneOffset.x)
			ret = true;
		fCloneOffset.x = f;

		f = object->FindFloatProperty(PROPERTY_Y_OFFSET, fCloneOffset.y);
		if (f != fCloneOffset.y)
			ret = true;
		fCloneOffset.y = f;

		if (ret)
			Notify();
	}
	return ret;
}

// Name
const char*
CloneStroke::Name() const
{
	LanguageManager* manager = LanguageManager::Default();
	const char* name = manager->GetString(CLONE, "Clone");
	return name;
}

// ToolID
int32
CloneStroke::ToolID() const
{
	return TOOL_CLONE;
}
