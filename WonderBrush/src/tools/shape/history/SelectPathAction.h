// SelectPathAction.h

#ifndef SELECT_PATH_ACTION_H
#define SELECT_PATH_ACTION_H

#include "PathAction.h"

class SelectPathAction : public PathAction {
 public:
								SelectPathAction(ShapeState* state,
												 ShapeStroke* modifier,
												 VectorPath* newPath,
												 const int32* selected,
												 int32 count);
	virtual						~SelectPathAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			VectorPath*			fNewPath;

			int32*				fOldSelection;
			int32				fOldSelectionCount;
};

#endif // SELECT_PATH_ACTION_H
