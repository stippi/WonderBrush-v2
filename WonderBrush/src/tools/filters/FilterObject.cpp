// FilterObject.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <ClassInfo.h>
#include <Message.h>

#include "FilterFactory.h"

#include "FilterObject.h"

// constructor
FilterObject::FilterObject(int32 id)
	: Stroke((rgb_color){0, 0, 0, 0}, MODE_UNDEFINED),
	  fFilterID(id)
{
	Touch(BRect(-10000.0, -10000.0, 20000.0, 20000.0));
}

// copy constructor
FilterObject::FilterObject(const FilterObject& other)
	: Stroke(other),
	  fFilterID(other.fFilterID)
{
}

// BArchivable constructor
FilterObject::FilterObject(BMessage* archive)
	: Stroke(archive),
	  fFilterID(-1)
{
	if (!archive || archive->FindInt32("filter id", &fFilterID) < B_OK) {
		fFilterID = -1;
		SetMode(MODE_UNDEFINED);
		Touch(BRect(-10000.0, -10000.0, 20000.0, 20000.0));
	}
}

// destructor
FilterObject::~FilterObject()
{
}

// SetTo
bool
FilterObject::SetTo(const Stroke* from)
{
	const FilterObject* filterObject = dynamic_cast<const FilterObject*>(from);

	AutoNotificationSuspender _(this);

	if (filterObject && Stroke::SetTo(from)) {
		fFilterID = filterObject->fFilterID;
		Notify();
		return true;
	}
	return false;
}

// Archive
status_t
FilterObject::Archive(BMessage* into, bool deep) const
{
	status_t status = Stroke::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddInt32("filter id", fFilterID);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "FilterObject");

	return status;
}


// DrawLastLine
bool
FilterObject::DrawLastLine(BBitmap* bitmap, BRect& updateRect)
{
	return false;
}

// Draw
void
FilterObject::Draw(BBitmap* bitmap)
{
}

// Draw
void
FilterObject::Draw(BBitmap* bitmap, BRect constrainRect)
{
	// unfortunately, we can't do anything here,
	// because we can't get to the layer bitmap
}

// Finish
void
FilterObject::Finish(BBitmap* bitmap)
{
	// override default behaviour and
	// don't do anything
}

// Reset
void
FilterObject::Reset()
{
	// override default behaviour and
	// don't do anything
}

// MergeWithBitmap
//
// on the fly drop shadow generation
void
FilterObject::MergeWithBitmap(BBitmap* dest, BBitmap* strokeBitmap,
						BRect area, uint32 colorSpace) const
{
	ProcessBitmap(dest, strokeBitmap, area);
}

// MergeWithBitmap
void
FilterObject::MergeWithBitmap(BBitmap* from, BBitmap* dest,
						BBitmap* strokeBitmap, BRect area, uint32 colorSpace) const
{
	MergeWithBitmap(dest, strokeBitmap, area, colorSpace);
}

// Name
const char*
FilterObject::Name() const
{
	return FilterFactory::Default()->NameFor(fFilterID);
}

// ToolID
int32
FilterObject::ToolID() const
{
	return TOOL_UNDEFINED;
}

// SaveSettings
void
FilterObject::SaveSettings() const
{
	FilterFactory* factory = FilterFactory::Default();
	factory->SaveSettings(this);
}

