// AddPointAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "VectorPath.h"

#include "AddPointAction.h"

// constructor
AddPointAction::AddPointAction(ShapeState* state,
							   ShapeStroke* modifier,
							   int32 index,
							   const int32* selected,
							   int32 count)
	: PathAction(state, modifier),
	  fIndex(index),
	  fOldSelection(NULL),
	  fOldSelectionCount(count)
{
	if (fOldSelectionCount > 0 && selected) {
		fOldSelection = new int32[fOldSelectionCount];
		memcpy(fOldSelection, selected, fOldSelectionCount * sizeof(int32));
	}
}

// destructor
AddPointAction::~AddPointAction()
{
	delete[] fOldSelection;
}

// Perform
status_t
AddPointAction::Perform(CanvasView* view)
{
	// path point is already added,
	// but we don't know the parameters yet
	if (fPath && !fPath->GetPointsAt(fIndex, fPoint, fPointIn, fPointOut)) {
		fPath = NULL;
	}
	return fPath ? B_OK : B_NO_INIT;
}

// Undo
status_t
AddPointAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current bounds
		BRect r;
		BRect cr;
		_GetPathBounds(r, cr);
		// do the necessary
		if (fPath->RemovePoint(fIndex)) {
			// redraw path
			_RedrawPath(r, cr);
			// restore selection
			_Select(fOldSelection, fOldSelectionCount);
		} else {
printf("cannot remove index: %ld\n", fIndex);
			status = B_ERROR;
		}
	}
	return status;
}

// Redo
status_t
AddPointAction::Redo(CanvasView* view)
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
			// redraw path
			_RedrawPath(r, cr);
			// select added point
			_Select(&fIndex, 1);
		} else {
			status = B_ERROR;
		}
	}
	return status;
}

// GetName
void
AddPointAction::GetName(BString& name)
{
		name << _GetString(ADD_CONTROL_POINT, "Add Control Point");
}
