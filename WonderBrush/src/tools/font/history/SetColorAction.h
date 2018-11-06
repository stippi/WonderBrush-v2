// SetColorAction.h

#ifndef SET_COLOR_ACTION_H
#define SET_COLOR_ACTION_H

#include "TextAction.h"

class SetColorAction : public TextAction {
 public:
								SetColorAction(TextState* state,
											   TextStroke* modifier);
	virtual						~SetColorAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
 			rgb_color			fColor;
};

#endif // SET_COLOR_ACTION_H
