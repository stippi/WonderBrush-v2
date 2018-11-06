// RemovePathAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"
#include "VectorPath.h"

#include "RemovePathAction.h"

// constructor
RemovePathAction::RemovePathAction(ShapeState* state,
								   ShapeStroke* modifier)
	: PathAction(state, modifier),
	  fRemoved(false),
	  fIndex(fShapeModifier ? fShapeModifier->IndexOf(fPath) : - 1)
{
}

// destructor
RemovePathAction::~RemovePathAction()
{
	if (fRemoved)
		delete fPath;
}

// Perform
status_t
RemovePathAction::Perform(CanvasView* view)
{
	if (InitCheck() >= B_OK) {
		// remember current bounds
		BRect r;
		BRect cr;
		_GetPathBounds(r, cr);
		if (fShapeModifier->RemovePath(fPath)) {
			fRemoved = true;
			// select another path
			int32 index = fIndex;
			if (index == fShapeModifier->CountPaths())
				index--;
			_RedrawPath(r, cr);
			fShapeState->_SetPath(fShapeModifier->PathAt(index));
			return B_OK;
		} else {
			return B_ERROR;
		}
	}
	return B_NO_INIT;
}

// Undo
status_t
RemovePathAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current bounds
		BRect r;
		BRect cr;
		_GetPathBounds(r, cr);
		if (fShapeModifier->AddPath(fPath, fIndex)) {
			fRemoved = false;
			fShapeState->_SetPath(fPath);
			// select all points from the new path
			int32 count = fPath->CountPoints();
			int32 selection[count];
			for (int32 i = 0; i < count; i++)
				selection[i] = i;
			_Select(selection, count);
			_RedrawPath(r, cr);
		} else
			status = B_NO_MEMORY;
	}
	return status;
}

// Redo
status_t
RemovePathAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
RemovePathAction::GetName(BString& name)
{
	name << _GetString(DELETE_PATH, "Remove Path");
}
