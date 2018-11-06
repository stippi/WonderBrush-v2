// ContainerAction.h

#ifndef CONTAINER_ACTION_H
#define CONTAINER_ACTION_H

#include "Action.h"

class LayerContainer;

class ContainerAction : public Action {
 public:
								ContainerAction(LayerContainer* container);
	virtual						~ContainerAction();
	
	virtual	status_t			InitCheck();
	virtual void				GetName(BString& name);

 protected:
			LayerContainer*		fContainer;

};

#endif // CONTAINER_ACTION_H
