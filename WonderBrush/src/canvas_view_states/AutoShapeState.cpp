// AutoShapeState.cpp

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
#include "AutoShapeBox.h"
#include "History.h"
#include "HistoryManager.h"
#include "HistoryListView.h"
#include "Layer.h"
#include "MainWindow.h"
#include "ShapeStroke.h"
#include "VectorPath.h"

// actions
#include "AddModifierAction.h"
#include "AutoShapeAlphaAction.h"
#include "AutoShapeOutlineAction.h"
#include "AutoShapeOutlineWidthAction.h"
#include "AutoShapeColorAction.h"

#include "AutoShapeState.h"

// constructor
AutoShapeState::AutoShapeState(CanvasView* parent,
							   uint32 opacityWhat,
							   uint32 outlineWhat,
							   uint32 outlineWidthWhat)
	: CanvasViewState(parent),
	  fCanvas(NULL),
	  fLayer(NULL),
	  fPrivateHistory(new HistoryManager()),
	  fShapeStroke(NULL),
	  fBox(NULL),
	  fAlpha(255),
	  fOutline(false),
	  fOutlineWidth(1.0),
	  fCancelling(false),
	  fOpacityWhat(opacityWhat),
	  fOutlineWhat(outlineWhat),
	  fOutlineWidthWhat(outlineWidthWhat)
{
}

// destructor
AutoShapeState::~AutoShapeState()
{
	delete fPrivateHistory;
}

// Init
void
AutoShapeState::Init(Canvas* canvas, Layer* layer, BPoint lastMousePosition)
{
	CanvasViewState::Init(canvas, layer, lastMousePosition);
	fLayer = layer;
	fCanvas = canvas;
}

// CleanUp
void
AutoShapeState::CleanUp()
{
	_Perform();
	CanvasViewState::CleanUp();
	fLayer = NULL;
	fCanvas = NULL;
}

// MouseDown
void
AutoShapeState::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	CanvasViewState::MouseDown(where, canvasWhere, eraser);

	if (fLayer) {

		if (fOutline && !fPrecise) {
			float offset = fmod(fOutlineWidth, 2.0) / 2.0;
			canvasWhere.point += BPoint(offset, offset);
		}

		if (!fShapeStroke) {
			if (ShapeStroke* shape = _MakeAutoShape(canvasWhere.point)) {
				if (!fCanvasView->AddStroke(shape)) {
					delete shape;
				} else {
					shape->UpdateBounds();
					_SetModifier(shape);
	
					fBox = new AutoShapeBox(fCanvasView,
											this,
											fShapeStroke);
	
					_RedrawStroke(fShapeStroke,
								  fShapeStroke->Bounds(),
								  _ControlPointRect(),
								  false, true, true);

					fBox->ModifiersChanged(modifiers());
					fBox->DragRightBottom();
				}
			}
		}
		if (fBox) {
			fBox->MouseDown(canvasWhere.point);
		}
	}
}

// MouseUp
void
AutoShapeState::MouseUp(BPoint where, Point canvasWhere)
{
	CanvasViewState::MouseUp(where, canvasWhere);

	if (fBox)
		fCanvasView->Perform(fBox->MouseUp());

	if (fShapeStroke)
		fShapeStroke->Notify();
}

// MouseMoved
void
AutoShapeState::MouseMoved(BPoint where, Point canvasWhere,
						 uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

	fLastMousePos = where;

	if (fCanvasView->IsBusy()) {
		if (fMouseDown)
			fCanvasView->EventDropped();
		return;
	}

	if (fOutline && !fPrecise) {
		float offset = fmod(fOutlineWidth, 2.0) / 2.0;
		canvasWhere.point += BPoint(offset, offset);
	}

	if (fBox)
		fBox->MouseMoved(canvasWhere.point, fCanvasView->ZoomLevel());
}

// ModifierSelectionChanged
void
AutoShapeState::ModifierSelectionChanged(bool itemsSelected)
{
	_Perform();
}

// Draw
void
AutoShapeState::Draw(BView* into, BRect updateRect)
{
	if (fBox)
		fBox->Draw(into);
}

// MessageReceived
bool
AutoShapeState::MessageReceived(BMessage* message)
{
	bool result = true;
	bool sliderAction = message->HasBool("begin");
	switch (message->what) {
		case MSG_SET_COLOR: {
			rgb_color color;
			if (restore_color_from_message(message, color) == B_OK) {
				if (!fCanvasView->IsBusy() || sliderAction) {
					SetColor(color, sliderAction, false);
				} else {
					fCanvasView->EventDropped();
				}
			}
			result = false;
			break;
		}
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

	if (!result) {
		result = true;
		if (message->what == fOpacityWhat) {
			float value;
			if (message->FindFloat("max value", &value) >= B_OK) {
				if (!fCanvasView->IsBusy() || sliderAction)
					SetAlpha((uint8)floorf(value * 255.0 + 0.5),
							 sliderAction, false);
				else
					fCanvasView->EventDropped();
			}
		} else if (message->what == fOutlineWhat) {
			int32 outline;
			if (message->FindInt32("be:value", &outline) == B_OK) {
				SetOutline(outline == B_CONTROL_ON, true, false);
			}
		} else if (message->what == fOutlineWidthWhat) {
			float outlineWidth;
			if (message->FindFloat("max value", &outlineWidth) == B_OK) {
				if (!fCanvasView->IsBusy() || sliderAction)
					SetOutlineWidth((outlineWidth * outlineWidth) * 100.0,
									sliderAction, false);
				else
					fCanvasView->EventDropped();
			}
		} else {
			result = false;
		}
	}

	return result;
}

// ModifiersChanged
void
AutoShapeState::ModifiersChanged()
{
	if (fBox)
		fBox->ModifiersChanged(modifiers());
}


// HandleKeyDown
bool
AutoShapeState::HandleKeyDown(uint32 key, uint32 modifiers)
{
	bool result = true;

/*	float nudgeDist = 1.0;
	if (modifiers & B_SHIFT_KEY)
		nudgeDist /= fCanvasView->ZoomLevel();*/

	switch (key) {
		// commit
		case B_RETURN:
			_Perform();
			break;
		// cancel
		case B_ESCAPE:
			_Cancel();
			break;

		default:
			result = false;
			break;
	}
	return result;
}

// UpdateToolCursor
void
AutoShapeState::UpdateToolCursor()
{
	if (fBox) {
		fBox->UpdateToolCursor();
		return;
	}
	_UpdateToolCursor();
}

// RebuildLayer
bool
AutoShapeState::RebuildLayer(Layer* layer, BRect area) const
{
	if (fLayer && fShapeStroke && layer == fLayer) {
		_RedrawStroke(fShapeStroke, area, area, false, true, true);
		return true;
	}
	return false;
}

// PrepareForObjectPropertyChange
void
AutoShapeState::PrepareForObjectPropertyChange()
{
	if (fBox) {
		fCanvasView->Perform(fBox->Perform());
		_Perform();
	}
}

// ObjectChanged
void
AutoShapeState::ObjectChanged(const Observable* object)
{
	const ShapeStroke* shapeObject = dynamic_cast<const ShapeStroke*>(object);
	if (shapeObject && shapeObject == fShapeStroke) {

		// update all controls

		// opacity
		if (fShapeStroke->Alpha() != fAlpha) {

			fAlpha = fShapeStroke->Alpha();

			if (fCanvasView->Window()) {
				BMessage opacity(MSG_SET_ELLIPSE_OPACITY);
				opacity.AddFloat("value", fAlpha / 255.0);
				fCanvasView->Window()->PostMessage(&opacity);
			}
		}

		// outline
		if (fShapeStroke->IsOutline() != fOutline) {

			fOutline = fShapeStroke->IsOutline();

			if (fCanvasView->Window()) {
				BMessage outline(MSG_SET_ELLIPSE_OUTLINE);
				outline.AddBool("outline", fOutline);
				fCanvasView->Window()->PostMessage(&outline);
			}
		}

		// outline width
		if (fShapeStroke->OutlineWidth() != fOutlineWidth) {

			fOutlineWidth = fShapeStroke->OutlineWidth();

			if (fCanvasView->Window()) {
				BMessage outlineWidth(MSG_SET_ELLIPSE_OUTLINE_WIDTH);
				outlineWidth.AddFloat("value", sqrtf(fOutlineWidth / 100.0));
				fCanvasView->Window()->PostMessage(&outlineWidth);
			}
		}

		// color
		if (fColor.red != fShapeStroke->Color().red ||
			fColor.green != fShapeStroke->Color().green ||
			fColor.blue != fShapeStroke->Color().blue) {

			fColor = fShapeStroke->Color();

			if (fCanvasView->Window()) {
				BMessage cm(MSG_SET_COLOR);
				store_color_in_message(&cm, fShapeStroke->Color());
				fCanvasView->Window()->PostMessage(&cm);
			}
			// update list view
			if (fLayer) {
				History* history = fLayer->GetHistory();
				HistoryListView* listView = fCanvasView->GetHistoryListView();
				if (history && listView) {
					int32 index = history->IndexOf(fShapeStroke);
					listView->SetPainter(index, fShapeStroke->Painter());
				}
			}
		}
	}
}

// SetSomething
template <class V, class F>
void
AutoShapeState::SetSomething(V value, V control, F func, bool ignorBusy)
{
	// handle invalidation
	if (fShapeStroke) {
		BRect r(fShapeStroke->Bounds());
		BRect cr(_ControlPointRect());
		(fShapeStroke->*func)(value);
		_RedrawStroke(fShapeStroke, r, cr, false, true, ignorBusy);
	}
}

// SetAlpha
void
AutoShapeState::SetAlpha(uint8 opacity, bool action, bool notify)
{
	if (fAlpha != opacity) {
		if (!fShapeStroke || !notify) {
			fAlpha = opacity;
		}

		if (fShapeStroke) {
			if (action)
				fCanvasView->Perform(new AutoShapeAlphaAction(this, fShapeStroke));

			SetSomething(opacity, fAlpha, &ShapeStroke::SetAlpha, notify);
		}
	}
}

// SetColor
void
AutoShapeState::SetColor(rgb_color color, bool action, bool notify)
{
	if (fColor.red != color.red ||
		fColor.green != color.green ||
		fColor.blue != color.blue) {

		if (!fShapeStroke || !notify) {
			fColor = color;
		}

		if (fShapeStroke) {
			if (action)
				fCanvasView->Perform(new AutoShapeColorAction(this, fShapeStroke));
			// handle invalidation
			BRect r(fShapeStroke->Bounds());
			BRect cr(_ControlPointRect());
			fShapeStroke->SetColor(color);
			_RedrawStroke(fShapeStroke, r, cr, false, true, notify);

			// update list view
			if (fLayer) {
				History* history = fLayer->GetHistory();
				HistoryListView* listView = fCanvasView->GetHistoryListView();
				if (history && listView) {
					int32 index = history->IndexOf(fShapeStroke);
					listView->SetPainter(index, fShapeStroke->Painter());
				}
			}
		}
	}
}

// SetOutline
void
AutoShapeState::SetOutline(bool outline, bool action, bool notify)
{
	if (fOutline != outline) {
		if (!fShapeStroke || !notify) {
			fOutline = outline;
		}

		if (fShapeStroke) {
			if (action)
				fCanvasView->Perform(new AutoShapeOutlineAction(this, fShapeStroke));

			SetSomething(outline, fOutline, &ShapeStroke::SetOutline, notify);
		}
	}
}

// SetOutlineWidth
void
AutoShapeState::SetOutlineWidth(float width, bool action, bool notify)
{
	if (fOutlineWidth != width) {
		if (!fShapeStroke || !notify) {
			fOutlineWidth = width;
		}

		if (fShapeStroke) {
			if (action)
				fCanvasView->Perform(new AutoShapeOutlineWidthAction(this, fShapeStroke));

			SetSomething(width, fOutlineWidth, &ShapeStroke::SetOutlineWidth, notify);
		}
	}
}

// GetPoints
bool
AutoShapeState::GetPoints(control_point* points, int32 count,
						  double xScale, double yScale) const
{
	return false;
}

// _Perform
void
AutoShapeState::_Perform()
{
	if (!fCancelling && fShapeStroke) {
		if (fShapeStroke) {
			fShapeStroke->RemoveObserver(this);
			// remove private history from canvas
			// this needs to be done before any action is performed, since
			// we don't want to add these actions to the private history
			if (fCanvas) {
				fCanvas->SetTemporaryHistory(NULL);
				fCanvasView->UpdateHistoryItems();
			}
			// perform and cause the canvas view to switch buffers
			fShapeStroke->Path()->CleanUp();
			if (!fCanvasView->Perform(new AddModifierAction(fLayer, fShapeStroke))) {
				delete fShapeStroke;
			} else {
				BRect r(fShapeStroke->Bounds());
				clear_area(fCanvasView->StrokeBitmap(), r);
				fCanvasView->SwitchBuffers(r);
				_InvalidateCanvasRect(r);
			}
			// we will stop drawing control points
			_InvalidateCanvasRect(_ControlPointRect(), false);
			// history needs to be empty
			fPrivateHistory->Clear();
			// unfocus the object item (we're now done editing it)
			HistoryListView* listView = fCanvasView->GetHistoryListView();
			if (listView)
				listView->SetItemFocused(-1);
			// this is not ours anymore
			fShapeStroke = NULL;
			delete fBox;
			fBox = NULL;
			_SetConfirmationEnabled(false);
		}
		UpdateToolCursor();
	}
}

// _Cancel
void
AutoShapeState::_Cancel()
{
	fCancelling = true;
	if (fShapeStroke) {
		fShapeStroke->RemoveObserver(this);
		// unfocus the shape modifier item, remove it
		// remove modifier from layers history
		HistoryListView* listView = fCanvasView->GetHistoryListView();
		History* history = fLayer->GetHistory();
		if (history && listView) {
			listView->SetItemFocused(-1);
			// we have added this modifier and need to get rid of it
			delete listView->RemoveItem(history->IndexOf(fShapeStroke));
			history->RemoveItem(fShapeStroke);

			fShapeStroke->UpdateBounds();
			// clear the stroke bitmap area, restore the layer bitmap
			BRect r(fShapeStroke->Bounds());
			clear_area(fCanvasView->StrokeBitmap(), r);
			copy_area(fCanvasView->BackBitmap(),
					  fLayer->Bitmap(), r);
			_InvalidateCanvasRect(r, true);
			_InvalidateCanvasRect(_ControlPointRect(), false);

			delete fShapeStroke;
		}
		fShapeStroke = NULL;
		delete fBox;
		fBox = NULL;
		if (fCanvas) {
			fCanvas->SetTemporaryHistory(NULL);
			fCanvasView->UpdateHistoryItems();
		}
		fPrivateHistory->Clear();
		fCanvasView->_UpdateNavigatorView();
	}
	_SetConfirmationEnabled(false);

	UpdateToolCursor();

	fCancelling = false;
}

// _SetConfirmationEnabled
void
AutoShapeState::_SetConfirmationEnabled(bool enable) const
{
	if (MainWindow* window = dynamic_cast<MainWindow*>(fCanvasView->Window()))
		window->SetConfirmationEnabled(enable);
}

// _SetModifier
void
AutoShapeState::_SetModifier(ShapeStroke* modifier)
{
	if (fShapeStroke)
		_Perform();
	if (modifier) {
		fShapeStroke = modifier;
		fShapeStroke->AddObserver(this);
		// set the shape stroke focused in the history list
		HistoryListView* listView = fCanvasView->GetHistoryListView();
		History* history = fLayer->GetHistory();
		if (history && listView) {
			listView->SetItemFocused(history->IndexOf(fShapeStroke));
		}
		if (fCanvas) {
			fCanvas->SetTemporaryHistory(fPrivateHistory);
			fCanvasView->UpdateHistoryItems();
		}
		// customize the modifier
		fShapeStroke->SuspendNotifications(true);

		fShapeStroke->SetAlpha(fAlpha);
		fShapeStroke->SetOutline(fOutline);
		fShapeStroke->SetOutlineWidth(fOutlineWidth);

		fShapeStroke->SuspendNotifications(false);

		_InvalidateCanvasRect(_ControlPointRect(), false);
		_SetConfirmationEnabled(true);
	}
}

// _SetBox
void
AutoShapeState::_SetBox(AutoShapeBox* box)
{
	if (fBox != box) {
		if (fBox) {
			// get rid of transform box display
			_InvalidateCanvasRect(fBox->Bounds(), false);
		}
		delete fBox;
		fBox = box;

		if (fBox) {
			BPoint canvasPos = fLastMousePos;
			fCanvasView->ConvertToCanvas(canvasPos);

			fBox->MouseMoved(canvasPos, fCanvasView->ZoomLevel());
		}
	}
}

// _RedrawStroke
void
AutoShapeState::_RedrawStroke(ShapeStroke* object,
							  BRect oldStrokeBounds, BRect oldControlPointsBounds,
							  bool rebuild, bool forceStrokeDrawing, bool ignorBusy) const
{
	if (object && fLayer) {
		if (!fCanvasView->IsBusy() || ignorBusy) {

			fCanvasView->_SetBusy(true);

			if (rebuild) {
				object->UpdateBounds();
				
				BRect r(object->Bounds());
				fLayer->Touch(r);

				r = r | oldStrokeBounds;


				fCanvasView->RebuildBitmap(r, fLayer);
				_InvalidateCanvasRect(oldControlPointsBounds | _ControlPointRect(), false);
			} else {
				object->UpdateBounds();
		
				BRect r(object->Bounds());
				fLayer->Touch(r);
	
				clear_area(fCanvasView->StrokeBitmap(), oldStrokeBounds);
				object->Draw(fCanvasView->StrokeBitmap(), r);
	
				r = r | oldStrokeBounds;
	
				if (forceStrokeDrawing)
					_ApplyStroke(fCanvasView->BackBitmap(),
								 fLayer->Bitmap(), r, object);
	
				_InvalidateCanvasRect(r, true);
				_InvalidateCanvasRect(oldControlPointsBounds | _ControlPointRect(), false);
			}
		} else {
			fCanvasView->EventDropped();
		}
	}
}

// _ControlPointRect
BRect
AutoShapeState::_ControlPointRect() const
{
	BRect r;
	if (fBox) {
		r = fBox->Bounds();
		
	}
	return r;
}


