// Event.h

#ifndef EVENT_H
#define EVENT_H

#include <OS.h>

class Event {
 public:
							Event(bool autoDelete = true);
							Event(bigtime_t time, bool autoDelete = true);
	virtual					~Event();

			void			SetTime(bigtime_t time);
			bigtime_t		Time() const;

			void			SetAutoDelete(bool autoDelete);
			bool			GetAutoDelete() const;

	virtual	void			Execute();

 private:
			bigtime_t		fTime;
			bool			fAutoDelete;
};

#endif	// EVENT_H
