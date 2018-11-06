// ColorReduction.h

#ifndef COLOR_REDUCTION_H
#define COLOR_REDUCTION_H

#include "FilterObject.h"

class ColorReduction : public FilterObject {
 public:
								ColorReduction();
								ColorReduction(const ColorReduction& other);
								ColorReduction(BMessage* archive);
	virtual						~ColorReduction();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	void				ProcessBitmap(BBitmap* bitmap,
											  BBitmap* alphaBitmap,
											  BRect constrainRect) const;

//	virtual	PropertyObject*		MakePropertyObject() const;
//	virtual	bool				SetToPropertyObject(PropertyObject* object);
};

#endif	// COLOR_REDUCTION_H
