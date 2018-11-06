// GuidesState.cpp

//#include <math.h>
#include <stdio.h>

#include <Cursor.h>
#include <Message.h>
#include <TextControl.h>
#include <Window.h>

#include "cursors.h"
#include "defines.h"
#include "support.h"

#include "Canvas.h"
#include "CanvasView.h"
#include "Guide.h"
#include "HistoryManager.h"
#include "MainWindow.h"

#include "GuidesState.h"

#define ACTION_TRIGGER_TIMEOUT 500000

static const float kGoldenProportion = (sqrtf(5.0) - 1.0) / 2.0;

// constructor
GuidesState::GuidesState(CanvasView* parent)
	: CanvasViewState(parent),
	  fPrivateHistory(new HistoryManager()),
	  fCanvas(NULL),

	  fGuide(NULL),
	  fGuideUnderMouse(NULL),
	  fCanceling(false),

	  fHPos(0.5),
	  fVPos(0.5),
	  fAngle(0.0),
	  fUnitsMode(GUIDE_UNITS_PERCENT),

	  fLastNudgeTime(system_time()),
	  fLastNudgeAngleTime(fLastNudgeTime)
{
}

// destructor
GuidesState::~GuidesState()
{
	delete fPrivateHistory;
}

// Init
void
GuidesState::Init(Canvas* canvas, Layer* layer, BPoint lastMousePosition)
{
	CanvasViewState::Init(canvas, layer, lastMousePosition);
	fCanvas = canvas;
	if (fCanvas) {
		fShowGuides = fCanvas->ShowGuides();
		fCanvas->SetShowGuides(false);
		if (fCanvasView->Window()) {
			BMessage message(MSG_SHOW_GUIDES);
			message.AddBool("show guides", fShowGuides);
			fCanvasView->Window()->PostMessage(&message);
		}
	}
}

// CleanUp
void
GuidesState::CleanUp()
{
	if (fCanvas) {
		fCanvas->SetShowGuides(fShowGuides);
	}

	_Perform();
	CanvasViewState::CleanUp();
	fCanvas = NULL;
}

// MouseDown
void
GuidesState::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	CanvasViewState::MouseDown(where, canvasWhere, eraser);

/*	if (fLayer) {
		// start new stroke
		if (!fTextStroke) {
			TextStroke* textStroke = new TextStroke(fCanvasView->Color());
			if (!fCanvasView->AddStroke(textStroke)) {
				delete textStroke;
			} else {
				textStroke->TranslateBy(canvasWhere.point);
				_SetModifier(textStroke);
			}
		}
		if (fTextStroke) {
			// move text on layer
			fTrackingStart = canvasWhere.point;
			fLastCanvasPos = canvasWhere.point;
			fStartOffset = BPoint(0.0, 0.0);
			fTextStroke->Transform(&fStartOffset);

			fCanvasView->SetAutoScrolling(true);
		}
	}*/

	if (fCanvas) {
		_SetGuide(fGuideUnderMouse);
		fLastCanvasPos = canvasWhere.point;
		if (fGuide) {
			fCanvasView->SetAutoScrolling(true);
		}
	}

}

// MouseUp
void
GuidesState::MouseUp(BPoint where, Point canvasWhere)
{
	CanvasViewState::MouseUp(where, canvasWhere);
/*	if (fTextStroke) {
		BPoint p(0.0, 0.0);
		fTextStroke->Transform(&p);
		if (fStartOffset != p)
			fCanvasView->Perform(new MoveTextAction(this, fTextStroke, p - fStartOffset));
	}*/
}

// MouseMoved
void
GuidesState::MouseMoved(BPoint where, Point canvasWhere,
						uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

/*	if (fMouseDown && fTextStroke) {
		// since the tablet is generating mouse moved messages
		// even if only the pressure changes (and not the actual mouse position)
		// we insert this additional check to prevent too much calculation
		if (fLastCanvasPos != canvasWhere.point) {
			fLastCanvasPos = canvasWhere.point;
			if (!fCanvasView->IsBusy()) {
				BRect r(fTextStroke->Bounds());
				fTextStroke->TranslateBy(canvasWhere.point - fTrackingStart);
				fTrackingStart = canvasWhere.point;
				_RedrawStroke(r, true);
			} else {
				fCanvasView->EventDropped();
			}
		}
	}*/
	if (fCanvas) {
		if (fMouseDown) {
			if (fGuide) {
				fGuide->MoveBy(canvasWhere.point - fLastCanvasPos,
							   fCanvas->Bounds());
				fLastCanvasPos = canvasWhere.point;
			}
		} else {
			fGuideUnderMouse = NULL;
			BRect canvasFrame = fCanvasView->CanvasFrame();
			BRect viewBounds = fCanvasView->CanvasFrame();
			// search for guide under mouse
			for (int32 i = 0; Guide* guide = fCanvas->GuideAt(i); i++) {
				if (guide->HitTest(where, canvasFrame, viewBounds)) {
					fGuideUnderMouse = guide;
					break;
				}
			}

			// figure out if we're not showing the move cursor anymore
			UpdateToolCursor();
		}
	}
}

// Draw
void
GuidesState::Draw(BView* into, BRect updateRect)
{
	if (fCanvas && fShowGuides) {
		rgb_color color = (rgb_color){ 120, 120, 255, 255 };
		BRect bounds(into->Bounds());
		BRect canvasFrame = fCanvasView->CanvasFrame();
		for (int32 i = 0; Guide* guide = fCanvas->GuideAt(i); i++) {
			if (guide != fGuide) {
				// draw other guide
				guide->Draw(into, color, canvasFrame, bounds);
			} else {
				// draw current guide
				rgb_color c = (rgb_color){ 255, 120, 120, 255 };
				guide->Draw(into, c, canvasFrame, bounds);
			}
		}
	}
}


// MessageReceived
bool
GuidesState::MessageReceived(BMessage* message)
{
	if (!fCanvas)
		return false;

	bool result = true;
	switch (message->what) {
		case MSG_SHOW_GUIDES: {
			int32 value;
			if (message->FindInt32("be:value", &value) >= B_OK) {
				fShowGuides = value == B_CONTROL_ON;
				fCanvasView->Invalidate();
			}
			break;
		}
		case MSG_NEW_GUIDE: {
			Guide* guide = new Guide(fHPos, fVPos, fAngle);
			if (fCanvas->AddGuide(guide))
				_SetGuide(guide);
			else
				delete guide;
			break;
		}
		case MSG_REMOVE_GUIDE:
			_DeleteGuide();
			break;
		case MSG_GUIDE_GOLDEN_PROPORTION:
			if (fGuide) {
				float minPos = 1.0 - kGoldenProportion;
				float maxPos = kGoldenProportion;

				if (fabs(minPos - fGuide->HPos()) < fabs(maxPos - fGuide->HPos()))
					SetHPos(minPos);
				else
					SetHPos(maxPos);

				if (fabs(minPos - fGuide->VPos()) < fabs(maxPos - fGuide->VPos()))
					SetVPos(minPos);
				else
					SetVPos(maxPos);
			}
			break;

		case MSG_SET_GUIDE_H_POS:
		case MSG_SET_GUIDE_V_POS:
		case MSG_SET_GUIDE_ANGLE: {
			BView* source;
			if (message->FindPointer("source", (void**)&source) >= B_OK) {
				if (BTextControl* textView = dynamic_cast<BTextControl*>(source)) {
					const char* text = textView->Text();
					float value = atof(text);
					BRect canvasBounds = fCanvas->Bounds();
					switch (message->what) {
						case MSG_SET_GUIDE_H_POS:
							SetHPos(_ConvertFromUnits(value,
													  canvasBounds.left,
													  canvasBounds.Width()),
									true, false);
							break;
						case MSG_SET_GUIDE_V_POS:
							SetVPos(_ConvertFromUnits(value,
													  canvasBounds.top,
													  canvasBounds.Height()),
									true, false);
							break;
						case MSG_SET_GUIDE_ANGLE:
							SetAngle(value, true, false);
							break;
					}
				}
			}
			break;
		}
		case MSG_SET_GUIDE_UNITS: {
			uint32 units;
			if (message->FindInt32("units", (int32*)&units) >= B_OK)
				_SetUnits(units);
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
	return result;
}

/*
// ModifiersChanged
void
GuidesState::ModifiersChanged()
{
	if (fMode == MODE_CLONE) {
		if (modifiers() & B_COMMAND_KEY) {
			fCloneStep = CLONE_NEEDS_OFFSET;
			DrawCursor(fCursor.position);
		}
	}
}
*/
// HandleKeyDown
bool
GuidesState::HandleKeyDown(uint32 key, uint32 modifiers)
{
	bool result = true;

	switch (key) {
		// commit
		case B_RETURN:
			_Perform();
			break;
		// cancel
		case B_ESCAPE:
			_Cancel();
			break;

		case B_DELETE:
			if (fGuide)
				_DeleteGuide();
			else
				// pass on key event for someone else to hanle
				result = false;
			break;

		// nudging position and rotation
		case B_UP_ARROW:
			if (modifiers & B_COMMAND_KEY) {
			} else {
				_Nudge(BPoint(0.0, -1.0));
			}
			break;
		case B_DOWN_ARROW:
			if (modifiers & B_COMMAND_KEY) {
			} else {
				_Nudge(BPoint(0.0, 1.0));
			}
			break;
		case B_LEFT_ARROW:
			if (modifiers & B_COMMAND_KEY) {
				// rotation
				if (modifiers & B_SHIFT_KEY)
					_NudgeRotation(0.1);
				else
					_NudgeRotation(5.0);
			} else
				_Nudge(BPoint(-1.0, 0.0));
			break;
		case B_RIGHT_ARROW:
			if (modifiers & B_COMMAND_KEY) {
				// rotation
				if (modifiers & B_SHIFT_KEY)
					_NudgeRotation(-0.1);
				else
					_NudgeRotation(-5.0);
			} else
				_Nudge(BPoint(1.0, 0.0));
			break;

		default:
			result = false;
	}
	return result;
}
/*
// HandleKeyUp
bool
GuidesState::HandleKeyUp(uint32 key, uint32 modifiers)
{
	return false;
}
*/
// UpdateToolCursor
void
GuidesState::UpdateToolCursor()
{
	const unsigned char* cursorData = fGuideUnderMouse ? kMoveCursor : B_HAND_CURSOR;

	BCursor cursor(cursorData);
	fCanvasView->SetViewCursor(&cursor, true);
}

// ObjectChanged
void
GuidesState::ObjectChanged(const Observable* object)
{
	const Guide* guide = dynamic_cast<const Guide*>(object);
	if (guide && guide == fGuide) {

		bool invalidate = false;

		// h_pos
		if (fGuide->HPos() != fHPos) {
			fHPos = fGuide->HPos();
			_UpdateHPosControl();
			invalidate = true;
		}

		// v_pos
		if (fGuide->VPos() != fVPos) {
			fVPos = fGuide->VPos();
			_UpdateVPosControl();
			invalidate = true;
		}

		// angle
		if (fGuide->Angle() != fAngle) {

			fAngle = fGuide->Angle();
			if (fCanvasView->Window()) {
				BMessage message(MSG_SET_GUIDE_ANGLE);
				message.AddFloat("value", fAngle);
				fCanvasView->Window()->PostMessage(&message);
			}

			invalidate = true;
		}

		if (invalidate)
			fCanvasView->Invalidate();
	}
}

// SetHPos
void
GuidesState::SetHPos(float hPos, bool action, bool notify)
{
	if (fHPos != hPos) {
		if (!fGuide || !notify) {
			fHPos = hPos;
			fCanvasView->Invalidate();
		}
		if (fGuide) {
/*			if (action)
				fCanvasView->Perform(new ChangeFontAdvanceScaleAction(this, fTextStroke));*/
			fGuide->SetHPos(hPos);
		}
	}
}

// SetVPos
void
GuidesState::SetVPos(float vPos, bool action, bool notify)
{
	if (fVPos != vPos) {
		if (!notify) {
			fVPos = vPos;
			fCanvasView->Invalidate();
		}
		if (fGuide) {
/*			if (action)
				fCanvasView->Perform(new ChangeFontAdvanceScaleAction(this, fTextStroke));*/
			fGuide->SetVPos(vPos);
		}
	}
}

// SetAngle
void
GuidesState::SetAngle(double angle, bool action, bool notify)
{
	if (fAngle != angle) {
		if (!notify) {
			fAngle = angle;
			fCanvasView->Invalidate();
		}
		if (fGuide) {
/*			if (action)
				fCanvasView->Perform(new ChangeFontAdvanceScaleAction(this, fTextStroke));*/
			fGuide->SetAngle(angle);
		}
	}
}

// _SetGuide
void
GuidesState::_SetGuide(Guide* guide)
{
/*	if (fTextStroke)
		_Perform();
	if (modifier) {
		fTextStroke = modifier;
		fTextStroke->AddObserver(this);
		// set the shape stroke focused in the history list
		HistoryListView* listView = fCanvasView->GetHistoryListView();
		History* history = fLayer->GetHistory();
		if (history && listView) {
			listView->SetItemFocused(history->IndexOf(fTextStroke));
		}
		if (fCanvas) {
			fCanvas->SetTemporaryHistory(fPrivateHistory);
			fCanvasView->UpdateHistoryItems();
		}

		if (edit)
			fEditAction = new EditModifierAction(fLayer, fTextStroke);
		if (fEditAction) {
			// customize ourself to the text stroke

			// customize ourself to the text stroke
			// and trigger the notifications
			ObjectChanged(fTextStroke);
		} else {
			// customize the text stroke
			fTextStroke->SuspendNotifications(true);

			fTextStroke->SetSize(fFontSize);
			fTextStroke->SetText(fText.String());
			fTextStroke->SetFamilyAndStyle(fFontFamily, fFontStyle);
			fTextStroke->SetOpacity(fOpacity);
			fTextStroke->SetAdvanceScale(fAdvanceScale);

			fTextStroke->SuspendNotifications(false);

			_RedrawStroke(fTextStroke->Bounds(), true);
		}
		_SetConfirmationEnabled(true);
	}
	UpdateToolCursor();*/
	if (fGuide != guide) {
		if (fGuide) {
			fGuide->RemoveObserver(this);
		}
		fGuide = guide;
		BMessage message(MSG_GUIDE_SELECTED);
		if (fGuide) {
			fGuide->AddObserver(this);
			ObjectChanged(fGuide);
			message.AddBool("selected", true);
		}
		if (BWindow* window = fCanvasView->Window())
			window->PostMessage(&message);
		fCanvasView->Invalidate();
	}
}

// _Perform
void
GuidesState::_Perform()
{
	if (!fCanceling) {
		_SetGuide(NULL);
/*		if (fTextStroke) {
			fTextStroke->RemoveObserver(this);
			// remove private history from canvas
			if (fCanvas) {
				fCanvas->SetTemporaryHistory(NULL);
				fCanvasView->UpdateHistoryItems();
			}
			if (fEditAction) {
				if (fPrivateHistory->IsSaved()) {
					// this means the stroke has not been edited at all
					// we don't need to perform the edit action in this case
					delete fEditAction;
				} else {
					// we will want to be able to undo the changes
					fCanvasView->Perform(fEditAction);
				}
				fEditAction = NULL;
			} else {
				// perform and cause the canvas view to switch buffers
				if (!fCanvasView->Perform(new AddModifierAction(fLayer, fTextStroke))) {
					delete fTextStroke;
				} else {
					BRect r(fTextStroke->Bounds());
					clear_area(fCanvasView->StrokeBitmap(), r);
					fCanvasView->SwitchBuffers(r);
					fCanvasView->_InvalidateCanvas(r);
				}
			}
			// history needs to be empty
			fPrivateHistory->Clear();
			// unfocus the text stroke item (we're now done editing it)
			HistoryListView* listView = fCanvasView->GetHistoryListView();
			if (listView)
				listView->SetItemFocused(-1);
			// this is not ours anymore
			fTextStroke = NULL;
			_SetConfirmationEnabled(false);
		}
		UpdateToolCursor();*/
	}
}

// _Cancel
void
GuidesState::_Cancel()
{
	fCanceling = true;
	_SetGuide(NULL);
	// clean up
/*	if (fTextStroke) {
		fTextStroke->RemoveObserver(this);
		// unfocus the shape modifier item, remove it
		// remove modifier from layers history
		HistoryListView* listView = fCanvasView->GetHistoryListView();
		History* history = fLayer->GetHistory();
		if (history && listView) {
			listView->SetItemFocused(-1);
			if (fEditAction) {
				// we have only edited this modifier
				// undo all the changes
				fEditAction->Undo(fCanvasView);
				delete fEditAction;
				fEditAction = NULL;
			} else {
				// we have added this modifier and need to get rid of it
				delete listView->RemoveItem(history->IndexOf(fTextStroke));
				history->RemoveItem(fTextStroke);

				// clear the stroke bitmap area, restore the layer bitmap
				BRect r(fTextStroke->Bounds());
				clear_area(fCanvasView->StrokeBitmap(), r);
				copy_area(fCanvasView->BackBitmap(),
						  fLayer->Bitmap(), r);
				fCanvasView->_InvalidateCanvas(r, true);

				delete fTextStroke;
			}
		}
		fTextStroke = NULL;
		if (fCanvas) {
			fCanvas->SetTemporaryHistory(NULL);
			fCanvasView->UpdateHistoryItems();
		}
		fPrivateHistory->Clear();
	}
	_SetConfirmationEnabled(false);
	UpdateToolCursor();*/
	fCanceling = false;
}

// _SetConfirmationEnabled
void
GuidesState::_SetConfirmationEnabled(bool enable) const
{
	if (MainWindow* window = dynamic_cast<MainWindow*>(fCanvasView->Window()))
		window->SetConfirmationEnabled(enable);
}

// _Nudge
void
GuidesState::_Nudge(BPoint offset)
{
	if (fCanvas && fGuide) {
		// remember current unit mode and switch to pixels,
		// so we can nugde by pixel offsets
		uint32 unit = fUnitsMode;
		fUnitsMode = GUIDE_UNITS_PIXELS;

		// get current guide position
		BRect canvasBounds = fCanvas->Bounds();
		BPoint position;
		position.x = _ConvertToUnits(fGuide->HPos(),
									 canvasBounds.left,
									 canvasBounds.Width());
		position.y = _ConvertToUnits(fGuide->VPos(),
									 canvasBounds.top,
									 canvasBounds.Height());
		// apply offset
		position += offset;

		float hPos = _ConvertFromUnits(position.x,
									   canvasBounds.left,
									   canvasBounds.Width());
		float vPos = _ConvertFromUnits(position.y,
									   canvasBounds.top,
									   canvasBounds.Height());

		// restore the unit mode
		fUnitsMode = unit;

		if (offset.x != 0.0) {
			SetHPos(hPos);
		}
		if (offset.y != 0.0) {
			SetVPos(vPos);
		}
	}
/*	if (fTextStroke && !fCanvasView->IsBusy()) {
		bigtime_t now = system_time();
		bool action = now - fLastNudgeTime > ACTION_TRIGGER_TIMEOUT;

		BPoint dummy;
		SetSomething(offset, dummy, &TextStroke::TranslateBy, true);

		if (action)
			fCanvasView->Perform(new MoveTextAction(this, fTextStroke,
													BPoint(-offset.x, -offset.y)));

		fLastNudgeTime = now;
	}*/
}
// _NudgeRotation
void
GuidesState::_NudgeRotation(float diff)
{
/*	float rotation = fAdvanceScale + diff;

	if (rotation < 0.0)
		rotation = rotation + 360.0;
	if (rotation > 360.0)
		rotation = rotation - 360.0;

	bigtime_t now = system_time();
	bool action = now - fLastNudgeRotationTime > ACTION_TRIGGER_TIMEOUT;

	SetRotation(rotation, action);

	fLastNudgeRotationTime = now;*/
}

// _SetUnits
void
GuidesState::_SetUnits(uint32 units)
{
	if (fUnitsMode != units) {
		fUnitsMode = units;
		_UpdateHPosControl();
		_UpdateVPosControl();
	}
}

// _ConvertToUnits
float
GuidesState::_ConvertToUnits(float value, float start, float size) const
{
	if (fCanvas) {
		switch (fUnitsMode) {
			case GUIDE_UNITS_PIXELS:
				value = start + size * value;
				break;
			default:
			case GUIDE_UNITS_PERCENT:
				value *= 100.0;
				break;
		}
	}
	return value;
}

// _ConvertFromUnits
float
GuidesState::_ConvertFromUnits(float value, float start, float size) const
{
	if (fCanvas) {
		switch (fUnitsMode) {
			case GUIDE_UNITS_PIXELS:
				if (size > 0)
					value = (value - start) / size;
				else
					value = 0;
				break;
			default:
			case GUIDE_UNITS_PERCENT:
				value /= 100.0;
				break;
		}
	}
	return value;
}

// _UpdateHPosControl
void
GuidesState::_UpdateHPosControl() const
{
	if (fCanvas && fCanvasView->Window()) {
		BMessage message(MSG_SET_GUIDE_H_POS);
		message.AddFloat("value", _ConvertToUnits(fHPos,
												  fCanvas->Bounds().left,
												  fCanvas->Bounds().Width()));
		fCanvasView->Window()->PostMessage(&message);
	}
}

// _UpdateVPosControl
void
GuidesState::_UpdateVPosControl() const
{
	if (fCanvas && fCanvasView->Window()) {
		BMessage message(MSG_SET_GUIDE_V_POS);
		message.AddFloat("value", _ConvertToUnits(fVPos,
												  fCanvas->Bounds().top,
												  fCanvas->Bounds().Height()));
		fCanvasView->Window()->PostMessage(&message);
	}
}

// _DeleteGuide
void
GuidesState::_DeleteGuide()
{
	if (fCanvas && fGuide) {
		Guide* guide = fGuide;
		if (fCanvas->RemoveGuide(guide)) {
			_SetGuide(NULL);
			delete guide;
		}
	}
}
