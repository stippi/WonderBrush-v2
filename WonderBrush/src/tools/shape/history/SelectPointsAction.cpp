// SelectPointsAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "VectorPath.h"

#include "SelectPointsAction.h"

// constructor
SelectPointsAction::SelectPointsAction(ShapeState* state,
							   ShapeStroke* modifier,
							   const int32* oldSelection,
							   int32 oldCount,
							   const int32* newSelection,
							   int32 newCount)
	: PathAction(state, modifier),
	  fOldSelection(NULL),
	  fOldSelectionCount(oldCount),
	  fNewSelection(NULL),
	  fNewSelectionCount(newCount)
{
	if (fOldSelectionCount > 0 && oldSelection) {
		fOldSelection = new int32[fOldSelectionCount];
		memcpy(fOldSelection, oldSelection, fOldSelectionCount * sizeof(int32));
	}
	if (fNewSelectionCount > 0 && newSelection) {
		fNewSelection = new int32[fNewSelectionCount];
		memcpy(fNewSelection, newSelection, fNewSelectionCount * sizeof(int32));
	}
}

// destructor
SelectPointsAction::~SelectPointsAction()
{
	delete[] fOldSelection;
	delete[] fNewSelection;
}

// Perform
status_t
SelectPointsAction::Perform(CanvasView* view)
{
	return B_OK;
}

// Undo
status_t
SelectPointsAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// restore selection
		_Select(fOldSelection, fOldSelectionCount);
	}
	return status;
}

// Redo
status_t
SelectPointsAction::Redo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// change selection again
		_Select(fNewSelection, fNewSelectionCount);
	}
	return status;
}

// GetName
void
SelectPointsAction::GetName(BString& name)
{
	name << _GetString(CHANGE_POINT_SELECTION, "Change Selection");
}
