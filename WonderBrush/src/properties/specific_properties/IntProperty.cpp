// IntProperty.cpp

#include <Message.h>

#include "IntValueView.h"

#include "IntProperty.h"

// constructor
IntProperty::IntProperty(const char* name, int32 id,
						 int32 value, int32 min, int32 max)
	: Property(name, id),
	  fValue(value),
	  fMin(min),
	  fMax(max)
{
	if (fValue < fMin)
		fValue = fMin;
	if (fValue > fMax)
		fValue = fMax;
}

// archive constructor
IntProperty::IntProperty(BMessage* archive)
	: Property(archive),
	  fValue(255),
	  fMin(0),
	  fMax(255)
{
	if (archive) {
		if (archive->FindInt32("value", &fValue) < B_OK)
			fValue = 255;
		if (archive->FindInt32("min", &fMin) < B_OK)
			fMin = 0;
		if (archive->FindInt32("max", &fMax) < B_OK)
			fMax = 255;
	}
}

// destrucor
IntProperty::~IntProperty()
{
}

// Archive
status_t
IntProperty::Archive(BMessage* into, bool deep) const
{
	status_t status = Property::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddInt32("value", fValue);
	if (status >= B_OK)
		status = into->AddInt32("min", fMin);
	if (status >= B_OK)
		status = into->AddInt32("max", fMax);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "IntProperty");

	return status;
}

// Instantiate
BArchivable*
IntProperty::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "IntProperty"))
		return new IntProperty(archive);
	return NULL;
}

// Editor
PropertyItemValueView*		
IntProperty::Editor()
{
	return new IntValueView(this);
}

// SetMinMax
void
IntProperty::SetMinMax(int32 min, int32 max)
{
	if (min <= max) {
		fMin = min;
		fMax = max;
	}
}

// 
void
IntProperty::GetMinMax(int32* min, int32* max) const
{
	if (min && max) {
		*min = fMin;
		*max = fMax;
	}
}

// SetValue
void
IntProperty::SetValue(int32 value)
{
	if (value < fMin)
		value = fMin;
	if (value > fMax)
		value = fMax;
	fValue = value;
}

// Value
int32
IntProperty::Value() const
{
	return fValue;
}



