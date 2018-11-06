// AutoShapeOutlineWidthAction.cpp

#include <stdio.h>

#include "AutoShapeState.h"
#include "ShapeStroke.h"

#include "AutoShapeOutlineWidthAction.h"

// constructor
AutoShapeOutlineWidthAction::AutoShapeOutlineWidthAction(AutoShapeState* state,
														 ShapeStroke* object)
	: AutoShapeAction(state, object)
{
	if (fShape) {
		fOutlineWidth = fShape->OutlineWidth();
	}
}

// destructor
AutoShapeOutlineWidthAction::~AutoShapeOutlineWidthAction()
{
}

// Perform
status_t
AutoShapeOutlineWidthAction::Perform(CanvasView* view)
{
	// changes are already performed
	return B_OK;
}

// Undo
status_t
AutoShapeOutlineWidthAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// do the necessary
		float outlineWidth = fShape->OutlineWidth();
		fState->SetOutlineWidth(fOutlineWidth);
		fOutlineWidth = outlineWidth;
	}
	return status;
}

// Redo
status_t
AutoShapeOutlineWidthAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
AutoShapeOutlineWidthAction::GetName(BString& name)
{
	name << _GetString(CHANGE_OUTLINE_WIDTH, "Change Outline Width");
}
