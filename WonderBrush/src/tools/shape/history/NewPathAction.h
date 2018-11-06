// NewPathAction.h

#ifndef NEW_PATH_ACTION_H
#define NEW_PATH_ACTION_H

#include "PathAction.h"

class NewPathAction : public PathAction {
 public:
								NewPathAction(ShapeState* state,
											  ShapeStroke* modifier,
											  BPoint firstPoint,
											  const int32* selected,
											  int32 count);
	virtual						~NewPathAction();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			VectorPath*			fNewPath;
			bool				fAdded;

			int32*				fOldSelection;
			int32				fOldSelectionCount;
};

#endif // NEW_PATH_ACTION_H
