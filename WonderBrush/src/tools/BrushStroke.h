// BrushStroke.h

#ifndef BRUSH_STROKE_H
#define BRUSH_STROKE_H

#include "defines.h"

#include "Stroke.h"

class Brush;

class BrushStroke : public Stroke {
 public:
								BrushStroke(range radius, range hardness,
											range alpha, range spacing,
											uint32 flags,
											rgb_color color, uint32 mode);
								BrushStroke(const BrushStroke& other);
								BrushStroke(BMessage* archive);
	virtual						~BrushStroke();

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

			Brush*				GetBrush() const
									{ return fBrush; }

	virtual	const char*			Name() const;
	virtual	int32				ToolID() const;

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

 protected:
			void				_Draw(BBitmap* bitmap, BRect constrainRect);
			bool				_StrokeLine(const Point* a, const Point* b,
											uint8* dest, uint32 bpr,
											BRect constrainRect);
			float				_StepDist() const;

			void				_RecalcBounds();

			Brush*				fBrush;
			uint32				fFlags;
			range				fAlpha;
			range				fSpacing;
			float				fDistLeftOver;
};

#endif	// BRUSH_STROKE_H
