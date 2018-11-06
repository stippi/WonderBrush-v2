// ResizeAction.h

#ifndef RESIZE_ACTION_H
#define RESIZE_ACTION_H

#include "CanvasAction.h"

class Layer;

class ResizeAction : public CanvasAction {
 public:
								ResizeAction(Canvas* canvas,
											 BRect newBounds);
	virtual						~ResizeAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			status_t			_ChangeSize(CanvasView* view,
											BRect newBounds);

			BRect				fNewBounds;
			BRect				fOldBounds;
};

#endif // RESIZE_ACTION_H
