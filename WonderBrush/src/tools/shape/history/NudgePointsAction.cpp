// NudgePointsAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"
#include "PointSelection.h"
#include "VectorPath.h"

#include "NudgePointsAction.h"

// constructor
NudgePointsAction::NudgePointsAction(ShapeState* state,
									 ShapeStroke* object,
									 const int32* indices,
									 const control_point* points,
									 int32 count)
//	: TransformAction(BPoint(0.0, 0.0),
//					  0.0,
//					  1.0,
//					  1.0,
	: TransformAction(Transformable(),
					  BPoint(0.0, 0.0),
					  count > 1 ? "Nudge Control Points" : "Nudge Control Point",
					  count > 1 ? NUDGE_CONTROL_POINTS : NUDGE_CONTROL_POINT),
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
NudgePointsAction::~NudgePointsAction()
{
	delete[] fIndices;
	delete[] fPoints;
}

// InitCheck
status_t
NudgePointsAction::InitCheck()
{
	if (fShapeState && fIndices && fPoints)
		return TransformAction::InitCheck();
	else
		return B_NO_INIT;
}

// _SetTransformation
status_t
NudgePointsAction::_SetTransformation(CanvasView* view,
//									  BPoint translation,
//									  double rotation,
//									  double xScale,
//									  double yScale,
									  const Transformable& t,
									  BPoint centerOffset) const
{
	// restore original path
	VectorPath* path = fShapeObject->Path();
	if (path) {
		BRect cr = path->ControlPointBounds();
		BRect r = fShapeObject->Bounds();

		BPoint translation(t.tx, t.ty);

		for (int32 i = 0; i < fCount; i++) {
			path->SetPoint(fIndices[i], fPoints[i].point + translation,
										fPoints[i].point_in + translation,
										fPoints[i].point_out + translation,
										fPoints[i].connected);
		}
		fShapeState->_RedrawStroke(r, cr, true, true);
	}
	return B_OK;
}

