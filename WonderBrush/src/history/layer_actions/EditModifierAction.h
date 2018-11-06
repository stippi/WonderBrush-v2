// EditModifierAction.h

#ifndef EDIT_MODIFIER_ACTION_H
#define EDIT_MODIFIER_ACTION_H

#include "LayerAction.h"

class Layer;

class EditModifierAction : public LayerAction {
 public:
								EditModifierAction(Layer* layer,
											   	   Stroke* modifier);
	virtual						~EditModifierAction();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
	Stroke*						fModifier;
	Stroke*						fClonedModifier;
};

#endif // EDIT_PATH_ACTION_H
