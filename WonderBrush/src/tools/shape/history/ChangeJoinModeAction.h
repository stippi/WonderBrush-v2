// ChangeJoinModeAction.h

#ifndef CHANGE_JOIN_MODE_ACTION_H
#define CHANGE_JOIN_MODE_ACTION_H

#include "PathAction.h"

class ShapeStroke;

class ChangeJoinModeAction : public PathAction {
 public:
								ChangeJoinModeAction(ShapeState* state,
													  	 ShapeStroke* modifier);
	virtual						~ChangeJoinModeAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			uint32				fJoinMode;
};

#endif // CHANGE_JOIN_MODE_ACTION_H
