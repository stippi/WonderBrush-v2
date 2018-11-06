// EnterTransformPointsAction.h

#ifndef ENTER_TRANSFORM_POINTS_ACTION_H
#define ENTER_TRANSFORM_POINTS_ACTION_H

#include <String.h>

#include "Action.h"

class ShapeState;
class ShapeStroke;

class EnterTransformPointsAction : public Action {
 public:
								EnterTransformPointsAction(ShapeState* state,
													 ShapeStroke* object,
													 const int32* indices,
													 int32 count);

	virtual						~EnterTransformPointsAction();
	
	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual	status_t			InitCheck();
	virtual void				GetName(BString& name);

 protected:
			ShapeState*			fShapeState;
			ShapeStroke*		fShapeObject;

			int32*				fIndices;
			int32				fCount;
};

#endif // ENTER_TRANSFORM_POINTS_ACTION_H
