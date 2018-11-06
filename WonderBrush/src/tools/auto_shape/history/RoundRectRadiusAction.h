// RoundRectRadiusAction.h

#ifndef ROUND_RECT_RADIUS_ACTION_H
#define ROUND_RECT_RADIUS_ACTION_H

#include "Action.h"

class RoundRectState;

class RoundRectRadiusAction : public Action {
 public:
								RoundRectRadiusAction(RoundRectState* state,
														    float radius);
	virtual						~RoundRectRadiusAction();

	virtual status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

	virtual	bool				CombineWithNext(const Action* next);

 private:
			RoundRectState*		fState;
			float				fCornerRadius;
};

#endif // ROUND_RECT_RADIUS_ACTION_H
