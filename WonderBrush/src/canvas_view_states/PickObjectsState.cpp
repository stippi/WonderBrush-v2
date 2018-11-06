// PickObjectsState.cpp

//#include <math.h>
#include <stdio.h>

#include <Cursor.h>
#include <Message.h>
#include <TextControl.h>
#include <Window.h>

#include "cursors.h"
#include "defines.h"
#include "support.h"

#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"
#include "Stroke.h"

#include "PickObjectsState.h"

// constructor
PickObjectsState::PickObjectsState(CanvasView* parent)
	: CanvasViewState(parent),
	  fHistory(NULL),
	  fShiftDown(false),
	  fTracking(false),
	  fBoundingBox(0.0, 0.0, -1.0, -1.0)
{
}

// destructor
PickObjectsState::~PickObjectsState()
{
}

// Init
void
PickObjectsState::Init(Canvas* canvas, Layer* layer, BPoint lastMousePosition)
{
	if (layer)
		fHistory = layer->GetHistory();

	ModifiersChanged();

	if (HistoryListView* listView = fCanvasView->GetHistoryListView())
		ModifierSelectionChanged(listView->CurrentSelection(0) >= 0);
}

// CleanUp
void
PickObjectsState::CleanUp()
{
	fHistory = NULL;
	ModifierSelectionChanged(false);
}

// MouseDown
void
PickObjectsState::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	CanvasViewState::MouseDown(where, canvasWhere, eraser);

	fTrackingStart = canvasWhere.point;
}

// MouseUp
void
PickObjectsState::MouseUp(BPoint where, Point canvasWhere)
{
	if (fMouseDown) {
		if (!fTracking) {
			fCanvasView->PickObjects(fTrackingStart, fShiftDown);
		} else {
			BRect r(min_c(fTrackingStart.x, canvasWhere.point.x),
					min_c(fTrackingStart.y, canvasWhere.point.y),
					max_c(fTrackingStart.x, canvasWhere.point.x),
					max_c(fTrackingStart.y, canvasWhere.point.y));
			fCanvasView->PickObjects(r, fShiftDown);
			fCanvasView->EndRectTracking();
		}
	}
	fTracking = false;

	CanvasViewState::MouseUp(where, canvasWhere);
}

// MouseMoved
void
PickObjectsState::MouseMoved(BPoint where, Point canvasWhere,
						uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

	if (fMouseDown) {
		if (!fTracking) {
			BPoint p(fTrackingStart);
			fCanvasView->ConvertFromCanvas(p);
			fCanvasView->BeginRectTracking(BRect(p, where), B_TRACK_RECT_CORNER);
			fTracking = true;
		}
	}
}

// Draw
void
PickObjectsState::Draw(BView* into, BRect updateRect)
{
	if (fBoundingBox.IsValid()) {
		into->SetHighColor(kBlack);
		into->SetLowColor(kWhite);
		BRect box = fCanvasView->ConvertFromCanvas(fBoundingBox);
		into->StrokeRect(box, kDotted);
		if (box.Width() >= 2.0 && box.Height() >= 2.0) {
			box.InsetBy(1.0, 1.0);
			into->StrokeRect(box, kDotted);
		}
	}
}

// MessageReceived
bool
PickObjectsState::MessageReceived(BMessage* message)
{
	bool result = true;
	switch (message->what) {
		default:
			result = false;
			break;
	}
	return result;
}

// ModifierSelectionChanged
void
PickObjectsState::ModifierSelectionChanged(bool itemsSelected)
{
	BRect box;
	if (itemsSelected && fHistory) {
		if (HistoryListView* listView = fCanvasView->GetHistoryListView()) {
			for (int32 i = 0; Stroke* object = fHistory->ModifierAt(listView->CurrentSelection(i)); i++) {
				if (object->ToolID() != TOOL_UNDEFINED)
					box = box.IsValid() ? object->Bounds() | box : object->Bounds();
			}
		}
	}
	if (fBoundingBox != box) {
		if (fBoundingBox.IsValid() && box.IsValid())
			_InvalidateCanvasRect(fBoundingBox | box);
		else if (fBoundingBox.IsValid())
			_InvalidateCanvasRect(fBoundingBox);
		else if (box.IsValid())
			_InvalidateCanvasRect(box);
		fBoundingBox = box;
	}
}

// ModifiersChanged
void
PickObjectsState::ModifiersChanged()
{
	fShiftDown = modifiers() & B_SHIFT_KEY;
	UpdateToolCursor();
}

// UpdateToolCursor
void
PickObjectsState::UpdateToolCursor()
{
	BCursor cursor(fShiftDown ? kCopyCursor : B_HAND_CURSOR);
	fCanvasView->SetViewCursor(&cursor, true);
}

