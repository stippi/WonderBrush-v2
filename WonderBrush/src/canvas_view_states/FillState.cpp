// FillState.cpp

#include <stdio.h>

#include <Control.h>
#include <Cursor.h>
#include <Message.h>

#include "cursors.h"

#include "AddModifiersAction.h"
#include "CanvasView.h"
#include "History.h"
#include "Layer.h"
#include "FillStroke.h"

#include "FillState.h"

// constructor
FillState::FillState(CanvasView* parent)
	: CanvasViewState(parent),
	  fLayer(NULL),
	  fOpacity(255),
	  fTolerance(0),
	  fSoftness(0),
	  fContiguous(true)
{
}

// destructor
FillState::~FillState()
{
}

// Init
void
FillState::Init(Canvas* canvas, Layer* layer, BPoint lastMousePosition)
{
	CanvasViewState::Init(canvas, layer, lastMousePosition);
	fLayer = layer;
}

// CleanUp
void
FillState::CleanUp()
{
	CanvasViewState::CleanUp();
	fLayer = NULL;
}

// MouseDown
void
FillState::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	CanvasViewState::MouseDown(where, canvasWhere, eraser);

	if (fLayer) {
		if (History* history = fLayer->GetHistory()) {
			FillStroke* stroke = new FillStroke(canvasWhere.point, fCanvasView->Color(),
												fOpacity, fTolerance, fSoftness);
			if (stroke->Fill(fLayer->Bitmap(), fCanvasView->StrokeBitmap(), fContiguous)) {
				int32 index = history->CountItems();
				Stroke** strokes = new Stroke*[1];
				strokes[0] = stroke;
				fCanvasView->Perform(new AddModifiersAction(fLayer, strokes, 1, index));
			} else
				delete stroke;
		}
	}
}

/*
// MouseMoved
void
FillState::MouseMoved(BPoint where, Point canvasWhere,
						uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

	if (fMouseDown) {
		fCanvasView->_SetToColorAtPixel(canvasWhere.point);
	}
}*/

bool
FillState::MessageReceived(BMessage* message)
{
	if (message->what == MSG_SET_FILL_OPACITY) {
		float value;
		if (message->FindFloat("max value", &value) >= B_OK) {
			fOpacity = (uint8)(value * 255.0);
		}
		return true;
	} else if (message->what == MSG_SET_TOLERANCE) {
		float value;
		if (message->FindFloat("max value", &value) >= B_OK) {
			fTolerance = (uint8)(value * 255.0);
		}
		return true;
	} else if (message->what == MSG_SET_SOFTNESS) {
		float value;
		if (message->FindFloat("max value", &value) >= B_OK) {
			fSoftness = (uint8)(value * 254.0);
		}
		return true;
	} else if (message->what == MSG_SET_FILL_CONTIGUOUS) {
		int32 value;
		if (message->FindInt32("be:value", &value) >= B_OK) {
			fContiguous = (value == B_CONTROL_ON);
		}
	}
	return false;
}

// UpdateToolCursor
void
FillState::UpdateToolCursor()
{
	BCursor cursor(kFillBucketCursor);
	fCanvasView->SetViewCursor(&cursor, true);
}

// SetOpacity
void
FillState::SetOpacity(uint8 opacity)
{
	fOpacity = opacity;
}

// SetTolerance
void
FillState::SetTolerance(uint8 tolerance)
{
	fTolerance = tolerance;
}

// SetSoftness
void
FillState::SetSoftness(uint8 softness)
{
	fSoftness = softness;
}

// SetContiguous
void
FillState::SetContiguous(bool contiguous)
{
	fContiguous = contiguous;
}
