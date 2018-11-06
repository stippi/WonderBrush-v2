// MoveLayersAction.cpp

#include <stdio.h>

#include "Layer.h"
#include "Stroke.h"

// to workarround a limitation of current design
#include "Canvas.h"

#include "CanvasView.h"
#include "Layer.h"
#include "LayerContainer.h"
#include "LayersListView.h"

#include "MoveLayersAction.h"

// constructor
MoveLayersAction::MoveLayersAction(LayerContainer* container,
								   Layer** layers,
								   int32 count,
								   int32 index)
	: ContainerAction(container),
	  fLayers(layers),
	  fIndices(new int32[count]),
	  fIndex(index),
	  fCount(count)
{
	if (fContainer) {
		for (int32 i = 0; i < fCount; i++) {
			fIndices[i] = fContainer->IndexOf(fLayers[i]);
			if (fIndices[i] < fIndex)
				fIndex--;
		}
	}
}

// destructor
MoveLayersAction::~MoveLayersAction()
{
	delete[] fLayers;
	delete[] fIndices;
}

// InitCheck
status_t
MoveLayersAction::InitCheck()
{
	status_t status = ContainerAction::InitCheck();
	if (status >= B_OK)
		status = fLayers && fIndices ? B_OK : B_NO_INIT;
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
MoveLayersAction::Perform(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {

// workarround to remember selected layer with current design
Layer* selected = NULL;
Canvas* canvas = dynamic_cast<Canvas*>(fContainer);
if (canvas) {
	selected = canvas->LayerAt(canvas->CurrentLayer());
}

		BRect area(0.0, 0.0, -1.0, -1.0);
		// remove layers from container,
		// remember the combined bounds they touch
		for (int32 i = 0; i < fCount; i++) {
			if (Layer* layer = fLayers[i]) {
				fContainer->RemoveLayer(layer);
				if (!area.IsValid())
					area = layer->ActiveBounds();
				else
					area = area | layer->ActiveBounds();
			}
		}
		// add layers to container at index
		int32 index = fIndex;
		for (int32 i = 0; i < fCount; i++) {
			if (Layer* layer = fLayers[i]) {
				fContainer->AddLayer(layer, index++);
			}
		}

// workarround to remember selected layer with current design
if (canvas && selected) {
	canvas->SetCurrentLayer(canvas->IndexOf(selected));
}

		// update views
		if (LayersListView* listView = view->GetLayersListView())
			listView->Update();
		if (area.IsValid())
			view->InvalidateCanvas(area, true);
	}
	return status;
}

// Undo
status_t
MoveLayersAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {

// workarround to remember selected layer with current design
Layer* selected = NULL;
Canvas* canvas = dynamic_cast<Canvas*>(fContainer);
if (canvas) {
	selected = canvas->LayerAt(canvas->CurrentLayer());
}

		BRect area(0.0, 0.0, -1.0, -1.0);
		// remove modifiers from history
		for (int32 i = 0; i < fCount; i++) {
			if (Layer* layer = fLayers[i]) {
				fContainer->RemoveLayer(layer);
				if (!area.IsValid())
					area = layer->ActiveBounds();
				else
					area = area | layer->ActiveBounds();
			}
		}
		// add layers to container at remembered indices
		for (int32 i = 0; i < fCount; i++) {
			if (Layer* layer = fLayers[i])
				fContainer->AddLayer(layer, fIndices[i]);
		}

// workarround to remember selected layer with current design
if (canvas && selected) {
	canvas->SetCurrentLayer(canvas->IndexOf(selected));
}

		// update views
		if (LayersListView* listView = view->GetLayersListView())
			listView->Update();
		if (area.IsValid())
			view->InvalidateCanvas(area, true);
	}
	return status;
}

// Redo
status_t
MoveLayersAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
MoveLayersAction::GetName(BString& name)
{
	if (fCount > 1)
		name << _GetString(MOVE_LAYERS, "Move Layers");
	else
		name << _GetString(MOVE_LAYER, "Move Layer");
}
