// EditModifierAction.cpp

#include <stdio.h>

#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"
#include "Stroke.h"

#include "EditModifierAction.h"

// constructor
EditModifierAction::EditModifierAction(Layer* layer,
									   Stroke* modifier)
	: LayerAction(layer),
	  fModifier(modifier),
	  fClonedModifier(modifier ? modifier->Clone() : NULL)
{
}

// destructor
EditModifierAction::~EditModifierAction()
{
	delete fClonedModifier;
}

// InitCheck
status_t
EditModifierAction::InitCheck()
{
	return fModifier && fClonedModifier ? LayerAction::InitCheck() : B_NO_INIT;
}

// Perform
status_t
EditModifierAction::Perform(CanvasView* view)
{
	// modifier is already edited
	return B_OK;
}

// Undo
status_t
EditModifierAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		status = B_ERROR;

		view->PrepareForObjectPropertyChange();

		// swap the modifiers around
		Stroke* newClone = fModifier->Clone();
		if (newClone && fModifier->SetTo(fClonedModifier)) {
			BRect r = newClone->Bounds() | fClonedModifier->Bounds();
			view->RebuildLayer(fLayer, r);
			delete fClonedModifier;
			fClonedModifier = newClone;

			// update list view, but only if it shows the same layer
			HistoryListView* listView = view->GetHistoryListView();
			if (listView && listView->GetLayer() == fLayer) {
				int32 index = fLayer->GetHistory()->IndexOf(fModifier);
				listView->SetPainter(index, fModifier->Painter());
			}

			status = B_OK;
		} else
			delete newClone;

		view->ObjectPropertyChangeDone();
	}
	return status;
}

// Redo
status_t
EditModifierAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
EditModifierAction::GetName(BString& name)
{
	// let the action have string members so that it can be customized
	name << _GetString(EDIT_MODIFIER, "Edit Object");
}
