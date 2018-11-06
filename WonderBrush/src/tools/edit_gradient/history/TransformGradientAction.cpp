// TransformGradientAction.cpp

#include <stdio.h>

#include "GradientBox.h"

#include "TransformGradientAction.h"

// constructor
TransformGradientAction::TransformGradientAction(const GradientBox* box,
//												 BPoint translation,
//												 double rotation,
//												 double xScale,
//												 double yScale,
												 const Transformable& t,
												 BPoint centerOffset,
												 const char* actionName,
												 uint32 nameIndex)
//	: TransformAction(translation,
//					  rotation,
//					  xScale,
//					  yScale,
	: TransformAction(t,
					  centerOffset,
					  actionName,
					  nameIndex),
	  fBox((GradientBox*)box)
{
}

// destructor
TransformGradientAction::~TransformGradientAction()
{
}

// InitCheck
status_t
TransformGradientAction::InitCheck()
{
	if (fBox)
		return TransformAction::InitCheck();
	else
		return B_NO_INIT;
}

// _SetTransformation
status_t
TransformGradientAction::_SetTransformation(CanvasView* view,
//											BPoint translation,
//											double rotation,
//											double xScale,
//											double yScale,
											const Transformable& t,
											BPoint centerOffset) const
{
//	fBox->SetTransformation(translation,
//							rotation,
//							xScale,
//							yScale);
	fBox->Set(t, centerOffset);
//	fBox->OffsetOrigin(centerOffset);
	return B_OK;
}

