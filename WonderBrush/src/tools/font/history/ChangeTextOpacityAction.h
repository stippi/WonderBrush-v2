// ChangeTextOpacityAction.h

#ifndef CHANGE_TEXT_OPACITY_ACTION_H
#define CHANGE_TEXT_OPACITY_ACTION_H

#include "TextAction.h"

class ChangeTextOpacityAction : public TextAction {
 public:
								ChangeTextOpacityAction(TextState* state,
													 TextStroke* modifier);
	virtual						~ChangeTextOpacityAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
 			uint8				fOpacity;
};

#endif // CHANGE_TEXT_OPACITY_ACTION_H
