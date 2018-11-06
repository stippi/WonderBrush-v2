// InsertPointAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "VectorPath.h"

#include "InsertPointAction.h"

// constructor
InsertPointAction::InsertPointAction(ShapeState* state,
									 ShapeStroke* modifier,
									 int32 index,
									 const int32* selected,
									 int32 count)
	: PathAction(state, modifier),
	  fIndex(index),
	  fOldSelection(NULL),
	  fOldSelectionCount(count)
{
	if (fPath && (!fPath->GetPointsAt(fIndex, fPoint, fPointIn, fPointOut)
				  || !fPath->GetPointOutAt(fIndex - 1, fPreviousOut)
				  || !fPath->GetPointInAt(fIndex + 1, fNextIn))) {
		fPath = NULL;
	}
	if (fOldSelectionCount > 0 && selected) {
		fOldSelection = new int32[fOldSelectionCount];
		memcpy(fOldSelection, selected, fOldSelectionCount * sizeof(int32));
	}
}

// destructor
InsertPointAction::~InsertPointAction()
{
	delete[] fOldSelection;
}

// Perform
status_t
InsertPointAction::Perform(CanvasView* view)
{
	// path point is already added
	// but in/out points might still have changed
	status_t status = InitCheck();
	if (status >= B_OK) {
		fPath->GetPointInAt(fIndex, fPointIn);
		fPath->GetPointOutAt(fIndex, fPointOut);
	}
	return status;
}

// Undo
status_t
InsertPointAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current bounds
		BRect r;
		BRect cr;
		_GetPathBounds(r, cr);
		// remove the inserted point
		if (fPath->RemovePoint(fIndex)) {
			// remember current previous "out" and restore it
			BPoint previousOut = fPreviousOut;
			fPath->GetPointOutAt(fIndex - 1, fPreviousOut);
			fPath->SetPointOut(fIndex - 1, previousOut);
			// remember current next "in" and restore it
			BPoint nextIn = fNextIn;
			fPath->GetPointInAt(fIndex, fNextIn);
			fPath->SetPointIn(fIndex, nextIn);
			// redraw path
			_RedrawPath(r, cr);
			// restore previous selection
			_Select(fOldSelection, fOldSelectionCount);
		} else {
			status = B_ERROR;
		}
	}
	return status;
}

// Redo
status_t
InsertPointAction::Redo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current bounds
		BRect r;
		BRect cr;
		_GetPathBounds(r, cr);
		// do the necessary
		if (fPath->AddPoint(fPoint, fIndex)) {
			fPath->SetPoint(fIndex, fPoint, fPointIn, fPointOut, true);
			// remember current previous "out" and restore it
			BPoint previousOut = fPreviousOut;
			fPath->GetPointOutAt(fIndex - 1, fPreviousOut);
			fPath->SetPointOut(fIndex - 1, previousOut);
			// remember current next "in" and restore it
			BPoint nextIn = fNextIn;
			fPath->GetPointInAt(fIndex + 1, fNextIn);
			fPath->SetPointIn(fIndex + 1, nextIn);
			// select inserted point
			_Select(&fIndex, 1);
			// redraw path
			_RedrawPath(r, cr);
		} else {
			status = B_ERROR;
		}
	}
	return status;
}

// GetName
void
InsertPointAction::GetName(BString& name)
{
	name << _GetString(INSERT_CONTROL_POINT, "Insert Control Point");
}

