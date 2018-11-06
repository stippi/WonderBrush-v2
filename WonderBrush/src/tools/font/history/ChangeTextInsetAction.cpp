// ChangeTextInsetAction.cpp

#include <stdio.h>

#include "TextState.h"
#include "TextStroke.h"
#include "TextRenderer.h"

#include "ChangeTextInsetAction.h"

// constructor
ChangeTextInsetAction::ChangeTextInsetAction(TextState* state,
											 TextStroke* modifier)
	: TextAction(state, modifier),
	  fTextInset(0.0)
{
	if (fTextModifier) {
		fTextInset = fTextModifier->ParagraphInset();
	}
}

// destructor
ChangeTextInsetAction::~ChangeTextInsetAction()
{
}

// Perform
status_t
ChangeTextInsetAction::Perform(CanvasView* view)
{
	// text inset is already changed
	return InitCheck();
}

// Undo
status_t
ChangeTextInsetAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current inset
		float inset = fTextModifier->ParagraphInset();
		// set the text inset
		fTextState->SetParagraphInset(fTextInset);
		// swap what we remember
		fTextInset = inset;
	}
	return status;
}

// Redo
status_t
ChangeTextInsetAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeTextInsetAction::GetName(BString& name)
{
	name << _GetString(CHANGE_PARAGRAPH_INSET, "Change Inset");
}

// CombineWithNext
bool
ChangeTextInsetAction::CombineWithNext(const Action* action)
{
	const ChangeTextInsetAction* next = dynamic_cast<const ChangeTextInsetAction*>(action);
	if (next && next->fTimeStamp - fTimeStamp < 1000000) {
		fTimeStamp = next->fTimeStamp;
		return true;
	}
	return false;
}


