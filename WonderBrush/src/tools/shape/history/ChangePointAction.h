// ChangePointAction.h

#ifndef CHANGE_POINT_ACTION_H
#define CHANGE_POINT_ACTION_H

#include "PathAction.h"

class ChangePointAction : public PathAction {
 public:
								ChangePointAction(ShapeState* state,
												  ShapeStroke* modifier,
												  int32 index,
												  const int32* selected,
												  int32 count);
	virtual						~ChangePointAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			int32				fIndex;

			BPoint				fPoint;
			BPoint				fPointIn;
			BPoint				fPointOut;
			bool				fConnected;

			int32*				fOldSelection;
			int32				fOldSelectionCount;
};

#endif // CHANGE_POINT_ACTION_H
