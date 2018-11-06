// TextAction.cpp

#include <stdio.h>

#include "TextState.h"
#include "TextStroke.h"

#include "TextAction.h"

// constructor
TextAction::TextAction(TextState* state, TextStroke* modifier)
	: fTextState(state),
	  fTextModifier(modifier)
{
}

// destructor
TextAction::~TextAction()
{
}

// InitCheck
status_t
TextAction::InitCheck()
{
	return fTextModifier && fTextState ? B_OK : B_NO_INIT;
}

// GetName
void
TextAction::GetName(BString& name)
{

	name << ("<modify text>");
}

// _GetModifierBounds
void
TextAction::_GetModifierBounds(BRect& renderbounds,
							   BRect& invalidationBounds) const
{
	if (fTextState && fTextModifier) {
		renderbounds = fTextModifier->Bounds();

		invalidationBounds = fTextState->_InvalidationRect();
	}
}


// _RedrawModifier
void
TextAction::_RedrawModifier(BRect renderbounds,
							BRect invalidationBounds) const
{
	if (fTextState) {
		fTextState->_RedrawStroke(renderbounds, invalidationBounds, true, true);
	}
}
