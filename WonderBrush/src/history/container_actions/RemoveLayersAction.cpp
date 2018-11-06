// RemoveLayersAction.cpp

#include <stdio.h>

#include <Bitmap.h>

#include "LayerContainer.h"
#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "LayersListView.h"

#include "RemoveLayersAction.h"

// constructor
RemoveLayersAction::RemoveLayersAction(LayerContainer* container,
									   int32* indices,
									   int32 count)
	: ContainerAction(container),
	  fLayers(NULL),
	  fIndices(indices),
	  fCount(count),
	  fLayersRemoved(false)
{
	// initialize the pointers to the layers we have to remove
	if (fCount > 0 && fContainer) {
		fLayers = new Layer*[fCount];
		for (int32 i = 0; i < fCount; i++) {
			fLayers[i] = fContainer->LayerAt(fIndices[i]);
		}
	}
}

// destructor
RemoveLayersAction::~RemoveLayersAction()
{
	if (fLayersRemoved) {
		for (int32 i = 0; i < fCount; i++)
			delete fLayers[i];
	}
	delete[] fLayers;
	delete[] fIndices;
}

// InitCheck
status_t
RemoveLayersAction::InitCheck()
{
	status_t status = fLayers ? B_OK : B_BAD_VALUE;
	return status;
}

// Perform
status_t
RemoveLayersAction::Perform(CanvasView* view)
{
	status_t status = B_ERROR;
	if (fLayers) {
		// remember selected layer
		bool selectNewLayer = false;
		int32 index = -1;
		// remove layers
		for (int32 i = 0; i < fCount; i++) {
			index = fContainer->IndexOf(fLayers[i]);
			if (fLayers[i]->IsCurrent())
				selectNewLayer = true;
			fContainer->RemoveLayer(fLayers[i]);
		}
		// update layer list view
		LayersListView* listView = view->GetLayersListView();
		if (selectNewLayer) {
			// figure out next layer to select
			int32 count = fContainer->CountLayers();
			if (index < 0)
				index = 0;
			if (index >= count)
				index = count - 1;
			// select next layer
			view->SetToLayer(index);
		}
		if (listView) {
			listView->Update();
		}
		view->InvalidateCanvas(fContainer->Bounds(), true);
		fLayersRemoved = true;
		status = B_OK;
	}
	return status;
}

// Undo
status_t
RemoveLayersAction::Undo(CanvasView* view)
{
	status_t status = B_ERROR;
	if (fLayers) {
		// add layers
//		Layer* selected = fContainer->LayerAt(fContainer->CurrentLayer());
		for (int32 i = 0; i < fCount; i++) {
			fContainer->AddLayer(fLayers[i], fIndices[i]);
		}
//		int32 selectedIndex = fContainer->IndexOf(selected);
		int32 selectedIndex = fContainer->IndexOf(fLayers[fCount - 1]);
		// update layer list view
		if (LayersListView* listView = view->GetLayersListView()) {
			listView->Update();
			listView->Select(selectedIndex, false);
		}
		view->InvalidateCanvas(fContainer->Bounds(), true);
//		fContainer->SetCurrentLayer(selectedIndex);
		fLayersRemoved = false;
		status = B_OK;
	}
	return status;
}

// Redo
status_t
RemoveLayersAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
RemoveLayersAction::GetName(BString& name)
{
	if (fCount > 1)
		name << _GetString(REMOVE_LAYERS, "Remove Layers");
	else
		name << _GetString(REMOVE_LAYER, "Remove Layer");
}
