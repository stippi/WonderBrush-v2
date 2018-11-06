// CropAction.h

#ifndef CROP_ACTION_H
#define CROP_ACTION_H

#include "CanvasAction.h"

class Layer;

class CropAction : public CanvasAction {
 public:
								CropAction(Canvas* canvas,
										   BRect newBounds);
	virtual						~CropAction();
	
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

#endif // MERGE_LAYERS_ACTION_H
