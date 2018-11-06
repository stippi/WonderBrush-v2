// ObjectSelection.cpp

#include <stdio.h>
#include <string.h>

#include "Stroke.h"
#include "TransformState.h"
#include "TransformAction.h"

#include "ObjectSelection.h"

// constructor
ObjectSelection::ObjectSelection(CanvasView* view,
								 TransformState* state,
								 Stroke** objects,
								 int32 count)
	: TransformBox(view, BRect(0.0, 0.0, 1.0, 1.0)),
	  fTransformState(state),

	  fObjects(objects && count > 0 ? new Stroke*[count] : NULL),
	  fCount(count),

	  fOriginals(NULL)
{
	BRect box(0.0, 0.0, -1.0, -1.0);
	if (fObjects) {
		// allocate storage for the current transformations
		// of each object
		fOriginals = new double[fCount * 9];

		// figure out bounds and init transformations
		for (int32 i = 0; i < fCount; i++) {
			fObjects[i] = objects[i];
			if (fObjects[i]) {
				fObjects[i]->AddObserver(this);
				box = !box.IsValid() ? objects[i]->Bounds()
									 : box | objects[i]->Bounds();
	
				fObjects[i]->StoreTo(&fOriginals[i * 9]);
			}
		}
	}
	// offset box to middle of pixels
//	box.OffsetBy(0.5, 0.5);
	SetBox(box);
}

// copy constructor
ObjectSelection::ObjectSelection(const ObjectSelection& other)
	: TransformBox(other),
	  fTransformState(other.fTransformState),
	  fObjects(other.fCount > 0 ? new Stroke*[other.fCount] : NULL),
	  fCount(other.fCount),
	  fOriginals(fCount > 0 ? new double[fCount * 9] : NULL)
{
	if (fObjects) {
		memcpy(fObjects, other.fObjects, fCount * sizeof(Stroke*));
		for (int32 i = 0; i < fCount; i++) {
			if (fObjects[i]) {
				fObjects[i]->AddObserver(this);
			}
		}
	}
	if (fOriginals) {
		memcpy(fOriginals, other.fOriginals, fCount * sizeof(double) * 9);
	}
}

// destructor
ObjectSelection::~ObjectSelection()
{
	for (int32 i = 0; i < fCount; i++) {
		if (fObjects[i]) {
			fObjects[i]->RemoveObserver(this);
		}
	}
	delete[] fObjects;
	delete[] fOriginals;
}

// Bounds
BRect
ObjectSelection::Bounds() const
{
	return TransformBox::Bounds();
}

// Update
void
ObjectSelection::Update(bool deep)
{
	// remember current bounds
	BRect r = Bounds();

	TransformBox::Update(deep);

	if (deep) {
		BRect rebuildArea = r;
		if (fObjects) {
			for (int32 i = 0; i < fCount; i++) {
				if (fObjects[i]) {
					// reset the objects transformation to the saved state
					fObjects[i]->RemoveObserver(this);
					fObjects[i]->LoadFrom(&fOriginals[i * 9]);
					// combined with the current transformation
					fObjects[i]->Multiply(*this);
					fObjects[i]->AddObserver(this);

					rebuildArea = rebuildArea | fObjects[i]->Bounds();
				}
			}
		}
		fTransformState->_RebuildLayer(rebuildArea);
	}

	r = r | Bounds();
	fTransformState->_InvalidateCanvasRect(r);
}

// ObjectChanged
void
ObjectSelection::ObjectChanged(const Observable* object)
{
printf("ObjectSelection::ObjectChanged()\n");
	// recalc bounds
	BRect box(0.0, 0.0, -1.0, -1.0);
	// figure out bounds and init transformations
	for (int32 i = 0; i < fCount; i++) {
		if (fObjects[i]) {
			// reset the objects transformation to the saved state
			fObjects[i]->RemoveObserver(this);
			fObjects[i]->LoadFrom(&fOriginals[i * 9]);

			box = !box.IsValid() ? fObjects[i]->Bounds()
								 : box | fObjects[i]->Bounds();

			// combined with the current transformation
			fObjects[i]->Multiply(*this);
			fObjects[i]->AddObserver(this);
		}
	}
	// update with the changed box
	SetBox(box);
}

// Perform
Action*
ObjectSelection::Perform()
{
	return NULL;
}

// Cancel
Action*
ObjectSelection::Cancel()
{
	SetTransformation(BPoint(0.0, 0.0), 0.0, 1.0, 1.0);

	return NULL;
}

// MakeAction
TransformAction*
ObjectSelection::MakeAction(const char* actionName, uint32 nameIndex) const
{
	TransformAction* action =
//		fTransformState->ChangeTransformation(Translation(),LocalRotation(),
//											  LocalXScale(),LocalYScale(),
		fTransformState->ChangeTransformation(*this,
											  CenterOffset(), true);
	if (action)
		action->SetName(actionName, nameIndex);
	return action;
}
