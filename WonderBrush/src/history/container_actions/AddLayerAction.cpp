// AddLayerAction.cpp

#include <stdio.h>

#include <Bitmap.h>

#include "LayerContainer.h"
#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "LayersListView.h"

#include "AddLayerAction.h"

// constructor
AddLayerAction::AddLayerAction(LayerContainer* container,
							   Layer* layer,
							   int32 index)
	: ContainerAction(container),
	  fLayer(layer),
	  fIndex(index)
{
}

// destructor
AddLayerAction::~AddLayerAction()
{
	if (fIndex >= 0) {
		// this means we have removed the layer,
		// we need to delete it
		delete fLayer;
	}
}

// InitCheck
status_t
AddLayerAction::InitCheck()
{
	status_t status = fContainer && fLayer ? B_OK : B_BAD_VALUE;
	return status;
}

// Perform
status_t
AddLayerAction::Perform(CanvasView* view)
{
	status_t status = B_ERROR;
	if (fContainer && fLayer) {
		if (fContainer->AddLayer(fLayer, fIndex)) {
			fIndex = fContainer->IndexOf(fLayer);
			view->SetToLayer(fIndex);
			if (LayersListView* listView = view->GetLayersListView())
				listView->AddLayer(fLayer->Name(), fIndex);
			// invalidate canvas
			if (fLayer->ActiveBounds().IsValid())
				view->InvalidateCanvas(fLayer->ActiveBounds(), true);
			status = B_OK;
			// forget index (so we know not to delete the layer)
			fIndex = -1;
		}
	}
	return status;
}

// Undo
status_t
AddLayerAction::Undo(CanvasView* view)
{
	status_t status = B_ERROR;
	if (fLayer && fContainer) {
		// remember index again
		fIndex = fContainer->IndexOf(fLayer);
		if (fContainer->RemoveLayer(fLayer)) {
			// set the view to the next layer
			int32 index = fIndex;
			int32 count = fContainer->CountLayers();
			if (index >= count)
				index = count - 1;
			view->SetToLayer(index);
			if (LayersListView* listView = view->GetLayersListView())
				listView->Update();
			// invalidate canvas
			if (fLayer->ActiveBounds().IsValid())
				view->InvalidateCanvas(fLayer->ActiveBounds(), true);
			status = B_OK;
		}
	}
	return status;
}

// Redo
status_t
AddLayerAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
AddLayerAction::GetName(BString& name)
{
	name << _GetString(ADD_LAYER, "Add Layer");
}
