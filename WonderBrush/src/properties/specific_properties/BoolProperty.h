// BoolProperty.h

#ifndef BOOL_PROPERTY_H
#define BOOL_PROPERTY_H

#include "Property.h"

class BoolProperty : public Property {
 public:
								BoolProperty(const char* name,
											 int32 id,
											 bool value = true);
								BoolProperty(BMessage* archive);
	virtual						~BoolProperty();

								// BArchivable
	virtual	status_t			Archive(BMessage* archive, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

								// Property
	virtual	PropertyItemValueView*		Editor();

								// BoolProperty
			void				SetValue(bool value);
			bool				Value() const;

 private:
			bool				fValue;
};


#endif // BOOL_PROPERTY_H


