// AutoShapeOutlineAction.cpp

#include <stdio.h>

#include "AutoShapeState.h"
#include "ShapeStroke.h"

#include "AutoShapeOutlineAction.h"

// constructor
AutoShapeOutlineAction::AutoShapeOutlineAction(AutoShapeState* state,
											   ShapeStroke* object)
	: AutoShapeAction(state, object)
{
	if (fShape) {
		fOutline = fShape->IsOutline();
	}
}

// destructor
AutoShapeOutlineAction::~AutoShapeOutlineAction()
{
}

// Perform
status_t
AutoShapeOutlineAction::Perform(CanvasView* view)
{
	// changes are already performed
	return B_OK;
}

// Undo
status_t
AutoShapeOutlineAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// do the necessary
		bool outline = fShape->IsOutline();
		fState->SetOutline(fOutline);
		fOutline = outline;
	}
	return status;
}

// Redo
status_t
AutoShapeOutlineAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
AutoShapeOutlineAction::GetName(BString& name)
{
	name << _GetString(TOGGLE_OUTLINE, "Toggle Outline");
}
