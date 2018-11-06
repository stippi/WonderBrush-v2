// Event.cpp

#include <stdio.h>

#include "Event.h"

// constructor
Event::Event(bool autoDelete)
	: fTime(0),
	  fAutoDelete(autoDelete)
{
}

// constructor
Event::Event(bigtime_t time, bool autoDelete)
	: fTime(time),
	  fAutoDelete(autoDelete)
{
}

// destructor
Event::~Event()
{
}

// SetTime
void
Event::SetTime(bigtime_t time)
{
	fTime = time;
}

// Time
bigtime_t
Event::Time() const
{
	return fTime;
}

// SetAutoDelete
void
Event::SetAutoDelete(bool autoDelete)
{
	fAutoDelete = autoDelete;
}

// GetAutoDelete
bool
Event::GetAutoDelete() const
{
	return fAutoDelete;
}

// Execute
void
Event::Execute()
{
	printf("Event::Execute() - %Ld\n", fTime);
}

