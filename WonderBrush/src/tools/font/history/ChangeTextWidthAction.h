// ChangeTextWidthAction.h

#ifndef CHANGE_TEXT_WIDTH_ACTION_H
#define CHANGE_TEXT_WIDTH_ACTION_H

#include "TextAction.h"

class ChangeTextWidthAction : public TextAction {
 public:
								ChangeTextWidthAction(TextState* state,
													  TextStroke* modifier);
	virtual						~ChangeTextWidthAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

	virtual	bool				CombineWithNext(const Action* next);

 private:
 			float				fTextWidth;
};

#endif // CHANGE_TEXT_WIDTH_ACTION_H
