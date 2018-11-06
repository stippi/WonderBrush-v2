// ChangeTransformationAction.h

#ifndef CHANGE_TRANSFORMATION_ACTION_H
#define CHANGE_TRANSFORMATION_ACTION_H

#include "TransformAction.h"

class TransformState;

class ChangeTransformationAction : public TransformAction {
 public:
								ChangeTransformationAction(TransformState* state,

//														   BPoint translation,
//														   double rotation,
//														   double xScale,
//														   double yScale,
														   const Transformable& t,
														   BPoint centerOffset,

														   const char* actionName,
														   int32 actionNameIndex);

	virtual						~ChangeTransformationAction();

	virtual	status_t			InitCheck();

 protected:
	virtual	status_t			_SetTransformation(CanvasView* view,
//												   BPoint translation,
//												   double rotation,
//												   double xScale,
//												   double yScale,
												   const Transformable& t,
												   BPoint centerOffset) const;

			TransformState*		fTransformState;
};

#endif // CHANGE_TRANSFORMATION_ACTION_H
