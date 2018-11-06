// SetTextAction.h

#ifndef SET_TEXT_ACTION_H
#define SET_TEXT_ACTION_H

#include "TextAction.h"

class SetTextAction : public TextAction {
 public:
								SetTextAction(TextState* state,
											  TextStroke* modifier);
	virtual						~SetTextAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
 			char*				fText;
};

#endif // SET_TEXT_ACTION_H
