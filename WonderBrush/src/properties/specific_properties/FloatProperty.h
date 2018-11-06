// FloatProperty.h

#ifndef FLOAT_PROPERTY_H
#define FLOAT_PROPERTY_H

#include "Property.h"

class FloatProperty : public Property {
 public:
								FloatProperty(const char* name,
											 int32 id,
											 float value = 1.0,
											 float min = 0.0,
											 float max = 1.0);
								FloatProperty(BMessage* archive);
	virtual						~FloatProperty();

								// BArchivable
	virtual	status_t			Archive(BMessage* archive, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

								// Property
	virtual	PropertyItemValueView*		Editor();

								// FloatProperty
			void				SetMinMax(float min, float max);
			void				GetMinMax(float* min, float* max) const;

			void				SetValue(float value);
			float				Value() const;

 private:
			float				fValue;
			float				fMin;
			float				fMax;
};


#endif // FLOAT_PROPERTY_H


