// FloatProperty.cpp

#include <Message.h>

#include "FloatValueView.h"

#include "FloatProperty.h"

// constructor
FloatProperty::FloatProperty(const char* name, int32 id,
							 float value, float min, float max)
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
FloatProperty::FloatProperty(BMessage* archive)
	: Property(archive),
	  fValue(1.0),
	  fMin(0.0),
	  fMax(1.0)
{
	if (archive) {
		if (archive->FindFloat("value", &fValue) < B_OK)
			fValue = 1.0;
		if (archive->FindFloat("min", &fMin) < B_OK)
			fMin = 0.0;
		if (archive->FindFloat("max", &fMax) < B_OK)
			fMax = 1.0;
	}
}

// destrucor
FloatProperty::~FloatProperty()
{
}

// Archive
status_t
FloatProperty::Archive(BMessage* into, bool deep) const
{
	status_t status = Property::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddFloat("value", fValue);
	if (status >= B_OK)
		status = into->AddFloat("min", fMin);
	if (status >= B_OK)
		status = into->AddFloat("max", fMax);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "FloatProperty");

	return status;
}

// Instantiate
BArchivable*
FloatProperty::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "FloatProperty"))
		return new FloatProperty(archive);
	return NULL;
}

// Editor
PropertyItemValueView*		
FloatProperty::Editor()
{
	return new FloatValueView(this);
}

// SetMinMax
void
FloatProperty::SetMinMax(float min, float max)
{
	if (min <= max) {
		fMin = min;
		fMax = max;
	}
}

// 
void
FloatProperty::GetMinMax(float* min, float* max) const
{
	if (min && max) {
		*min = fMin;
		*max = fMax;
	}
}

// SetValue
void
FloatProperty::SetValue(float value)
{
	if (value < fMin)
		value = fMin;
	if (value > fMax)
		value = fMax;
	fValue = value;
}

// Value
float
FloatProperty::Value() const
{
	return fValue;
}



