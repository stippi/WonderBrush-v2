// Stroke.h

#ifndef STROKE_H
#define STROKE_H

#include <List.h>
#include <Rect.h>
#include <GraphicsDefs.h>

#include "defines.h" 

#include "Observable.h"
//#include "Transformable.h"
#include "ArchivableTransformable.h"

class BBitmap;
class Gradient;
class MaskRenderer;
class ObjectItemPainter;
class PropertyObject;
class RLEBuffer;

//class Stroke : public Transformable,
class Stroke : public ArchivableTransformable,
			   public Observable {
 public:
								Stroke(rgb_color color, uint32 mode);
								Stroke(const Stroke& other);
								Stroke(BMessage* archive);
	virtual						~Stroke();

	virtual	Stroke*				Clone() const = 0;
	virtual	bool				SetTo(const Stroke* from);

	// the BArchivable protocol
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;

	// basic Stroke methods
			bool				AddPoint(Point point);
			bool				GetPointAt(int32 index, Point& point) const;
			bool				GetLastPoint(Point& point) const;
			bool				SetLastPoint(BPoint point,
											 float pressure);

	// hook function that the Transformable calls
	virtual	void				TransformationChanged();
	// this function tells the stroke that no more points will be added
	// a RLE buffer will be created from the provided stroke bitmap.
	// the stroke will watch out that the buffer contains valid data
	virtual	void				Finish(BBitmap* bitmap);
			bool				IsCacheValid() const;

	virtual	bool				DrawLastLine(BBitmap* bitmap,
											 BRect& updateRect) = 0;
	virtual	void				Draw(BBitmap* bitmap) = 0;
	virtual	void				Draw(BBitmap* bitmap,
									 BRect constrainRect) = 0;

	// the next functions implement the calculating the
	// bounding box of the stroke (the area in which it modifies
	// the canvas visually)
			void				Touch(BRect area);

	virtual	void				LayerBoundsChanged(const BRect& layerBounds);

	virtual	BRect				Bounds() const;
	virtual	void				Update();

	virtual	void				Reset();

	virtual	bool				IsPickable(uint32 pickMask) const;
	virtual	bool				HitTest(const BPoint& where,
										BBitmap* alphaMap);
	virtual	bool				HitTest(const BRect& area,
										BBitmap* alphaMap);

	// deletes all the tracking points
	virtual	void				MakeEmpty();

	// this function can be used to modify the area in which
	// the canvas is rebuild to apply changes by doing undo or
	// reordering the modifier stack. For example, the clone
	// can extend the area by the offset that it uses to copy
	// one part of the bitmap into another part.
	virtual	void				ExtendRebuildArea(BRect& area,
												  BRect layerBounds) const
									{ }

	virtual	bool				IsTransformable() const
									{ return true; }


			MaskRenderer*		DefaultRenderer() const;
			void				SetRenderer(MaskRenderer* renderer);
// TODO: rename these to ModifyBitmap
	virtual	void				MergeWithBitmap(BBitmap* dest,
												BBitmap* strokeBitmap,
												BRect area,
												uint32 colorSpace) const;
	virtual	void				MergeWithBitmap(BBitmap* from,
												BBitmap* dest,
												BBitmap* strokeBitmap,
												BRect area,
												uint32 colorSpace) const;

	virtual	const char*			Name() const = 0;
	virtual	int32				ToolID() const = 0;

			void				SetColor(rgb_color);
			rgb_color			Color() const;
	virtual	bool				IsColorObject() const;
	
			void				SetGradient(const Gradient* gradient);
			Gradient*			GetGradient() const;

	virtual	ObjectItemPainter*	Painter() const;
	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

			void				SetMode(uint32 mode);
	inline	uint32				Mode() const
									{ return fMode; }

			void				SetTrackPoints(const BList& points);

	virtual	void				PrintToStream() const;

 protected:
			void				_MakeEmpty();
			void				_SetCache(BBitmap* bitmap, BRect frame);
			void				_InitCacheMatrix();

			BList				fTrackPoints;
			rgb_color			fColor;
			Gradient*			fGradient;
			uint32				fMode;

			BRect				fBounds;

			RLEBuffer*			fCache;
			// the matrix at which the Cache has been updated
			double				fCacheMatrix[9];

			MaskRenderer*		fRenderer;
};

#endif	// STROKE_H
