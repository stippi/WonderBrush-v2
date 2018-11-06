// AppendCanvasAction.h

#ifndef APPEND_CANVAS_ACTION_H
#define APPEND_CANVAS_ACTION_H

#include "CanvasAction.h"

class Layer;

class AppendCanvasAction : public CanvasAction {
 public:
								AppendCanvasAction(Canvas* dest,
												   const Canvas* source,
												   int32 appendIndex = -1);
	virtual						~AppendCanvasAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			Layer**				fLayers;
			int32				fCount;

			int32				fAppendIndex;

			bool				fAppended;
};

#endif // APPEND_CANVAS_ACTION_H
