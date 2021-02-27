// PenStroke.h

#ifndef PEN_STROKE_H
#define PEN_STROKE_H

#include "defines.h"

#include "Stroke.h"

class PenStroke : public Stroke {
 public:
								PenStroke(rgb_color color, range alpha,
										  uint32 flags, uint32 mode);
								PenStroke(const PenStroke& other);
								PenStroke(BMessage* archive);
	virtual						~PenStroke();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

								// Stroke
	virtual	bool				DrawLastLine(BBitmap* bitmap,
											 BRect& updateRect);
	virtual	void				Draw(BBitmap* bitmap);
	virtual	void				Draw(BBitmap* bitmap,
									 BRect constrainRect);

	virtual	void				Reset();

	virtual	const char*			Name() const;
	virtual	int32				ToolID() const;

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

								// PenStroke
			range				Alpha() const
									{ return fAlpha; }

 private:
			void				_Draw(BBitmap* bitmap, BRect constrainRect);
			void				_StrokeLine(Point* a, Point* b,
											uint8* dest, uint32 bpr,
											BRect& constrainRect);
			void				_StrokePoint(float x, float y, float pressure,
											 uint8* dest, int bpr,
											 BRect& constrainRect) const;

			range				fAlpha;
			uint32				fFlags;

			float				fDistLeftOver;
};

#endif	// PEN_STROKE_H
