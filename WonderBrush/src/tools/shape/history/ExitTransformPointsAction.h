// ExitTransformPointsAction.h

#ifndef EXIT_TRANSFORM_POINTS_ACTION_H
#define EXIT_TRANSFORM_POINTS_ACTION_H

#include <String.h>

#include "Action.h"
#include "Transformable.h"

class ShapeState;
class ShapeStroke;

struct control_point;

class ExitTransformPointsAction : public Action {
 public:
								ExitTransformPointsAction(ShapeState* state,
													ShapeStroke* object,
													const int32* indices,
													const control_point* points,
													int32 count,

//													BPoint translation,
//													double rotation,
//													double xScale,
//													double yScale,
													const Transformable& t,
													
													bool cancel);

	virtual						~ExitTransformPointsAction();
	
	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual	status_t			InitCheck();
	virtual void				GetName(BString& name);

 protected:
			ShapeState*			fShapeState;
			ShapeStroke*		fShapeObject;

			int32*				fIndices;
			control_point*		fPoints;
			int32				fCount;

//			BPoint				fTranslation;
//			double				fRotation;
//			double				fXScale;
//			double				fYScale;
			Transformable		fTransformation;

			bool				fCancel;
};

#endif // EXIT_TRANSFORM_POINTS_ACTION_H
