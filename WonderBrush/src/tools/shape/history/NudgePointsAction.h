// NudgePointsAction.h

#ifndef NUDGE_POINTS_ACTION_H
#define NUDGE_POINTS_ACTION_H

#include <String.h>

#include "TransformAction.h"

class ShapeState;
class ShapeStroke;

struct control_point;

class NudgePointsAction : public TransformAction {
 public:
								NudgePointsAction(ShapeState* state,
												  ShapeStroke* object,

												  const int32* indices,
												  const control_point* points,
												  int32 count);

	virtual						~NudgePointsAction();
	
	virtual	status_t			InitCheck();

 protected:
	virtual	status_t			_SetTransformation(CanvasView* view,
//												   BPoint translation,
//												   double rotation,
//												   double xScale,
//												   double yScale,
												   const Transformable& t,
												   BPoint centerOffset) const;

			ShapeState*			fShapeState;
			ShapeStroke*		fShapeObject;

			int32*				fIndices;
			control_point*		fPoints;
			int32				fCount;
};

#endif // NUDGE_POINTS_ACTION_H
