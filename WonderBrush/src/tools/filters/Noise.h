// Noise.h

#ifndef NOISE_H
#define NOISE_H

#include "FilterObject.h"

class Noise : public FilterObject {
 public:
								Noise(float radius);
								Noise(const Noise& other);
								Noise(BMessage* archive);
	virtual						~Noise();

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

								// Noise
			void				Randomize();

 protected:
			bool				fLuminanceOnly;
			int32				fStrength;

			int32				fRandomSeed;
};

#endif	// NOISE_H
