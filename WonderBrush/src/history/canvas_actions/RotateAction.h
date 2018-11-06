// RotateAction.h

#ifndef ROTATE_ACTION_H
#define ROTATE_ACTION_H

#include "CanvasAction.h"

class Layer;

class RotateAction : public CanvasAction {
 public:
								RotateAction(Canvas* canvas,
											 double angle);
	virtual						~RotateAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			status_t			_Rotate(CanvasView* view,
										double angle);

			double				fAngle;
};

#endif // ROTATE_ACTION_H
