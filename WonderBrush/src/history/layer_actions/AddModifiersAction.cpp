// AddModifiersAction.cpp

#include <stdio.h>

#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"

#include "AddModifiersAction.h"

// constructor
AddModifiersAction::AddModifiersAction(Layer* layer,
									   Stroke** strokes,
									   int32 count,
									   int32 index)
	: LayerAction(layer),
	  fStrokes(strokes),
	  fIndex(index),
	  fCount(count),
	  fItemsRemoved(true)
{
}

// destructor
AddModifiersAction::~AddModifiersAction()
{
	if (fItemsRemoved) {
		for (int32 i = 0; i < fCount; i++)
			delete fStrokes[i];
	}
	delete[] fStrokes;
}

// InitCheck
status_t
AddModifiersAction::InitCheck()
{
	status_t status = LayerAction::InitCheck();
	if (status >= B_OK)
		status = fStrokes ? B_OK : B_NO_INIT;
	return status;
}

// Perform
status_t
AddModifiersAction::Perform(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		if (History* history = fLayer->GetHistory()) {
			fItemsRemoved = false;
			BRect area(0.0, 0.0, -1.0, -1.0);
			HistoryListView* listView = view->GetHistoryListView();
			int32 index = fIndex;
			bool isListViewValid = listView && listView->GetLayer() == fLayer;
			int32 indices[fCount];
			for (int32 i = 0; i < fCount; i++) {
				if (Stroke* stroke = fStrokes[i]) {
					history->AddModifier(stroke, index);
					if (i == 0)
						area = stroke->Bounds();
					else
						area = area | stroke->Bounds();
					indices[i] = index++;
				}
			}
			if (isListViewValid)
				listView->AddModifiers(fStrokes, indices, fCount);

			if (area.IsValid()) {
				fLayer->Touch(area);
				view->RebuildBitmap(area, fLayer);
			}
		}
	}
	return status;
}

// Undo
status_t
AddModifiersAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		if (History* history = fLayer->GetHistory()) {
			fItemsRemoved = true;
			BRect area(0.0, 0.0, -1.0, -1.0);
			HistoryListView* listView = view->GetHistoryListView();
			bool isListViewValid = listView && listView->GetLayer() == fLayer;
			for (int32 i = 0; i < fCount; i++) {
				if (Stroke* stroke = fStrokes[i]) {
					int32 index = history->IndexOf(stroke);
					history->RemoveModifier(stroke);
					if (i == 0)
						area = stroke->Bounds();
					else
						area = area | stroke->Bounds();
					if (isListViewValid) {
						listView->DeleteItem(index);
					}
				}
			}
			if (area.IsValid())
				view->RebuildBitmap(area, fLayer);
		}
	}
	return status;
}

// Redo
status_t
AddModifiersAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
AddModifiersAction::GetName(BString& name)
{
	if (fCount > 1)
		name << _GetString(ADD_MODIFIERS, "Add Modifiers");
	else
		name << _GetString(ADD_MODIFIER, "Add Object");
}
