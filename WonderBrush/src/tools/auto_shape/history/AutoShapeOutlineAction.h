// AutoShapeOutlineAction.h

#ifndef AUTO_SHAPE_OUTLINE_ACTION_H
#define AUTO_SHAPE_OUTLINE_ACTION_H

#include "AutoShapeAction.h"

class AutoShapeOutlineAction : public AutoShapeAction {
 public:
								AutoShapeOutlineAction(AutoShapeState* state,
													   ShapeStroke* object);
	virtual						~AutoShapeOutlineAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			bool				fOutline;
};

#endif // AUTO_SHAPE_OUTLINE_ACTION_H
