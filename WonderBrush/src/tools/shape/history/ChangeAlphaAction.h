// ChangeAlphaAction.h

#ifndef CHANGE_ALPHA_ACTION_H
#define CHANGE_ALPHA_ACTION_H

#include "PathAction.h"

class ChangeAlphaAction : public PathAction {
 public:
								ChangeAlphaAction(ShapeState* state,
												  ShapeStroke* modifier);
	virtual						~ChangeAlphaAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			uint8				fAlpha;
};

#endif // CHANGE_ALPHA_ACTION_H
