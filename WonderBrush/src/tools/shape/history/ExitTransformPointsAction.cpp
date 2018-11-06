// ExitTransformPointsAction.cpp

#include <stdio.h>

#include "PointSelection.h"
#include "ShapeState.h"
#include "ShapeStroke.h"
#include "Strings.h"
#include "VectorPath.h"

#include "ExitTransformPointsAction.h"

// constructor
ExitTransformPointsAction::ExitTransformPointsAction(ShapeState* state,
										 ShapeStroke* object,
										 const int32* indices,
										 const control_point* points,
										 int32 count,

//										 BPoint translation,
//										 double rotation,
//										 double xScale,
//										 double yScale,
										 const Transformable& t,

										 bool cancel)
	: fShapeState(state),
	  fShapeObject(object),
	  fIndices(NULL),
	  fPoints(NULL),
	  fCount(count),

//	  fTranslation(translation),
//	  fRotation(rotation),
//	  fXScale(xScale),
//	  fYScale(yScale),
	  fTransformation(t),

	  fCancel(cancel)
{
	if (fCount > 0 && indices) {
		fIndices = new int32[fCount];
		memcpy(fIndices, indices, fCount * sizeof(int32));
	}
	if (fCount > 0 && points) {
		fPoints = new control_point[fCount];
		memcpy(fPoints, points, fCount * sizeof(control_point));
	}
}

// destructor
ExitTransformPointsAction::~ExitTransformPointsAction()
{
	delete[] fIndices;
	delete[] fPoints;
}

// InitCheck
status_t
ExitTransformPointsAction::InitCheck()
{
	return fShapeState && fIndices && fPoints ? B_OK : B_NO_INIT;
}

// Perform
status_t
ExitTransformPointsAction::Perform(CanvasView* view)
{
	// transformation is already current
	return InitCheck();
}

// Undo
status_t
ExitTransformPointsAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// restore original path
		VectorPath* path = fShapeObject->Path();
		if (path) {
			for (int32 i = 0; i < fCount; i++) {
				path->SetPoint(fIndices[i], fPoints[i].point,
											fPoints[i].point_in,
											fPoints[i].point_out,
											fPoints[i].connected);
			}
		}
		PointSelection* selection = new PointSelection(view,
													   fShapeState,
													   fShapeObject,
													   path,
													   fIndices, fCount);
		// the order of calls here seems a bit weird,
		// the reason is that _SetPointSelection() triggers the selection
		// to update to the current mouse position, and this makes only
		// sense after the transformation has been set
		fShapeState->_Select(fIndices, fCount, false);
//		selection->SetTransformation(fTranslation, fRotation, fXScale, fYScale);
		selection->Set(fTransformation, B_ORIGIN);
		fShapeState->_SetPointSelection(selection);
	}
	return status;
}

// Redo
status_t
ExitTransformPointsAction::Redo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		if (fCancel) {
			BRect r = fShapeObject->Bounds();
			// restore original path
			VectorPath* path = fShapeObject->Path();
			if (path) {
				for (int32 i = 0; i < fCount; i++) {
					path->SetPoint(fIndices[i], fPoints[i].point,
												fPoints[i].point_in,
												fPoints[i].point_out,
												fPoints[i].connected);
				}
			}
			fShapeState->_RedrawStroke(r, r, true, true);
		}
		fShapeState->_SetPointSelection(NULL);
	}
	return status;
}

// GetName
void
ExitTransformPointsAction::GetName(BString& name)
{
	name << _GetString(FINISH_TRANSFORMATION, "Finish Transformation");
}

