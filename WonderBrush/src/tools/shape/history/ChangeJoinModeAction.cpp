// ChangeJoinModeAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"

#include "ChangeJoinModeAction.h"

// constructor
ChangeJoinModeAction::ChangeJoinModeAction(ShapeState* state,
										   ShapeStroke* modifier)
	: PathAction(state, modifier)
{
	if (fShapeModifier) {
		fJoinMode = fShapeModifier->JoinMode();
	}
}

// destructor
ChangeJoinModeAction::~ChangeJoinModeAction()
{
}

// Perform
status_t
ChangeJoinModeAction::Perform(CanvasView* view)
{
	// changes are already performed
	return B_OK;
}

// Undo
status_t
ChangeJoinModeAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// do the necessary
		uint32 joinMode = fShapeModifier->JoinMode();
		fShapeState->SetJoinMode(fJoinMode);
		fJoinMode = joinMode;
	}
	return status;
}

// Redo
status_t
ChangeJoinModeAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeJoinModeAction::GetName(BString& name)
{
	name << _GetString(CHANGE_JOIN_MODE, "Change Join Mode");
}
