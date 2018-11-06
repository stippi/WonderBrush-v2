// MergeShapesAction.h

#ifndef MERGE_SHAPES_ACTION_H
#define MERGE_SHAPES_ACTION_H

#include "LayerAction.h"

class ShapeStroke;

class MergeShapesAction : public LayerAction {
 public:
								MergeShapesAction(Layer* layer,
												  ShapeStroke** shapes,
												  int32 count);
	virtual						~MergeShapesAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			ShapeStroke**		fShapes;
			int32*				fIndices;
			int32				fCount;
			ShapeStroke*		fMergedShape;
			bool				fMerged;
};

#endif // MERGE_SHAPES_ACTION_H
