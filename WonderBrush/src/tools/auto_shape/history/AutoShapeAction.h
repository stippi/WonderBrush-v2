// AutoShapeAction.h

#ifndef AUTO_SHAPE_ACTION_H
#define AUTO_SHAPE_ACTION_H

#include "Action.h"

class AutoShapeState;
class ShapeStroke;

class AutoShapeAction : public Action {
 public:
								AutoShapeAction(AutoShapeState* state,
												ShapeStroke* object);
	virtual						~AutoShapeAction();
	
	virtual	status_t			InitCheck();
	virtual void				GetName(BString& name);

 protected:
			void				_GetPathBounds(BRect& pathBounds,
											   BRect& controlPointBounds) const;
			void				_RedrawPath(BRect pathBounds,
											BRect controlPointBounds) const;

			AutoShapeState*		fState;
			ShapeStroke*		fShape;
};

#endif // AUTO_SHAPE_ACTION_H
