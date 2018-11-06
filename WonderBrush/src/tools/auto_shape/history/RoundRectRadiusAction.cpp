// RoundRectRadiusAction.cpp

#include <stdio.h>

#include "RoundRectState.h"

#include "RoundRectRadiusAction.h"

// constructor
RoundRectRadiusAction::RoundRectRadiusAction(RoundRectState* state,
											 float radius)
	: Action(),
	  fState(state),
	  fCornerRadius(radius)
{
}

// destructor
RoundRectRadiusAction::~RoundRectRadiusAction()
{
}

// InitCheck
status_t
RoundRectRadiusAction::InitCheck()
{
	return B_OK;
}

// Perform
status_t
RoundRectRadiusAction::Perform(CanvasView* view)
{
	// changes are already performed
	return B_OK;
}

// Undo
status_t
RoundRectRadiusAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// do the necessary
		float radius = fState->RoundCornerRadius();
		fState->SetRoundCornerRadius(fCornerRadius);
		fCornerRadius = radius;
	}
	return status;
}

// Redo
status_t
RoundRectRadiusAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
RoundRectRadiusAction::GetName(BString& name)
{
	name << _GetString(CHANGE_ROUND_CORNER_RADIUS, "Change Round Corner Radius");
}

// CombineWithNext
bool
RoundRectRadiusAction::CombineWithNext(const Action* action)
{
	const RoundRectRadiusAction* next = dynamic_cast<const RoundRectRadiusAction*>(action);
	if (next && next->fTimeStamp - fTimeStamp < 1000000) {
		fTimeStamp = next->fTimeStamp;
		return true;
	}
	return false;
}

