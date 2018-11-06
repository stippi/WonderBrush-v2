// CloseAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "VectorPath.h"

#include "CloseAction.h"

// constructor
CloseAction::CloseAction(ShapeState* state,
						 ShapeStroke* modifier)
	: PathAction(state, modifier)
{
}

// destructor
CloseAction::~CloseAction()
{
}

// Perform
status_t
CloseAction::Perform(CanvasView* view)
{
	// path is already closed
	return B_OK;
}

// Undo
status_t
CloseAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		fShapeState->SetClosed(!fPath->IsClosed());
	}
	return status;
}

// Redo
status_t
CloseAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
CloseAction::GetName(BString& name)
{
	name << _GetString(CLOSE_PATH_ACTION, "Close Path");
}
