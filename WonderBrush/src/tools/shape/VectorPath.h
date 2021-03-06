// VectorPath

#ifndef VECTOR_PATH_H
#define VECTOR_PATH_H

#include <Archivable.h>
#include <Rect.h>

#include <agg_path_storage.h>

#include "defines.h"

#include "Transformable.h"

class BBitmap;
class BMessage;
class BView;

struct control_point {
	BPoint		point;		// actual point on path
	BPoint		point_in;	// control point for incomming curve
	BPoint		point_out;	// control point for outgoing curve
	bool		connected;	// if all 3 points should be on one line
};

class VectorPath : public BArchivable,
				   public Transformable {
 public:

	class Iterator {
	 public:
								Iterator() {}
		virtual					~Iterator() {}
	
		virtual	void			MoveTo(BPoint point) = 0;
		virtual	void			LineTo(BPoint point) = 0;
	};

								VectorPath();
								VectorPath(const VectorPath& from);
								VectorPath(const BMessage* archive);
	virtual						~VectorPath();

			VectorPath&			operator=(const VectorPath& from);
//			bool				operator==(const VectorPath& frrom) const;

			void				MakeEmpty();

								// the BArchivable protocoll
			status_t			Archive(BMessage* into, bool deep = true) const;

			bool				AddPoint(BPoint point);
			bool				AddPoint(BPoint point, int32 index);

			bool				RemovePoint(int32 index);

								// modify existing points position
			bool				SetPoint(int32 index, BPoint point);
			bool				SetPoint(int32 index, BPoint point,
													  BPoint pointIn,
													  BPoint pointOut,
													  bool connected);
			bool				SetPointIn(int32 index, BPoint point);
			bool				SetPointOut(int32 index, BPoint point,
										   bool mirrorDist = false);

			bool				SetInOutConnected(int32 index, bool connected);

								// query existing points position
			bool				GetPointAt(int32 index, BPoint& point) const;
			bool				GetPointInAt(int32 index, BPoint& point) const;
			bool				GetPointOutAt(int32 index, BPoint& point) const;
			bool				GetPointsAt(int32 index,
											BPoint& point, 
											BPoint& pointIn,
											BPoint& pointOut,
											bool* connected = NULL) const;

			int32				CountPoints() const;

								// iterates over curve segments and returns
								// the distance and index of the point that
								// started the segment that is closest
			bool				GetDistance(BPoint point,
											float* distance, int32* index) const;

								// at curve segment indicated by "index", this
								// function looks for the closest point
								// directly on the curve and returns a "scale"
								// that indicates the distance on the curve
								// between [0..1]
			bool				FindBezierScale(int32 index, BPoint point,
												double* scale) const;
								// this function can be used to get a point
								// directly on the segment indicated by "index"
								// "scale" is on [0..1] indicating the distance
								// from the start of the segment to the end
			bool				GetPoint(int32 index, double scale,
										 BPoint& point) const;

			void				SetClosed(bool closed);
			bool				IsClosed() const
									{ return fClosed; }

			BRect				Bounds() const;
			BRect				Bounds(bool outline,
									   float outlineWidth = 1.0,
									   uint32 capMode = CAP_MODE_BUTT,
									   uint32 joinMode = JOIN_MODE_MITER) const;
			BRect				ControlPointBounds() const;

	virtual	void				TransformationChanged();

/*			bool				Draw(const BBitmap* alphaMap, BRect area,
									 uint8 cover = 255,
									 bool outline = false,
									 float outlineWidth = 1.0,
									 uint32 capMode = CAP_MODE_BUTT,
									 uint32 joinMode = JOIN_MODE_MITER,
									 bool solid = false) const;*/

			void				Iterate(Iterator* iterator,
										float smoothScale = 1.0) const;

			void				CleanUp();
			void				FreezeTransformations();
			void				Reverse();

			void				PrintToStream() const;

			bool				GetAGGPathStorage(agg::path_storage& path,
												  bool outlineMode) const;

 private:
			bool				_SetPoint(int32 index, BPoint point);
			bool				_SetPointCount(int32 count);

			control_point*		fPath;

			bool				fClosed;

			int32				fPointCount;
			int32				fAllocCount;

	mutable	BRect				fCachedBounds;
};

#endif // VECTOR_PATH_H

