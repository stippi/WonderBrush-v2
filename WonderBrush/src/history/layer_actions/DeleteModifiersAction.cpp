// DeleteModifiersAction.cpp

#include <stdio.h>

#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"

#include "DeleteModifiersAction.h"

// constructor
DeleteModifiersAction::DeleteModifiersAction(Layer* layer,
											 Stroke** strokes,
											 int32 count)
	: LayerAction(layer),
	  fStrokes(strokes),
	  fIndices(new int32[count]),
	  fCount(count),
	  fItemsRemoved(false)
{
	if (fLayer) {
		if (History* history = fLayer->GetHistory()) {
			for (int32 i = 0; i < fCount; i++)
				fIndices[i] = history->IndexOf(fStrokes[i]);
		}
	}
}

// destructor
DeleteModifiersAction::~DeleteModifiersAction()
{
	if (fItemsRemoved) {
		for (int32 i = 0; i < fCount; i++)
			delete fStrokes[i];
	}
	delete[] fStrokes;
	delete[] fIndices;
}

// InitCheck
status_t
DeleteModifiersAction::InitCheck()
{
	status_t status = LayerAction::InitCheck();
	if (status >= B_OK)
		status = fStrokes && fIndices ? B_OK : B_NO_INIT;
	return status;
}

// Perform
status_t
DeleteModifiersAction::Perform(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		if (History* history = fLayer->GetHistory()) {
			fItemsRemoved = true;
			BRect area(0.0, 0.0, -1.0, -1.0);
			HistoryListView* listView = view->GetHistoryListView();
			for (int32 i = 0; i < fCount; i++) {
				if (Stroke* stroke = fStrokes[i]) {
					history->RemoveModifier(stroke);
					if (i == 0)
						area = stroke->Bounds();
					else
						area = area | stroke->Bounds();
				}
			}
			// delete items from listview in reverse order
			if (listView && listView->GetLayer() == fLayer) {
				for (int32 i = fCount - 1; i >= 0; i--)
					listView->DeleteItem(fIndices[i]);
			}
			if (area.IsValid())
				view->RebuildBitmap(area, fLayer);
		}
	}
	return status;
}

// Undo
status_t
DeleteModifiersAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		if (History* history = fLayer->GetHistory()) {
			fItemsRemoved = false;
			BRect area(0.0, 0.0, -1.0, -1.0);
			HistoryListView* listView = view->GetHistoryListView();
			for (int32 i = 0; i < fCount; i++) {
				if (Stroke* stroke = fStrokes[i]) {
					history->AddModifier(stroke, fIndices[i]);
					if (i == 0)
						area = stroke->Bounds();
					else
						area = area | stroke->Bounds();
				}
			}
			if (listView && listView->GetLayer() == fLayer)
				listView->AddModifiers(fStrokes, fIndices, fCount);
			if (area.IsValid())
				view->RebuildBitmap(area, fLayer);
		}
	}
	return status;
}

// Redo
status_t
DeleteModifiersAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
DeleteModifiersAction::GetName(BString& name)
{
	if (fCount > 1)
		name << _GetString(REMOVE_MODIFIERS, "Remove Objects");
	else
		name << _GetString(REMOVE_MODIFIER, "Remove Object");
}
