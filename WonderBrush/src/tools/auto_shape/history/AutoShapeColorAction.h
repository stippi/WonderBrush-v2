// AutoShapeColorAction.h

#ifndef AUTO_SHAPE_COLOR_ACTION_H
#define AUTO_SHAPE_COLOR_ACTION_H

#include "AutoShapeAction.h"

// TODO: this class is pretty much the same as
// SetShapeColorAction, except it does not inherit from
// PathAction and takes an EllipseState instead of ShapeState :-(

class AutoShapeColorAction : public AutoShapeAction {
 public:
								AutoShapeColorAction(AutoShapeState* state,
													 ShapeStroke* object);
	virtual						~AutoShapeColorAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
 			rgb_color			fColor;
};

#endif // AUTO_SHAPE_COLOR_ACTION_H
