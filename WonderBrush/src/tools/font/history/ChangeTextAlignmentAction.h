// ChangeTextAlignmentAction.h

#ifndef CHANGE_TEXT_ALIGNMENT_ACTION_H
#define CHANGE_TEXT_ALIGNMENT_ACTION_H

#include "TextAction.h"

class ChangeTextAlignmentAction : public TextAction {
 public:
								ChangeTextAlignmentAction(TextState* state,
													 TextStroke* modifier);
	virtual						~ChangeTextAlignmentAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
 			uint32				fAlignment;
};

#endif // CHANGE_TEXT_ALIGNMENT_ACTION_H
