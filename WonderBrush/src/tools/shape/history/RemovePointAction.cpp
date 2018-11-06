// RemovePointAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"
#include "VectorPath.h"

#include "RemovePointAction.h"

// constructor
// * when clicking a point in Remove mode, with other points selected
RemovePointAction::RemovePointAction(ShapeState* state,
									 ShapeStroke* modifier,
									 int32 index,
									 const int32* selected,
									 int32 count)
	: PathAction(state, modifier),
	  fIndex(NULL),
	  fPoint(NULL),
	  fPointIn(NULL),
	  fPointOut(NULL),
	  fConnected(NULL),
	  fCount(0),
	  fOldSelection(NULL),
	  fOldSelectionCount(count)
{
	_Init(&index, 1, selected, count);
}

// constructor
// * when hitting the Delete key, so the selected points are the
// same as the ones to be removed
RemovePointAction::RemovePointAction(ShapeState* state,
									 ShapeStroke* modifier,
									 const int32* selected,
									 int32 count)
	: PathAction(state, modifier),
	  fIndex(NULL),
	  fPoint(NULL),
	  fPointIn(NULL),
	  fPointOut(NULL),
	  fConnected(NULL),
	  fCount(0),
	  fOldSelection(NULL),
	  fOldSelectionCount(count)
{
	_Init(selected, count, selected, count);
}

// destructor
RemovePointAction::~RemovePointAction()
{
	delete[] fIndex;
	delete[] fPoint;
	delete[] fPointIn;
	delete[] fPointOut;
	delete[] fConnected;
	delete[] fOldSelection;
}

// Perform
status_t
RemovePointAction::Perform(CanvasView* view)
{
	// path points are already removed
	return B_OK;
}

// Undo
status_t
RemovePointAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current bounds
		BRect r;
		BRect cr;
		_GetPathBounds(r, cr);
		// do the necessary
		for (int32 i = 0; i < fCount; i++) {
			if (fPath->AddPoint(fPoint[i], fIndex[i])) {
				fPath->SetPoint(fIndex[i],
								fPoint[i],
								fPointIn[i],
								fPointOut[i],
								fConnected[i]);
			} else {
				status = B_ERROR;
			}
		}
		fShapeModifier->Notify();

		fShapeState->SetClosed(fWasClosed);

		if (status >= B_OK) {
			// select the added points
			_Select(fIndex, fCount);
			// redraw path
			_RedrawPath(r, cr);
		} else {
			status = B_ERROR;
		}
	}
	return status;
}

// Redo
status_t
RemovePointAction::Redo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current bounds
		BRect r;
		BRect cr;
		_GetPathBounds(r, cr);
		// do the necessary
		// the loop assumes the indices in the collection
		// are increasing (removal at "index[i] - i" to account
		// for items already removed)
		for (int32 i = 0; i < fCount; i++) {
			if (!fPath->RemovePoint(fIndex[i] - i)) {
				status = B_ERROR;
				break;
			}
		}
		fShapeModifier->Notify();

		fShapeState->SetClosed(fWasClosed && fPath->CountPoints() > 1);

		if (status >= B_OK) {
			// redraw path
			_RedrawPath(r, cr);
			// restore selection
			_Select(fOldSelection, fOldSelectionCount);
		}
	}
	return status;
}

// GetName
void
RemovePointAction::GetName(BString& name)
{
	if (fCount > 1)
		name << _GetString(REMOVE_CONTROL_POINTS, "Remove Control Points");
	else
		name << _GetString(REMOVE_CONTROL_POINT, "Remove Control Point");
}

// _Init
void
RemovePointAction::_Init(const int32* indices, int32 count,
						 const int32* selection, int32 selectionCount)
{
	if (fPath) {
		if (indices && count > 0) {
			fIndex = new int32[count];
			fPoint = new BPoint[count];
			fPointIn = new BPoint[count];
			fPointOut = new BPoint[count];
			fConnected = new bool[count];
			fCount = count;
			memcpy(fIndex, indices, count * sizeof(int32));
			for (int32 i = 0; i < count; i++) {
				if (!fPath->GetPointsAt(fIndex[i],
										fPoint[i],
										fPointIn[i],
										fPointOut[i],
										&fConnected[i])) {
					fPath = NULL;
					break;
				}
			}
			fWasClosed = fPath->IsClosed();
		} else {
			fPath = NULL;
		}
	}

	if (selectionCount > 0 && selection) {
		fOldSelectionCount = selectionCount;
		fOldSelection = new int32[fOldSelectionCount];
		memcpy(fOldSelection, selection, fOldSelectionCount * sizeof(int32));
	}
}
