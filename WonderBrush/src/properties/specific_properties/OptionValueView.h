// OptionValueView.h

#ifndef OPTION_VALUE_VIEW_H
#define OPTION_VALUE_VIEW_H

#include <String.h>

#include "PropertyItemValueView.h"

class OptionProperty;

class OptionValueView : public PropertyItemValueView {
 public:
								OptionValueView(OptionProperty* property);
	virtual						~OptionValueView();

	virtual	void				Draw(BRect updateRect);
	virtual	void				FrameResized(float width, float height);

	virtual	void				MakeFocus(bool focused);

	virtual	void				MessageReceived(BMessage* message);

	virtual	void				MouseDown(BPoint where);
	virtual	void				KeyDown(const char* bytes, int32 numBytes);

	virtual	void				ValueChanged();

	virtual	bool				SetToProperty(const Property* property);

 private:
			OptionProperty*		fProperty;

			BString				fCurrentOption;
};

#endif // OPTION_VALUE_VIEW_H


