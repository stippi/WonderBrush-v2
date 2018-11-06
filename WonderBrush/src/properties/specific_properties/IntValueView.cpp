// IntValueView.cpp

#include <stdio.h>

#include "IntProperty.h"
#include "NummericalTextView.h"

#include "IntValueView.h"

// constructor
IntValueView::IntValueView(IntProperty* property)
	: NummericalInputValueView(property),
	  fProperty(property)
{
	fTextView->SetFloatMode(false);
	if (fProperty)
		fTextView->SetValue(fProperty->Value());
}

// destructor
IntValueView::~IntValueView()
{
}

// ValueChanged
void
IntValueView::ValueChanged()
{
	if (fProperty) {
		fProperty->SetValue(fTextView->IntValue());
		fTextView->SetValue(fProperty->Value());
		NummericalInputValueView::ValueChanged();
	}
}

// SetToProperty
bool
IntValueView::SetToProperty(const Property* property)
{
	const IntProperty* p = dynamic_cast<const IntProperty*>(property);
	if (p) {
		if (p->Value() != fProperty->Value()) {
			fProperty->SetValue(p->Value());
			fTextView->SetValue(fProperty->Value());
		}
		return true;
	}
	return false;
}

