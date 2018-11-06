// FloatValueView.h

#ifndef FLOAT_VALUE_VIEW_H
#define FLOAT_VALUE_VIEW_H

#include "NummericalInputValueView.h"

class FloatProperty;
class NummericalTextView;

class FloatValueView : public NummericalInputValueView {
 public:
								FloatValueView(FloatProperty* property);
	virtual						~FloatValueView();

	virtual	void				ValueChanged();

	virtual	bool				SetToProperty(const Property* property);

 private:
			FloatProperty*		fProperty;
};

#endif // FLOAT_VALUE_VIEW_H


