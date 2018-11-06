// ChangeCapModeAction.h

#ifndef CHANGE_CAP_MODE_ACTION_H
#define CHANGE_CAP_MODE_ACTION_H

#include "PathAction.h"

class ShapeStroke;

class ChangeCapModeAction : public PathAction {
 public:
								ChangeCapModeAction(ShapeState* state,
													  	 ShapeStroke* modifier);
	virtual						~ChangeCapModeAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			uint32				fCapMode;
};

#endif // CHANGE_CAP_MODE_ACTION_H
