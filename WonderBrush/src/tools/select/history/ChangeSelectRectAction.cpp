// ChangeSelectRectAction.cpp

#include <stdio.h>

#include "SimpleSelectState.h"

#include "ChangeSelectRectAction.h"

// constructor
ChangeSelectRectAction::ChangeSelectRectAction(SimpleSelectState* state, BRect selectRect, bool dummy)
	: fSelectState(state),
	  fSelectRect(selectRect)
{
}

// destructor
ChangeSelectRectAction::~ChangeSelectRectAction()
{
}

// InitCheck
status_t
ChangeSelectRectAction::InitCheck()
{
	return fSelectState ? B_OK : B_NO_INIT;
}

// Perform
status_t
ChangeSelectRectAction::Perform(CanvasView* view)
{
	// crop rect is already changed
	return InitCheck();
}

// Undo
status_t
ChangeSelectRectAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// swap the rects
		BRect currentRect = fSelectState->fCropRect;
		fSelectState->_SetRect(fSelectRect);
		fSelectRect = currentRect;
	}
	return status;
}

// Redo
status_t
ChangeSelectRectAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeSelectRectAction::GetName(BString& name)
{

	name << _GetString(MODIFY_SELECT_RECT, "Modify Selection Rect");
}

