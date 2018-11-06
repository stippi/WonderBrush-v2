// ChangeFontSizeAction.cpp

#include <stdio.h>

#include "TextState.h"
#include "TextStroke.h"
#include "TextRenderer.h"

#include "ChangeFontSizeAction.h"

// constructor
ChangeFontSizeAction::ChangeFontSizeAction(TextState* state,
							 TextStroke* modifier)
	: TextAction(state, modifier),
	  fFontSize(12.0)
{
	if (fTextModifier) {
		fFontSize = fTextModifier->GetTextRenderer()->PointSize();
	}
}

// destructor
ChangeFontSizeAction::~ChangeFontSizeAction()
{
}

// Perform
status_t
ChangeFontSizeAction::Perform(CanvasView* view)
{
	// font is already changed
	return InitCheck();
}

// Undo
status_t
ChangeFontSizeAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current font
		float fontSize = fTextModifier->GetTextRenderer()->PointSize();
		// set the rotation
		fTextState->SetFontSize(fFontSize);
		// swap what we remember
		fFontSize = fontSize;
	}
	return status;
}

// Redo
status_t
ChangeFontSizeAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeFontSizeAction::GetName(BString& name)
{
	name << _GetString(CHANGE_FONT_SIZE, "Change Font Size");
}

