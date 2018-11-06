// AppendCanvasAction.cpp

#include <stdio.h>

#include "Canvas.h"
#include "Layer.h"

#include "CanvasView.h"
#include "LayersListView.h"

#include "AppendCanvasAction.h"

// constructor
AppendCanvasAction::AppendCanvasAction(Canvas* dest,
									   const Canvas* source,
									   int32 appendIndex)
	: CanvasAction(dest),
	  fLayers(NULL),
	  fCount(0),
	  fAppendIndex(appendIndex),
	  fAppended(false)
{
	if (fCanvas) {
		if (fAppendIndex < 0 || fAppendIndex > fCanvas->CountLayers())
			fAppendIndex = fCanvas->CountLayers();
		
		fCount = source->CountLayers();
		fLayers = new Layer*[fCount];

		for (int32 i = 0; i < fCount; i++) {

			if (Layer* fromLayer = source->LayerAt(i))
				fLayers[i] = new Layer(fromLayer, fCanvas->Bounds());
			else
				fLayers[i] = NULL;

			if (fLayers[i]->InitCheck() < B_OK) {
				delete fLayers[i];
				fLayers[i] = NULL;
			}
		}
	}
}

// destructor
AppendCanvasAction::~AppendCanvasAction()
{
	if (!fAppended) {
		// this means we have removed the layers or never added them,
		// we need to delete them
		for (int32 i = 0; i < fCount; i++)
			delete fLayers[i];
	}
	delete[] fLayers;
}

// InitCheck
status_t
AppendCanvasAction::InitCheck()
{
	status_t status = fLayers ? B_OK : B_BAD_VALUE;
	return status;
}

// Perform
status_t
AppendCanvasAction::Perform(CanvasView* view)
{
	status_t status = B_ERROR;
	if (fLayers) {
		status = B_OK;
		fAppended = true;

		Layer* currentLayer = fCanvas->LayerAt(fCanvas->CurrentLayer());

		int32 index = fAppendIndex;
		for (int32 i = 0; i < fCount; i++) {
			if (!fCanvas->AddLayer(fLayers[i], index)) {
				// at least don't leak anything in case of error
				delete fLayers[i];
				fLayers[i] = NULL;
				status = B_ERROR;
				break;
			} else {
				index++;
			}
		}
		// update canvas view and rerender canvas
		if (view->CurrentCanvas() == fCanvas) {
			view->SetToLayer(fCanvas->IndexOf(currentLayer));
			view->InvalidateCanvas(fCanvas->Bounds(), true);
			// update list view
			if (LayersListView* listView = view->GetLayersListView())
				listView->SetToCanvas(fCanvas, true);
		}
	}
	return status;
}

// Undo
status_t
AppendCanvasAction::Undo(CanvasView* view)
{
	status_t status = B_ERROR;
	if (fLayers) {
		status = B_OK;
		fAppended = false;

		Layer* currentLayer = fCanvas->LayerAt(fCanvas->CurrentLayer());

		for (int32 i = 0; i < fCount; i++) {
			fCanvas->RemoveLayer(fLayers[i]);
		}

		// update canvas view and rerender canvas
		if (view->CurrentCanvas() == fCanvas) {
			view->SetToLayer(fCanvas->IndexOf(currentLayer));
			view->InvalidateCanvas(fCanvas->Bounds(), true);
			// update list view
			if (LayersListView* listView = view->GetLayersListView())
				listView->SetToCanvas(fCanvas, true);
		}
	}
	return status;
}

// Redo
status_t
AppendCanvasAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
AppendCanvasAction::GetName(BString& name)
{
	name << _GetString(INSERT_CANVAS, "Insert Canvas");
}
