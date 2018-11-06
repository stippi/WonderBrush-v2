// AutoShapeAlphaAction.h

#ifndef AUTO_SHAPE_ALPHA_ACTION_H
#define AUTO_SHAPE_ALPHA_ACTION_H

#include "AutoShapeAction.h"

class AutoShapeAlphaAction : public AutoShapeAction {
 public:
								AutoShapeAlphaAction(AutoShapeState* state,
													 ShapeStroke* object);
	virtual						~AutoShapeAlphaAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			uint8				fAlpha;
};

#endif // AUTO_SHAPE_ALPHA_ACTION_H
