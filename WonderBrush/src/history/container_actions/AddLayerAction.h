// AddLayerAction.h

#ifndef ADD_LAYER_ACTION_H
#define ADD_LAYER_ACTION_H

#include "ContainerAction.h"

class Layer;

class AddLayerAction : public ContainerAction {
 public:
								AddLayerAction(LayerContainer* container,
											   Layer* layer,
											   int32 index);
	virtual						~AddLayerAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			Layer*				fLayer;
			int32				fIndex;
};

#endif // ADD_LAYER_ACTION_H
