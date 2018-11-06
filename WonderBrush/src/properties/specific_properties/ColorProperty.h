// ColorProperty.h

#ifndef COLOR_PROPERTY_H
#define COLOR_PROPERTY_H

#include "Property.h"

class ColorProperty : public Property {
 public:
								ColorProperty(const char* name,
											  int32 id,
											  rgb_color color);
								ColorProperty(BMessage* archive);
	virtual						~ColorProperty();

								// BArchivable
	virtual	status_t			Archive(BMessage* archive, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

								// Property
	virtual	PropertyItemValueView*		Editor();

			void				SetColor(rgb_color color);
			rgb_color			Color() const;

 private:
			rgb_color			fColor;
};


#endif // COLOR_PROPERTY_H


