// ChangeAlphaAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"

#include "ChangeAlphaAction.h"

// constructor
ChangeAlphaAction::ChangeAlphaAction(ShapeState* state,
									 ShapeStroke* modifier)
	: PathAction(state, modifier)
{
	if (fShapeModifier) {
		fAlpha = fShapeModifier->Alpha();
	}
}

// destructor
ChangeAlphaAction::~ChangeAlphaAction()
{
}

// Perform
status_t
ChangeAlphaAction::Perform(CanvasView* view)
{
	// changes are already performed
	return B_OK;
}

// Undo
status_t
ChangeAlphaAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// do the necessary
		uint8 alpha = fShapeModifier->Alpha();
		fShapeState->SetAlpha(fAlpha);
		fAlpha = alpha;
	}
	return status;
}

// Redo
status_t
ChangeAlphaAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeAlphaAction::GetName(BString& name)
{
	name << _GetString(CHANGE_OPACITY, "Change Opacity");
}
