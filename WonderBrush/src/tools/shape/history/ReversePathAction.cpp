// ReversePathAction.cpp

#include <stdio.h>

#include "ShapeState.h"

#include "ReversePathAction.h"

// constructor
ReversePathAction::ReversePathAction(ShapeState* state,
								   ShapeStroke* modifier)
	: PathAction(state, modifier)
{
}

// destructor
ReversePathAction::~ReversePathAction()
{
}

// Perform
status_t
ReversePathAction::Perform(CanvasView* view)
{
	if (InitCheck() >= B_OK) {
		fShapeState->_ReversePath();
		return B_OK;
	}
	return B_NO_INIT;
}

// Undo
status_t
ReversePathAction::Undo(CanvasView* view)
{
	return Perform(view);
}

// Redo
status_t
ReversePathAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
ReversePathAction::GetName(BString& name)
{
	name << _GetString(REVERSE_PATH, "Reverse Path");
}
