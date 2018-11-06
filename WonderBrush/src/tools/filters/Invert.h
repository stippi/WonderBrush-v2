// Invert.h

#ifndef INVERT_H
#define INVERT_H

#include "FilterObject.h"

class Invert : public FilterObject {
 public:
								Invert();
								Invert(const Invert& other);
								Invert(BMessage* archive);
	virtual						~Invert();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	void				ProcessBitmap(BBitmap* bitmap,
											  BBitmap* alphaBitmap,
											  BRect constrainRect) const;

 protected:
};

#endif	// INVERT_H
