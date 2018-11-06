// IntProperty.h

#ifndef INT_PROPERTY_H
#define INT_PROPERTY_H

#include "Property.h"

// Beispielhaft: IntProperty
// Min Max müssten eigentlich nicht wirklich Members hier sein, vielmehr könnte
// das Editor() entsprechend konfiguriert werden.
class IntProperty : public Property {
 public:
								IntProperty(const char* name,
											int32 id,
											int32 value = 255,
											int32 min = 0,
											int32 max = 255);
								IntProperty(BMessage* archive);
	virtual						~IntProperty();

								// BArchivable
	virtual	status_t			Archive(BMessage* archive, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

								// Property
	virtual	PropertyItemValueView*		Editor();

			void				SetMinMax(int32 min, int32 max);
			void				GetMinMax(int32* min, int32* max) const;

			void				SetValue(int32 value);
			int32				Value() const;

 private:
			int32				fValue;
			int32				fMin;
			int32				fMax;
};


#endif // INT_PROPERTY_H


