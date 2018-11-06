// ChangeTextWidthAction.cpp

#include <stdio.h>

#include "TextState.h"
#include "TextStroke.h"
#include "TextRenderer.h"

#include "ChangeTextWidthAction.h"

// constructor
ChangeTextWidthAction::ChangeTextWidthAction(TextState* state,
											 TextStroke* modifier)
	: TextAction(state, modifier),
	  fTextWidth(0.0)
{
	if (fTextModifier) {
		fTextWidth = fTextModifier->TextWidth();
	}
}

// destructor
ChangeTextWidthAction::~ChangeTextWidthAction()
{
}

// Perform
status_t
ChangeTextWidthAction::Perform(CanvasView* view)
{
	// text width is already changed
	return InitCheck();
}

// Undo
status_t
ChangeTextWidthAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current font
		float width = fTextModifier->TextWidth();
		// set the text width
		fTextState->SetTextWidth(fTextWidth);
		// swap what we remember
		fTextWidth = width;
	}
	return status;
}

// Redo
status_t
ChangeTextWidthAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeTextWidthAction::GetName(BString& name)
{
	name << _GetString(CHANGE_TEXT_WIDTH, "Change Text Width");
}

// CombineWithNext
bool
ChangeTextWidthAction::CombineWithNext(const Action* action)
{
	const ChangeTextWidthAction* next = dynamic_cast<const ChangeTextWidthAction*>(action);
	if (next && next->fTimeStamp - fTimeStamp < 1000000) {
		fTimeStamp = next->fTimeStamp;
		return true;
	}
	return false;
}


