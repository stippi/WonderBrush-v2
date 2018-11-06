// GaussianBlur.h

#ifndef GAUSSIAN_BLUR_H
#define GAUSSIAN_BLUR_H

#include "FilterObject.h"

class GaussianBlur : public FilterObject {
 public:
								GaussianBlur(float radius);
								GaussianBlur(const GaussianBlur& other);
								GaussianBlur(BMessage* archive);
	virtual						~GaussianBlur();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

	virtual	void				ExtendRebuildArea(BRect& area,
												  BRect layerBounds) const;

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	void				ProcessBitmap(BBitmap* bitmap,
											  BBitmap* alphaBitmap,
											  BRect constrainRect) const;

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

 protected:
			float				fRadius;
};

#endif	// GAUSSIAN_BLUR_H
