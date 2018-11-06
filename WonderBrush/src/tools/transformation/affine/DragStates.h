// DragStates.h

#ifndef DRAG_STATES_H
#define DRAG_STATES_H

#include <Point.h>

#include <agg_trans_affine.h>

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

 private:
			uint32				fSide;

			float				fOffsetFromSide;
			double				fOldXScale;
			double				fOldYScale;
			double				fOldSideDist;
			agg::trans_affine	fMatrix;
			BPoint				fOldOffset;
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

 private:
			BPoint				fOldTranslation;
};

// rotate state
class RotateBoxState : public DragState {
 public:
								RotateBoxState(TransformBox* parent);
	virtual						~RotateBoxState() {}

	virtual	void				SetOrigin(BPoint origin);
	virtual	void				DragTo(BPoint current, uint32 modifiers);
	virtual	void				UpdateViewCursor(BView* view, BPoint current) const;

 private:
			double				_Angle(BPoint origin, BPoint from, BPoint to) const;
			double				fOldAngle;
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
};

#endif // DRAG_STATES_H
