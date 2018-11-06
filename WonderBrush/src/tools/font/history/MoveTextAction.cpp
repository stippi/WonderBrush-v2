// MoveTextAction.cpp

#include <stdio.h>

#include "TextState.h"
#include "TextStroke.h"

#include "MoveTextAction.h"

// constructor
MoveTextAction::MoveTextAction(TextState* state,
							   TextStroke* modifier,
							   BPoint offset)
	: TextAction(state, modifier),
	  fOffset(offset)
{
}

// destructor
MoveTextAction::~MoveTextAction()
{
}

// InitCheck
status_t
MoveTextAction::InitCheck()
{
	status_t status = TextAction::InitCheck();
	if (status >= B_OK)
		status = fOffset != BPoint(0.0, 0.0) ? B_OK : B_BAD_VALUE;
	return status;
}

// Perform
status_t
MoveTextAction::Perform(CanvasView* view)
{
	// font is already moved
	return InitCheck();
}

// Undo
status_t
MoveTextAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember bounds
		BRect rr;
		BRect ir;
		_GetModifierBounds(rr, ir);
		// toggle direction
		fOffset.x = -fOffset.x;
		fOffset.y = -fOffset.y;
		// set the offset
		fTextModifier->TranslateBy(fOffset);
		_RedrawModifier(rr, ir);
	}
	return status;
}

// Redo
status_t
MoveTextAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
MoveTextAction::GetName(BString& name)
{
	name << _GetString(MOVE_TEXT, "Move Text");
}

