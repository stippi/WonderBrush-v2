// Tool.cpp

#include <stdio.h>

#include "Tool.h"

// constructor
Tool::Tool(const char* name)
	: BHandler(name)
{
}

// destructor
Tool::~Tool()
{
}

// SaveSettings
status_t
Tool::SaveSettings(BMessage* message)
{
	if (message)
		return B_OK;
	return B_BAD_VALUE;
}

// LoadSettings
status_t
Tool::LoadSettings(BMessage* message)
{
	if (message)
		return B_OK;
	return B_BAD_VALUE;
}

// ConfigView
MView*
Tool::ConfigView()
{
	return NULL;
}

// State
CanvasViewState*
Tool::State()
{
	return NULL;
}

// SetModifier
status_t
Tool::SetModifier(Modifier* modifier)
{
	return B_OK;
}

// Confirm
status_t
Tool::Confirm()
{
	return B_OK;
}

// Cancel
status_t
Tool::Cancel()
{
	return B_OK;
}


