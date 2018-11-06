// AutoShapeBox.h

#include <stdio.h>
#include <string.h>

#include "AutoShapeState.h"
#include "ShapeStroke.h"
#include "AutoShapeTransformAction.h"
//#include "ExitTransformPointsAction.h"
#include "VectorPath.h"

#include "AutoShapeBox.h"

// constructor
AutoShapeBox::AutoShapeBox(CanvasView* view,
						   AutoShapeState* state,
						   ShapeStroke* object)
	: TransformBox(view, BRect(0.0, 0.0, 1.0, 1.0)),
	  fState(state),
	  fShape(NULL),
	  fPath(NULL),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fPoints(NULL),
	  fCount(0),
	  fNoAction(false)
{
	SetToShape(object);
}

// destructor
AutoShapeBox::~AutoShapeBox()
{
	delete[] fPoints;
}
/*
// Bounds
BRect
AutoShapeBox::Bounds() const
{
	if (fPath) {
		BRect r = fPath->ControlPointBounds();
		r = r | TransformBox::Bounds();
//		r.InsetBy(-8.0, -8.0);
		return r;
	}
	return TransformBox::Bounds();
}*/

// Update
void
AutoShapeBox::Update(bool deep)
{
	// remember current bounds
	BRect cr = Bounds();

	TransformBox::Update(deep);

	if (fState && fShape) {
		if (deep) {
			if (fPoints) {

				// get current shape from state
				fState->GetPoints(fPoints, fCount,
								  LocalXScale(), LocalYScale());
	
				for (int32 i = 0; i < fCount; i++) {
	
					fPath->SetPoint(i, Transform(fPoints[i].point),
									   Transform(fPoints[i].point_in),
									   Transform(fPoints[i].point_out),
									   fPoints[i].connected);
				}

				// the Path property has changed
				fShape->Notify();
	
				BRect r = fShape->Bounds();
				cr = cr | Bounds();
				fState->_RedrawStroke(fShape, r, cr, false, true, true);
			}
		} else {
			cr = cr | Bounds();
			fState->_InvalidateCanvasRect(cr, false);
		}
	}
}

// Perform
Action*
AutoShapeBox::Perform()
{
/*	Action* action =  new ExitTransformPointsAction(fState,
													fShape,
													fIndices,
													fPoints,
													fCount,
													Translation(),
													LocalRotation(),
													LocalXScale(),
													LocalYScale(),
													false);

	return action;*/
	return NULL;
}

// Cancel
Action*
AutoShapeBox::Cancel()
{
/*	Action* action =  new ExitTransformPointsAction(fState,
													fShape,
													fIndices,
													fPoints,
													fCount,
													Translation(),
													LocalRotation(),
													LocalXScale(),
													LocalYScale(),
													true);

	SetTransformation(BPoint(0.0, 0.0), 0.0, 1.0, 1.0);

	return action;*/
	return NULL;
}

// MakeAction
TransformAction*
AutoShapeBox::MakeAction(const char* actionName, uint32 nameIndex) const
{
	if (fNoAction) {
		fNoAction = false;
		return NULL;
	}
	return new AutoShapeTransformAction(fState,
										fShape,
										fPoints,
										fCount,
//										Translation(),
//										LocalRotation(),
//										LocalXScale(),
//										LocalYScale(),
										*this,
										CenterOffset(),
										actionName,
										nameIndex);
}

// DragRightBottom
void
AutoShapeBox::DragRightBottom()
{
	_SetState(fDragRBState);
	fNoAction = true;
}

// SetToShape
void
AutoShapeBox::SetToShape(ShapeStroke* object)
{
	delete[] fPoints;
	fPoints = NULL;

	if (object && (fPath = object->Path())) {
		fShape = object;
		fCount = fPath->CountPoints();
		if (fCount > 0)
			fPoints = new control_point[fCount];
	}
	if (fPoints) {
		// make a copy of the points as they are and calculate bounds
		for (int32 i = 0; i < fCount; i++) {
			if (fPath->GetPointsAt(i, fPoints[i].point,
									  fPoints[i].point_in,
									  fPoints[i].point_out,
									  &fPoints[i].connected)) {
				BRect dummy(fPoints[i].point, fPoints[i].point);
				if (i == 0) {
					fBounds = dummy;
				} else {
					fBounds = fBounds | dummy;
				}
				dummy.Set(fPoints[i].point_in.x, fPoints[i].point_in.y,
						  fPoints[i].point_in.x, fPoints[i].point_in.y);
				fBounds = fBounds | dummy;
				dummy.Set(fPoints[i].point_out.x, fPoints[i].point_out.y,
						  fPoints[i].point_out.x, fPoints[i].point_out.y);
				fBounds = fBounds | dummy;
			} else {
				memset(&fPoints[i], 0, sizeof(control_point));
			}
		}
	}
	SetBox(fBounds);
}
