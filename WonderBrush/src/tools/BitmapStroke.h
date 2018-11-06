// BitmapStroke.h

#ifndef BITMAP_STROKE_H
#define BITMAP_STROKE_H

#include "Stroke.h"

class BitmapStroke : public Stroke {
 public:
								BitmapStroke(BBitmap* bitmap);
								BitmapStroke(const BitmapStroke& other);
								BitmapStroke(BMessage* archive);
	virtual						~BitmapStroke();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	bool				DrawLastLine(BBitmap* bitmap,
											 BRect& updateRect);
	virtual	void				Draw(BBitmap* bitmap);
	virtual	void				Draw(BBitmap* bitmap, BRect constrainRect);

	virtual	void				Reset();

	virtual	bool				IsPickable(uint32 pickMask) const;
	virtual	bool				HitTest(const BPoint& where,
										BBitmap* alphaMap);
	virtual	bool				HitTest(const BRect& area,
										BBitmap* alphaMap);

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

	virtual	const char*			Name() const;

	virtual	int32				ToolID() const;

			const BBitmap*		Bitmap() const
									{ return fBitmap; }

	enum {
		INTERPOLATION_NN = 0,
		INTERPOLATION_BILINEAR,
		INTERPOLATION_RESAMPLE
	};

			uint32				Interpolation() const
									{ return fInterpolation; }

 protected:
			void				_Premultiply(BBitmap* bitmap);

			BBitmap*			fBitmap;
			uint32				fInterpolation;
			uint8				fOpacity;
};

#endif	// BITMAP_STROKE_H
