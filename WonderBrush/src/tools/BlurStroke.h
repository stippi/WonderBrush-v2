// BlurStroke.h

#ifndef BLUR_STROKE_H
#define BLUR_STROKE_H

#include <Point.h>

#include "BrushStroke.h"

class Brush;

class BlurStroke : public BrushStroke {
 public:
								BlurStroke(range radius, range hardness,
											range alpha, range spacing,
											uint32 flags,
											rgb_color color,
											float blurRadius);
								BlurStroke(const BlurStroke& other);
								BlurStroke(BMessage* archive);
	virtual						~BlurStroke();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocol
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

								// Stroke
	virtual	void				ExtendRebuildArea(BRect& area,
												  BRect layerBounds) const;

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

								// BlurStroke
	inline	float				BlurRadius() const
									{ return fBlurRadius; }

	virtual	const char*			Name() const;
	virtual	int32				ToolID() const;

 private:
			float				fBlurRadius;
};

#endif	// BLUR_STROKE_H
