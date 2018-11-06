// MotionBlur.h

#ifndef MOTION_BLUR_H
#define MOTION_BLUR_H

#include "FilterObject.h"

struct blur_delta {
	int32	dx;
	int32	dy;
};


class MotionBlur : public FilterObject {
 public:
								MotionBlur();
								MotionBlur(const MotionBlur& other);
								MotionBlur(BMessage* archive);
	virtual						~MotionBlur();

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
			blur_delta*			_CalculateDeltas(uint32* count) const;

			void				_Blur(uint8 *dst, uint8 *src,
									  uint32 dstBPR, uint32 srcBPR,
									  uint32 extend, uint32 lines, uint32 pixels,
									  blur_delta* deltas, uint32 count,
									  bool blurAlpha) const;

			float				fDirection;
			float				fBlurRadius;
			bool				fBlurAlpha;
};

#endif	// MOTION_BLUR_H
