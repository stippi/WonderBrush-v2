// AddModifierAction.cpp

#include <stdio.h>

#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"
#include "Stroke.h"

#include "AddModifierAction.h"

// constructor
AddModifierAction::AddModifierAction(Layer* layer,
									 Stroke* stroke)
	: LayerAction(layer),
	  fStroke(stroke),
	  fIndex(-1)
{
}

// destructor
AddModifierAction::~AddModifierAction()
{
	// We remember the index of the stroke in the
	// history before we remove it in order to put
	// it back in place. If the index is not -1,
	// it means the stroke is not in the history and
	// can be deleted.
	if (fIndex >= 0)
		delete fStroke;
}

// InitCheck
status_t
AddModifierAction::InitCheck()
{
	status_t status = LayerAction::InitCheck();
	if (status >= B_OK)
		status = fStroke ? B_OK : B_NO_INIT;
	return status;
}

// Perform
status_t
AddModifierAction::Perform(CanvasView* view)
{
	// action is already carried out
	fIndex = -1; // stroke is in the history
	return B_OK;
}

// Undo
status_t
AddModifierAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		if (History* history = fLayer->GetHistory()) {
			// remember index
			fIndex = history->IndexOf(fStroke);
			if (history->RemoveModifier(fStroke)) {
				// remove stroke from listview
				HistoryListView* listView = view->GetHistoryListView();
				if (listView && listView->GetLayer() == fLayer)
					listView->DeleteItem(fIndex);
				// rebuild layer with stroke excluded
				view->RebuildBitmap(fStroke->Bounds(), fLayer);
			} else {
				status = B_ERROR;
				fIndex = -1;
			}
		} else
			status = B_ERROR;
	}
	return status;
}

// Redo
status_t
AddModifierAction::Redo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		if (History* history = fLayer->GetHistory()) {
			// add stroke at index
			if (history->AddModifier(fStroke, fIndex)) {
				// add stroke to listview
				HistoryListView* listView = view->GetHistoryListView();
				if (listView && listView->GetLayer() == fLayer)
					listView->AddModifier(fStroke, fIndex);
				// rebuild layer with stroke included
				BRect area(fStroke->Bounds());
				if (area.IsValid()) {
					fLayer->Touch(area);
					view->RebuildBitmap(area, fLayer);
				}
				// remember index in undo
				fIndex = -1;
			} else
				status = B_NO_MEMORY;
		} else
			status = B_ERROR;
	}
	return status;
}

// GetName
void
AddModifierAction::GetName(BString& name)
{
	if (fStroke)
		name << fStroke->Name();
	else
		name << _GetString(UNKOWN_MODIFIER, "Unkown Object");
}
