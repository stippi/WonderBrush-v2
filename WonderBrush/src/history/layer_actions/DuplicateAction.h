// DuplicateAction.h

#ifndef DUPLICATE_ACTION_H
#define DUPLICATE_ACTION_H

#include "LayerAction.h"

class Stroke;

class DuplicateAction : public LayerAction {
 public:
								DuplicateAction(Layer* layer,
												const Stroke** strokes,
												int32 count,
												int32 index,

												int32 duplicateCount,

												float xOffset,
												float yOffset,
												float rotation,
												float xScale,
												float yScale);
	virtual						~DuplicateAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			Stroke**			fStrokes;
			int32				fIndex;
			int32				fCount;
			bool				fItemsRemoved;
};

#endif // DUPLICATE_ACTION_H
