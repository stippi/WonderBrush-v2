// MergeShapesAction.cpp

#include <new.h>
#include <stdio.h>

#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"
#include "ShapeStroke.h"
#include "VectorPath.h"

#include "MergeShapesAction.h"

// constructor
MergeShapesAction::MergeShapesAction(Layer* layer,
									 ShapeStroke** shapes,
									 int32 count)
	: LayerAction(layer),
	  fShapes(shapes),
	  fIndices(NULL),
	  fCount(count),
	  fMergedShape(NULL),
	  fMerged(false)
{
	if (fShapes && fCount > 1) {
		fIndices = new(nothrow) int32[fCount];
		if (fIndices && fShapes[0] && (fMergedShape = dynamic_cast<ShapeStroke*>(fShapes[0]->Clone()))) {
			fMergedShape->FreezeTransformations();
			for (int32 i = 1; i < fCount; i++) {
				if (fShapes[i]) {
					for (int32 j = 0; VectorPath* path = fShapes[i]->PathAt(j); j++) {
						VectorPath* clonedPath = new VectorPath(*path);
						clonedPath->FreezeTransformations();
						fMergedShape->AddPath(clonedPath);
					}
				}
			}
			fMergedShape->UpdateBounds();
		}
	}
}

// destructor
MergeShapesAction::~MergeShapesAction()
{
	if (fMerged) {
		for (int32 i = 0; i < fCount; i++)
			delete fShapes[i];
	} else {
		delete fMergedShape;
	}
	delete[] fShapes;
	delete[] fIndices;
}

// InitCheck
status_t
MergeShapesAction::InitCheck()
{
	status_t status = LayerAction::InitCheck();
	if (status >= B_OK)
		status = fMergedShape ? B_OK : B_NO_INIT;
	return status;
}

// Perform
status_t
MergeShapesAction::Perform(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		if (History* history = fLayer->GetHistory()) {
			fMerged = true;
			BRect area(0.0, 0.0, -1.0, -1.0);
			HistoryListView* listView = view->GetHistoryListView();
			bool isListViewValid = listView && listView->GetLayer() == fLayer;
			// remove shapes
			for (int32 i = 0; i < fCount; i++) {
				if (ShapeStroke* shape = fShapes[i]) {
					int32 index = history->IndexOf(shape);
					fIndices[i] = index + i; // account for already removed shapes
					history->RemoveModifier(shape);
					area = area.IsValid() ? area | shape->Bounds()
										  : shape->Bounds();
					if (isListViewValid) {
						listView->DeleteItem(index);
					}
				}
			}
			// add merged shape at first index of removed shapes
			history->AddModifier(fMergedShape, fIndices[0]);
			area = area.IsValid() ? area | fMergedShape->Bounds() : fMergedShape->Bounds();
			if (isListViewValid) {
				listView->AddModifier(fMergedShape, fIndices[0]);
			}
			if (area.IsValid())
				view->RebuildBitmap(area, fLayer);
		}
	}
	return status;
}

// Undo
status_t
MergeShapesAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		if (History* history = fLayer->GetHistory()) {
			fMerged = false;
			HistoryListView* listView = view->GetHistoryListView();
			bool isListViewValid = listView && listView->GetLayer() == fLayer;
			// remove merged shape
			BRect area = fMergedShape->Bounds();
			if (isListViewValid) {
				listView->DeleteItem(history->IndexOf(fMergedShape));
			}
			history->RemoveModifier(fMergedShape);
			// add original shapes
			Stroke** strokes = new Stroke*[fCount];
			for (int32 i = 0; i < fCount; i++) {
				strokes[i] = fShapes[i];
				if (ShapeStroke* shape = fShapes[i]) {
					history->AddModifier(shape, fIndices[i]);
					area = area.IsValid() ? area | shape->Bounds()
										  : shape->Bounds();
				}
			}
			if (isListViewValid)
				listView->AddModifiers(strokes, fIndices, fCount);
			delete[] strokes;

			if (area.IsValid()) {
				fLayer->Touch(area);
				view->RebuildBitmap(area, fLayer);
			}
		}
	}
	return status;
}

// Redo
status_t
MergeShapesAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
MergeShapesAction::GetName(BString& name)
{
	name << _GetString(MERGE_SHAPE_OBJECTS, "Merge Shapes");
}
