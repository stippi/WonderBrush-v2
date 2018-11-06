// ColorValueView.h

#ifndef COLOR_VALUE_VIEW_H
#define COLOR_VALUE_VIEW_H

#include "PropertyItemValueView.h"

class ColorProperty;
class SwatchValueView;

class ColorValueView : public PropertyItemValueView {
 public:
								ColorValueView(ColorProperty* property);
	virtual						~ColorValueView();

								// BView
	virtual	void				Draw(BRect updateRect);
	virtual	void				FrameResized(float width, float height);

	virtual	void				MakeFocus(bool focused);

	virtual	void				MessageReceived(BMessage* message);

								// PropertyItemValueView
	virtual	bool				IsFocused() const;

	virtual	bool				SetToProperty(const Property* property);

 protected:
			ColorProperty*		fProperty;

			SwatchValueView*	fSwatchView;
};

#endif // COLOR_VALUE_VIEW_H


