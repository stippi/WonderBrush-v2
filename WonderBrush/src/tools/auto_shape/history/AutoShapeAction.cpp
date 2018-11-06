// AutoShapeAction.cpp

#include <stdio.h>

#include "AutoShapeState.h"
#include "ShapeStroke.h"

#include "AutoShapeAction.h"

// constructor
AutoShapeAction::AutoShapeAction(AutoShapeState* state,
								 ShapeStroke* object)
	: fState(state),
	  fShape(object)
{
}

// destructor
AutoShapeAction::~AutoShapeAction()
{
}

// InitCheck
status_t
AutoShapeAction::InitCheck()
{
	return fState && fShape ? B_OK : B_NO_INIT;
}

// GetName
void
AutoShapeAction::GetName(BString& name)
{

	name << "<modify auto shape>";
}

// _GetPathBounds
void
AutoShapeAction::_GetPathBounds(BRect& pathBounds,
							  BRect& controlPointBounds) const
{
	if (fState && fShape) {
		pathBounds = fShape->Bounds();
		controlPointBounds = fState->_ControlPointRect();
	}
}


// _RedrawPath
void
AutoShapeAction::_RedrawPath(BRect pathBounds,
						   BRect controlPointBounds) const
{
	if (fState && fShape) {
		fState->_RedrawStroke(fShape, pathBounds, controlPointBounds,
							  false, true, true);
	}
}
