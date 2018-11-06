// MoveModifiersAction.cpp

#include <stdio.h>

#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"

#include "MoveModifiersAction.h"

// constructor
MoveModifiersAction::MoveModifiersAction(Layer* fromLayer,
										 Layer* destLayer,
										 Stroke** strokes,
										 int32 count,
										 int32 index)
	: LayerAction(fromLayer),
	  fTargetLayer(destLayer),
	  fStrokes(strokes),
	  fIndices(new int32[count]),
	  fIndex(index),
	  fCount(count)
{
	// adjust move target index compensating for items that
	// are removed before that index
	if (fLayer) {
		if (History* history = fLayer->GetHistory()) {
			int32 itemsBeforeIndex = 0;
			for (int32 i = 0; i < fCount; i++) {
				fIndices[i] = history->IndexOf(fStrokes[i]);
				if (fIndices[i] < fIndex) {
					itemsBeforeIndex++;
				}
			}
			// if items are moved within the same list,
			// the target index needs to be compensated
			// for items removed in front of the index
			if (fTargetLayer == fLayer)
				fIndex -= itemsBeforeIndex;
		}
	}
}

// destructor
MoveModifiersAction::~MoveModifiersAction()
{
	delete[] fStrokes;
	delete[] fIndices;
}

// InitCheck
status_t
MoveModifiersAction::InitCheck()
{
	status_t status = LayerAction::InitCheck();
	if (status >= B_OK)
		status = fTargetLayer && fStrokes && fIndices ? B_OK : B_NO_INIT;

	if (status < B_OK)
		return status;

	// analyse the move, don't return B_OK in case
	// the container state does not change...

	int32 index = fIndices[0];
		// NOTE: fIndices == NULL if fCount < 1

	if (index != fIndex) {
		// a change is guaranteed
		return B_OK;
	}

	// the insertion index is the same as the index of the first
	// moved item, a change only occures if the indices of the
	// moved items is not contiguous
	bool isContiguous = true;
	for (int32 i = 1; i < fCount; i++) {
		if (fIndices[i] != index + 1) {
			isContiguous = false;
			break;
		}
		index = fIndices[i];
	}
	if (isContiguous) {
		// the container state will not change because of the move
		return B_ERROR;
	}

	return B_OK;
}

// Perform
status_t
MoveModifiersAction::Perform(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		History* fromHistory = fLayer->GetHistory();
		History* destHistory = fTargetLayer->GetHistory();
		if (fromHistory && destHistory) {
			BRect area(0.0, 0.0, -1.0, -1.0);
			HistoryListView* listView = view->GetHistoryListView();
			// remove modifiers from history
			for (int32 i = 0; i < fCount; i++) {
				if (Stroke* stroke = fStrokes[i]) {
					fromHistory->RemoveModifier(stroke);
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
			// add modifiers to history at index
			int32 index = fIndex;
			int32 *indices = new int32[fCount];
			for (int32 i = 0; i < fCount; i++) {
				if (Stroke* stroke = fStrokes[i]) {
					destHistory->AddModifier(stroke, index);
					indices[i] = index++;
				}
			}
			// add items in listview
			if (listView && listView->GetLayer() == fTargetLayer)
				listView->AddModifiers(fStrokes, indices, fCount);
			if (area.IsValid()) {
				// rebuild target layer
				if (fTargetLayer != fLayer) {
					fTargetLayer->Touch(area);
					view->RebuildBitmap(area, fTargetLayer);
				}
				// rebuild layer
				view->RebuildBitmap(area, fLayer);
			}
			delete[] indices;
		}
	}
	return status;
}

// Undo
status_t
MoveModifiersAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		History* fromHistory = fTargetLayer->GetHistory();
		History* destHistory = fLayer->GetHistory();
		if (fromHistory && destHistory) {
			BRect area(0.0, 0.0, -1.0, -1.0);
			HistoryListView* listView = view->GetHistoryListView();
			bool isListViewValid = listView && listView->GetLayer() == fTargetLayer;
			// remove modifiers from history and listview
			for (int32 i = 0; i < fCount; i++) {
				if (Stroke* stroke = fStrokes[i]) {
					int32 index = fromHistory->IndexOf(stroke);
					fromHistory->RemoveModifier(stroke);
					if (i == 0)
						area = stroke->Bounds();
					else
						area = area | stroke->Bounds();
					if (isListViewValid) {
						listView->DeleteItem(index);
					}
				}
			}
			// add modifiers to history at remembered indices
			for (int32 i = 0; i < fCount; i++) {
				if (Stroke* stroke = fStrokes[i])
					destHistory->AddModifier(stroke, fIndices[i]);
			}
			// add items in listview
			if (listView && listView->GetLayer() == fLayer)
				listView->AddModifiers(fStrokes, fIndices, fCount);
			if (area.IsValid()) {
				// rebuild layer
				fLayer->Touch(area);
				view->RebuildBitmap(area, fLayer);
				// rebuild target layer
				if (fTargetLayer != fLayer)
					view->RebuildBitmap(area, fTargetLayer);
			}
		}
	}
	return status;
}

// Redo
status_t
MoveModifiersAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
MoveModifiersAction::GetName(BString& name)
{
	if (fCount > 1)
		name << _GetString(MOVE_MODIFIERS, "Move Objects");
	else
		name << _GetString(MOVE_MODIFIER, "Move Object");
}
