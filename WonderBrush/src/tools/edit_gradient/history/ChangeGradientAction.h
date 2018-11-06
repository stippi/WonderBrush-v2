// ChangeGradientAction.h

#ifndef CHANGE_GRADIENT_ACTION_H
#define CHANGE_GRADIENT_ACTION_H

#include "GradientAction.h"

class Gradient;

class ChangeGradientAction : public GradientAction {
 public:
								ChangeGradientAction(EditGradientState* state,
													 Stroke* object);
	virtual						~ChangeGradientAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

	virtual	bool				CombineWithNext(const Action* next);

 private:
			Gradient*			fGradient;
};

#endif // SET_TEXT_ACTION_H
