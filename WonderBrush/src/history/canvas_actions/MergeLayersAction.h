// MergeLayersAction.h

#ifndef MERGE_LAYERS_ACTION_H
#define MERGE_LAYERS_ACTION_H

#include "CanvasAction.h"

class Layer;

class MergeLayersAction : public CanvasAction {
 public:
								MergeLayersAction(Canvas* canvas,
												  Layer** layers,
												  int32 count);
	virtual						~MergeLayersAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			Layer**				fLayers;
			Layer*				fMergedLayer;
			int32				fMergedIndex;
			int32				fCount;
			int32*				fIndices;
};

#endif // MERGE_LAYERS_ACTION_H
