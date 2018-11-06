// SetShapeColorAction.h

#ifndef SET_CHAPE_COLOR_ACTION_H
#define SET_CHAPE_COLOR_ACTION_H

#include "PathAction.h"

class SetShapeColorAction : public PathAction {
 public:
								SetShapeColorAction(ShapeState* state,
													ShapeStroke* modifier);
	virtual						~SetShapeColorAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
 			rgb_color			fColor;
};

#endif // SET_CHAPE_COLOR_ACTION_H
