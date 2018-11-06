// ChangeTransformationAction.cpp

#include <stdio.h>

#include "TransformState.h"

#include "ChangeTransformationAction.h"

// constructor
ChangeTransformationAction::ChangeTransformationAction(TransformState* state,

//													   BPoint translation,
//													   double rotation,
//													   double xScale,
//													   double yScale,
													   const Transformable& t,
													   BPoint centerOffset,

													   const char* actionName,
													   int32 actionNameIndex)
//	: TransformAction(translation,
//					  rotation,
//					  xScale,
//					  yScale,
	: TransformAction(t,
					  centerOffset,
					  actionName,
					  actionNameIndex),
	  fTransformState(state)
{
}

// destructor
ChangeTransformationAction::~ChangeTransformationAction()
{
}

// InitCheck
status_t
ChangeTransformationAction::InitCheck()
{
	if (fTransformState)
		return TransformAction::InitCheck();
	else
		return B_NO_INIT;
}

// _SetTransformation
status_t
ChangeTransformationAction::_SetTransformation(CanvasView* view,
//											   BPoint translation,
//											   double rotation,
//											   double xScale,
//											   double yScale,
											   const Transformable& t,
											   BPoint centerOffset) const
{
//	fTransformState->ChangeTransformation(translation,
//										  rotation,
//										  xScale,
//										  yScale,
	fTransformState->ChangeTransformation(t,
										  centerOffset);

	return B_OK;
}

