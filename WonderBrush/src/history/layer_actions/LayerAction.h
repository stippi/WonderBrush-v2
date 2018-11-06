// LayerAction.h

#ifndef LAYER_ACTION_H
#define LAYER_ACTION_H

#include "Action.h"

class Layer;

class LayerAction : public Action {
 public:
								LayerAction(Layer* layer);
	virtual						~LayerAction();
	
	virtual	status_t			InitCheck();
	virtual void				GetName(BString& name);

 protected:
			Layer*		fLayer;

};

#endif // LAYER_ACTION_H
