// Contrast.h

#ifndef CONTRAST_H
#define CONTRAST_H

#include "FilterObject.h"

class Contrast : public FilterObject {
 public:
								Contrast();
								Contrast(const Contrast& other);
								Contrast(BMessage* archive);
	virtual						~Contrast();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	void				ProcessBitmap(BBitmap* bitmap,
											  BBitmap* alphaBitmap,
											  BRect constrainRect) const;

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

 protected:
			uint8				_AverageLuminance(const BBitmap* bitmap) const;

			float				fContrast;
};

#endif	// CONTRAST_H
