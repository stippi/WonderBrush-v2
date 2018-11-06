// TransformAction.cpp

#include <stdio.h>

#include "TransformAction.h"

// constructor
//TransformAction::TransformAction(BPoint translation,
//								 double rotation,
//								 double xScale,
//								 double yScale,
TransformAction::TransformAction(const Transformable& t,
								 BPoint centerOffset,
								 const char* actionName,
								 uint32 nameIndex)
//	: fOldTranslation(translation),
//	  fOldRotation(rotation),
//	  fOldXScale(xScale),
//	  fOldYScale(yScale),
	: fOldTransform(t),
	  fOldCenterOffset(centerOffset),

//	  fNewTranslation(translation),
//	  fNewRotation(rotation),
//	  fNewXScale(xScale),
//	  fNewYScale(yScale),
	  fNewTransform(t),
	  fNewCenterOffset(centerOffset),

	  fName(actionName),
	  fNameIndex(nameIndex)
{
}

// destructor
TransformAction::~TransformAction()
{
}

// InitCheck
status_t
TransformAction::InitCheck()
{
//	if (fNewTranslation != fOldTranslation
//		|| fNewRotation != fOldRotation
//		|| fNewXScale != fOldXScale
//		|| fNewYScale != fOldYScale
	if (fNewTransform != fOldTransform
		|| fNewCenterOffset != fOldCenterOffset)
		return B_OK;
	else
		return B_NO_INIT;
}

// Perform
status_t
TransformAction::Perform(CanvasView* view)
{
	// transformation is already current
	return InitCheck();
}

// Undo
status_t
TransformAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
//		_SetTransformation(view, fOldTranslation,
//								 fOldRotation,
//								 fOldXScale,
//								 fOldYScale,
		_SetTransformation(view, fOldTransform,
								 fOldCenterOffset);
	}
	return status;
}

// Redo
status_t
TransformAction::Redo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
//		_SetTransformation(view, fNewTranslation,
//								 fNewRotation,
//								 fNewXScale,
//								 fNewYScale,
		_SetTransformation(view, fNewTransform,
								 fNewCenterOffset);
	}
	return status;
}

// GetName
void
TransformAction::GetName(BString& name)
{
	name << _GetString(fNameIndex, fName.String());
}

// SetNewTransformation
void
//TransformAction::SetNewTransformation(BPoint translation,
//									  double rotation,
//									  double xScale,
//									  double yScale,
TransformAction::SetNewTransformation(const Transformable& t,
									  BPoint centerOffset)
{
//	fNewTranslation = translation;
//	fNewRotation = rotation;
//	fNewXScale = xScale;
//	fNewYScale = yScale;
	fNewTransform = t;
	fNewCenterOffset = centerOffset;
}

// SetNewTranslation
void
TransformAction::SetNewTranslation(BPoint translation)
{
//	fNewTranslation = translation;
	fNewTransform.tx = translation.x;
	fNewTransform.ty = translation.y;
}

// SetName
void
TransformAction::SetName(const char* actionName, uint32 nameIndex)
{
	fName.SetTo(actionName);
	fNameIndex = nameIndex;
}

