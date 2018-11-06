// LayerAction.cpp

#include <stdio.h>

#include "LayerAction.h"

// constructor
LayerAction::LayerAction(Layer* layer)
	: fLayer(layer)
{
}

// destructor
LayerAction::~LayerAction()
{
}

// InitCheck
status_t
LayerAction::InitCheck()
{
	return fLayer ? B_OK : B_NO_INIT;
}

// GetName
void
LayerAction::GetName(BString& name)
{
	name << "<modify layer>";
}
