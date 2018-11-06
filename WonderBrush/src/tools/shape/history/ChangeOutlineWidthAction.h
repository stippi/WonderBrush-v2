// ChangeOutlineWidthAction.h

#ifndef CHANGE_OUTLINE_WIDTH_ACTION_H
#define CHANGE_OUTLINE_WIDTH_ACTION_H

#include "PathAction.h"

class ShapeStroke;

class ChangeOutlineWidthAction : public PathAction {
 public:
								ChangeOutlineWidthAction(ShapeState* state,
													  	 ShapeStroke* modifier);
	virtual						~ChangeOutlineWidthAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			float				fOutlineWidth;
};

#endif // CHANGE_OUTLINE_WIDTH_ACTION_H
