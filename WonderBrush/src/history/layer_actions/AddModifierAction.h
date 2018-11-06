// AddModifierAction.h

#ifndef ADD_MODIFIER_ACTION_H
#define ADD_MODIFIER_ACTION_H

#include "LayerAction.h"

class Stroke;

class AddModifierAction : public LayerAction {
 public:
								AddModifierAction(Layer* layer,
												  Stroke* stroke);
	virtual						~AddModifierAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			Stroke*				fStroke;
			int32				fIndex;

};

#endif // ADD_MODIFIER_ACTION_H
