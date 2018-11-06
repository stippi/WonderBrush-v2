// NewPathAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"
#include "VectorPath.h"

#include "NewPathAction.h"

// constructor
NewPathAction::NewPathAction(ShapeState* state,
							 ShapeStroke* modifier,
							 BPoint firstPoint,
							 const int32* selected,
							 int32 count)
	: PathAction(state, modifier),
	  fNewPath(new VectorPath()),
	  fAdded(false),
	  fOldSelection(NULL),
	  fOldSelectionCount(count)
{
	if (fOldSelectionCount > 0 && selected) {
		fOldSelection = new int32[fOldSelectionCount];
		memcpy(fOldSelection, selected, fOldSelectionCount * sizeof(int32));
	}
	if (fNewPath && modifier) {
		fNewPath->SetTransformable(*modifier);
		fNewPath->AddPoint(firstPoint);
	}
}

// destructor
NewPathAction::~NewPathAction()
{
	delete[] fOldSelection;
	if (!fAdded)
		delete fNewPath;
}

// InitCheck
status_t
NewPathAction::InitCheck()
{
	return fNewPath && fShapeState ? B_OK : B_NO_INIT;
}

// Perform
status_t
NewPathAction::Perform(CanvasView* view)
{
	status_t ret = InitCheck();
	if (ret >= B_OK) {
		if (fShapeModifier->AddPath(fNewPath)) {
			fAdded = true;
			fShapeState->_SetPath(fNewPath);
		} else {
			delete fNewPath;
			fNewPath = NULL;
			ret = B_NO_MEMORY;
		}
	}
	return ret;
}

// Undo
status_t
NewPathAction::Undo(CanvasView* view)
{
	status_t ret = InitCheck();
	if (ret >= B_OK) {
		if (fShapeModifier->RemovePath(fNewPath)) {
			fAdded = false;
			fShapeState->_SetPath(fPath);
			// restore selection
			_Select(fOldSelection, fOldSelectionCount);
		}
	}
	return ret;
}

// Redo
status_t
NewPathAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
NewPathAction::GetName(BString& name)
{
	name << _GetString(CREATE_NEW_PATH, "Create New Path");
}
