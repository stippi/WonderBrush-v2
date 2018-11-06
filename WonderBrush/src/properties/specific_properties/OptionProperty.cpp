// OptionProperty.cpp

#include <Message.h>

#include "OptionValueView.h"

#include "OptionProperty.h"

// constructor
OptionProperty::OptionProperty(const char* name, int32 id)
	: Property(name, id),
	  fOptions(2),
	  fCurrentOptionID(-1)
{
}

// archive constructor
OptionProperty::OptionProperty(BMessage* archive)
	: Property(archive),
	  fOptions(2),
	  fCurrentOptionID(-1)
{
	if (archive) {
		if (archive->FindInt32("option", &fCurrentOptionID) < B_OK)
			fCurrentOptionID = -1;
	}
}

// destrucor
OptionProperty::~OptionProperty()
{
	for (int32 i = 0; option* o = (option*)fOptions.ItemAt(i); i++)
		delete o;
}

// Archive
status_t
OptionProperty::Archive(BMessage* into, bool deep) const
{
	status_t status = Property::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddInt32("option", fCurrentOptionID);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "OptionProperty");

	return status;
}

// Instantiate
BArchivable*
OptionProperty::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "OptionProperty"))
		return new OptionProperty(archive);
	return NULL;
}

// Editor
PropertyItemValueView*		
OptionProperty::Editor()
{
	return new OptionValueView(this);
}

// AddOption
void
OptionProperty::AddOption(int32 id, const char* name)
{
	if (name) {
		option* o = new option;
		o->id = id;
		o->name = name;
		fOptions.AddItem((void*)o);
	}
}

// CurrentOptionID
int32
OptionProperty::CurrentOptionID() const
{
	return fCurrentOptionID;
}

// SetCurrentOptionID
void
OptionProperty::SetCurrentOptionID(int32 id)
{
	fCurrentOptionID = id;
}

// GetOption
bool
OptionProperty::GetOption(int32 index, BString* string, int32* id) const
{
	if (option* o = (option*)fOptions.ItemAt(index)) {
		*id = o->id;
		*string = o->name;
		return true;
	} else {
		*id = -1;
		*string = "";
		return false;
	}
}

// GetCurrentOption
bool
OptionProperty::GetCurrentOption(BString* string) const
{
	for (int32 i = 0; option* o = (option*)fOptions.ItemAt(i); i++) {
		if (o->id == fCurrentOptionID) {
			*string = o->name;
			return true;
		}
	}
	return false;
}

// SetOptionAtOffset
bool
OptionProperty::SetOptionAtOffset(int32 indexOffset)
{
	if (fOptions.CountItems() > 1) {
		int32 index = -1;
		for (int32 i = 0; option* o = (option*)fOptions.ItemAt(i); i++) {
			if (o->id == fCurrentOptionID) {
				index = i;
			}
		}
		if (index >= 0) {
			// offset index
			index += indexOffset;
			// keep index in range by wrapping arround
			if (index >= fOptions.CountItems())
				index = 0;
			if (index < 0)
				index = fOptions.CountItems() - 1;
			if (option* o = (option*)fOptions.ItemAt(index)) {
				SetCurrentOptionID(o->id);
				return true;
			}
		}
	}
	return false;
}



