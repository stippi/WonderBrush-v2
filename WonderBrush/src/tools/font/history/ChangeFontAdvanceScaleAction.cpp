// ChangeFontAdvanceScaleAction.cpp

#include <stdio.h>

#include "TextState.h"
#include "TextStroke.h"
#include "TextRenderer.h"

#include "ChangeFontAdvanceScaleAction.h"

// constructor
ChangeFontAdvanceScaleAction::ChangeFontAdvanceScaleAction(TextState* state,
														   TextStroke* modifier)
	: TextAction(state, modifier),
	  fAdvanceScale(0.5)
{
	if (fTextModifier) {
		fAdvanceScale = fTextModifier->AdvanceScale();
	}
}

// destructor
ChangeFontAdvanceScaleAction::~ChangeFontAdvanceScaleAction()
{
}

// Perform
status_t
ChangeFontAdvanceScaleAction::Perform(CanvasView* view)
{
	// font is already changed
	return InitCheck();
}

// Undo
status_t
ChangeFontAdvanceScaleAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current font
		float scale = fTextModifier->AdvanceScale();
		// set the spacing
		fTextState->SetAdvanceScale(fAdvanceScale / 2.0);
		// swap what we remember
		fAdvanceScale = scale;
	}
	return status;
}

// Redo
status_t
ChangeFontAdvanceScaleAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeFontAdvanceScaleAction::GetName(BString& name)
{
	name << _GetString(CHANGE_FONT_ADVANCE_SCALE, "Change Glyph Spacing");
}

