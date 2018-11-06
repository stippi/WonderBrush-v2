// ChangeTextAlignmentAction.cpp

#include <stdio.h>

#include "TextState.h"
#include "TextStroke.h"
#include "TextRenderer.h"

#include "ChangeTextAlignmentAction.h"

// constructor
ChangeTextAlignmentAction::ChangeTextAlignmentAction(TextState* state,
													 TextStroke* modifier)
	: TextAction(state, modifier),
	  fAlignment(ALIGN_LEFT)
{
	if (fTextModifier) {
		fAlignment = fTextModifier->Alignment();
	}
}

// destructor
ChangeTextAlignmentAction::~ChangeTextAlignmentAction()
{
}

// Perform
status_t
ChangeTextAlignmentAction::Perform(CanvasView* view)
{
	// font is already changed
	return InitCheck();
}

// Undo
status_t
ChangeTextAlignmentAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current font
		uint32 alignment = fTextModifier->Alignment();
		// set the spacing
		fTextState->SetAlignment(fAlignment);
		// swap what we remember
		fAlignment = alignment;
	}
	return status;
}

// Redo
status_t
ChangeTextAlignmentAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeTextAlignmentAction::GetName(BString& name)
{
	name << _GetString(CHANGE_ALIGNMENT, "Change Alignment");
}

