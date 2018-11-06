// ChangeCropRectAction.cpp

#include <stdio.h>

#include "CropState.h"

#include "ChangeCropRectAction.h"

// constructor
ChangeCropRectAction::ChangeCropRectAction(CropState* state, BRect cropRect)
	: fCropState(state),
	  fCropRect(cropRect)
{
}

// destructor
ChangeCropRectAction::~ChangeCropRectAction()
{
}

// InitCheck
status_t
ChangeCropRectAction::InitCheck()
{
	return fCropState ? B_OK : B_NO_INIT;
}

// Perform
status_t
ChangeCropRectAction::Perform(CanvasView* view)
{
	// crop rect is already changed
	return InitCheck();
}

// Undo
status_t
ChangeCropRectAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// swap the rects
		BRect currentRect = fCropState->fCropRect;
		fCropState->_SetRect(fCropRect);
		fCropRect = currentRect;
	}
	return status;
}

// Redo
status_t
ChangeCropRectAction::Redo(CanvasView* view)
{
	return Undo(view);
}

// GetName
void
ChangeCropRectAction::GetName(BString& name)
{

	name << _GetString(MODIFY_CROP_RECT, "Modify Crop Rect");
}

