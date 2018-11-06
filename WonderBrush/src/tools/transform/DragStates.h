// DragStates.h

#ifndef DRAG_STATES_H
#define DRAG_STATES_H

#include <Point.h>

#include "Transformable.h"

class BView;
class TransformBox;

// base class
class DragState {
 public:
								DragState(TransformBox* parent);
	virtual						~DragState() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers) = 0;
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const = 0;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 protected:
			void				_SetViewCursor(BView* view,
											   const uchar* cursorData) const;

			BPoint				fOrigin;
			TransformBox*		fParent;
};

// scaling states
class DragCornerState : public DragState {
 public:
	enum {
		LEFT_TOP_CORNER,
		RIGHT_TOP_CORNER,
		LEFT_BOTTOM_CORNER,
		RIGHT_BOTTOM_CORNER,
	};
								DragCornerState(TransformBox* parent,
												uint32 corner);
	virtual						~DragCornerState() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 private:
			uint32				fCorner;

			float				fXOffsetFromCorner;
			float				fYOffsetFromCorner;
			double				fOldXScale;
			double				fOldYScale;
			double				fOldWidth;
			double				fOldHeight;
			agg::trans_affine	fMatrix;
			BPoint				fOldOffset;
};

// scaling states
class ScaleCornerState : public DragState {
 public:
	enum {
		LEFT_TOP_CORNER,
		RIGHT_TOP_CORNER,
		LEFT_BOTTOM_CORNER,
		RIGHT_BOTTOM_CORNER,
	};
								ScaleCornerState(TransformBox* parent,
												 uint32 corner);
	virtual						~ScaleCornerState() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 private:
			uint32				fCorner;
			double				fVectorX;
			double				fVectorY;
			double				fLtX;
			double				fLtY;
			double				fRtX;
			double				fRtY;
			double				fLbX;
			double				fLbY;
			double				fRbX;
			double				fRbY;
};

class ScaleCornerState2 : public DragState {
 public:
	enum {
		LEFT_TOP_CORNER,
		RIGHT_TOP_CORNER,
		LEFT_BOTTOM_CORNER,
		RIGHT_BOTTOM_CORNER,
	};
								ScaleCornerState2(TransformBox* parent,
												  uint32 corner);
	virtual						~ScaleCornerState2() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 private:
			uint32				fCorner;

			struct vector {
				vector()
					: x(0),
					  y(0)
				{}
				vector(BPoint a, BPoint b)
					: x(b.x - a.x),
					  y(b.y - a.y)
				{}
				vector(const vector& other)
					: x(other.x),
					  y(other.y)
				{}
				vector& operator=(const vector& other)
				{
					x = other.x;
					y = other.y;
					return *this;
				}

				double x;
				double y;
			};

			// anchor corner (C)
			BPoint				fAnchor;
			// anchor corner (C) to neighbor corners
			vector				fCD;
			vector				fCB;
			// dragged corner (A) to neighbor corners
			vector				fAD;
			vector				fAB;
};


class ScaleCornerState3 : public DragState {
 public:
	enum {
		LEFT_TOP_CORNER,
		RIGHT_TOP_CORNER,
		LEFT_BOTTOM_CORNER,
		RIGHT_BOTTOM_CORNER,
	};
								ScaleCornerState3(TransformBox* parent,
												  uint32 corner);
	virtual						~ScaleCornerState3() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 private:
			uint32				fCorner;

			BPoint				fOffsetFromCorner;
			Transformable		fOldTransform;
};

class DragSideState : public DragState {
 public:
	enum {
		LEFT_SIDE,
		TOP_SIDE,
		RIGHT_SIDE,
		BOTTOM_SIDE,
	};
								DragSideState(TransformBox* parent,
											  uint32 side);
	virtual						~DragSideState() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 private:
			uint32				fSide;

			float				fOffsetFromSide;
			double				fOldXScale;
			double				fOldYScale;
			double				fOldSideDist;
			agg::trans_affine	fMatrix;
			BPoint				fOldOffset;
};

class ScaleSideState : public DragState {
 public:
	enum {
		LEFT_SIDE,
		TOP_SIDE,
		RIGHT_SIDE,
		BOTTOM_SIDE,
	};
								ScaleSideState(TransformBox* parent,
											   uint32 side);
	virtual						~ScaleSideState() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 private:
			uint32				fSide;

			float				fOffsetFromSide;
			Transformable		fOldTransform;
};

// translate state
class DragBoxState : public DragState {
 public:
								DragBoxState(TransformBox* parent)
									: DragState(parent) {}
	virtual						~DragBoxState() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 private:
			BPoint				fOldTranslation;
};

// translate state
class DragBoxState2 : public DragState {
 public:
								DragBoxState2(TransformBox* parent)
									: DragState(parent) {}
	virtual						~DragBoxState2() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 private:
			Transformable		fOldTransform;
};

// rotate state
class RotateBoxState : public DragState {
 public:
								RotateBoxState(TransformBox* parent);
	virtual						~RotateBoxState() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 private:
			double				fOldAngle;
};

// rotate state
class RotateBoxState2 : public DragState {
 public:
								RotateBoxState2(TransformBox* parent);
	virtual						~RotateBoxState2() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 private:
			Transformable		fOldTransform;
};

// offset center state
class OffsetCenterState : public DragState {
 public:
								OffsetCenterState(TransformBox* parent)
									: DragState(parent) {}
	virtual						~OffsetCenterState() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

};


// offset corner state
class OffsetCornerState : public DragState {
 public:
	enum {
		LEFT_TOP_CORNER,
		RIGHT_TOP_CORNER,
		LEFT_BOTTOM_CORNER,
		RIGHT_BOTTOM_CORNER,
	};
								OffsetCornerState(TransformBox* parent,
												  uint32 corner);
	virtual						~OffsetCornerState() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

	virtual	const char*			ActionName() const;
	virtual	uint32				ActionNameIndex() const;

 private:
			uint32				fCorner;
};


#endif // DRAG_STATES_H
