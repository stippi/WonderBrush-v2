// ChangeCropRectAction.h

#ifndef CHANGE_CROP_RECT_ACTION_H
#define CHANGE_CROP_RECT_ACTION_H

#include <Rect.h>

#include "Action.h"

class CropState;

class ChangeCropRectAction : public Action {
 public:
								ChangeCropRectAction(CropState* state,
													 BRect cropRect);
	virtual						~ChangeCropRectAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual	status_t			Undo(CanvasView* view);
	virtual	status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 protected:
			CropState*			fCropState;
			BRect				fCropRect;
};

#endif // CHANGE_CROP_RECT_ACTION_H
