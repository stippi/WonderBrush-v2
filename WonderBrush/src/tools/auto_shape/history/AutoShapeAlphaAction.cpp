// AutoShapeAlphaAction.cpp

#include <stdio.h>

#include "AutoShapeState.h"
#include "ShapeStroke.h"

#include "AutoShapeAlphaAction.h"

// constructor
AutoShapeAlphaAction::AutoShapeAlphaAction(AutoShapeState* state,
										   ShapeStroke* object)
	: AutoShapeAction(state, object)
{
	if (fShape) {
		fAlpha = fShape->Alpha();
	}
}

// destructor
AutoShapeAlphaAction::~AutoShapeAlphaAction()
{
}

// Perform
status_t
AutoShapeAlphaAction::Perform(CanvasView* view)
{
	// changes are already performed
	return B_OK;
}

// Undo
status_t
AutoShapeAlphaAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// do the necessary
		uint8 alpha = fShape->Alpha();
		fState->SetAlpha(fAlpha);
		fAlpha = alpha;
	}
	return status;
}

// Redo
status_t
AutoShapeAlphaAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
AutoShapeAlphaAction::GetName(BString& name)
{
	name << _GetString(CHANGE_OPACITY, "Change Opacity");
}
