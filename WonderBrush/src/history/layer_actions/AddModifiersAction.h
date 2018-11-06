// AddModifiersAction.h

#ifndef ADD_MODIFIERS_ACTION_H
#define ADD_MODIFIERS_ACTION_H

#include "LayerAction.h"

class Stroke;

class AddModifiersAction : public LayerAction {
 public:
								AddModifiersAction(Layer* layer,
												   Stroke** strokes,
												   int32 count,
												   int32 index);
	virtual						~AddModifiersAction();
	
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

#endif // ADD_MODIFIERS_ACTION_H
