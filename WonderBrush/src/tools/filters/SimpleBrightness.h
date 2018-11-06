// SimpleBrightness.h

#ifndef SIMPLE_BRIGHTNESS_H
#define SIMPLE_BRIGHTNESS_H

#include "FilterObject.h"

class SimpleBrightness : public FilterObject {
 public:
								SimpleBrightness();
								SimpleBrightness(const SimpleBrightness& other);
								SimpleBrightness(BMessage* archive);
	virtual						~SimpleBrightness();

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
			float				fBrightness;
};

#endif	// SIMPLE_BRIGHTNESS_H
