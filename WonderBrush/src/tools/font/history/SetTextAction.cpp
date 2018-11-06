// SetTextAction.cpp

#include <stdio.h>

#include "TextState.h"
#include "TextStroke.h"
#include "TextRenderer.h"

#include "SetTextAction.h"

// constructor
SetTextAction::SetTextAction(TextState* state,
							 TextStroke* modifier)
	: TextAction(state, modifier),
	  fText(NULL)
{
	if (fTextModifier) {
		fText = strdup(fTextModifier->GetTextRenderer()->Text());
	}
	if (!fText)
		fTextModifier = NULL;
}

// destructor
SetTextAction::~SetTextAction()
{
	if (fText)
		free(fText);
}

// Perform
status_t
SetTextAction::Perform(CanvasView* view)
{
	// font is already changed
	return InitCheck();
}

// Undo
status_t
SetTextAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current font
		char* text = strdup(fTextModifier->GetTextRenderer()->Text());
		// set the font
		fTextState->SetText(fText);
		// swap what we remember
		free(fText);
		fText = text;
	}
	return status;
}

// Redo
status_t
SetTextAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
SetTextAction::GetName(BString& name)
{
	name << _GetString(CHANGE_TEXT, "Change Text");
}

