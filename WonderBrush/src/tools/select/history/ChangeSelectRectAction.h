// ChangeSelectRectAction.h

#ifndef CHANGE_SELECT_RECT_ACTION_H
#define CHANGE_SELECT_RECT_ACTION_H

#include <Rect.h>

#include "Action.h"

class SimpleSelectState;

class ChangeSelectRectAction : public Action {
 public:
								ChangeSelectRectAction(SimpleSelectState* state,
													   BRect selectRect, bool dummy = false);
	virtual						~ChangeSelectRectAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual	status_t			Undo(CanvasView* view);
	virtual	status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 protected:
			SimpleSelectState*	fSelectState;
			BRect				fSelectRect;
};

#endif // CHANGE_SELECT_RECT_ACTION_H
