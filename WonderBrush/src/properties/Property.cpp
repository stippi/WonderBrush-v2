// Property.cpp

#include <Message.h>

#include "CommonPropertyIDs.h"

#include "Property.h"

// constructor
Property::Property(const char* name, int32 id)
	: fName(name ? name : ""),
	  fID(id)
{
}

// archive constructor
Property::Property(BMessage* archive)
	: fName(""),
	  fID(-1)
{
	if (archive) {
		if (archive->FindInt32("property id", &fID) < B_OK)
			fID = -1;
		if (archive->FindString("property name", &fName) < B_OK)
			fName = "";
	}
}

// destructor
Property::~Property()
{
}

// Archive
status_t
Property::Archive(BMessage* archive, bool deep) const
{
	status_t ret = BArchivable::Archive(archive, deep);

	if (ret >= B_OK)
		ret = archive->AddInt32("property id", fID);

	if (ret >= B_OK && fName.CountChars() > 0)
		ret = archive->AddString("property name", fName.String());

	if (ret >= B_OK)
		ret = archive->AddString("class", "Property");

	return ret;
}

// Name
const char*
Property::Name() const
{
	return name_for_id(fID);
}

