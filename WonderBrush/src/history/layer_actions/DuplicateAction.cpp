// DuplicateAction.cpp

#include <stdio.h>

#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"

#include "DuplicateAction.h"

// constructor
DuplicateAction::DuplicateAction(Layer* layer,
								 const Stroke** strokes,
								 int32 count,
								 int32 index,
								 int32 duplicateCount,

								 float xOffset,
								 float yOffset,
								 float rotation,
								 float xScale,
								 float yScale)
	: LayerAction(layer),
	  fStrokes(NULL),
	  fIndex(index),
	  fCount(0),
	  fItemsRemoved(true)
{
printf("count: %ld\n", count);
printf("copies: %ld\n", duplicateCount);
printf("xOffset: %.2f\n", xOffset);
printf("yOffset: %.2f\n", yOffset);
printf("rotation: %.2f\n", rotation);
printf("xScale: %.2f\n", xScale);
printf("yScale: %.2f\n", yScale);
	if (strokes && count > 0 && duplicateCount > 0) {
		fCount = duplicateCount * count;
		fStrokes = new Stroke*[fCount];
		
		// make the clones
		const Stroke** originals = strokes;
		Stroke** clones = fStrokes;

		for (int32 i = 0; i < duplicateCount; i++) {
			for (int32 j = 0; j < count; j++) {
				clones[0] = originals[0]->Clone();
				BRect b = clones[0]->Bounds();
				BPoint center(b.left + b.Width() / 2.0,
							  b.top + b.Height() / 2.0);
				clones[0]->ScaleBy(center, xScale, yScale);
				clones[0]->RotateBy(center, rotation);
				clones[0]->TranslateBy(BPoint(xOffset, yOffset));

				originals++;
				clones++;
			}
			originals = (const Stroke**)(clones - count);
		}
	}
}

// destructor
DuplicateAction::~DuplicateAction()
{
	if (fItemsRemoved) {
		for (int32 i = 0; i < fCount; i++)
			delete fStrokes[i];
	}
	delete[] fStrokes;
}

// InitCheck
status_t
DuplicateAction::InitCheck()
{
	status_t status = LayerAction::InitCheck();
	if (status >= B_OK)
		status = fStrokes ? B_OK : B_NO_INIT;
	return status;
}

// Perform
status_t
DuplicateAction::Perform(CanvasView* view)
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
DuplicateAction::Undo(CanvasView* view)
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
DuplicateAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
DuplicateAction::GetName(BString& name)
{
	if (fCount > 1)
		name << _GetString(ADD_MODIFIERS, "Add Modifiers");
	else
		name << _GetString(ADD_MODIFIER, "Add Object");
}
