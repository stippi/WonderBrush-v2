// TransformObjectsAction.cpp

#include <stdio.h>

#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "TransformState.h"

#include "TransformObjectsAction.h"

// constructor
TransformObjectsAction::TransformObjectsAction(TransformState* state,
											   Layer* layer,
											   Stroke** objects,
											   int32 count)
	: LayerAction(layer),
	  AdvancedTransformable(),
	  fTransformState(state),
	  fObjects(objects && count > 0 ? new Stroke*[count] : NULL),
	  fCount(count),
	  fOriginals(NULL),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fCenterOffset(0.0, 0.0)
{
	if (fObjects) {
		memcpy(fObjects, objects, fCount * sizeof(Stroke*));

		// store original transform matrices
		fOriginals = new double[fCount * 9];

		// figure out bounds and init transformations
		for (int32 i = 0; i < fCount; i++) {
			fObjects[i] = objects[i];
			if (fObjects[i]) {
				fBounds = !fBounds.IsValid() ? fObjects[i]->Bounds()
											 : fBounds | fObjects[i]->Bounds();
	
				fObjects[i]->StoreTo(&fOriginals[i * 9]);
			}
		}
	}
}

// destructor
TransformObjectsAction::~TransformObjectsAction()
{
	delete[] fObjects;
	delete[] fOriginals;
}

// InitCheck
status_t
TransformObjectsAction::InitCheck()
{
	status_t status = LayerAction::InitCheck();
	if (status >= B_OK)
		status = fObjects && !IsIdentity() ? B_OK : B_NO_INIT;
	return status;
}

// Perform
status_t
TransformObjectsAction::Perform(CanvasView* view)
{
	// objects are already transformed
	return InitCheck();
}

// Undo
status_t
TransformObjectsAction::Undo(CanvasView* view)
{
	status_t ret = InitCheck();
	if (ret >= B_OK) {
		// restore original transformations
		BRect b = TransformBounds(fBounds);
		for (int32 i = 0; i < fCount; i++) {
			if (fObjects[i]) {
				fObjects[i]->LoadFrom(&fOriginals[i * 9]);
			}
		}
		b = b | fBounds;
		// rebuild layer bitmap
		view->RebuildBitmap(b, fLayer);

		// this trick will make sure that
		// the TransformState is up to date
		fTransformState->UpdateBox();
	}
	return ret;
}

// Redo
status_t
TransformObjectsAction::Redo(CanvasView* view)
{
	status_t ret = InitCheck();
	if (ret >= B_OK) {
		// restore original transformations
		// and add our own transformation
		BRect b = fBounds;
		for (int32 i = 0; i < fCount; i++) {
			if (fObjects[i]) {
				fObjects[i]->LoadFrom(&fOriginals[i * 9]);
				fObjects[i]->Multiply(*this);
			}
		}
		b = b | TransformBounds(fBounds);
		// rebuild layer bitmap
		view->RebuildBitmap(b, fLayer);

		// this trick will make sure that
		// the TransformState is up to date
		fTransformState->UpdateBox();
	}
	return ret;
}

// GetName
void
TransformObjectsAction::GetName(BString& name)
{
	if (fCount > 1)
		name << _GetString(TRANSFORM_OBJECTS, "Transform Objects");
	else
		name << _GetString(TRANSFORM_OBJECT, "Transform Object");
}


// ObjectAt
Stroke*
TransformObjectsAction::ObjectAt(int32 index) const
{
	if (fObjects && index >= 0 && index < fCount)
		return fObjects[index];
	return NULL;
}

// SetCenterOffset
void
TransformObjectsAction::SetCenterOffset(BPoint offset)
{
	fCenterOffset = offset;
}


