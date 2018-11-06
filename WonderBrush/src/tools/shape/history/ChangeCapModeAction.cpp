// ChangeCapModeAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"

#include "ChangeCapModeAction.h"

// constructor
ChangeCapModeAction::ChangeCapModeAction(ShapeState* state,
										 ShapeStroke* modifier)
	: PathAction(state, modifier)
{
	if (fShapeModifier) {
		fCapMode = fShapeModifier->CapMode();
	}
}

// destructor
ChangeCapModeAction::~ChangeCapModeAction()
{
}

// Perform
status_t
ChangeCapModeAction::Perform(CanvasView* view)
{
	// changes are already performed
	return B_OK;
}

// Undo
status_t
ChangeCapModeAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// do the necessary
		uint32 capMode = fShapeModifier->CapMode();
		fShapeState->SetCapMode(fCapMode);
		fCapMode = capMode;
	}
	return status;
}

// Redo
status_t
ChangeCapModeAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeCapModeAction::GetName(BString& name)
{
	name << _GetString(CHANGE_CAP_MODE, "Change Cap Mode");
}
