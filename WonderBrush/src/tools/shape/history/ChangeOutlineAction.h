// ChangeOutlineAction.h

#ifndef CHANGE_OUTLINE_ACTION_H
#define CHANGE_OUTLINE_ACTION_H

#include "PathAction.h"

class ChangeOutlineAction : public PathAction {
 public:
								ChangeOutlineAction(ShapeState* state,
												 	ShapeStroke* modifier);
	virtual						~ChangeOutlineAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			bool				fOutline;
};

#endif // CHANGE_OUTLINE_ACTION_H
