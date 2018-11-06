// SolidColor.h

#ifndef SOLID_COLOR_H
#define SOLID_COLOR_H

#include "FilterObject.h"

class SolidColor : public FilterObject {
 public:
								SolidColor();
								SolidColor(const SolidColor& other);
								SolidColor(BMessage* archive);
	virtual						~SolidColor();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	void				LayerBoundsChanged(const BRect& layerBounds);
	virtual	BRect				Bounds() const;

	virtual	void				ExtendRebuildArea(BRect& area,
												  BRect layerBounds) const;

	virtual	void				Draw(BBitmap* bitmap);
	virtual	void				Draw(BBitmap* bitmap, BRect constrainRect);

	virtual	void				ProcessBitmap(BBitmap* bitmap,
											  BBitmap* alphaBitmap,
											  BRect constrainRect) const;

	virtual	void				MergeWithBitmap(BBitmap* dest,
												BBitmap* strokeBitmap,
												BRect area,
												uint32 colorSpace) const;
	virtual	void				MergeWithBitmap(BBitmap* from,
												BBitmap* dest,
												BBitmap* strokeBitmap,
												BRect area,
												uint32 colorSpace) const;

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

 protected:
			uint8				fAlpha;

	mutable	BRect				fLayerBounds;
};

#endif	// SOLID_COLOR_H
