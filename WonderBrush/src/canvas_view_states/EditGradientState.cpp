// EditGradientState.cpp

#include <stdio.h>

#include <Control.h>
#include <Cursor.h>
#include <Message.h>

#include "bitmap_support.h"
#include "cursors.h"
//#include "defines.h"
#include "support.h"

#include "Canvas.h"
#include "CanvasView.h"
#include "Gradient.h"
#include "GradientBox.h"
#include "GradientControl.h"
#include "History.h"
#include "HistoryManager.h"
#include "HistoryListView.h"
#include "Layer.h"
#include "MainWindow.h"
#include "Stroke.h"

#include "ChangeGradientAction.h"
#include "MasterGradientAction.h"

#include "EditGradientState.h"

//#define TRACE(x) printf x
#define TRACE(x)

// constructor
EditGradientState::EditGradientState(CanvasView* parent)
	: CanvasViewState(parent),
	  fCanvas(NULL),
	  fLayer(NULL),
	  fPrivateHistory(new HistoryManager()),
	  fGradientAction(NULL),
	  fObject(NULL),

	  fGradient(new Gradient()),
	  fControl(NULL),

	  fBox(NULL),
	  fCancelling(false),
	  fItemsSelected(false),
	  fGenerateAction(true)
{
	fGradient->AddObserver(this);
}

// destructor
EditGradientState::~EditGradientState()
{
	delete fBox;
	delete fPrivateHistory;
	delete fGradientAction;
}

// Init
void
EditGradientState::Init(Canvas* canvas, Layer* layer, BPoint lastMousePosition)
{
	CanvasViewState::Init(canvas, layer, lastMousePosition);
	fLayer = layer;
	fCanvas = canvas;

	ModifierSelectionChanged(fItemsSelected);
}

// CleanUp
void
EditGradientState::CleanUp()
{
	_SetModifier(NULL);

	CanvasViewState::CleanUp();
	fLayer = NULL;
	fCanvas = NULL;
}

// MouseDown
void
EditGradientState::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	CanvasViewState::MouseDown(where, canvasWhere, eraser);

	if (fBox) {
		fBox->MouseDown(canvasWhere.point);

		if (!fBox->IsRotating())
			fCanvasView->SetAutoScrolling(true);
	}
}

// MouseUp
void
EditGradientState::MouseUp(BPoint where, Point canvasWhere)
{
	CanvasViewState::MouseUp(where, canvasWhere);

	if (fBox) {
		fCanvasView->Perform(fBox->MouseUp());
		if (fObject)
			fObject->Notify();
	}
}

// MouseMoved
void
EditGradientState::MouseMoved(BPoint where, Point canvasWhere,
						 uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

	fLastCanvasPos = canvasWhere.point;

	if (fCanvasView->IsBusy()) {
		if (fMouseDown)
			fCanvasView->EventDropped();
		return;
	}

	if (fBox)
		fBox->MouseMoved(canvasWhere.point, fCanvasView->ZoomLevel() * fBox->InheritedScale());
}

// ModifierSelectionChanged
void
EditGradientState::ModifierSelectionChanged(bool itemsSelected)
{
//TRACE(("EditGradientState::ModifierSelectionChanged()\n"));
	fItemsSelected = itemsSelected;

	Stroke* object = NULL;
	if (fLayer && fItemsSelected) {
		HistoryListView* listView = fCanvasView->GetHistoryListView();
		History* history = fLayer ? fLayer->GetHistory() : NULL;
		if (listView && history) {
			int32 count = listView->CountSelectedItems();
			if (count > 0) {
				object = history->ModifierAt(listView->CurrentSelection(0));
			}
		}
	}
	_SetModifier(object);
}

// Draw
void
EditGradientState::Draw(BView* into, BRect updateRect)
{
	if (fBox)
		fBox->Draw(into);
}

// MessageReceived
bool
EditGradientState::MessageReceived(BMessage* message)
{
	bool result = true;
	switch (message->what) {

		case MSG_SET_GRADIENT_TYPE: {
			gradient_type type;
			if (message->FindInt32("type", (int32*)&type) >= B_OK) {
				fGradient->SetType(type);
			}
			break;
		}
		case MSG_SET_GRADIENT_INTERPOLATION: {
			interpolation_type type;
			if (message->FindInt32("type", (int32*)&type) >= B_OK) {
				fGradient->SetInterpolation(type);
			}
			break;
		}
		case MSG_SET_GRADIENT_INHERITS_TRANSFORM: {
			int32 value;
			if (message->FindInt32("be:value", &value) >= B_OK) {
				fGradient->SetInheritTransformation(value == B_CONTROL_ON);
			}
			break;
		}

		case MSG_SET_COLOR: {
			rgb_color color;
			if (restore_color_from_message(message, color) >= B_OK
				&& fControl->IsFocus()) {
				fControl->SetCurrentStop(color);
//				_UpdateControls();
			}
			// let this message get to the CanvasView
			result = false;
			break;
		}
		case MSG_SET_GRADIENT_STOP_OPACITY:
			if (fControl->IsFocus()) {
				float value;
				rgb_color color;
				if (fControl->GetCurrentStop(&color) &&
					message->FindFloat("max value", &value) >= B_OK) {
					uint8 alpha = (uint8)floorf(255.0 * value + 0.5);
					if (color.alpha != alpha) {
						color.alpha = alpha;
						fControl->SetCurrentStop(color);
					}
				}
			}
			break;

		case MSG_CONFIRM_TOOL:
			_Perform();
			break;
		case MSG_CANCEL_TOOL:
			_Cancel();
			break;

		default:
			result = false;
			break;
	}
	return result;
}

// ModifiersChanged
void
EditGradientState::ModifiersChanged()
{
	if (fBox)
		fBox->ModifiersChanged(modifiers());
}

// HandleKeyDown
bool
EditGradientState::HandleKeyDown(uint32 key, uint32 modifiers)
{
	bool handled = true;
	switch (key) {
		// confirm/cancel
		case B_RETURN:
			_Perform();
			break;
		case B_ESCAPE:
			_Cancel();
			break;
		default:
			handled = false;
			break;
	}
	return handled;
}

// UpdateToolCursor
void
EditGradientState::UpdateToolCursor()
{
	if (fBox) {
		fBox->UpdateToolCursor();
	} else {
		BCursor cursor(kStopCursor);
		fCanvasView->SetViewCursor(&cursor, true);
	}
}

// RebuildLayer
bool
EditGradientState::RebuildLayer(Layer* layer, BRect area) const
{
/*	if (fLayer && fObject && layer == fLayer) {
		_RedrawStroke(fObject, area, area, false, true, true);
		return true;
	}*/
	return false;
}

// PrepareForObjectPropertyChange
void
EditGradientState::PrepareForObjectPropertyChange()
{
	if (fBox) {
		fGenerateAction = false;
		fCanvasView->Perform(fBox->Perform());
		_Perform();
	}
}

// ObjectPropertyChangeDone
void
EditGradientState::ObjectPropertyChangeDone()
{
	fGenerateAction = true;
}

// ObjectChanged
void
EditGradientState::ObjectChanged(const Observable* object)
{
// This class works entirely by this function. The assumtion is
// that an objects gradient is only edited by this class or by
// a MasterGradientAction, which are added by this object to the
// real canvas history (not the private history of this object).
// So the MasterGradientAction knows this object, and takes care
// that it doesn't think a gradient change was caused by itself.
// Because any change would otherwise causing this object to
// enter Private History mode and it generates an action for the
// change.

	const Gradient* gradient = dynamic_cast<const Gradient*>(object);
	if (gradient) {
		if (gradient != fGradient) {
TRACE(("control gradient changed\n"));
			fGradient->SuspendNotifications(true);

			// NOTE: *fGradient = *gradient would copy the transformation,
			// type and interpolation - all of which we don't need
			fGradient->SetColors(*gradient);

			fGradient->SuspendNotifications(false);
		} else {
TRACE(("internal gradient changed\n"));
			if (fObject) {
				Gradient* objectGradient = fObject->GetGradient();
				if (objectGradient) {
					// don't do anything if the change comes from the outside
					if (fGenerateAction) {
						if (*objectGradient != *fGradient) {
TRACE(("  object gradient and internal differ\n"));
	
							_InitPrivateHistory();
	
							fCanvasView->Perform(new ChangeGradientAction(this, fObject));
							// remember transform box bounds
							BRect boxRect = fBox ? fBox->Bounds() : BRect(0.0, 0.0, -1.0, -1.0);
							// update object
							objectGradient->SetColors(*fGradient);
							objectGradient->SetType(fGradient->Type());
							objectGradient->SetInterpolation(fGradient->Interpolation());
							objectGradient->SetInheritTransformation(fGradient->InheritTransformation());
							// rerender object
							_RebuildLayer(fObject->Bounds());
							// invalidate the transform box bounds
							if (fBox)
								_InvalidateCanvasRect(boxRect | fBox->Bounds());
							// update object listview item
							if (HistoryListView* listView = fCanvasView->GetHistoryListView()) {
								int32 index = fLayer->GetHistory()->IndexOf(fObject);
								listView->SetPainter(index, fObject->Painter());
							}
							// trigger notifications for other objects
							fObject->RemoveObserver(this);
							fObject->Notify();
							fObject->AddObserver(this);
						}
					} else {
TRACE(("  ignore property change (updating box)\n"));
						_SetBox(new GradientBox(fCanvasView, this, fObject));
					}
				} else {
TRACE(("  object has no gradient\n"));
					fObject->SetGradient(fGradient);
				}
			}
			// the interpolation having changed is the only reason to update the control
			if (fControl && fControl->GetGradient() &&
				fControl->GetGradient()->Interpolation() != fGradient->Interpolation()) {
				fControl->SetGradient(fGradient);
			}
		}
	}
	const Stroke* o = dynamic_cast<const Stroke*>(object);
	if (o && o == fObject && fControl) {
TRACE(("object changed\n"));
		// update control (which will update internal gradient as well)
		if (Gradient* objectGradient = fObject->GetGradient()) {

			if (!fBox) {
TRACE(("  no box\n"));
				_SetModifier(fObject);
			}

			if (*objectGradient != *fGradient) {
TRACE(("  re-rendering\n"));
				// rerender object
				_RebuildLayer(fObject->Bounds(), true);
				// update object listview item
				if (HistoryListView* listView = fCanvasView->GetHistoryListView()) {
					int32 index = fLayer->GetHistory()->IndexOf(fObject);
					listView->SetPainter(index, fObject->Painter());
				}
			}

			_UpdateControls();
		} else {
TRACE(("  no more gradient\n"));
			ModifierSelectionChanged(fItemsSelected);
		}
	}
/*	const ShapeStroke* shapeObject = dynamic_cast<const ShapeStroke*>(object);
	if (shapeObject && shapeObject == fObject) {

		// update all controls

		// opacity
		if (fObject->Alpha() != fAlpha) {

			fAlpha = fObject->Alpha();

			if (fCanvasView->Window()) {
				BMessage opacity(MSG_SET_ELLIPSE_OPACITY);
				opacity.AddFloat("value", fAlpha / 255.0);
				fCanvasView->Window()->PostMessage(&opacity);
			}
		}
	}*/
}

// SetGradientControl
void
EditGradientState::SetGradientControl(GradientControl* control)
{
	if (control) {
		fControl = control;
		Gradient* gradient = control->GetGradient();
		*fGradient = *gradient;
		gradient->AddObserver(this);
	}
}

// SetGradient
void
EditGradientState::SetGradient(const Gradient* gradient)
{
	if (gradient) {
TRACE(("EditGradientState::SetGradient()\n"));
		// update object
		if (fObject) {
			// suspend notifications
			fGradient->SuspendNotifications(true);
			fObject->SetGradient(gradient);
			// re-enable notification
			fGradient->SuspendNotifications(false);
		} else {
			if (fControl)
				fControl->SetGradient(gradient);
		}
TRACE(("EditGradientState::SetGradient() - done\n\n"));
	}
}

// _Perform
void
EditGradientState::_Perform()
{
	if (!fCancelling) {
		if (fObject) {
			fObject->RemoveObserver(this);
			// this is not ours anymore
			fObject = NULL;
		}
		// remove private history from canvas
		// this needs to be done before any action is performed, since
		// we don't want to add these actions to the private history
		if (fCanvas) {
			fCanvas->SetTemporaryHistory(NULL);
			fCanvasView->UpdateHistoryItems();
		}
		// history needs to be empty
		fPrivateHistory->Clear();

		// perform master gradient action
		fCanvasView->Perform(fGradientAction);
		fGradientAction = NULL;

if (HistoryListView* listView = fCanvasView->GetHistoryListView())
listView->SetAllowColorDrops(true);

		_SetConfirmationEnabled(false);

		_SetBox(NULL);
		UpdateToolCursor();
		// reset to currently selected object
		ModifierSelectionChanged(fItemsSelected);
	}
}

// _Cancel
void
EditGradientState::_Cancel()
{
	fCancelling = true;
	if (fObject) {
		fObject->RemoveObserver(this);
		fObject = NULL;
	}

	_SetBox(NULL);
	if (fCanvas) {
		fCanvas->SetTemporaryHistory(NULL);
		fCanvasView->UpdateHistoryItems();
	}
	// undo any changes
	if (fGradientAction) {
		fGradientAction->Undo(fCanvasView);

		delete fGradientAction;
		fGradientAction = NULL;
	}

	fPrivateHistory->Clear();
	fCanvasView->_UpdateNavigatorView();

	if (HistoryListView* listView = fCanvasView->GetHistoryListView())
		listView->SetAllowColorDrops(true);

	_SetConfirmationEnabled(false);

	UpdateToolCursor();

	fCancelling = false;

	// reset to currently selected object
	ModifierSelectionChanged(fItemsSelected);
}

// _SetConfirmationEnabled
void
EditGradientState::_SetConfirmationEnabled(bool enable) const
{
	if (MainWindow* window = dynamic_cast<MainWindow*>(fCanvasView->Window()))
		window->SetConfirmationEnabled(enable);
}

// _InitPrivateHistory
void
EditGradientState::_InitPrivateHistory()
{
	if (!fGradientAction) {
TRACE(("creating master action - fCanvas: %p\n", fCanvas));
		fGradientAction = new MasterGradientAction(fLayer, fObject);
		// init private undo stack
		fCanvas->SetTemporaryHistory(fPrivateHistory);
		fCanvasView->UpdateHistoryItems();

		if (HistoryListView* listView = fCanvasView->GetHistoryListView())
			listView->SetAllowColorDrops(false);

		_SetConfirmationEnabled(true);
	}
}

// _UpdateControls
void
EditGradientState::_UpdateControls()
{
	if (fObject) {
		if (Gradient* objectGradient = fObject->GetGradient()) {
			// update configuration GUI
			fControl->GetGradient()->SuspendNotifications(true);
			fControl->SetGradient(objectGradient);
			fControl->GetGradient()->SuspendNotifications(false);

			fGradient->SuspendNotifications(true);
			*fGradient = *objectGradient;
			fGradient->SuspendNotifications(false);
	
			if (BWindow* window = fCanvasView->Window()) {
				BMessage type(MSG_SET_GRADIENT_TYPE);
				type.AddInt32("type", objectGradient->Type());
				window->PostMessage(&type);
	
				BMessage interpolation(MSG_SET_GRADIENT_INTERPOLATION);
				interpolation.AddInt32("type", objectGradient->Interpolation());
				window->PostMessage(&interpolation);

				BMessage inheritsTransform(MSG_SET_GRADIENT_INHERITS_TRANSFORM);
				inheritsTransform.AddBool("inherits", objectGradient->InheritTransformation());
				window->PostMessage(&inheritsTransform);
			}
		}
	}
}

// _SetModifier
void
EditGradientState::_SetModifier(Stroke* object)
{
	if (fObject)
		_Perform();

	fObject = object;
	if (fObject) {
		fObject->AddObserver(this);

		// take on object gradient if it has one
		if (Gradient* objGradient = fObject->GetGradient()) {
			_SetBox(new GradientBox(fCanvasView, this, fObject));
			*fGradient = *objGradient;
		}

		// update controls to object state
		_UpdateControls();

	} else
		_SetBox(NULL);
}

// _SetBox
void
EditGradientState::_SetBox(GradientBox* box)
{
	if (fBox != box) {
		if (fBox) {
			// get rid of transform box display
			_InvalidateCanvasRect(fBox->Bounds(), false);
		}
		delete fBox;
		fBox = box;

		if (fBox) {
			fBox->MouseMoved(fLastCanvasPos, fCanvasView->ZoomLevel());

			_InvalidateCanvasRect(fBox->Bounds(), false);
		}
	}
}

// _RebuildLayer
void
EditGradientState::_RebuildLayer(BRect objectBounds, bool ignorBusy) const
{
	if (fLayer) {
		if (!fCanvasView->IsBusy() || ignorBusy) {

			fCanvasView->_SetBusy(true);

			fLayer->Touch(objectBounds);

			fCanvasView->RebuildBitmap(objectBounds, fLayer);
			_InvalidateCanvasRect(objectBounds, false);
		} else {
			fCanvasView->EventDropped();
		}
	}
}

// _ControlPointRect
BRect
EditGradientState::_ControlPointRect() const
{
	BRect r;
	if (fBox) {
		r = fBox->Bounds();
		
	}
	return r;
}


