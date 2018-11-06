// SetColorAction.cpp

#include <stdio.h>

#include "TextState.h"
#include "TextStroke.h"

#include "SetColorAction.h"

// constructor
SetColorAction::SetColorAction(TextState* state,
								 TextStroke* modifier)
	: TextAction(state, modifier)
{
	if (fTextModifier) {
		fColor = fTextModifier->Color();
	}
}

// destructor
SetColorAction::~SetColorAction()
{
}

// Perform
status_t
SetColorAction::Perform(CanvasView* view)
{
	// font is already changed
	return InitCheck();
}

// Undo
status_t
SetColorAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current color
		rgb_color color = fTextModifier->Color();
		// set the offset
		fTextState->SetColor(fColor);
		// swap what we remember
		fColor = color;
	}
	return status;
}

// Redo
status_t
SetColorAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
SetColorAction::GetName(BString& name)
{
	name << _GetString(CHANGE_COLOR, "Change Color");
}

