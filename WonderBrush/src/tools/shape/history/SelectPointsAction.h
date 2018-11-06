// SelectPointsAction.h

#ifndef SELECT_POINTS_ACTION_H
#define SELECT_POINTS_ACTION_H

#include "PathAction.h"

class SelectPointsAction : public PathAction {
 public:
								SelectPointsAction(ShapeState* state,
												   ShapeStroke* modifier,
												   const int32* oldSelection,
												   int32 oldCount,
												   const int32* newSelection,
												   int32 newCount);
	virtual						~SelectPointsAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			int32*				fOldSelection;
			int32				fOldSelectionCount;

			int32*				fNewSelection;
			int32				fNewSelectionCount;
};

#endif // SELECT_POINTS_ACTION_H
