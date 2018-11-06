// SetFontAction.h

#ifndef SET_FONT_ACTION_H
#define SET_FONT_ACTION_H

#include "TextAction.h"

class SetFontAction : public TextAction {
 public:
								SetFontAction(TextState* state,
											  TextStroke* modifier);
	virtual						~SetFontAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
 			char*				fFamily;
 			char*				fStyle;
};

#endif // SET_FONT_ACTION_H
