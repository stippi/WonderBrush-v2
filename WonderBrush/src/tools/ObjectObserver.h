// ObjectObserver.h

#ifndef OBJECT_OBSERVER_H
#define OBJECT_OBSERVER_H

#include <SupportDefs.h>

class Observable;

class ObjectObserver {
 public:
								ObjectObserver();
	virtual						~ObjectObserver();

	virtual	void				ObjectChanged(const Observable* object) = 0;
};

#endif // OBJECT_OBSERVER
