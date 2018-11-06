// AutoShapeColorAction.cpp

#include <stdio.h>

#include "AutoShapeState.h"
#include "ShapeStroke.h"

#include "AutoShapeColorAction.h"

// constructor
AutoShapeColorAction::AutoShapeColorAction(AutoShapeState* state,
										   ShapeStroke* object)
	: AutoShapeAction(state, object)
{
	if (fShape) {
		fColor = fShape->Color();
	}
}

// destructor
AutoShapeColorAction::~AutoShapeColorAction()
{
}

// Perform
status_t
AutoShapeColorAction::Perform(CanvasView* view)
{
	// color is already changed
	return InitCheck();
}

// Undo
status_t
AutoShapeColorAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current color
		rgb_color color = fShape->Color();
		// set the offset
		fState->SetColor(fColor);
		// swap what we remember
		fColor = color;
	}
	return status;
}

// Redo
status_t
AutoShapeColorAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
AutoShapeColorAction::GetName(BString& name)
{
	name << _GetString(CHANGE_COLOR, "Change Color");
}

