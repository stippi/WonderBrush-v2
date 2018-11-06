// ChangeOutlineWidthAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"

#include "ChangeOutlineWidthAction.h"

// constructor
ChangeOutlineWidthAction::ChangeOutlineWidthAction(ShapeState* state,
												   ShapeStroke* modifier)
	: PathAction(state, modifier)
{
	if (fShapeModifier) {
		fOutlineWidth = fShapeModifier->OutlineWidth();
	}
}

// destructor
ChangeOutlineWidthAction::~ChangeOutlineWidthAction()
{
}

// Perform
status_t
ChangeOutlineWidthAction::Perform(CanvasView* view)
{
	// changes are already performed
	return B_OK;
}

// Undo
status_t
ChangeOutlineWidthAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// do the necessary
		float outlineWidth = fShapeModifier->OutlineWidth();
		fShapeState->SetOutlineWidth(fOutlineWidth);
		fOutlineWidth = outlineWidth;
	}
	return status;
}

// Redo
status_t
ChangeOutlineWidthAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeOutlineWidthAction::GetName(BString& name)
{
	name << _GetString(CHANGE_OUTLINE_WIDTH, "Change Outline Width");
}
