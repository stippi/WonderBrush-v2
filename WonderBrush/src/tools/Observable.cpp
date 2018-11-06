// Observable.cpp

#include <stdio.h>

#include "ObjectObserver.h"

#include "Observable.h"

// constructor
Observable::Observable()
	: fObservers(2),
	  fSuspended(0),
	  fPendingNotifications(false)
{
}

// destructor
Observable::~Observable()
{
}

// AddObserver
bool
Observable::AddObserver(ObjectObserver* observer)
{
	if (observer && !fObservers.HasItem((void*)observer)) {
		return fObservers.AddItem((void*)observer);
	}
	return false;
}

// RemoveObserver
bool
Observable::RemoveObserver(ObjectObserver* observer)
{
	return fObservers.RemoveItem((void*)observer);
}

// Notify
void
Observable::Notify() const
{
	if (!fSuspended) {
		for (int32 i = 0; ObjectObserver* observer = (ObjectObserver*)fObservers.ItemAt(i); i++)
			observer->ObjectChanged(this);
		fPendingNotifications = false;
	} else {
		fPendingNotifications = true;
	}
}

// SuspendNotifications
void
Observable::SuspendNotifications(bool suspend)
{
	if (suspend)
		fSuspended++;
	else
		fSuspended--;

	if (fSuspended < 0) {
printf("Observable::SuspendNotifications(false) - error: suspend level below zero!\n");
		fSuspended = 0;
	}

	if (!fSuspended && fPendingNotifications)
		Notify();
}
