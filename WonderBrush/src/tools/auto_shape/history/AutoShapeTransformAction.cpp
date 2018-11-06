// AutoShapeTransformAction.cpp

#include <stdio.h>

#include "AutoShapeState.h"
#include "ShapeStroke.h"
#include "AutoShapeBox.h"
#include "VectorPath.h"

#include "AutoShapeTransformAction.h"

// constructor
AutoShapeTransformAction::AutoShapeTransformAction(AutoShapeState* state,
												   ShapeStroke* object,
												   const control_point* points,
												   int32 count,
//												   BPoint translation,
//												   double rotation,
//												   double xScale,
//												   double yScale,
												   const Transformable& t,
												   BPoint centerOffset,
												   const char* actionName,
												   uint32 nameIndex)
//	: transformaction(translation,
//					  rotation,
//					  xscale,
//					  yscale,
	: TransformAction(t,
					  centerOffset,
					  actionName,
					  nameIndex),
	  fState(state),
	  fShape(object),
	  fPoints(NULL),
	  fCount(count)
{
	if (fCount > 0 && points) {
		fPoints = new control_point[fCount];
		memcpy(fPoints, points, fCount * sizeof(control_point));
	}
}

// destructor
AutoShapeTransformAction::~AutoShapeTransformAction()
{
	delete[] fPoints;
}

// InitCheck
status_t
AutoShapeTransformAction::InitCheck()
{
	if (fState && fPoints)
		return TransformAction::InitCheck();
	else
		return B_NO_INIT;
}

// _SetTransformation
status_t
AutoShapeTransformAction::_SetTransformation(CanvasView* view,
//										  BPoint translation,
//										  double rotation,
//										  double xScale,
//										  double yScale,
										  const Transformable& t,
										  BPoint centerOffset) const
{
	// restore original path
	VectorPath* path = fShape->Path();
	if (path) {
		for (int32 i = 0; i < fCount; i++) {
			path->SetPoint(i, fPoints[i].point,
							  fPoints[i].point_in,
							  fPoints[i].point_out,
							  fPoints[i].connected);
		}
	}
	AutoShapeBox* box = new AutoShapeBox(view,
										 fState,
										 fShape);
	// the order of calls here seems a bit weird,
	// the reason is that _SetPointSelection() triggers the selection
	// to update to the current mouse position, and this makes only
	// sense after the transformation has been set
//	box->SetTransformation(translation,
//						   rotation,
//						   xScale,
//						   yScale);
	box->Set(t, centerOffset);
	fState->_SetBox(box);
	return B_OK;
}

