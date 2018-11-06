// ChangeTextInsetAction.h

#ifndef CHANGE_TEXT_INSET_ACTION_H
#define CHANGE_TEXT_INSET_ACTION_H

#include "TextAction.h"

class ChangeTextInsetAction : public TextAction {
 public:
								ChangeTextInsetAction(TextState* state,
													  TextStroke* modifier);
	virtual						~ChangeTextInsetAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

	virtual	bool				CombineWithNext(const Action* next);

 private:
 			float				fTextInset;
};

#endif // CHANGE_TEXT_INSET_ACTION_H
