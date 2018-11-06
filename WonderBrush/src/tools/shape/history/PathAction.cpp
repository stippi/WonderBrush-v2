// PathAction.cpp

#include <stdio.h>

#include "ShapeState.h"
#include "ShapeStroke.h"
#include "VectorPath.h"

#include "PathAction.h"

// constructor
PathAction::PathAction(ShapeState* shapeState, ShapeStroke* modifier)
	: fShapeState(shapeState),
	  fShapeModifier(modifier),
	  fPath(modifier ? modifier->Path() : NULL)
{
}

// destructor
PathAction::~PathAction()
{
}

// InitCheck
status_t
PathAction::InitCheck()
{
	return fPath && fShapeState ? B_OK : B_NO_INIT;
}

// GetName
void
PathAction::GetName(BString& name)
{

	name << ("<modify path>");
}

// _GetPathBounds
void
PathAction::_GetPathBounds(BRect& pathBounds,
						   BRect& controlPointBounds) const
{
	if (fShapeState && fPath) {
		pathBounds = fShapeModifier->Bounds();
		controlPointBounds = fShapeState->_ControlPointRect();
	}
}


// _RedrawPath
void
PathAction::_RedrawPath(BRect pathBounds,
						BRect controlPointBounds) const
{
	if (fShapeState) {
		fShapeState->_RedrawStroke(pathBounds, controlPointBounds, true, true);
	}
}

// _Select
void
PathAction::_Select(const int32* indices, int32 count,
					bool extend) const
{
	fShapeState->_Select(indices, count, extend);
}
