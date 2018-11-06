// DropShadow.h

#ifndef DROP_SHADOW_H
#define DROP_SHADOW_H

#include "FilterObject.h"

class DropShadow : public FilterObject {
 public:
								DropShadow(rgb_color color,
										   BPoint offset,
										   float blurRadius);
								DropShadow(const DropShadow& other);
								DropShadow(BMessage* archive);
	virtual						~DropShadow();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	void				ExtendRebuildArea(BRect& area,
												  BRect layerBounds) const;

	virtual	void				ProcessBitmap(BBitmap* bitmap,
											  BBitmap* alphaMap,
											  BRect constrainRect) const;

	virtual	bool				IsColorObject() const
									{ return true; }

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

	virtual	void				TransformationChanged();

 protected:
			BPoint 				fOffset;
			float				fBlurRadius;
			uint8				fOpacity;
};

#endif	// DROP_SHADOW_H
