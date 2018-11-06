// DeleteModifiersAction.h

#ifndef DELETE_MODIFIERS_ACTION_H
#define DELETE_MODIFIERS_ACTION_H

#include "LayerAction.h"

class Stroke;

class DeleteModifiersAction : public LayerAction {
 public:
								DeleteModifiersAction(Layer* layer,
													  Stroke** strokes,
													  int32 count);
	virtual						~DeleteModifiersAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			Stroke**			fStrokes;
			int32*				fIndices;
			int32				fCount;
			bool				fItemsRemoved;
};

#endif // DELETE_MODIFIERS_ACTION_H
