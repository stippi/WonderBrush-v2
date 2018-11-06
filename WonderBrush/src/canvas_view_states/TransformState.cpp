// TransformState.cpp

#include <math.h>
#include <stdio.h>

#include <AppDefs.h>
#include <Cursor.h>
#include <Window.h>

#include "bitmap_support.h"
#include "cursors.h"

#include "Canvas.h"
#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "HistoryManager.h"
#include "Layer.h"
#include "MainWindow.h"
#include "ObjectSelection.h"
#include "Stroke.h"

#include "TransformObjectsAction.h"
#include "ChangeTransformationAction.h"

#include "TransformState.h"

/*
fTransformAction ist NULL solange keine Transformation vorgenommen wurde.
Wenn sich die Selektion ändert, wird eine vorherige Transformation abgeschlossen
und eine fTransformAction ist erstmal wieder NULL. Die Confirmation Knöpfe sind
entsprechend fTransformAction == NULL an oder aus. Die Private History zeichnet
die Änderungen an fTransformAction auf.
*/

// constructor
TransformState::TransformState(CanvasView* parent)
	: CanvasViewState(parent),
	  fLayer(NULL),
	  fPrivateHistory(new HistoryManager()),
	  fCanvas(NULL),
	  fItemsSelected(false),
	  fCanceling(false),
	  fTransformBox(NULL),
	  fShiftDown(false),
	  fCommandDown(false),
	  fTransformAction(NULL),
	  fNudgeOffset(0.0, 0.0),
	  fLastNudgeTime(system_time()),
	  fNudgeAction(NULL)
{
}

// destructor
TransformState::~TransformState()
{
	delete fPrivateHistory;
	delete fTransformBox;
	delete fTransformAction;
	delete fNudgeAction;
}

// Init
void
TransformState::Init(Canvas* canvas, Layer* layer, BPoint currentMousePos)
{
	CanvasViewState::Init(canvas, layer, currentMousePos);

	_Perform();

	fLayer = layer;
	fCanvas = canvas;

	ModifierSelectionChanged(fItemsSelected);
	ModifiersChanged();
}

// CleanUp
void
TransformState::CleanUp()
{
	CanvasViewState::CleanUp();

	_Perform();
	_SetTransformBox(NULL);

	fLayer = NULL;
	fCanvas = NULL;
}

// MouseDown
void
TransformState::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	CanvasViewState::MouseDown(where, canvasWhere, eraser);

	if (fCommandDown) {
		fCanvasView->PickObjects(canvasWhere.point, fShiftDown);
		return;
	}

	if (fTransformBox) {
		fTransformBox->MouseDown(canvasWhere.point);

		if (!fTransformBox->IsRotating())
			fCanvasView->SetAutoScrolling(true);
	}

	fLastCanvasPos = canvasWhere.point;

	UpdateToolCursor();
}

// MouseUp
void
TransformState::MouseUp(BPoint where, Point canvasWhere)
{
	CanvasViewState::MouseUp(where, canvasWhere);

	if (fTransformBox) {
		if (!fCanvasView->Perform(fTransformBox->MouseUp())) {
			if (fTransformAction && fTransformAction->InitCheck() < B_OK)
				_Perform();
		}
		_RefreshCaches();
	}

	UpdateToolCursor();
}

// MouseMoved
void
TransformState::MouseMoved(BPoint where, Point canvasWhere,
						   uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

	// since the tablet is generating mouse moved messages
	// even if only the pressure changes (and not the actual mouse position)
	// we insert this additional check to prevent too much calculation
	if (fLastCanvasPos != canvasWhere.point) {
		fLastCanvasPos = canvasWhere.point;

		if (!fCommandDown && fTransformBox) {
			if (!fCanvasView->IsBusy()) {
	
				if (!fPrecise) {
					// TODO: support turning off subpixel precision
				}

				fTransformBox->MouseMoved(canvasWhere.point, fCanvasView->ZoomLevel());
				if (fMouseDown)
					_UpdateControls();
			} else {
				fCanvasView->EventDropped();
			}
		}
	}
}

// ModifierSelectionChanged
void
TransformState::ModifierSelectionChanged(bool itemsSelected)
{
	fItemsSelected = itemsSelected;

	if (fTransformAction)
		_RefreshCaches();

	if (fLayer && fItemsSelected) {
		HistoryListView* listView = fCanvasView->GetHistoryListView();
		History* history = fLayer ? fLayer->GetHistory() : NULL;
		if (listView && history) {
			int32 count = listView->CountSelectedItems();
			if (count > 0) {
				BList objects(count);
				for (int32 i = 0; i < count; i++) {
					Stroke* object = history->ModifierAt(listView->CurrentSelection(i));
					if (object && object->Bounds().IsValid() && object->IsTransformable())
						objects.AddItem((void*)object);
				}
				if (objects.CountItems() > 0)
					_SetTransformBox(new ObjectSelection(fCanvasView, this, (Stroke**)objects.Items(), objects.CountItems()));
				else
					_SetTransformBox(NULL);
			}
		}
	} else {
		_SetTransformBox(NULL);
	}
}

// Draw
void
TransformState::Draw(BView* into, BRect updateRect)
{
	if (fTransformBox)
		fTransformBox->Draw(into);
}

// MessageReceived
bool
TransformState::MessageReceived(BMessage* message)
{
	bool handled = true;
	switch (message->what) {
		case MSG_TRANSLATE:
		case MSG_ROTATE:
		case MSG_SCALE:
			if (fTransformBox) {
				BPoint centerOffset = fTransformBox->CenterOffset();
				BPoint translation;
				double rotation;
				double xScale;
				double yScale;
				if (message->FindPoint("translation", &translation) >= B_OK
						&& message->FindDouble("rotation", &rotation) >= B_OK
						&& message->FindDouble("x scale", &xScale) >= B_OK
						&& message->FindDouble("y scale", &yScale) >= B_OK) {

					if (!fPrecise) {
						translation.x = floorf(translation.x + 0.5);
						translation.y = floorf(translation.y + 0.5);
					}
					
					fTransformBox->ConvertFromLocal(&translation,
													rotation,
													xScale,
													yScale);

					AdvancedTransformable t;
					t.SetTransformation(translation, rotation, xScale, yScale);
					
//					TransformAction* action = ChangeTransformation(translation,
//																   rotation,
//																   xScale,
//																   yScale,
					TransformAction* action = ChangeTransformation(t,
																   centerOffset,
																   true);
					if (action) {
						switch (message->what) {
							case MSG_TRANSLATE:
								action->SetName("Move", MOVE);
								break;
							case MSG_ROTATE:
								action->SetName("Rotate", ROTATE);
								break;
							case MSG_SCALE:
								action->SetName("Scale", SCALE);
								break;
						}
						fCanvasView->Perform(action);
					}
					
			}
			break;
		}
		case MSG_CONFIRM_TOOL:
			_Perform();
			break;
		case MSG_CANCEL_TOOL:
			_Cancel();
			break;
		default:
			handled = false;
			break;
	}
	return handled;
}

// ModifiersChanged
void
TransformState::ModifiersChanged()
{
	fShiftDown = modifiers() & B_SHIFT_KEY;
	fCommandDown = modifiers() & B_COMMAND_KEY;

	if (!fCommandDown && fTransformBox)
		fTransformBox->ModifiersChanged(modifiers());

	UpdateToolCursor();
}

// HandleKeyDown
bool
TransformState::HandleKeyDown(uint32 key, uint32 modifiers)
{
	bool handled = true;
	float nudgeDist = 1.0;
	if (modifiers & B_SHIFT_KEY)
		nudgeDist /= fCanvasView->ZoomLevel();
	switch (key) {
		// confirm/cancel
		case B_RETURN:
			_Perform();
			break;
		case B_ESCAPE:
			_Cancel();
			break;
		// nudging
		case B_UP_ARROW:
			_Nudge(BPoint(0.0, -nudgeDist));
			break;
		case B_DOWN_ARROW:
			_Nudge(BPoint(0.0, nudgeDist));
			break;
		case B_LEFT_ARROW:
			_Nudge(BPoint(-nudgeDist, 0.0));
			break;
		case B_RIGHT_ARROW:
			_Nudge(BPoint(nudgeDist, 0.0));
			break;
		default:
			handled = false;
			break;
	}
	return handled;
}

// HandleKeyUp
bool
TransformState::HandleKeyUp(uint32 key, uint32 modifiers)
{
	bool handled = true;
	switch (key) {
		// nudging
		case B_UP_ARROW:
		case B_DOWN_ARROW:
		case B_LEFT_ARROW:
		case B_RIGHT_ARROW:
			_FinishNudging();
			break;
		default:
			handled = false;
			break;
	}
	return handled;
}

// UpdateToolCursor
void
TransformState::UpdateToolCursor()
{
	if (!fCommandDown && fTransformBox) {
		fTransformBox->UpdateToolCursor();
	} else {
		const uchar* cursorData = kStopCursor;
		if (fCommandDown) {
			if (fShiftDown)
				cursorData = kCopyCursor;
			else
				cursorData = B_HAND_CURSOR;
		}
		BCursor cursor(cursorData);
		fCanvasView->SetViewCursor(&cursor, true);
	}
}

// PrepareForObjectPropertyChange
void
TransformState::PrepareForObjectPropertyChange()
{
	_Perform();
}

// ChangeTransformation
TransformAction*
//TransformState::ChangeTransformation(BPoint translation, double rotation,
//									 double xScale, double yScale, BPoint centerOffset,
TransformState::ChangeTransformation(const Transformable& t, BPoint centerOffset,
									 bool generateAction, bool notify)
{
//printf("ChangeTransformation(translation(%f, %f), rotation(%f), xScale(%f), "
//	   "yScale(%f), centerOffset(%f, %f))\n",
//	   translation.x, translation.y,
//	   rotation, xScale, yScale,
//	   centerOffset.x, centerOffset.y);

	TransformAction* action = NULL;
	if (fCanvas && fLayer && fTransformBox) {

		if (!fTransformAction) {
			// start a new transformation
			fTransformAction = new TransformObjectsAction(this, fLayer,
														  fTransformBox->Objects(),
														  fTransformBox->CountObjects());

			// init private undo stack
			fCanvas->SetTemporaryHistory(fPrivateHistory);
			fCanvasView->UpdateHistoryItems();

			generateAction = true;

			// we want no change color actions in our private history
			if (HistoryListView* listView = fCanvasView->GetHistoryListView())
				listView->SetAllowColorDrops(false);
		}

		if (fTransformAction) {
			if (generateAction) {
				action = new ChangeTransformationAction(this,
//														fTransformBox->Translation(),
//														fTransformBox->LocalRotation(),
//														fTransformBox->LocalXScale(),
//														fTransformBox->LocalYScale(),
														*fTransformBox,
														fTransformBox->CenterOffset(),
														NULL,
														0);
	
	
			}

//			fTransformAction->SetTransformation(translation, rotation, xScale, yScale);
			fTransformAction->SetTransformable(t);
				// TODO: doesn't update affine params!
			fTransformAction->SetCenterOffset(centerOffset);
	
			// update transform box at this point
			// NOTE: it is only necessary if the change in transformation
			// comes from an action or because of numerical input from the user
			if (!fMouseDown) {
//				fTransformBox->SetTransformation(translation, rotation, xScale, yScale);
//				fTransformBox->OffsetOrigin(centerOffset - fTransformBox->CenterOffset());
				fTransformBox->Set(t, centerOffset - fTransformBox->CenterOffset());
				fTransformBox->MouseMoved(fLastCanvasPos, fCanvasView->ZoomLevel());
				UpdateToolCursor();
			}
	
			// update change action with the new transformation
			if (action) {
//				action->SetNewTransformation(translation,
//											 rotation,
//											 xScale,
//											 yScale,
				action->SetNewTransformation(t,
											 centerOffset);
			}
		}
	
		if (notify)
			_UpdateControls();
	}
	return action;
}

// UpdateBox
void
TransformState::UpdateBox()
{
	ModifierSelectionChanged(fItemsSelected);
}

// _Perform
void
TransformState::_Perform()
{
	if (!fCanceling) {
		// remove private history from canvas
		if (fCanvas) {
			fCanvas->SetTemporaryHistory(NULL);
			fCanvasView->UpdateHistoryItems();
		}
		// update with last state of transform box
		if (fTransformBox && fTransformAction) {
			fTransformAction->SetTransformation(fTransformBox->Translation(),
												fTransformBox->LocalRotation(),
												fTransformBox->LocalXScale(),
												fTransformBox->LocalYScale());
		}
		// perform by adding one single transformation action
		fCanvasView->Perform(fTransformAction);
		fTransformAction = NULL;
		if (HistoryListView* listView = fCanvasView->GetHistoryListView())
			listView->SetAllowColorDrops(true);
	
		// history needs to be empty
		fPrivateHistory->Clear();
	
		_SetConfirmationEnabled(false);
	
		UpdateBox();
	}
}

// _Cancel
void
TransformState::_Cancel()
{
	// the implications here are quite complex.
	// because we want to automatically _Perform() when
	// the selection changes, _Perform() is called whenever
	// we update the transform box arround the objects,
	// and the sequence of calls here (fTransformAction->Undo())
	// also triggers this. That's why we bracked _Cancel()
	// in fCanceling = true and fCanceling = false to disable
	// actually _Perform()ing inside _Cancel()
	fCanceling = true;

	// update with last state of transform box and undo
	if (fTransformAction) {
		if (fTransformBox) {
			fTransformAction->SetTransformation(fTransformBox->Translation(),
												fTransformBox->LocalRotation(),
												fTransformBox->LocalXScale(),
												fTransformBox->LocalYScale());
		}
		fTransformAction->Undo(fCanvasView);

		// get rid of transform action
		delete fTransformAction;
		fTransformAction = NULL;
	}

	if (HistoryListView* listView = fCanvasView->GetHistoryListView())
		listView->SetAllowColorDrops(true);
	// clean up
	fPrivateHistory->Clear();

	// remove private history from canvas
	if (fCanvas) {
		fCanvas->SetTemporaryHistory(NULL);
		fCanvasView->UpdateHistoryItems();
	}

	_SetConfirmationEnabled(false);

	UpdateBox();

	fCanceling = false;
}

// _SetConfirmationEnabled
void
TransformState::_SetConfirmationEnabled(bool enable) const
{
	if (MainWindow* window = dynamic_cast<MainWindow*>(fCanvasView->Window()))
		window->SetConfirmationEnabled(enable);
}

// _SetTransformBox
void
TransformState::_SetTransformBox(ObjectSelection* box)
{
	if (fTransformAction)
		_Perform();
	if (fTransformBox != box) {
		if (fTransformBox) {
			// get rid of transform box display
			_InvalidateCanvasRect(fTransformBox->Bounds());
		}
		delete fTransformBox;
		fTransformBox = box;
		if (fTransformBox) {
			fTransformBox->MouseMoved(fLastCanvasPos, fCanvasView->ZoomLevel());
		}
		_UpdateControls();
	}
}

// _UpdateControls
void
TransformState::_UpdateControls() const
{
	if (BWindow* window = fCanvasView->Window()) {
		BMessage message(MSG_TRANSFORMATION_CHANGED);
		if (fTransformBox) {
			message.AddPoint("translation", fTransformBox->LocalTranslation());
			message.AddDouble("rotation", fTransformBox->LocalRotation());
			message.AddDouble("x scale", fTransformBox->LocalXScale());
			message.AddDouble("y scale", fTransformBox->LocalYScale());
			// update the confirmable state
			if (fTransformAction/*!fTransformBox->IsIdentity()*/) {
				_SetConfirmationEnabled(true);
			} else {
				_SetConfirmationEnabled(false);
			}
		} else {
			_SetConfirmationEnabled(false);
		}
		window->PostMessage(&message);
	}
}

// _RebuildLayer
void
TransformState::_RebuildLayer(BRect area)
{
	if (fLayer) {
		fCanvasView->_SetBusy(true);
		fLayer->Touch(area);
		fCanvasView->RebuildBitmap(area, fLayer);
	}
}

// _RefreshCaches
void
TransformState::_RefreshCaches() const
{
	// TODO: this looks not very robust:
	BBitmap* bitmap = fCanvasView->StrokeBitmap();
	if (fTransformAction && bitmap) {
		BRect usedArea;
		for (int32 i = 0; Stroke* object = fTransformAction->ObjectAt(i); i++){
// TODO: make this smarter
// reinsert if statement, make function "if (stroke->CacheRebuildPreferred())"
			if (!object->IsCacheValid()) {
				clear_area(bitmap, object->Bounds());
				object->Draw(bitmap, object->Bounds());
				object->Finish(bitmap);
				// remember the area we used
				if (i == 0)
					usedArea = object->Bounds();
				else
					usedArea = usedArea | object->Bounds();
			}
		}
		// clean up after ourselves
		clear_area(bitmap, usedArea);
	}
}

// _Nudge
void
TransformState::_Nudge(BPoint direction)
{
	bigtime_t now = system_time();
	if (now - fLastNudgeTime > 500000) {
		_FinishNudging();
	}
	if (fTransformBox && !fCanvasView->IsBusy()) {
		fLastNudgeTime = now;
		fNudgeOffset += direction;
		Transformable t(*fTransformBox);
		t.TranslateBy(direction);
		if (!fNudgeAction) {
//			fNudgeAction = ChangeTransformation(fTransformBox->Translation() + direction,
//								 fTransformBox->LocalRotation(),
//								 fTransformBox->LocalXScale(), 
//								 fTransformBox->LocalYScale(),
			fNudgeAction = ChangeTransformation(t,
								 fTransformBox->CenterOffset(), true);
		} else {
//			delete ChangeTransformation(fTransformBox->Translation() + direction,
//								 fTransformBox->LocalRotation(),
//								 fTransformBox->LocalXScale(), 
//								 fTransformBox->LocalYScale(),
			delete ChangeTransformation(t,
								 fTransformBox->CenterOffset(), false);
			fNudgeAction->SetNewTranslation(fNudgeOffset);
		}
	}
}

// _FinishNudging
void
TransformState::_FinishNudging()
{
	fNudgeOffset = BPoint(0.0, 0.0);
	fCanvasView->Perform(fNudgeAction);
	fNudgeAction = NULL;
}


