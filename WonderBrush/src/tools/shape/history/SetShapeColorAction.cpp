// SetShapeColorAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"

#include "SetShapeColorAction.h"

// constructor
SetShapeColorAction::SetShapeColorAction(ShapeState* state,
										 ShapeStroke* modifier)
	: PathAction(state, modifier)
{
	if (fShapeModifier) {
		fColor = fShapeModifier->Color();
	}
}

// destructor
SetShapeColorAction::~SetShapeColorAction()
{
}

// Perform
status_t
SetShapeColorAction::Perform(CanvasView* view)
{
	// color is already changed
	return InitCheck();
}

// Undo
status_t
SetShapeColorAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current color
		rgb_color color = fShapeModifier->Color();
		// set the offset
		fShapeState->SetColor(fColor);
		// swap what we remember
		fColor = color;
	}
	return status;
}

// Redo
status_t
SetShapeColorAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
SetShapeColorAction::GetName(BString& name)
{
	name << _GetString(CHANGE_COLOR, "Change Color");
}

