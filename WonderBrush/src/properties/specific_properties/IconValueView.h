// IconValueView.h

#ifndef ICON_VALUE_VIEW_H
#define ICON_VALUE_VIEW_H

#include "PropertyItemValueView.h"

class IconProperty;
class NummericalTextView;

class IconValueView : public PropertyItemValueView {
 public:
								IconValueView(IconProperty* property);
	virtual						~IconValueView();

								// BView
	virtual	void				Draw(BRect updateRect);

								// IconValueView
			status_t			SetIcon(const unsigned char* bitsFromQuickRes,
										uint32 width, uint32 height,
										color_space format);

	virtual	bool				SetToProperty(const Property* property);

 protected:
			IconProperty*		fProperty;
			BBitmap*			fIcon;
};

#endif // ICON_VALUE_VIEW_H


