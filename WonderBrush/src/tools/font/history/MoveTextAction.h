// MoveTextAction.h

#ifndef MOVE_TEXT_ACTION_H
#define MOVE_TEXT_ACTION_H

#include <Point.h>

#include "TextAction.h"

class MoveTextAction : public TextAction {
 public:
								MoveTextAction(TextState* state,
											   TextStroke* modifier,
											   BPoint offset);
	virtual						~MoveTextAction();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
 			BPoint				fOffset;
};

#endif // MOVE_TEXT_ACTION_H
