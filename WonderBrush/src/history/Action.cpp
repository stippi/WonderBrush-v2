// Action.cpp

#include <stdio.h>

#include "LanguageManager.h"

#include "Action.h"

// constructor
Action::Action()
	: fTimeStamp(system_time())
{
}

// destructor
Action::~Action()
{
}

// InitCheck
status_t
Action::InitCheck()
{
	return B_NO_INIT;
}

// Perform
status_t
Action::Perform(CanvasView* view)
{
	return B_ERROR;
}

// Undo
status_t
Action::Undo(CanvasView* view)
{
	return B_ERROR;
}

// Redo
status_t
Action::Redo(CanvasView* view)
{
	return B_ERROR;
}

// GetName
void
Action::GetName(BString& name)
{
	name << "Name of action goes here.";
}

// CombineWithNext
bool
Action::CombineWithNext(const Action* next)
{
	return false;
}

// CombineWithPrevious
bool
Action::CombineWithPrevious(const Action* previous)
{
	return false;
}


// _GetString
const char*
Action::_GetString(uint32 key, const char* defaultString) const
{
	if (LanguageManager* manager = LanguageManager::Default())
		return manager->GetString(key, defaultString);
	else
		return defaultString;
}
