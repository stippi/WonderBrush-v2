// Observable.h

#ifndef OBSERVABLE_H
#define OBSERVABLE_H

#include <List.h>

class ObjectObserver;

class Observable {
 public:
								Observable();
	virtual						~Observable();

			bool				AddObserver(ObjectObserver* observer);
			bool				RemoveObserver(ObjectObserver* observer);

			void				Notify() const;

			void				SuspendNotifications(bool suspend);

 private:
			BList				fObservers;

			int32				fSuspended;
	mutable	bool				fPendingNotifications;
};

class AutoNotificationSuspender {
 public:
								AutoNotificationSuspender(Observable* object)
									: fObject(object)
								{
									fObject->SuspendNotifications(true);
								}

	virtual						~AutoNotificationSuspender()
								{
									fObject->SuspendNotifications(false);
								}
 private:
			Observable*			fObject;
};

#endif // OBSERVABLE_H
