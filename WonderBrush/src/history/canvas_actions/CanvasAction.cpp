// CanvasAction.cpp

#include <stdio.h>

#include "CanvasAction.h"

// constructor
CanvasAction::CanvasAction(Canvas* canvas)
	: fCanvas(canvas)
{
}

// destructor
CanvasAction::~CanvasAction()
{
}

// InitCheck
status_t
CanvasAction::InitCheck()
{
	return fCanvas ? B_OK : B_NO_INIT;
}

// GetName
void
CanvasAction::GetName(BString& name)
{
	name << "<modify canvas>";
}
