// TransformBox.h

#ifndef TRANSFORM_BOX_H
#define TRANSFORM_BOX_H

#include "Transformable.h"

class BView;
class CanvasView;
class DragState;

class TransformBox : public Transformable {
 public:
								TransformBox(CanvasView* canvasView,
											 BRect box);
	virtual						~TransformBox();

	virtual	BRect				Bounds();
	virtual	void				Update();

			void				Draw(BView* into);

			void				OffsetOrigin(BPoint offset);
			BPoint				Origin() const;
			void				SetBox(BRect box);
			BRect				Box() const
									{ return fOriginalBox; }

			double				Width() const;
			double				Height() const;
			double				ScaledWidth() const;
			double				ScaledHeight() const;

			void				MouseDown(BPoint where);
			void				MouseMoved(BPoint where);
			void				MouseUp();

			void				ModifiersChanged(uint32 modifiers);

			void				UpdateToolCursor();

 private:
			DragState*			_DragStateFor(BPoint where);
			void				_SetState(DragState* state);
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

			DragState*			fCurrentState;
			bool				fDragging;
			BPoint				fMousePos;
			uint32				fModifiers;

			// "static" state objects
			DragState*			fDragLTState;
			DragState*			fDragRTState;
			DragState*			fDragLBState;
			DragState*			fDragRBState;

			DragState*			fDragLState;
			DragState*			fDragRState;
			DragState*			fDragTState;
			DragState*			fDragBState;

			DragState*			fRotateState;
			DragState*			fTranslateState;
			DragState*			fOffsetCenterState;
};

#endif // TRANSFORM_BOX_H
