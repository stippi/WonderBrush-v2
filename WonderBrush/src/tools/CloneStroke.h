// CloneStroke.h

#ifndef CLONE_STROKE_H
#define CLONE_STROKE_H

#include <Point.h>

#include "BrushStroke.h"

class Brush;

class CloneStroke : public BrushStroke {
 public:
								CloneStroke(range radius, range hardness,
											range alpha, range spacing,
											uint32 flags,
											rgb_color color, uint32 mode,
											BPoint offset);
								CloneStroke(const CloneStroke& other);
								CloneStroke(BMessage* archive);
	virtual						~CloneStroke();

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

								// CloneStroke
	inline	BPoint				CloneOffset() const
									{ return fCloneOffset; }

	virtual	const char*			Name() const;
	virtual	int32				ToolID() const;

 private:
			BPoint				fCloneOffset;
};

#endif	// CLONE_STROKE_H
