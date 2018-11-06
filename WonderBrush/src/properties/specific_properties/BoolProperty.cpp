// BoolProperty.cpp

#include <Message.h>

#include "BoolValueView.h"

#include "BoolProperty.h"

// constructor
BoolProperty::BoolProperty(const char* name, int32 id, bool value)
	: Property(name, id),
	  fValue(value)
{
}

// archive constructor
BoolProperty::BoolProperty(BMessage* archive)
	: Property(archive),
	  fValue(true)
{
	if (archive) {
		if (archive->FindBool("value", &fValue) < B_OK)
			fValue = true;
	}
}

// destrucor
BoolProperty::~BoolProperty()
{
}

// Archive
status_t
BoolProperty::Archive(BMessage* into, bool deep) const
{
	status_t status = Property::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddBool("value", fValue);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "BoolProperty");

	return status;
}

// Instantiate
BArchivable*
BoolProperty::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "BoolProperty"))
		return new BoolProperty(archive);
	return NULL;
}

// Editor
PropertyItemValueView*		
BoolProperty::Editor()
{
	return new BoolValueView(this);
}

// SetValue
void
BoolProperty::SetValue(bool value)
{
	fValue = value;
}

// Value
bool
BoolProperty::Value() const
{
	return fValue;
}



