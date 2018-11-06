// Dispersion.h

#ifndef DISPERSION_H
#define DISPERSION_H

#include "FilterObject.h"

class Dispersion : public FilterObject {
 public:
								Dispersion(float radius);
								Dispersion(const Dispersion& other);
								Dispersion(BMessage* archive);
	virtual						~Dispersion();

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

								// Dispersion
			void				Randomize();

 protected:
			float				fRadius;

			int32				fRandomSeed;
};

#endif	// DISPERSION_H
