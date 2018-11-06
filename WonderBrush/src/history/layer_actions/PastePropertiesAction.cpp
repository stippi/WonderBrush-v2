// PastePropertiesAction.cpp

#include <stdio.h>

#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"
#include "PropertyObject.h"
#include "Stroke.h"

#include "PastePropertiesAction.h"

// constructor
PastePropertiesAction::PastePropertiesAction(CanvasView* view,
											 Layer* layer,
											 Stroke** objects,
											 int32 count,
											 PropertyObject* properties)
	: LayerAction(layer),
	  fObjects(NULL),
	  fClonedObjects(NULL),
	  fCount(count),
	  fPropertyCount(properties ? properties->CountProperties() : 0)
{
	if (fLayer && fCount > 0 && fPropertyCount > 0) {
		fObjects = new Stroke*[fCount];
		fClonedObjects = new Stroke*[fCount];
		memset(fObjects, 0, sizeof(Stroke*) * fCount);
		memset(fClonedObjects, 0, sizeof(Stroke*) * fCount);

		view->PrepareForObjectPropertyChange();

		HistoryListView* listView = view->GetHistoryListView();
		if (listView && listView->GetLayer() != fLayer)
			listView = NULL;

		BRect r(0.0, 0.0, -1.0, -1.0);
		// try to apply the properties,
		// if an object changes, add it to our list
		int32 count = fCount;
		int32 current = 0;
		for (int32 i = 0; i < count; i++) {
			if (objects[i]) {
				Stroke* clone = objects[i]->Clone();
				if (objects[i]->SetToPropertyObject(properties)) {
					fObjects[current] = objects[i];
					fClonedObjects[current] = clone;

					r = r.IsValid() ? r | (clone->Bounds() | fObjects[current]->Bounds())
									: clone->Bounds() | fObjects[current]->Bounds();

					// update object list item
					if (listView) {
						int32 index = fLayer->GetHistory()->IndexOf(fObjects[current]);
						listView->SetPainter(index, fObjects[current]->Painter());
					}

					current++;
				} else {
					delete clone;
				}
			}
		}

		view->ObjectPropertyChangeDone();

		fCount = current;
		if (fCount > 0) {
			fLayer->Touch(r);
			view->RebuildLayer(fLayer, r);
		}
	}
}

// destructor
PastePropertiesAction::~PastePropertiesAction()
{
	delete[] fObjects;
	delete[] fClonedObjects;
}

// InitCheck
status_t
PastePropertiesAction::InitCheck()
{
	return fCount > 0 ? LayerAction::InitCheck() : B_NO_INIT;
}

// Perform
status_t
PastePropertiesAction::Perform(CanvasView* view)
{
	// objects are already changed
	return B_OK;
}

// Undo
status_t
PastePropertiesAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {

		HistoryListView* listView = view->GetHistoryListView();
		if (listView && listView->GetLayer() != fLayer)
			listView = NULL;

		view->PrepareForObjectPropertyChange();

		BRect r(0.0, 0.0, -1.0, -1.0);
		// swap the objects around
		for (int32 i = 0; i < fCount; i++) {
			// make a new clone
			Stroke* newClone = fObjects[i]->Clone();

			// change object to old clone
			fObjects[i]->SetTo(fClonedObjects[i]);

			r = r.IsValid() ? r | (newClone->Bounds() | fObjects[i]->Bounds())
							: newClone->Bounds() | fObjects[i]->Bounds();

			// swap old clone for new one
			delete fClonedObjects[i];
			fClonedObjects[i] = newClone;

			// update object list item
			if (listView) {
				int32 index = fLayer->GetHistory()->IndexOf(fObjects[i]);
				listView->SetPainter(index, fObjects[i]->Painter());
			}

		}

		view->ObjectPropertyChangeDone();

		view->RebuildLayer(fLayer, r);
	}
	return status;
}

// Redo
status_t
PastePropertiesAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
PastePropertiesAction::GetName(BString& name)
{
	if (fPropertyCount > 1) {
		if (fCount > 1)
			name << _GetString(MUTLI_PASTE_PROPERTIES, "Multi Paste Properties");
		else
			name << _GetString(PASTE_PROPERTIES, "Paste Properties");
	} else {
		if (fCount > 1)
			name << _GetString(MULTI_PASTE_PROPERTY, "Multi Paste Property");
		else
			name << _GetString(PASTE_PROPERTY, "Paste Property");
	}
}
