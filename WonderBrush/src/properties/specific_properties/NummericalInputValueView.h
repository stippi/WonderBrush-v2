// NummericalInputValueView.h

#ifndef NUMMERICAL_INPUT_VALUE_VIEW_H
#define NUMMERICAL_INPUT_VALUE_VIEW_H

#include "PropertyItemValueView.h"

class NummericalTextView;

class NummericalInputValueView : public PropertyItemValueView {
 public:
								NummericalInputValueView(Property* property);
	virtual						~NummericalInputValueView();

								// BView
	virtual	void				AttachedToWindow();

	virtual	void				Draw(BRect updateRect);
	virtual	void				FrameResized(float width, float height);

	virtual	void				MakeFocus(bool focused);

	virtual	void				MessageReceived(BMessage* message);

								// PropertyItemValueView
	virtual	bool				IsFocused() const;

								// NummericalInputValueView
			void				SetFloatMode(bool floatingPoint);

 protected:
			NummericalTextView*	fTextView;
};

#endif // NUMMERICAL_INPUT_VALUE_VIEW_H


