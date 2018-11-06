// TransformGradientAction.h

#ifndef TRANSFORM_GRADIENT_ACTION_H
#define TRANSFORM_GRADIENT_ACTION_H

#include "TransformAction.h"

class GradientBox;

class TransformGradientAction : public TransformAction {
 public:
								TransformGradientAction(const GradientBox* box,

//														BPoint translation,
//														double rotation,
//														double xScale,
//														double yScale,
														const Transformable& t,
														BPoint centerOffset,

														const char* actionName,
														uint32 nameIndex);

	virtual						~TransformGradientAction();
	
	virtual	status_t			InitCheck();

 protected:
	virtual	status_t			_SetTransformation(CanvasView* view,
//												   BPoint translation,
//												   double rotation,
//												   double xScale,
//												   double yScale,
												   const Transformable& t,
												   BPoint centerOffset) const;

			GradientBox*		fBox;
};

#endif // TRANSFORM_GRADIENT_ACTION_H
