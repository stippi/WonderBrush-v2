// BoolValueView.h

#ifndef BOOL_VALUE_VIEW_H
#define BOOL_VALUE_VIEW_H

#include "PropertyItemValueView.h"

class BoolProperty;

class BoolValueView : public PropertyItemValueView {
 public:
								BoolValueView(BoolProperty* property);
	virtual						~BoolValueView();

	virtual	void				Draw(BRect updateRect);
	virtual	void				FrameResized(float width, float height);

	virtual	void				MakeFocus(bool focused);

	virtual	void				MouseDown(BPoint where);
	virtual	void				KeyDown(const char* bytes, int32 numBytes);

	virtual	bool				SetToProperty(const Property* property);

 private:
			void				_ToggleValue();

			BoolProperty*		fProperty;

			BRect				fCheckBoxRect;
};

#endif // BOOL_VALUE_VIEW_H


