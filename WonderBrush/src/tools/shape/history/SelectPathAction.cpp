// SelectPathAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "VectorPath.h"

#include "SelectPathAction.h"

// constructor
SelectPathAction::SelectPathAction(ShapeState* state,
								   ShapeStroke* modifier,
								   VectorPath* newPath,
								   const int32* selected,
								   int32 count)
	: PathAction(state, modifier),
	  fNewPath(newPath),
	  fOldSelection(NULL),
	  fOldSelectionCount(count)
{
	if (fOldSelectionCount > 0 && selected) {
		fOldSelection = new int32[fOldSelectionCount];
		memcpy(fOldSelection, selected, fOldSelectionCount * sizeof(int32));
	}
}

// destructor
SelectPathAction::~SelectPathAction()
{
	delete[] fOldSelection;
}

// Perform
status_t
SelectPathAction::Perform(CanvasView* view)
{
	if (fPath != fNewPath) {
		fShapeState->_SetPath(fNewPath);
		// select all points on newly selected path
		int32 selectionCount = fNewPath->CountPoints();
		int32 selection[selectionCount];
		for (int32 i = 0; i < selectionCount; i++) {
			selection[i] = i;
		}
		_Select(selection, selectionCount);
		return B_OK;
	}
	return B_NO_INIT;
}

// Undo
status_t
SelectPathAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		fShapeState->_SetPath(fPath);
		// restore selection
		_Select(fOldSelection, fOldSelectionCount);
	}
	return status;
}

// Redo
status_t
SelectPathAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
SelectPathAction::GetName(BString& name)
{
	name << _GetString(SELECT_PATH, "Select Path");
}
