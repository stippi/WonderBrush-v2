// TransformBox.h

#ifndef TRANSFORM_BOX_H
#define TRANSFORM_BOX_H

#include "AdvancedTransformable.h"

class Action;
class BView;
class CanvasView;
class DragState;
class TransformAction;

class TransformBox : public AdvancedTransformable {
 public:
								TransformBox(CanvasView* canvasView,
											 BRect box);
								TransformBox(const TransformBox& other);
	virtual						~TransformBox();

			void				Set(const Transformable& t,
									BPoint centerOffset);

	virtual	BRect				Bounds() const;
	virtual	void				Update(bool deep = true);

			void				Draw(BView* into);

			void				OffsetOrigin(BPoint offset);
			BPoint				CenterOffset() const
									{ return fCenterOffset; }
			BPoint				Origin() const;
			void				SetBox(BRect box);
			BRect				Box() const
									{ return fOriginalBox; }

			Action*				FinishTransaction();

	virtual	void				MouseDown(BPoint canvasWhere);
	virtual	void				MouseMoved(BPoint canvasWhere,
										   float canvasZoom);
	virtual	Action*				MouseUp();

			void				NudgeBy(BPoint offset);
			bool				IsNudging() const
									{ return fNudging; }
			Action*				FinishNudging();

	virtual	void				TransformFromCanvas(BPoint& point) const;
	virtual	void				TransformToCanvas(BPoint& point) const;


			void				ModifiersChanged(uint32 modifiers);

			void				UpdateToolCursor();

	virtual	TransformAction*	MakeAction(const char* actionName,
										   uint32 nameIndex) const = 0;

			void				ConvertFromLocal(BPoint* translation,
												 double rotation,
												 double xScale,
												 double yScale) const;
			BPoint				LocalTranslation() const;

	static	BPoint				LocalTranslation(BPoint origin,
												 const Transformable& transform);

			bool				IsRotating() const
									{ return fCurrentState == fRotateState; }
	virtual	double				ViewSpaceRotation() const;

	// added for perspective DragState
			void				OffsetQuadLeftTop(BPoint offset);
			void				OffsetQuadRightTop(BPoint offset);
			void				OffsetQuadLeftBottom(BPoint offset);
			void				OffsetQuadRightBottom(BPoint offset);
			void				SetQuad(const BPoint& lt,
										const BPoint& rt,
										const BPoint& lb,
										const BPoint& rb);

	inline	BPoint				LeftTop() const { return fLeftTop; }
	inline	BPoint				RightTop() const { return fRightTop; }
	inline	BPoint				LeftBottom() const { return fLeftBottom; }
	inline	BPoint				RightBottom() const { return fRightBottom; }

 private:
			void				_UpdateFromQuad(const BPoint& lt,
												const BPoint& rt,
												const BPoint& lb,
												const BPoint& rb);
			void				_UpdateAffine();

			DragState*			_DragStateFor(BPoint canvasWhere,
											  float canvasZoom);
			void				_StrokeBWLine(BView* into,
											  BPoint from, BPoint to) const;
			void				_StrokeBWPoint(BView* into,
											   BPoint point, double angle) const;

			CanvasView*			fCanvasView;

			BRect				fOriginalBox;

			BPoint				fLeftTop;
			BPoint				fRightTop;
			BPoint				fLeftBottom;
			BPoint				fRightBottom;

			BPoint				fOrigin;
			BPoint				fCenterOffset;

			TransformAction*	fCurrentAction;
			DragState*			fCurrentState;
			bool				fDragging;
			BPoint				fMousePos;
			uint32				fModifiers;

			bool				fNudging;

 protected:
			// "static" state objects
			void				_SetState(DragState* state);

			DragState*			fDragLTState;
			DragState*			fDragRTState;
			DragState*			fDragLBState;
			DragState*			fDragRBState;

			DragState*			fDragLTPerspState;
			DragState*			fDragRTPerspState;
			DragState*			fDragLBPerspState;
			DragState*			fDragRBPerspState;

			DragState*			fDragLState;
			DragState*			fDragRState;
			DragState*			fDragTState;
			DragState*			fDragBState;

			DragState*			fRotateState;
			DragState*			fTranslateState;
			DragState*			fOffsetCenterState;
};

#endif // TRANSFORM_BOX_H
