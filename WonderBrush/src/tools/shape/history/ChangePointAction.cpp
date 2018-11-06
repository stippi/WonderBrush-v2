// ChangePointAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "VectorPath.h"

#include "ChangePointAction.h"

// constructor
ChangePointAction::ChangePointAction(ShapeState* state,
									 ShapeStroke* modifier,
									 int32 index,
 									 const int32* selected,
									 int32 count)
	: PathAction(state, modifier),
	  fIndex(index),
	  fOldSelection(NULL),
	  fOldSelectionCount(count)
{
	if (fPath && !fPath->GetPointsAt(fIndex, fPoint, fPointIn, fPointOut, &fConnected))
		fPath = NULL;
	if (fOldSelectionCount > 0 && selected) {
		fOldSelection = new int32[fOldSelectionCount];
		memcpy(fOldSelection, selected, fOldSelectionCount * sizeof(int32));
	}
}

// destructor
ChangePointAction::~ChangePointAction()
{
	delete[] fOldSelection;
}

// Perform
status_t
ChangePointAction::Perform(CanvasView* view)
{
	// path point is already changed
	return B_OK;
}

// Undo
status_t
ChangePointAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current bounds
		BRect r;
		BRect cr;
		_GetPathBounds(r, cr);
		// do the necessary
		BPoint point;
		BPoint pointIn;
		BPoint pointOut;
		bool connected;
		if (fPath->GetPointsAt(fIndex, point, pointIn, pointOut, &connected)
			&& fPath->SetPoint(fIndex, fPoint, fPointIn, fPointOut, fConnected)) {
			// redraw path
			_RedrawPath(r, cr);
			// toggle the remembered settings
			fPoint = point;
			fPointIn = pointIn;
			fPointOut = pointOut;
			fConnected = connected;
			// restore old selection
			_Select(fOldSelection, fOldSelectionCount);
		} else {
			status = B_ERROR;
		}
	}
	return status;
}

// Redo
status_t
ChangePointAction::Redo(CanvasView* view)
{
	status_t status = Undo(view);
	if (status >= B_OK)
		_Select(&fIndex, 1);
	return status;
}

// GetName
void
ChangePointAction::GetName(BString& name)
{
	name << _GetString(MODIFY_CONTROL_POINT, "Modify Control Point");
}
