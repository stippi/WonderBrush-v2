// IntValueView.h

#ifndef INT_VALUE_VIEW_H
#define INT_VALUE_VIEW_H

#include "NummericalInputValueView.h"

class IntProperty;
class NummericalTextView;

class IntValueView : public NummericalInputValueView {
 public:
								IntValueView(IntProperty* property);
	virtual						~IntValueView();

	virtual	void				ValueChanged();

	virtual	bool				SetToProperty(const Property* property);

 private:
			IntProperty*		fProperty;
};

#endif // INT_VALUE_VIEW_H


