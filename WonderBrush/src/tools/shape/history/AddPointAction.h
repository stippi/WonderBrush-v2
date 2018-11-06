// AddPointAction.h

#ifndef ADD_POINT_ACTION_H
#define ADD_POINT_ACTION_H

#include "PathAction.h"

class AddPointAction : public PathAction {
 public:
								AddPointAction(ShapeState* state,
											   ShapeStroke* modifier,
											   int32 index,
											   const int32* selected,
											   int32 count);
	virtual						~AddPointAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			int32				fIndex;
			BPoint				fPoint;
			BPoint				fPointIn;
			BPoint				fPointOut;

			int32*				fOldSelection;
			int32				fOldSelectionCount;
};

#endif // ADD_POINT_ACTION_H
