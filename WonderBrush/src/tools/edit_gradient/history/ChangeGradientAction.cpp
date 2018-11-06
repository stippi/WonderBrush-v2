// ChangeGradientAction.cpp

#include <stdio.h>

#include "EditGradientState.h"
#include "Gradient.h"
#include "Stroke.h"

#include "ChangeGradientAction.h"

// constructor
ChangeGradientAction::ChangeGradientAction(EditGradientState* state,
										   Stroke* object)
	: GradientAction(state, object),
	  fGradient(NULL)
{
	if (fObject && fObject->GetGradient()) {
		fGradient = new Gradient(*fObject->GetGradient());
	}
	if (!fGradient)
		fObject = NULL;
}

// destructor
ChangeGradientAction::~ChangeGradientAction()
{
	delete fGradient;
}

// Perform
status_t
ChangeGradientAction::Perform(CanvasView* view)
{
	// gradient is already changed
	return InitCheck();
}

// Undo
status_t
ChangeGradientAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current gradient
		Gradient gradient = *fObject->GetGradient();
		// set the gradient
		fState->SetGradient(fGradient);
		// swap what we remember
		*fGradient = gradient;
	}
	return status;
}

// Redo
status_t
ChangeGradientAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeGradientAction::GetName(BString& name)
{
	name << _GetString(EDIT_GRADIENT, "Edit Gradient");
}

// CombineWithNext
bool
ChangeGradientAction::CombineWithNext(const Action* action)
{
	const ChangeGradientAction* next = dynamic_cast<const ChangeGradientAction*>(action);
	if (next && next->fTimeStamp - fTimeStamp < 1000000) {
		fTimeStamp = next->fTimeStamp;
		return true;
	}
	return false;
}

