// MoveLayersAction.h

#ifndef MOVE_LAYERS_ACTION_H
#define MOVE_LAYERS_ACTION_H

#include "ContainerAction.h"

class Stroke;

class MoveLayersAction : public ContainerAction {
 public:
								MoveLayersAction(LayerContainer* container,
												 Layer** layers,
												 int32 count,
												 int32 index);
	virtual						~MoveLayersAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			Layer**				fLayers;
			int32*				fIndices;
			int32				fIndex;
			int32				fCount;
};

#endif // MOVE_LAYERS_ACTION_H
