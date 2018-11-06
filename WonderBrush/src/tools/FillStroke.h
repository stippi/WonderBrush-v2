// FillStroke.h

#ifndef FILL_STROKE_H
#define FILL_STROKE_H

#include "Stroke.h"

class FillStroke : public Stroke {
 public:
								FillStroke(BPoint fillStart,
										   rgb_color fillColor,
										   uint8 opacity,
										   uint8 tolerance,
										   uint8 softness);
								FillStroke(BMessage* archive);
								FillStroke(const FillStroke& other);
	virtual						~FillStroke();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	void				Finish(BBitmap* bitmap);
	virtual	bool				DrawLastLine(BBitmap* bitmap,
											 BRect& updateRect);
	virtual	void				Draw(BBitmap* bitmap);
	virtual	void				Draw(BBitmap* bitmap, BRect constrainRect);


	virtual	void				Reset();

	virtual	const char*			Name() const;

	virtual	int32				ToolID() const;

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

								// FillStroke
			bool				Fill(BBitmap* bitmap,
									 BBitmap* strokeBitmap,
									 bool contiguous = true);

 protected:
								FillStroke(RLEBuffer* cache,
										   BPoint offset);

			BPoint				fFillStart;
			uint8				fOpacity;
			uint8				fTolerance;
			uint8				fSoftness;

			BBitmap*			fBitmap;
};

#endif	// BITMAP_STROKE_H
