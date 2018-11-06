// ChangeFontAdvanceScaleAction.h

#ifndef CHANGE_FONT_ADVANCE_SCALE_ACTION_H
#define CHANGE_FONT_ADVANCE_SCALE_ACTION_H

#include "TextAction.h"

class ChangeFontAdvanceScaleAction : public TextAction {
 public:
								ChangeFontAdvanceScaleAction(TextState* state,
													 TextStroke* modifier);
	virtual						~ChangeFontAdvanceScaleAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
 			float				fAdvanceScale;
};

#endif // CHANGE_FONT_ADVANCE_SCALE_ACTION_H
