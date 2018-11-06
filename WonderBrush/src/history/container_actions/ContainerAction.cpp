// ContainerAction.cpp

#include <stdio.h>

#include "ContainerAction.h"

// constructor
ContainerAction::ContainerAction(LayerContainer* container)
	: fContainer(container)
{
}

// destructor
ContainerAction::~ContainerAction()
{
}

// InitCheck
status_t
ContainerAction::InitCheck()
{
	return fContainer ? B_OK : B_NO_INIT;
}

// GetName
void
ContainerAction::GetName(BString& name)
{
	name << "<modify layer container>";
}
