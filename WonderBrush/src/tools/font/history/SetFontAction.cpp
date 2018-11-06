// SetFontAction.cpp

#include <stdio.h>

#include "TextState.h"
#include "TextStroke.h"
#include "TextRenderer.h"

#include "SetFontAction.h"

// constructor
SetFontAction::SetFontAction(TextState* state,
							 TextStroke* modifier)
	: TextAction(state, modifier),
	  fFamily(NULL),
	  fStyle(NULL)
{
	if (fTextModifier) {
		fFamily = strdup(fTextModifier->GetTextRenderer()->Family());
		fStyle = strdup(fTextModifier->GetTextRenderer()->Style());
	}
	if (!fFamily || !fStyle)
		fTextModifier = NULL;
}

// destructor
SetFontAction::~SetFontAction()
{
	if (fFamily)
		free(fFamily);
	if (fStyle)
		free(fStyle);
}

// Perform
status_t
SetFontAction::Perform(CanvasView* view)
{
	// font is already changed
	return InitCheck();
}

// Undo
status_t
SetFontAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remember current font
		char* family = strdup(fTextModifier->GetTextRenderer()->Family());
		char* style = strdup(fTextModifier->GetTextRenderer()->Style());
		// set the font
		fTextState->SetFamilyAndStyle(fFamily, fStyle);
		// swap what we remember
		free(fFamily);
		free(fStyle);
		fFamily = family;
		fStyle = style;
	}
	return status;
}

// Redo
status_t
SetFontAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
SetFontAction::GetName(BString& name)
{
	name << _GetString(CHANGE_FONT, "Change Font");
}

