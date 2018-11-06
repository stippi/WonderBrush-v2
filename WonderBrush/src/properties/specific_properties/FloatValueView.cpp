// FloatValueView.cpp

#include <stdio.h>

#include "FloatProperty.h"
#include "NummericalTextView.h"

#include "FloatValueView.h"

// constructor
FloatValueView::FloatValueView(FloatProperty* property)
	: NummericalInputValueView(property),
	  fProperty(property)
{
	fTextView->SetFloatMode(true);
	if (fProperty)
		fTextView->SetValue(fProperty->Value());
}

// destructor
FloatValueView::~FloatValueView()
{
}

// ValueChanged
void
FloatValueView::ValueChanged()
{
	if (fProperty) {
		fProperty->SetValue(fTextView->FloatValue());
		fTextView->SetValue(fProperty->Value());
		NummericalInputValueView::ValueChanged();
	}
}

// SetToProperty
bool
FloatValueView::SetToProperty(const Property* property)
{
	const FloatProperty* p = dynamic_cast<const FloatProperty*>(property);
	if (p) {
		if (p->Value() != fProperty->Value()) {
			fProperty->SetValue(p->Value());
			fTextView->SetValue(fProperty->Value());
		}
		return true;
	}
	return false;
}
