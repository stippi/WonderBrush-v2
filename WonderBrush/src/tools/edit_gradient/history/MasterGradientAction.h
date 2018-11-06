// MasterGradientAction.h

#ifndef MASTER_GRADIENT_ACTION_H
#define MASTER_GRADIENT_ACTION_H

#include "LayerAction.h"

class Gradient;
class Stroke;

class MasterGradientAction : public LayerAction {
 public:
								MasterGradientAction(Layer* layer,
													 Stroke* object);
	virtual						~MasterGradientAction();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

			// MasterGradientAction
			void				UpdateTransform(const Transformable& transform);

 private:
			Gradient*			fGradient;
			Stroke*				fObject;
};

#endif // MASTER_GRADIENT_ACTION_H
