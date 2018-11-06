// MergeLayersAction.cpp

#include <stdio.h>

#include <Bitmap.h>

#include "Canvas.h"
#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "LanguageManager.h"
#include "LayersListView.h"

#include "MergeLayersAction.h"

// constructor
MergeLayersAction::MergeLayersAction(Canvas* canvas,
									 Layer** layers,
									 int32 count)
	: CanvasAction(canvas),
	  fLayers(layers),
	  fMergedLayer(NULL),
	  fMergedIndex(-1),
	  fCount(count),
	  fIndices(NULL)
{
	if (fCount > 1) {
		fIndices = new int32[fCount];
	}
}

// destructor
MergeLayersAction::~MergeLayersAction()
{
	if (fMergedIndex < 0) {
		// this means we have removed the layers, we need to delete them
		for (int32 i = 0; i < fCount; i++)
			delete fLayers[i];
	} else {
		delete fMergedLayer;
	}
	delete[] fLayers;
	delete[] fIndices;
	// do not delete fMergedLayer, since it is in the fCanvas
}

// InitCheck
status_t
MergeLayersAction::InitCheck()
{
	status_t status = fLayers && fIndices ? B_OK : B_BAD_VALUE;
	return status;
}

// Perform
status_t
MergeLayersAction::Perform(CanvasView* view)
{
	status_t status = B_ERROR;
	if (!fMergedLayer) {
		// construct bitmap to hold result
		BBitmap* bitmap = NULL;
		if (fCanvas && fCount > 1) {
			if (Layer* layer = fLayers[fCount - 1]) {
				if (layer->Bitmap()) {
					bitmap = new BBitmap(layer->Bitmap());
				}
				fIndices[fCount - 1] = fCanvas->IndexOf(layer);
			}
		}
		if (bitmap) {
			if ((status = bitmap->InitCheck()) >= B_OK) {
				// compose each layer into the bitmap
				// (reverse order)
				fMergedIndex = 0;
				BRect area = bitmap->Bounds();
				for (int32 i = fCount - 2; i >= 0; i--) {
					if (Layer* layer = fLayers[i]) {
						layer->Compose(bitmap, area);
						// remember layer index
						fIndices[i] = fCanvas->IndexOf(layer);
						// adjust insertion index
						if (fMergedIndex < fIndices[i])
							fMergedIndex = fIndices[i];
					}
				}
				if (fMergedIndex < 0)
					fMergedIndex = 0;
				fMergedLayer = new Layer(bitmap);
			} else
				delete bitmap;
		}
	}
	if (fMergedLayer) {
		if ((status = fMergedLayer->InitCheck()) >= B_OK) {
			// insert merged layer and remove the others
			LanguageManager* m = LanguageManager::Default();
			fMergedLayer->SetName(m->GetString(MERGED, "Merged"));
			if (fCanvas->AddLayer(fMergedLayer, fMergedIndex)) {
				for (int32 i = 0; i < fCount; i++) {
					fCanvas->RemoveLayer(fLayers[i]);
				}
				view->SetToLayer(fMergedIndex);
				if (LayersListView* listView = view->GetLayersListView())
					listView->SetToCanvas(fCanvas, true);
				fMergedIndex = -1;
			} else
				status = B_ERROR;
		}
		if (status < B_OK) {
			delete fMergedLayer;
			fMergedLayer = NULL;
		}
	}
	view->InvalidateCanvas(fCanvas->Bounds(), true);
	return status;
}

// Undo
status_t
MergeLayersAction::Undo(CanvasView* view)
{
	status_t status = B_ERROR;
	if (fMergedLayer && fCanvas && fIndices) {
		fMergedIndex = fCanvas->IndexOf(fMergedLayer);
		if (fCanvas->RemoveLayer(fMergedLayer)) {
			for (int32 i = 0; i < fCount; i++) {
				fCanvas->AddLayer(fLayers[i], fIndices[i]);
			}
			view->SetToLayer(fIndices[0]);
			if (LayersListView* listView = view->GetLayersListView())
				listView->SetToCanvas(fCanvas, true);
			status = B_OK;
		}
		view->InvalidateCanvas(fCanvas->Bounds(), true);
	}
	return status;
}

// Redo
status_t
MergeLayersAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
MergeLayersAction::GetName(BString& name)
{
	name << _GetString(MERGE_LAYERS, "Merge Layers");
}
