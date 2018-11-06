// ChangeTextOpacityAction.cpp

#include <stdio.h>

#include "TextState.h"
#include "TextStroke.h"
#include "TextRenderer.h"

#include "ChangeTextOpacityAction.h"

// constructor
ChangeTextOpacityAction::ChangeTextOpacityAction(TextState* state,
												 TextStroke* modifier)
	: TextAction(state, modifier),
	  fOpacity(255)
{
	if (fTextModifier) {
		fOpacity = fTextModifier->GetTextRenderer()->Opacity();
	}
}

// destructor
ChangeTextOpacityAction::~ChangeTextOpacityAction()
{
}

// Perform
status_t
ChangeTextOpacityAction::Perform(CanvasView* view)
{
	// font is already changed
	return InitCheck();
}

// Undo
status_t
ChangeTextOpacityAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current font
		uint8 opacity = fTextModifier->GetTextRenderer()->Opacity();
		// set the rotation
		fTextState->SetOpacity(fOpacity);
		// swap what we remember
		fOpacity = opacity;
	}
	return status;
}

// Redo
status_t
ChangeTextOpacityAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeTextOpacityAction::GetName(BString& name)
{
	name << _GetString(CHANGE_OPACITY, "Change Opacity");
}

