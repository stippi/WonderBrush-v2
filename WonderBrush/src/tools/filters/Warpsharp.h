// Warpsharp.h

#ifndef WARPSHARP_H
#define WARPSHARP_H

#include "FilterObject.h"

class Warpsharp : public FilterObject {
 public:
								Warpsharp();
								Warpsharp(const Warpsharp& other);
								Warpsharp(BMessage* archive);
	virtual						~Warpsharp();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	void				ExtendRebuildArea(BRect& area,
												  BRect layerBounds) const;

	virtual	void				ProcessBitmap(BBitmap* bitmap,
											  BBitmap* alphaBitmap,
											  BRect constrainRect) const;

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

	virtual	void				TransformationChanged();

 protected:
			float				fLambda;
			float				fMu;
			bool				fNonMaximalSuppression;
};

#endif	// WARPSHARP_H
