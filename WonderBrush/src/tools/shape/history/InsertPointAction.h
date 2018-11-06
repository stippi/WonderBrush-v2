// InsertPointAction.h

#ifndef INSERT_POINT_ACTION_H
#define INSERT_POINT_ACTION_H

#include "PathAction.h"

class InsertPointAction : public PathAction {
 public:
								InsertPointAction(ShapeState* state,
												  ShapeStroke* modifier,
												  int32 index,
												  const int32* selected,
												  int32 count);
	virtual						~InsertPointAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			int32				fIndex;
			BPoint				fPoint;
			BPoint				fPointIn;
			BPoint				fPointOut;

			BPoint				fPreviousOut;
			BPoint				fNextIn;

			int32*				fOldSelection;
			int32				fOldSelectionCount;
};

#endif // INSERT_POINT_ACTION_H
