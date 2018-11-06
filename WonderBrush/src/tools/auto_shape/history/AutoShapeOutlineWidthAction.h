// AutoShapeOutlineWidthAction.h

#ifndef AUTO_SHAPE_OUTLINE_WIDTH_ACTION_H
#define AUTO_SHAPE_OUTLINE_WIDTH_ACTION_H

#include "AutoShapeAction.h"

class AutoShapeOutlineWidthAction : public AutoShapeAction {
 public:
								AutoShapeOutlineWidthAction(AutoShapeState* state,
														    ShapeStroke* object);
	virtual						~AutoShapeOutlineWidthAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			float				fOutlineWidth;
};

#endif // AUTO_SHAPE_OUTLINE_WIDTH_ACTION_H
