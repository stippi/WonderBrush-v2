// TransformPointsAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"
#include "PointSelection.h"
#include "VectorPath.h"

#include "TransformPointsAction.h"

// constructor
TransformPointsAction::TransformPointsAction(ShapeState* state,
											 ShapeStroke* object,
											 const int32* indices,
											 const control_point* points,
											 int32 count,
//											 BPoint translation,
//											 double rotation,
//											 double xScale,
//											 double yScale,
											 const Transformable& t,
											 BPoint centerOffset,
											 const char* actionName,
											 uint32 nameIndex)
//	: TransformAction(translation,
//					  rotation,
//					  xScale,
//					  yScale,
	: TransformAction(t,
					  centerOffset,
					  actionName,
					  nameIndex),
	  fShapeState(state),
	  fShapeObject(object),
	  fIndices(NULL),
	  fPoints(NULL),
	  fCount(count)
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
TransformPointsAction::~TransformPointsAction()
{
	delete[] fIndices;
	delete[] fPoints;
}

// InitCheck
status_t
TransformPointsAction::InitCheck()
{
	if (fShapeState && fIndices && fPoints)
		return TransformAction::InitCheck();
	else
		return B_NO_INIT;
}

// _SetTransformation
status_t
TransformPointsAction::_SetTransformation(CanvasView* view,
//										  BPoint translation,
//										  double rotation,
//										  double xScale,
//										  double yScale,
										  const Transformable& t,
										  BPoint centerOffset) const
{
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
//	selection->SetTransformation(translation,
//								 rotation,
//								 xScale,
//								 yScale);
//	selection->OffsetOrigin(centerOffset);
	selection->Set(t, centerOffset);
	fShapeState->_SetPointSelection(selection);
	return B_OK;
}

