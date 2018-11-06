// PointSelection.h

#include <stdio.h>
#include <string.h>

#include "ShapeState.h"
#include "ShapeStroke.h"
#include "TransformPointsAction.h"
#include "ExitTransformPointsAction.h"
#include "VectorPath.h"

#include "PointSelection.h"

// constructor
PointSelection::PointSelection(CanvasView* view,
							   ShapeState* state,
							   ShapeStroke* object,
							   VectorPath* path,
							   const int32* indices,
							   int32 count)
	: TransformBox(view, BRect(0.0, 0.0, 1.0, 1.0)),
	  fShapeState(state),
	  fShapeObject(object),
	  fPath(path),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fIndices(path && count > 0 ? new int32[count] : NULL),
	  fCount(count),
	  fPoints(NULL)
{
	if (fIndices) {
		// copy indices
		memcpy(fIndices, indices, fCount * sizeof(int32));
		fPoints = new control_point[fCount];
		// make a copy of the points as they are and calculate bounds
		for (int32 i = 0; i < fCount; i++) {
			if (fPath->GetPointsAt(fIndices[i], fPoints[i].point,
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

// destructor
PointSelection::~PointSelection()
{
	delete[] fIndices;
	delete[] fPoints;
}

// Bounds
BRect
PointSelection::Bounds() const
{
	if (fPath) {
		BRect r = fPath->ControlPointBounds();
//		if (fShapeObject)
//			r.OffsetBy(fShapeObject->Offset());
		r = r | TransformBox::Bounds();
		r.InsetBy(-8.0, -8.0);
		return r;
	}
	return TransformBox::Bounds();
}

// Update
void
PointSelection::Update(bool deep)
{
	// remember current bounds
	BRect cr = Bounds();

	TransformBox::Update(deep);

	if (deep) {
		if (fIndices && fPoints) {
			for (int32 i = 0; i < fCount; i++) {
	
				BPoint transformed = fPoints[i].point;
				BPoint transformedIn = fPoints[i].point_in;
				BPoint transformedOut = fPoints[i].point_out;
	
				Transform(&transformed);
				Transform(&transformedIn);
				Transform(&transformedOut);
	
				fPath->SetPoint(fIndices[i], transformed,
											 transformedIn,
											 transformedOut,
											 fPoints[i].connected);
			}
			if (fShapeState && fShapeObject) {
				BRect r = fShapeObject->Bounds();
				cr = cr | Bounds();
				fShapeState->_RedrawStroke(r, cr, true, true);
			}
		}
	} else {
		cr = cr | Bounds();
		fShapeState->_InvalidateCanvasRect(cr, false);
	}
}

// Perform
Action*
PointSelection::Perform()
{
	Action* action =  new ExitTransformPointsAction(fShapeState,
													fShapeObject,
													fIndices,
													fPoints,
													fCount,
//													Translation(),
//													LocalRotation(),
//													LocalXScale(),
//													LocalYScale(),
													*this,
													false);

	return action;
}

// Cancel
Action*
PointSelection::Cancel()
{
	Action* action =  new ExitTransformPointsAction(fShapeState,
													fShapeObject,
													fIndices,
													fPoints,
													fCount,
//													Translation(),
//													LocalRotation(),
//													LocalXScale(),
//													LocalYScale(),
													*this,
													true);

	SetTransformation(BPoint(0.0, 0.0), 0.0, 1.0, 1.0);

	return action;
}

// MakeAction
TransformAction*
PointSelection::MakeAction(const char* actionName, uint32 nameIndex) const
{
	return new TransformPointsAction(fShapeState,
									 fShapeObject,
									 fIndices,
									 fPoints,
									 fCount,
//									 Translation(),
//									 LocalRotation(),
//									 LocalXScale(),
//									 LocalYScale(),
									 *this,
									 CenterOffset(),
									 actionName,
									 nameIndex);
}
