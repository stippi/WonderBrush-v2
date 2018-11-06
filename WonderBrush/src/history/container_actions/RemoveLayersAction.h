// RemoveLayersAction.h

#ifndef REMOVE_LAYERS_ACTION_H
#define REMOVE_LAYERS_ACTION_H

#include "ContainerAction.h"

class Layer;

class RemoveLayersAction : public ContainerAction {
 public:
								RemoveLayersAction(LayerContainer* container,
												   int32* indices,
												   int32 count);
	virtual						~RemoveLayersAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			Layer**				fLayers;
			int32*				fIndices;
			int32				fCount;
			bool				fLayersRemoved;
};

#endif // REMOVE_LAYERS_ACTION_H
