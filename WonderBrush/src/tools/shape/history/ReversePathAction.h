// ReversePathAction.h

#ifndef REVERSE_PATH_ACTION_H
#define REVERSE_PATH_ACTION_H

#include "PathAction.h"

class ReversePathAction : public PathAction {
 public:
								ReversePathAction(ShapeState* state,
												 ShapeStroke* modifier);
	virtual						~ReversePathAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
};

#endif // REVERSE_PATH_ACTION_H
