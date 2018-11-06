// ColorpickState.cpp

#include <stdio.h>

#include <Control.h>
#include <Cursor.h>
#include <Message.h>

#include "cursors.h"

#include "CanvasView.h"

#include "ColorpickState.h"

// constructor
ColorpickState::ColorpickState(CanvasView* parent)
	: CanvasViewState(parent),
	  fTipSize(1),
	  fUseAllLayers(true)
{
}

// destructor
ColorpickState::~ColorpickState()
{
}

// MouseDown
void
ColorpickState::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	CanvasViewState::MouseDown(where, canvasWhere, eraser);

	fCanvasView->_SetToColorAtPixel(canvasWhere.point, fUseAllLayers, fTipSize);
}

// MouseMoved
void
ColorpickState::MouseMoved(BPoint where, Point canvasWhere,
						uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

	if (fMouseDown) {
		fCanvasView->_SetToColorAtPixel(canvasWhere.point, fUseAllLayers, fTipSize);
	}
}

// MessageReceived
bool
ColorpickState::MessageReceived(BMessage* message)
{
	if (message->what == MSG_DROPPER_TIP_SIZE) {
		uint32 size;
		if (message->FindInt32("size", (int32*)&size) >= B_OK)
			SetTipSize(size);
		return true;
	} else if (message->what == MSG_USE_ALL_LAYERS) {
		int32 value;
		if (message->FindInt32("be:value", &value) >= B_OK)
			SetUsesAllLayers(value == B_CONTROL_ON);
		return true;
	}
	return false;
}

// UpdateToolCursor
void
ColorpickState::UpdateToolCursor()
{
	BCursor cursor(kDropperCursor);
	fCanvasView->SetViewCursor(&cursor, true);
}

// SetTipSize
void
ColorpickState::SetTipSize(uint32 size)
{
	fTipSize = size;
}

// SetUsesAllLayers
void
ColorpickState::SetUsesAllLayers(bool all)
{
	fUseAllLayers = all;
}

