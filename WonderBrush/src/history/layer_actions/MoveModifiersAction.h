// MoveModifiersAction.h

#ifndef MOVE_MODIFIERS_ACTION_H
#define MOVE_MODIFIERS_ACTION_H

#include "LayerAction.h"

class Stroke;

class MoveModifiersAction : public LayerAction {
 public:
								MoveModifiersAction(Layer* fromLayer,
													Layer* destLayer,
													Stroke** strokes,
													int32 count,
													int32 index);
	virtual						~MoveModifiersAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			Layer*				fTargetLayer;
			Stroke**			fStrokes;
			int32*				fIndices;
			int32				fIndex;
			int32				fCount;
};

#endif // MOVE_MODIFIERS_ACTION_H
