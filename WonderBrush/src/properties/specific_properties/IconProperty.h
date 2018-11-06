// IconProperty.h

#ifndef ICON_PROPERTY_H
#define ICON_PROPERTY_H

#include "Property.h"

class IconProperty : public Property {
 public:
								IconProperty(const char* name,
											 int32 id,
											 const uchar* icon,
											 uint32 width, uint32 height,
											 color_space format,
											 BMessage* message = NULL);
								IconProperty(BMessage* archive);
	virtual						~IconProperty();

								// BArchivable
	virtual	status_t			Archive(BMessage* archive, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

								// IconProperty
			const	BMessage*	Message() const
									{ return fMessage; }

			void				SetMessage(const BMessage* message);

								// Property
	virtual	PropertyItemValueView*		Editor();

 private:
			BMessage*			fMessage;

			const uchar* 		fIcon;
			uint32				fWidth;
			uint32				fHeight;
			color_space			fFormat;
};


#endif // ICON_PROPERTY_H


