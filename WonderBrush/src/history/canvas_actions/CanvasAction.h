// CanvasAction.h

#ifndef CANVAS_ACTION_H
#define CANVAS_ACTION_H

#include "Action.h"

class Canvas;

class CanvasAction : public Action {
 public:
								CanvasAction(Canvas* canvas);
	virtual						~CanvasAction();
	
	virtual	status_t			InitCheck();
	virtual void				GetName(BString& name);

 protected:
			Canvas*				fCanvas;

};

#endif // CANVAS_ACTION_H
