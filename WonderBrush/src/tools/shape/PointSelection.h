// PointSelection.h

#ifndef POINT_SELECTION_H
#define POINT_SELECTION_H

#include "TransformBox.h"

class Action;
class CanvasView;
class ShapeState;
class ShapeStroke;
class VectorPath;

struct control_point;

class PointSelection : public TransformBox {
 public:
								PointSelection(CanvasView* view,
											   ShapeState* state,
											   ShapeStroke* object,
											   VectorPath* path,
											   const int32* indices,
											   int32 count);
	virtual						~PointSelection();

	virtual	BRect				Bounds() const;
	virtual	void				Update(bool deep = true);

								// convinience function
			Action*				Perform();
			Action*				Cancel();

	virtual	TransformAction*	MakeAction(const char* actionName,
										   uint32 nameIndex) const;

 private:
			ShapeState*			fShapeState;
			ShapeStroke*		fShapeObject;

			VectorPath*			fPath;
			BRect				fBounds;

			int32*				fIndices;
			int32				fCount;

			control_point*		fPoints;
};

#endif // POINT_SELECTION_H

