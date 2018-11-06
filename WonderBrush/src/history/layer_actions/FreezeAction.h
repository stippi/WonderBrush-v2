// FreezeAction.h

#ifndef FREEZE_ACTION_H
#define FREEZE_ACTION_H

#include "LayerAction.h"

class BitmapStroke;
class Stroke;

class FreezeAction : public LayerAction {
 public:
								FreezeAction(Layer* layer);
	virtual						~FreezeAction();

	virtual	status_t			InitCheck();
	
	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 protected:
			Stroke**			fStrokes;
			int32				fCount;
			bool				fItemsRemoved;
			BitmapStroke*		fBitmapStroke;
};

#endif // FREEZE_ACTION_H
