// AutoShapeBox.h

#ifndef AUTO_SHAPE_BOX_H
#define AUTO_SHAPE_BOX_H

#include "TransformBox.h"

class Action;
class CanvasView;
class AutoShapeState;
class ShapeStroke;
class VectorPath;

struct control_point;

class AutoShapeBox : public TransformBox {
 public:
								AutoShapeBox(CanvasView* view,
											 AutoShapeState* state,
											 ShapeStroke* shape);
	virtual						~AutoShapeBox();

//	virtual	BRect				Bounds() const;
	virtual	void				Update(bool deep = true);

								// convinience function
			Action*				Perform();
			Action*				Cancel();

	virtual	TransformAction*	MakeAction(const char* actionName,
										   uint32 nameIndex) const;

			void				DragRightBottom();

			void				SetToShape(ShapeStroke* object);

 private:
			AutoShapeState*		fState;
			ShapeStroke*		fShape;

			VectorPath*			fPath;
			BRect				fBounds;

			// a backup of the original points
			control_point*		fPoints;
			int32				fCount;

	mutable	bool				fNoAction;
};

#endif // AUTO_SHAPE_BOX_H

