// MasterGradientAction.cpp

#include <stdio.h>

#include "CanvasView.h"
#include "Gradient.h"
#include "Stroke.h"

#include "MasterGradientAction.h"

// constructor
MasterGradientAction::MasterGradientAction(Layer* layer,
										   Stroke* object)
	: LayerAction(layer),
	  fGradient(NULL),
	  fObject(object)
{
	if (fObject && fObject->GetGradient()) {
		fGradient = new Gradient(*fObject->GetGradient());
	}
	if (!fGradient)
		fObject = NULL;
}

// destructor
MasterGradientAction::~MasterGradientAction()
{
	delete fGradient;
}

// InitCheck
status_t
MasterGradientAction::InitCheck()
{
	status_t ret = LayerAction::InitCheck();
	if (ret >= B_OK) {
		if (*fGradient == *fObject->GetGradient())
			ret = B_ERROR;
	}
	return ret;
}

// Perform
status_t
MasterGradientAction::Perform(CanvasView* view)
{
	// the gradient is already changed
	return InitCheck();
}

// Undo
status_t
MasterGradientAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current gradient
		Gradient gradient = *fObject->GetGradient();

		// bracket the change in these calls in order to
		// avoid the EditGradientState generating an
		// action or entering the private history mode
		view->PrepareForObjectPropertyChange();
		// set the gradient
		fObject->SetGradient(fGradient);
		view->ObjectPropertyChangeDone();

		// swap what we remember
		*fGradient = gradient;
		// redraw the layer in that area
		view->RebuildBitmap(fObject->Bounds(), fLayer);
	}
	return status;
}

// Redo
status_t
MasterGradientAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
MasterGradientAction::GetName(BString& name)
{
	name << _GetString(EDIT_GRADIENT, "Edit Gradient");
}

