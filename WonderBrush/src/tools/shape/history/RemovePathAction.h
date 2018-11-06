// RemovePathAction.h

#ifndef REMOVE_PATH_ACTION_H
#define REMOVE_PATH_ACTION_H

#include "PathAction.h"

class RemovePathAction : public PathAction {
 public:
								RemovePathAction(ShapeState* state,
												 ShapeStroke* modifier);
	virtual						~RemovePathAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			bool				fRemoved;
			int32				fIndex;
};

#endif // REMOVE_PATH_ACTION_H
