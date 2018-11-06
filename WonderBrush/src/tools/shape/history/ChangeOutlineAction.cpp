// ChangeOutlineAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"

#include "ChangeOutlineAction.h"

// constructor
ChangeOutlineAction::ChangeOutlineAction(ShapeState* state,
										 ShapeStroke* modifier)
	: PathAction(state, modifier)
{
	if (fShapeModifier) {
		fOutline = fShapeModifier->IsOutline();
	}
}

// destructor
ChangeOutlineAction::~ChangeOutlineAction()
{
}

// Perform
status_t
ChangeOutlineAction::Perform(CanvasView* view)
{
	// changes are already performed
	return B_OK;
}

// Undo
status_t
ChangeOutlineAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// do the necessary
		bool outline = fShapeModifier->IsOutline();
		fShapeState->SetOutline(fOutline);
		fOutline = outline;
	}
	return status;
}

// Redo
status_t
ChangeOutlineAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeOutlineAction::GetName(BString& name)
{
	name << _GetString(TOGGLE_OUTLINE, "Toggle Outline");
}
