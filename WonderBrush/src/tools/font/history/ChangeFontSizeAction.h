// ChangeFontSizeAction.h

#ifndef CHANGE_FONT_SIZE_ACTION_H
#define CHANGE_FONT_SIZE_ACTION_H

#include "TextAction.h"

class ChangeFontSizeAction : public TextAction {
 public:
								ChangeFontSizeAction(TextState* state,
													 TextStroke* modifier);
	virtual						~ChangeFontSizeAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
 			float				fFontSize;
};

#endif // CHANGE_FONT_SIZE_ACTION_H
