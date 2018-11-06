// EnterTransformPointsAction.cpp

#include <stdio.h>

#include "PointSelection.h"
#include "ShapeState.h"
#include "ShapeStroke.h"
#include "Strings.h"
#include "VectorPath.h"

#include "EnterTransformPointsAction.h"

// constructor
EnterTransformPointsAction::EnterTransformPointsAction(ShapeState* state,
										   ShapeStroke* object,
										   const int32* indices,
										   int32 count)
	: fShapeState(state),
	  fShapeObject(object),
	  fIndices(NULL),
	  fCount(count)
{
	if (fCount > 0 && indices) {
		fIndices = new int32[fCount];
		memcpy(fIndices, indices, fCount * sizeof(int32));
	}
}

// destructor
EnterTransformPointsAction::~EnterTransformPointsAction()
{
	delete[] fIndices;
}

// InitCheck
status_t
EnterTransformPointsAction::InitCheck()
{
	return fShapeState && fIndices ? B_OK : B_NO_INIT;
}

// Perform
status_t
EnterTransformPointsAction::Perform(CanvasView* view)
{
	// transformation is already current
	return InitCheck();
}

// Undo
status_t
EnterTransformPointsAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		fShapeState->_SetPointSelection(NULL);
	}
	return status;
}

// Redo
status_t
EnterTransformPointsAction::Redo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		PointSelection* selection = new PointSelection(view,
													   fShapeState,
													   fShapeObject,
													   fShapeObject->Path(),
													   fIndices, fCount);
		fShapeState->_SetPointSelection(selection);
	}
	return status;
}

// GetName
void
EnterTransformPointsAction::GetName(BString& name)
{
	name << _GetString(BEGIN_TRANSFORMATION, "Begin Transformation");
}

