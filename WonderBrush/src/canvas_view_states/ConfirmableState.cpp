// ConfirmableState.cpp

#include <stdio.h>

#include <Message.h>

#include "Canvas.h"
#include "CanvasView.h"
#include "HistoryManager.h"

#include "ConfirmableState.h"

// constructor
ConfirmableState::ConfirmableState(CanvasView* parent)
	: CanvasViewState(parent),
	  fTemporaryHistory(NULL),
	  fCanvas(NULL)
{
}

// destructor
ConfirmableState::~ConfirmableState()
{
	delete fTemporaryHistory;
}

// Init
void
ConfirmableState::Init(Canvas* canvas,  Layer* layer,  BPoint currentMousePos)
{
	fCanvas = canvas;
	if (fCanvas) {
		// create a new temporary history and attach it to the canvas
		delete fTemporaryHistory;
		fTemporaryHistory = new HistoryManager();
		fCanvas->SetTemporaryHistory(fTemporaryHistory);
		// update Edit menu items in window
		fCanvasView->UpdateHistoryItems();
	}
}

// CleanUp
void
ConfirmableState::CleanUp()
{
	if (fCanvas) {
		// detach temporary history from canvas and delete it
		fCanvas->SetTemporaryHistory(NULL);
		delete fTemporaryHistory;
		fTemporaryHistory = NULL;
		// update Edit menu items in window
		fCanvasView->UpdateHistoryItems();
	}
	fCanvas = NULL;
}

// MessageReceived
bool
ConfirmableState::MessageReceived(BMessage* message)
{
	bool handled = true;
	switch (message->what) {
		case MSG_CONFIRM_TOOL:
			Confirm();
			break;
		case MSG_CANCEL_TOOL:
			Cancel();
			break;
		default:
			handled = false;
	}
	return handled;
}

// HandleKeyDown
bool
ConfirmableState::HandleKeyDown(uint32 key, uint32 modifiers)
{
	bool handled = true;
	switch (key) {
		case B_RETURN:
			Confirm();
			break;
		case B_ESCAPE:
			Cancel();
			break;
		default:
			handled = false;
	}
	return handled;
}

// Perform
//
// calling this function instead of the CanvasView method directly
// should prevent adding actions to the wrong history
bool
ConfirmableState::Perform(Action* action) const
{
	bool success = false;
	if (fCanvasView && fTemporaryHistory) {
		success = fCanvasView->Perform(action);
	}
	return success;
}

