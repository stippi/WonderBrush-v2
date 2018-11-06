// GradientAction.cpp

#include <stdio.h>

#include "EditGradientState.h"
#include "Stroke.h"

#include "GradientAction.h"

// constructor
GradientAction::GradientAction(EditGradientState* state, Stroke* object)
	: fState(state),
	  fObject(object)
{
}

// destructor
GradientAction::~GradientAction()
{
}

// InitCheck
status_t
GradientAction::InitCheck()
{
	return fObject && fState ? B_OK : B_NO_INIT;
}

// GetName
void
GradientAction::GetName(BString& name)
{

	name << ("<modify gradient>");
}

// _GetObjectBounds
void
GradientAction::_GetObjectBounds(BRect& renderbounds,
								 BRect& invalidationBounds) const
{
	if (fState && fObject) {
		renderbounds = fObject->Bounds();

//		invalidationBounds = fState->_InvalidationRect();
	}
}


// _RedrawObject
void
GradientAction::_RedrawObject(BRect renderbounds,
							  BRect invalidationBounds) const
{
	if (fState) {
//		fState->_RedrawStroke(renderbounds/*, invalidationBounds*/, true, true);
	}
}
