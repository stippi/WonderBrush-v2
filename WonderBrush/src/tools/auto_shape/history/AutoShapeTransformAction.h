// AutoShapeTransformAction.h

#ifndef AUTO_SHAPE_TRANSFORM_ACTION_H
#define AUTO_SHAPE_TRANSFORM_ACTION_H

#include <String.h>

#include "TransformAction.h"

class AutoShapeState;
class ShapeStroke;

struct control_point;

class AutoShapeTransformAction : public TransformAction {
 public:
								AutoShapeTransformAction(AutoShapeState* state,
														 ShapeStroke* object,

														 const control_point* points,
														 int32 count,
	
//														 BPoint translation,
//														 double rotation,
//														 double xScale,
//														 double yScale,
														 const Transformable& t,
														 BPoint centerOffset,
	
														 const char* actionName,
														 uint32 nameIndex);

	virtual						~AutoShapeTransformAction();
	
	virtual	status_t			InitCheck();

 protected:
	virtual	status_t			_SetTransformation(CanvasView* view,
//												   BPoint translation,
//												   double rotation,
//												   double xScale,
//												   double yScale,
												   const Transformable& t,
												   BPoint centerOffset) const;

			AutoShapeState*		fState;
			ShapeStroke*		fShape;

			control_point*		fPoints;
			int32				fCount;
};

#endif // AUTO_SHAPE_TRANSFORM_ACTION_H
