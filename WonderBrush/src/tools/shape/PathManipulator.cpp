// PathManipulator.cpp

#include <float.h>
#include <stdio.h>

#include <Bitmap.h>
#include <Control.h>
#include <Cursor.h>
#include <File.h>
#include <Message.h>
#include <Region.h>
#include <Window.h>

#include "bitmap_support.h"
#include "cursors.h"
#include "defines.h"
#include "support.h"

#include "Canvas.h"
#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "HistoryManager.h"
#include "Layer.h"
#include "MainWindow.h"

#include "PointSelection.h"
#include "ShapeStroke.h"
#include "TransformBox.h"
#include "VectorPath.h"

#include "AddModifierAction.h"

#include "AddPointAction.h"
#include "ChangeAlphaAction.h"
#include "ChangeCapModeAction.h"
#include "ChangeJoinModeAction.h"
#include "ChangeOutlineAction.h"
#include "ChangeOutlineWidthAction.h"
#include "ChangePointAction.h"
#include "CloseAction.h"
#include "EditModifierAction.h"
#include "InsertPointAction.h"
#include "NewPathAction.h"
#include "NudgePointsAction.h"
#include "RemovePathAction.h"
#include "RemovePointAction.h"
#include "ReversePathAction.h"
#include "SelectPathAction.h"
#include "SelectPointsAction.h"
#include "SetShapeColorAction.h"
#include "EnterTransformPointsAction.h"

#include "PathManipulator.h"

#define POINT_EXTEND 3.0
#define CONTROL_POINT_EXTEND 2.0
#define INSERT_DIST_THRESHOLD 7.0
#define MOVE_THRESHOLD 9.0

enum {
	UNDEFINED,

	NEW_PATH,

	ADD_POINT,
	INSERT_POINT,
	MOVE_POINT,
	MOVE_POINT_IN,
	MOVE_POINT_OUT,
	CLOSE_PATH,

	TOGGLE_SHARP,
	TOGGLE_SHARP_IN,
	TOGGLE_SHARP_OUT,

	REMOVE_POINT,
	REMOVE_POINT_IN,
	REMOVE_POINT_OUT,

	SELECT_POINTS,
	TRANSFORM_POINTS,
	TRANSLATE_POINTS,

	SELECT_SUB_PATH,
};

inline const char*
string_for_mode(uint32 mode)
{
	switch (mode) {
		case UNDEFINED:
			return "UNDEFINED";
		case NEW_PATH:
			return "NEW_PATH";
		case ADD_POINT:
			return "ADD_POINT";
		case INSERT_POINT:
			return "INSERT_POINT";
		case MOVE_POINT:
			return "MOVE_POINT";
		case MOVE_POINT_IN:
			return "MOVE_POINT_IN";
		case MOVE_POINT_OUT:
			return "MOVE_POINT_OUT";
		case CLOSE_PATH:
			return "CLOSE_PATH";
		case TOGGLE_SHARP:
			return "TOGGLE_SHARP";
		case TOGGLE_SHARP_IN:
			return "TOGGLE_SHARP_IN";
		case TOGGLE_SHARP_OUT:
			return "TOGGLE_SHARP_OUT";
		case REMOVE_POINT:
			return "REMOVE_POINT";
		case REMOVE_POINT_IN:
			return "REMOVE_POINT_IN";
		case REMOVE_POINT_OUT:
			return "REMOVE_POINT_OUT";
		case SELECT_POINTS:
			return "SELECT_POINTS";
		case TRANSFORM_POINTS:
			return "TRANSFORM_POINTS";
		case TRANSLATE_POINTS:
			return "TRANSLATE_POINTS";
		case SELECT_SUB_PATH:
			return "SELECT_SUB_PATH";
	}
	return "<unknown mode>";
}

class Selection : protected BList
{
public:
	inline Selection(int32 count = 20)
		: BList(count) {}
	inline ~Selection() {}

	inline void Add(int32 value)
		{
			if (value >= 0) {
				// keep the list sorted
				int32 count = CountItems();
				int32 index = 0;
				for (; index < count; index++) {
					if (IndexAt(index) > value) {
						break;
					}
				}
				BList::AddItem((void*)value, index);
			}
		}

	inline bool Remove(int32 value)
		{ return BList::RemoveItem((void*)value); }

	inline bool Contains(int32 value) const
		{ return BList::HasItem((void*)value); }

	inline bool IsEmpty() const
		{ return BList::IsEmpty(); }

	inline int32 IndexAt(int32 index) const
		{ return (int32)BList::ItemAt(index); }

	inline void MakeEmpty()
		{ BList::MakeEmpty(); }

	inline int32* Items() const
		{ return (int32*)BList::Items(); }

	inline const int32 CountItems() const
		{ return BList::CountItems(); }

	inline Selection& operator =(const Selection& other)
		{
			MakeEmpty();
			int32 count = other.CountItems();
			int32* items = other.Items();
			for (int32 i = 0; i < count; i++) {
				Add(items[i]);
			}
			return *this;
		}

	inline bool operator ==(const Selection& other)
		{
			if (other.CountItems() == CountItems()) {
				int32* items = Items();
				int32* otherItems = other.Items();
				for (int32 i = 0; i < CountItems(); i++) {
					if (items[i] != otherItems[i])
						return false;
					items++;
					otherItems++;
				}
				return true;
			} else
				return false;
		}

	inline bool operator !=(const Selection& other)
	{
		return !(*this == other);
	}
};


// constructor
PathManipulator::PathManipulator(CanvasView* parent)
	: fCanvasView(parent),

	  fCommandDown(false),
	  fOptionDown(false),
	  fShiftDown(false),
	  fAltDown(false),

	  fClickToClose(false),

	  fMode(NEW_PATH),
	  fFallBackMode(SELECT_POINTS),

	  fMouseDown(false),

	  fPath(NULL),
	  fCurrentPathPoint(-1),

	  fChangePointAction(NULL),
	  fInsertPointAction(NULL),
	  fAddPointAction(NULL),

	  fSelection(new Selection()),
	  fOldSelection(new Selection()),
	  fTransformBox(NULL),

	  fNudgeOffset(0.0, 0.0),
	  fLastNudgeTime(system_time()),
	  fNudgeAction(NULL)
{
}

// destructor
PathManipulator::~PathManipulator()
{
	delete fChangePointAction;
	delete fInsertPointAction;
	delete fAddPointAction;

	delete fSelection;
	delete fOldSelection;
	delete fTransformBox;

	delete fNudgeAction;
}

// SetTo
void
PathManipulator::SetTo(VectorPath* path, bool clickToClose)
{
	if (fPath)
		Unset();

	fPath = path;
	fClickToClose = clickToClose;

	if (!fMouseDown)
		_SetModeForMousePos(fLastCanvasPos);
}

// Unset
void
PathManipulator::Unset()
{
	_SetMode(UNDEFINED);
	fFallBackMode = SELECT_POINTS;

	fPath = NULL;

	fSelection->MakeEmpty();
	_UpdateSelection();
}

// MouseDown
void
PathManipulator::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	fMouseDown = true;

	if (!fPath)
		return;

	if (fMode == TRANSFORM_POINTS) {
		if (fTransformBox) {
			fTransformBox->MouseDown(canvasWhere.point);

			if (!fTransformBox->IsRotating())
				fCanvasView->SetAutoScrolling(true);
		}
		return;
	}

	if (fMode == MOVE_POINT &&
		fSelection->CountItems() > 1 &&
		fSelection->Contains(fCurrentPathPoint)) {
		fMode = TRANSLATE_POINTS;
	}

	// maybe we're changing some point, so we construct the
	// "ChangePointAction" here so that the point is remembered
	// in its current state
	delete fChangePointAction;
	fChangePointAction = NULL;
	switch (fMode) {
		case TOGGLE_SHARP:
		case TOGGLE_SHARP_IN:
		case TOGGLE_SHARP_OUT:
		case MOVE_POINT:
		case MOVE_POINT_IN:
		case MOVE_POINT_OUT:
		case REMOVE_POINT_IN:
		case REMOVE_POINT_OUT:
//			fChangePointAction = new ChangePointAction(this,
//													   fPath,
//													   fCurrentPathPoint,
//													   fSelection->Items(),
//													   fSelection->CountItems());
			_Select(fCurrentPathPoint, fShiftDown);
			break;
	}

	// at this point we init doing something
	switch (fMode) {
		case ADD_POINT:
			_AddPoint(canvasWhere);
			break;
		case INSERT_POINT:
			_InsertPoint(canvasWhere, fCurrentPathPoint);
			break;

		case TOGGLE_SHARP:
			_SetSharp(fCurrentPathPoint);
			// continue by dragging out the _connected_ in/out points
			break;
		case TOGGLE_SHARP_IN:
			_SetInOutConnected(fCurrentPathPoint, false);
			// continue by moving the "in" point
			_SetMode(MOVE_POINT_IN);
			break;
		case TOGGLE_SHARP_OUT:
			_SetInOutConnected(fCurrentPathPoint, false);
			// continue by moving the "out" point
			_SetMode(MOVE_POINT_OUT);
			break;

		case MOVE_POINT:
		case MOVE_POINT_IN:
		case MOVE_POINT_OUT:
			// the right thing happens since "fCurrentPathPoint"
			// points to the correct index
			break;

		case CLOSE_PATH:
			SetClosed(true, true);
			break;

		case REMOVE_POINT:
			if (fPath->CountPoints() == 1) {
//				fCanvasView->Perform(new RemovePathAction(this, fPath));
			} else {
//				fCanvasView->Perform(new RemovePointAction(this,
//														   fPath,
//														   fCurrentPathPoint,
//														   fSelection->Items(),
//														   fSelection->CountItems()));
				_RemovePoint(fCurrentPathPoint);
			}
			break;
		case REMOVE_POINT_IN:
			_RemovePointIn(fCurrentPathPoint);
			break;
		case REMOVE_POINT_OUT:
			_RemovePointOut(fCurrentPathPoint);
			break;

		case SELECT_POINTS:
			if (!fShiftDown) {
				fSelection->MakeEmpty();
				_UpdateSelection();
			}
			*fOldSelection = *fSelection;
			if (fCurrentPathPoint >= 0) {
				_Select(fCurrentPathPoint, fShiftDown);
			}
			fCanvasView->BeginRectTracking(BRect(where, where),
										   B_TRACK_RECT_CORNER);
			break;
	}

	fTrackingStart = canvasWhere.point;
	// remember the subpixel position
	// so that MouseMoved() will work even before
	// the integer position becomes different
	fCanvasView->ConvertToCanvas(where);
	fLastCanvasPos = where;

	// the reason to exclude the select mode
	// is that the BView rect tracking does not
	// scroll the rect starting point along with us
	// (since we're doing no real scrolling)
	if (fMode != SELECT_POINTS)
		fCanvasView->SetAutoScrolling(true);

	UpdateToolCursor();
}

// MouseUp
void
PathManipulator::MouseUp(BPoint where, Point canvasWhere)
{
	// prevent carrying out actions more than once by only
	// doing it if "fMouseDown" is true at the point of
	// entering this function
	if (!fMouseDown)
		return;
	fMouseDown = false;

	if (fMode == TRANSFORM_POINTS) {
		if (fTransformBox) {
			fCanvasView->Perform(fTransformBox->MouseUp());
		}
		return;
	}

	if (!fPath)
		return;

	switch (fMode) {

		case ADD_POINT:
			fCanvasView->Perform(fAddPointAction);
			fAddPointAction = NULL;
			_SetMode(MOVE_POINT_OUT);
			break;

		case INSERT_POINT:
			fCanvasView->Perform(fInsertPointAction);
			fInsertPointAction = NULL;
			break;

		case SELECT_POINTS:
			if (*fSelection != *fOldSelection) {
//				fCanvasView->Perform(new SelectPointsAction(this, fPath,
//															fOldSelection->Items(),
//															fOldSelection->CountItems(),
//															fSelection->Items(),
//															fSelection->CountItems()));
			}
			fCanvasView->EndRectTracking();
			break;

		case TOGGLE_SHARP:
		case TOGGLE_SHARP_IN:
		case TOGGLE_SHARP_OUT:
		case MOVE_POINT:
		case MOVE_POINT_IN:
		case MOVE_POINT_OUT:
		case REMOVE_POINT_IN:
		case REMOVE_POINT_OUT:
			fCanvasView->Perform(fChangePointAction);
			fChangePointAction = NULL;
			break;

		case TRANSLATE_POINTS:
			if (!fNudgeAction) {
				// select just the point that was clicked
				*fOldSelection = *fSelection;
				if (fCurrentPathPoint >= 0) {
					_Select(fCurrentPathPoint, fShiftDown);
				}
				if (*fSelection != *fOldSelection) {
//					fCanvasView->Perform(new SelectPointsAction(this, fPath,
//																fOldSelection->Items(),
//																fOldSelection->CountItems(),
//																fSelection->Items(),
//																fSelection->CountItems()));
				}
			} else {
				_FinishNudging();
			}
			break;
	}
}

// MouseMoved
void
PathManipulator::MouseMoved(BPoint where, Point canvasWhere,
					   uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

	// since the tablet is generating mouse moved messages
	// even if only the pressure changes (and not the actual mouse position)
	// we insert this additional check to prevent too much calculation
	if (fMouseDown && fLastCanvasPos == canvasWhere.point)
		return;

	fLastCanvasPos = canvasWhere.point;

	if (!fPath)
		return;

	if (fMode == TRANSFORM_POINTS) {
		if (fTransformBox) {
			if (fCanvasView->IsBusy()) {
				if (fMouseDown)
					fCanvasView->EventDropped();
				return;
			}
			fTransformBox->MouseMoved(canvasWhere.point, fCanvasView->ZoomLevel());
		}
		return;
	}

	if (fMouseDown!) {
		// hit testing
		// (use a subpixel mouse pos)
		fCanvasView->ConvertToCanvas(where);
		_SetModeForMousePos(where);
		return;
	}

	if (fCanvasView->IsBusy()) {
		if (fMouseDown)
			fCanvasView->EventDropped();
		return;
	}

	BRect r;
	BRect cr;
	_GetChangableAreas(&r, &cr);

	bool redraw = true;

	if (fMode == CLOSE_PATH) {
		// continue by moving the point
		_SetMode(MOVE_POINT);
		delete fChangePointAction;
//		fChangePointAction = new ChangePointAction(this,
//												   fPath,
//												   fCurrentPathPoint,
//												   fSelection->Items(),
//												   fSelection->CountItems());
	}

	if (fOutline && !fPrecise) {
		float offset = fmod(fOutlineWidth, 2.0) / 2.0;
		canvasWhere.point += BPoint(offset, offset);
	}

	switch (fMode) {
		case ADD_POINT:
		case INSERT_POINT:
		case TOGGLE_SHARP:
			// drag the "out" control point, mirror the "in" control point
			fPath->SetPointOut(fCurrentPathPoint, canvasWhere.point, true);
			break;
		case MOVE_POINT:
			// drag all three control points at once
			fPath->SetPoint(fCurrentPathPoint, canvasWhere.point);
			break;
		case MOVE_POINT_IN:
			// drag in control point
			fPath->SetPointIn(fCurrentPathPoint, canvasWhere.point);
			break;
		case MOVE_POINT_OUT:
			// drag out control point
			fPath->SetPointOut(fCurrentPathPoint, canvasWhere.point);
			break;
		
		case SELECT_POINTS: {
			// change the selection
			BRect r;
			r.left = min_c(fTrackingStart.x, canvasWhere.point.x);
			r.top = min_c(fTrackingStart.y, canvasWhere.point.y);
			r.right = max_c(fTrackingStart.x, canvasWhere.point.x);
			r.bottom = max_c(fTrackingStart.y, canvasWhere.point.y);
			_Select(r);
			redraw = false;
			break;
		}

		case TRANSLATE_POINTS: {
			BPoint offset = canvasWhere.point - fTrackingStart;
			_Nudge(offset);
			fTrackingStart = canvasWhere.point;
			break;
		}

		default:
			redraw = false;
	}

	if (redraw)
		fRenderer->Invalidate(r, cr, true);
}

class CanvasViewIterator : public VectorPath::Iterator {
 public:
					CanvasViewIterator(CanvasView* canvasView,
									   BView* drawingView)
						: fCanvasView(canvasView),
						  fDrawingView(drawingView)
					{
						fDrawingView->SetHighColor(0, 0, 0, 255);
//						fDrawingView->SetDrawingMode(B_OP_INVERT);
//						fDrawingView->SetDrawingMode(B_OP_COPY);
						fDrawingView->SetDrawingMode(B_OP_OVER);
					}
	virtual			~CanvasViewIterator()
					{}

	virtual	void	MoveTo(BPoint point)
					{
						fBlack = true;
						fDrawingView->SetHighColor(0, 0, 0, 255);

						fCanvasView->ConvertFromCanvas(point);
						fDrawingView->MovePenTo(point);
					}
	virtual	void	LineTo(BPoint point)
					{
						if (fBlack)
							fDrawingView->SetHighColor(255, 255, 255, 255);
						else
							fDrawingView->SetHighColor(0, 0, 0, 255);
						fBlack = !fBlack;

						fCanvasView->ConvertFromCanvas(point);
						fDrawingView->StrokeLine(point);
					}

 private:
	CanvasView*		fCanvasView;
	BView*			fDrawingView;
	bool			fBlack;
};

// Draw
void
PathManipulator::Draw(BView* into, BRect updateRect)
{
	if (!fPath)
		return;

	// draw the Bezier curve, but only if editing
	// if not "editing", the path is actually on top all other modifiers
	// TODO: make this customizable in the GUI
	CanvasViewIterator iterator(fCanvasView, into);
	fPath->Iterate(&iterator, fCanvasView->ZoomLevel()/* / 2.0*/);

	if (true/*TODO: fPath->IsEditable()*/) {
		into->SetLowColor(0, 0, 0, 255);
		BPoint point;
		BPoint pointIn;
		BPoint pointOut;
		rgb_color focus_color = (rgb_color){ 255, 0, 0, 255 }; //ui_color(B_KEYBOARD_NAVIGATION_COLOR);
		rgb_color highlight_color = (rgb_color){ 60, 60, 255, 255 }; //ui_color(B_KEYBOARD_NAVIGATION_COLOR);
		for (int32 i = 0; fPath->GetPointsAt(i, point, pointIn, pointOut); i++) {
			bool highlight = fCurrentPathPoint == i;
			bool selected = fSelection->Contains(i);
			rgb_color normal = selected ? focus_color : (rgb_color){ 0, 0, 0, 255 };
			into->SetLowColor(normal);
			into->SetHighColor(255, 255, 255, 255);
			// convert to view coordinate space
			fCanvasView->ConvertFromCanvas(point);
			fCanvasView->ConvertFromCanvas(pointIn);
			fCanvasView->ConvertFromCanvas(pointOut);
			// connect the points belonging to one control point
			into->SetDrawingMode(B_OP_INVERT);
			into->StrokeLine(point, pointIn);
			into->StrokeLine(point, pointOut);
			// draw main control point
			if (highlight && (fMode == MOVE_POINT ||
							  fMode == TOGGLE_SHARP ||
							  fMode == REMOVE_POINT ||
							  fMode == SELECT_POINTS ||
							  fMode == CLOSE_PATH)) {

				into->SetLowColor(highlight_color);
			}

			into->SetDrawingMode(B_OP_COPY);
			BRect r(point, point);
			r.InsetBy(-POINT_EXTEND, -POINT_EXTEND);
			into->StrokeRect(r, B_SOLID_LOW);
			r.InsetBy(1.0, 1.0);
			into->FillRect(r, B_SOLID_HIGH);
			// draw in control point
			if (highlight && (fMode == MOVE_POINT_IN ||
							  fMode == TOGGLE_SHARP_IN ||
							  fMode == REMOVE_POINT_IN ||
							  fMode == SELECT_POINTS))
				into->SetLowColor(highlight_color);
			else
				into->SetLowColor(normal);
			if (selected) {
				into->SetHighColor(220, 220, 220, 255);
			} else {
				into->SetHighColor(170, 170, 170, 255);
			}
			if (pointIn != point) {
				r.Set(pointIn.x - CONTROL_POINT_EXTEND, pointIn.y - CONTROL_POINT_EXTEND,
					  pointIn.x + CONTROL_POINT_EXTEND, pointIn.y + CONTROL_POINT_EXTEND);
				into->StrokeRect(r, B_SOLID_LOW);
				r.InsetBy(1.0, 1.0);
				into->FillRect(r, B_SOLID_HIGH);
			}
			// draw out control point
			if (highlight && (fMode == MOVE_POINT_OUT ||
							  fMode == TOGGLE_SHARP_OUT ||
							  fMode == REMOVE_POINT_OUT ||
							  fMode == SELECT_POINTS))
				into->SetLowColor(highlight_color);
			else
				into->SetLowColor(normal);
			if (pointOut != point) {
				r.Set(pointOut.x - CONTROL_POINT_EXTEND, pointOut.y - CONTROL_POINT_EXTEND,
					  pointOut.x + CONTROL_POINT_EXTEND, pointOut.y + CONTROL_POINT_EXTEND);
				into->StrokeRect(r, B_SOLID_LOW);
				r.InsetBy(1.0, 1.0);
				into->FillRect(r, B_SOLID_HIGH);
			}
		}
	}
	if (fTransformBox) {
		fTransformBox->Draw(into);
	}
}

// MessageReceived
bool
PathManipulator::MessageReceived(BMessage* message)
{
	bool result = true;
	switch (message->what) {
		case MSG_SHAPE_TRANSFORM:
			if (fPath && !fSelection->IsEmpty())
				_SetMode(TRANSFORM_POINTS);
			break;
		case MSG_SHAPE_REMOVE_POINTS:
			_Delete();
			break;
		case B_SELECT_ALL:
			if (fPath) {
				*fOldSelection = *fSelection;
				fSelection->MakeEmpty();
				int32 count = fPath->CountPoints();
				for (int32 i = 0; i < count; i++)
					fSelection->Add(i);
				if (*fOldSelection != *fSelection) {
//					fCanvasView->Perform(new SelectPointsAction(this, fPath,
//																fOldSelection->Items(),
//																fOldSelection->CountItems(),
//																fSelection->Items(),
//																fSelection->CountItems()));
					count = fSelection->CountItems();
					int32 indices[count];
					memcpy(indices, fSelection->Items(), count * sizeof(int32));
					_Select(indices, count);
				}
			} else
				result = false;
			break;
		default:
			result = false;
			break;
	}
	return result;
}


// ModifiersChanged
void
PathManipulator::ModifiersChanged(uint32 modifiers)
{
	fCommandDown = modifiers & B_COMMAND_KEY;
	fOptionDown = modifiers & B_CONTROL_KEY;
	fShiftDown = modifiers & B_SHIFT_KEY;
	fAltDown = modifiers & B_OPTION_KEY;

	if (fTransformBox) {
		fTransformBox->ModifiersChanged(modifiers);
		return;
	}
	// reevaluate mode
	if (!fMouseDown)
		_SetModeForMousePos(fLastCanvasPos);
}

// HandleKeyDown
bool
PathManipulator::HandleKeyDown(uint32 key, uint32 modifiers)
{
	bool result = true;

	float nudgeDist = 1.0;
	if (modifiers & B_SHIFT_KEY)
		nudgeDist /= fCanvasView->ZoomLevel();

	switch (key) {
		// commit
		case B_RETURN:
			if (fTransformBox) {
				fCanvasView->Perform(fTransformBox->Perform());
				_SetModeForMousePos(fLastCanvasPos);
			} else
//				_Perform();
			break;
		// cancel
		case B_ESCAPE:
			if (fTransformBox) {
				fCanvasView->Perform(fTransformBox->Cancel());
				_SetModeForMousePos(fLastCanvasPos);
			} else if (fFallBackMode == NEW_PATH) {
				fFallBackMode = SELECT_POINTS;
				_SetModeForMousePos(fLastCanvasPos);
			} else
//				_Cancel();
			break;
		case 't':
		case 'T':
			if (!fSelection->IsEmpty())
				_SetMode(TRANSFORM_POINTS);
			else
				result = false;
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

		case B_DELETE:
			if (!fSelection->IsEmpty())
				_Delete();
			else
				result = false;
			break;

/*		// keyboard control for text size
		case '+': {
			break;
		}
		case '-': {
			break;
		}*/

		default:
			result = false;
	}
	return result;
}

// HandleKeyUp
bool
PathManipulator::HandleKeyUp(uint32 key, uint32 modifiers)
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
PathManipulator::UpdateToolCursor()
{
	if (fTransformBox) {
		fTransformBox->UpdateToolCursor();
		return;
	}
	const unsigned char* cursorData = kPathAddCursor;
	if (fPath) {
		switch (fMode) {
			case ADD_POINT:
				cursorData = kPathAddCursor;
				break;
			case INSERT_POINT:
				cursorData = kPathInsertCursor;
				break;
			case MOVE_POINT:
			case MOVE_POINT_IN:
			case MOVE_POINT_OUT:
			case TRANSLATE_POINTS:
				cursorData = kPathMoveCursor;
				break;
			case CLOSE_PATH:
				cursorData = kPathCloseCursor;
				break;
			case TOGGLE_SHARP:
			case TOGGLE_SHARP_IN:
			case TOGGLE_SHARP_OUT:
				cursorData = kPathSharpCursor;
				break;
			case REMOVE_POINT:
			case REMOVE_POINT_IN:
			case REMOVE_POINT_OUT:
				cursorData = kPathRemoveCursor;
				break;
			case SELECT_POINTS:
				cursorData = kPathSelectCursor;
				break;

			case SELECT_SUB_PATH:
				cursorData = B_HAND_CURSOR;
				break;

			case UNDEFINED:
			default:
				cursorData = kStopCursor;
				break;
		}
	}
	BCursor cursor(cursorData);
	fCanvasView->SetViewCursor(&cursor, true);
	fCanvasView->Sync();
}

// ControlFlags
uint32
PathManipulator::ControlFlags() const
{
	uint32 flags = 0;

	if (fPath) {
		flags |= SHAPE_UI_FLAGS_CAN_REVERSE_PATH;

		if (!fSelection->IsEmpty())
			flags |= SHAPE_UI_FLAGS_HAS_SELECTION;
		if (fPath->CountPoints() > 1)
			flags |= SHAPE_UI_FLAGS_CAN_CLOSE_PATH;
		if (fPath->IsClosed())
			flags |= SHAPE_UI_FLAGS_PATH_IS_CLOSED;
		if (fTransformBox)
			flags |= SHAPE_UI_FLAGS_IS_TRANSFORMING;
	}

	return flags;
}

// ReversePath
void
PathManipulator::ReversePath()
{
	if (fPath) {
		int32 count = fSelection->CountItems();
		int32 pointCount = fPath->CountPoints();
		if (count > 0) {
			Selection temp;
			for (int32 i = 0; i < count; i++) {
				temp.Add((pointCount - 1) - fSelection->IndexAt(i));
			}
			*fSelection = temp;
		}
		fPath->Reverse();
	}
}

// _SetMode
void
PathManipulator::_SetMode(uint32 mode)
{
	if (fMode != mode) {
//printf("switching mode: %s -> %s\n", string_for_mode(fMode), string_for_mode(mode));
		fMode = mode;

/*		if (fMode == TRANSFORM_POINTS && fPath) {
			_SetPointSelection(new PointSelection(fCanvasView,
												  this, fShapeStroke,
												  fPath,
												  fSelection->Items(),
												  fSelection->CountItems()));
			_InvalidateCanvasRect(fTransformBox->Bounds(), false);
			fCanvasView->Perform(new EnterTransformPointsAction(this,
														  fShapeStroke,
														  fSelection->Items(),
														  fSelection->CountItems()));
		} else {
			if (fTransformBox) {
				_SetPointSelection(NULL);
			}
		}*/
		if (BWindow* window = fCanvasView->Window()) {
			window->PostMessage(MSG_UPDATE_SHAPE_UI);
		}
		UpdateToolCursor();
	}
}

// _SetPointSelection
void
PathManipulator::_SetPointSelection(PointSelection* selection)
{
	if (fTransformBox != selection) {
		if (fTransformBox) {
			// get rid of transform box display
			_InvalidateCanvasRect(fTransformBox->Bounds(), false);
		}
		delete fTransformBox;
		fTransformBox = selection;

		if (fTransformBox) {
			fTransformBox->MouseMoved(fLastCanvasPos, fCanvasView->ZoomLevel());
			if (fMode != TRANSFORM_POINTS) {
				fMode = TRANSFORM_POINTS;
			}
		} else {
			if (fMode == TRANSFORM_POINTS) {
				_SetModeForMousePos(fLastCanvasPos);
			}
		}
	}
}

// _AddPoint
void
PathManipulator::_AddPoint(Point where)
{
	BRect r(fPath->Bounds());
	BRect cr(_ControlPointRect());

	if (fPath->AddPoint(where.point)) {
		fCurrentPathPoint = fPath->CountPoints() - 1;

		delete fAddPointAction;
//		fAddPointAction = new AddPointAction(this, fPath, fCurrentPathPoint,
//											 fSelection->Items(),
//											 fSelection->CountItems());

		_Select(fCurrentPathPoint, fShiftDown);

		fRenderer->Invalidate(r, cr, true);
	}
}

// scale_point
BPoint
scale_point(BPoint a, BPoint b, float scale)
{
	return BPoint(a.x + (b.x - a.x) * scale,
				  a.y + (b.y - a.y) * scale);
}

// _InsertPoint
void
PathManipulator::_InsertPoint(Point where, int32 index)
{
	// TODO: more acurate path area!
	BRect r(fPath->Bounds());
	BRect cr(_ControlPointRect());

	double scale;

	BPoint point;
	BPoint pointIn;
	BPoint pointOut;

	BPoint previous;
	BPoint previousOut;
	BPoint next;
	BPoint nextIn;

	if (fPath->FindBezierScale(index - 1, where.point, &scale)
		&& scale >= 0.0 && scale <= 1.0
		&& fPath->GetPoint(index - 1, scale, point)) {

		fPath->GetPointAt(index - 1, previous);
		fPath->GetPointOutAt(index - 1, previousOut);
		fPath->GetPointAt(index, next);
		fPath->GetPointInAt(index, nextIn);

		where.point = scale_point(previousOut, nextIn, scale);

		previousOut = scale_point(previous, previousOut, scale);
		nextIn = scale_point(next, nextIn, 1 - scale);
		pointIn = scale_point(previousOut, where.point, scale);
		pointOut = scale_point(nextIn, where.point, 1 - scale);
		
		if (fPath->AddPoint(point, index)) {

			fPath->SetPointIn(index, pointIn);
			fPath->SetPointOut(index, pointOut);

			delete fInsertPointAction;
/*			fInsertPointAction = new InsertPointAction(this, fPath, index,
													   fSelection->Items(),
													   fSelection->CountItems());*/

			fPath->SetPointOut(index - 1, previousOut);
			fPath->SetPointIn(index + 1, nextIn);

			fCurrentPathPoint = index;
			_ShiftSelection(fCurrentPathPoint, 1);
			_Select(fCurrentPathPoint, fShiftDown);
			fRenderer->Invalidate(r, cr, true);
		}
	}
}

// _SetInOutConnected
void
PathManipulator::_SetInOutConnected(int32 index, bool connected)
{
	// TODO: more acurate path area!
	BRect r(fPath->Bounds());
	BRect cr(_ControlPointRect());

	if (fPath->SetInOutConnected(index, connected)) {
		fRenderer->Invalidate(r, cr, true);
	}
}

// _SetSharp
void
PathManipulator::_SetSharp(int32 index)
{
	// TODO: more acurate path area!
	BRect r(fPath->Bounds());
	BRect cr(_ControlPointRect());

	BPoint p;
	fPath->GetPointAt(index, p);
	if (fPath->SetPoint(index, p, p, p, true)) {
		fRenderer->Invalidate(r, cr, true);
	}
}

// _RemoveSelection
void
PathManipulator::_RemoveSelection()
{
	// TODO: more acurate path area!
	BRect r(fPath->Bounds());
	BRect cr(_ControlPointRect());

	int32 count = fSelection->CountItems();
	for (int32 i = 0; i < count; i++) {
		if (!fPath->RemovePoint(fSelection->IndexAt(i) - i))
			break;
	}

	fRenderer->Invalidate(r, cr, true);

	SetClosed(fPath->IsClosed() &&
			  fPath->CountPoints() > 1);

	fSelection->MakeEmpty();
}


// _RemovePoint
void
PathManipulator::_RemovePoint(int32 index)
{
	// TODO: more acurate path area!
	BRect r(fPath->Bounds());
	BRect cr(_ControlPointRect());

	if (fPath->RemovePoint(index)) {
		fRenderer->Invalidate(r, cr, true);
		_Deselect(index);
		_ShiftSelection(index + 1, -1);
	}
}

// _RemovePointIn
void
PathManipulator::_RemovePointIn(int32 index)
{
	// TODO: more acurate path area!
	BRect r(fPath->Bounds());
	BRect cr(_ControlPointRect());

	BPoint p;
	if (fPath->GetPointAt(index, p)) {
		fPath->SetPointIn(index, p);
		fPath->SetInOutConnected(index, false);

		fRenderer->Invalidate(r, cr, true);
	}
}

// _RemovePointOut
void
PathManipulator::_RemovePointOut(int32 index)
{
	// TODO: more acurate path area!
	BRect r(fPath->Bounds());
	BRect cr(_ControlPointRect());

	BPoint p;
	if (fPath->GetPointAt(index, p)) {
		fPath->SetPointOut(index, p);
		fPath->SetInOutConnected(index, false);

		fRenderer->Invalidate(r, cr, true);
	}
}

// _Select
void
PathManipulator::_Select(BRect r)
{
	BPoint p;
	int32 count = fPath->CountPoints();
	Selection temp;
	for (int32 i = 0; i < count && fPath->GetPointAt(i, p); i++) {
		if (r.Contains(p)) {
			temp.Add(i);
		}
	}
	// merge old and new selection
	count = fOldSelection->CountItems();
	for (int32 i = 0; i < count; i++) {
		int32 index = fOldSelection->IndexAt(i);
		if (temp.Contains(index))
			temp.Remove(index);
		else
			temp.Add(index);
	}
	if (temp != *fSelection) {
		*fSelection = temp;
		_UpdateSelection();
	}
}

// _Select
void
PathManipulator::_Select(int32 index, bool extend)
{
	if (!extend)
		fSelection->MakeEmpty();
	if (fSelection->Contains(index))
		fSelection->Remove(index);
	else
		fSelection->Add(index);
	// TODO: this can lead to unnecessary invalidation (maybe need to investigate)
	_UpdateSelection();
}

// _Select
void
PathManipulator::_Select(const int32* indices, int32 count, bool extend)
{
	if (extend) {
		for (int32 i = 0; i < count; i++) {
			if (!fSelection->Contains(indices[i]))
				fSelection->Add(indices[i]);
		}
	} else {
		fSelection->MakeEmpty();
		for (int32 i = 0; i < count; i++) {
			fSelection->Add(indices[i]);
		}
	}
	_UpdateSelection();
}

// _Deselect
void
PathManipulator::_Deselect(int32 index)
{
	if (fSelection->Contains(index)) {
		fSelection->Remove(index);
		_UpdateSelection();
	}
}

// _ShiftSelection
void
PathManipulator::_ShiftSelection(int32 startIndex, int32 direction)
{
	int32 count = fSelection->CountItems();
	if (count > 0) {
		int32* selection = fSelection->Items();
		for (int32 i = 0; i < count; i++) {
			if (selection[i] >= startIndex) {
				selection[i] += direction;
			}
		}
	}
	_UpdateSelection();
}

// _IsSelected
bool
PathManipulator::_IsSelected(int32 index) const
{
	return fSelection->Contains(index);
}

// _ControlPointRect
BRect
PathManipulator::_ControlPointRect() const
{
	BRect r(0.0, 0.0, -1.0, -1.0);
	if (fPath) {
		r = fPath->ControlPointBounds();
		r.InsetBy(-POINT_EXTEND, -POINT_EXTEND);
	}
	return r; 
}

// _ControlPointRect
BRect
PathManipulator::_ControlPointRect(int32 index, uint32 mode) const
{
	BRect rect(0.0, 0.0, -1.0, -1.0);
	if (fPath && index >= 0) {
		BPoint p, pIn, pOut;
		fPath->GetPointsAt(index, p, pIn, pOut);
		switch (mode) {
			case MOVE_POINT:
			case TOGGLE_SHARP:
			case REMOVE_POINT:
			case CLOSE_PATH:
				rect.Set(p.x, p.y, p.x, p.y);
				rect.InsetBy(-POINT_EXTEND, -POINT_EXTEND);
				break;
			case MOVE_POINT_IN:
			case TOGGLE_SHARP_IN:
			case REMOVE_POINT_IN:
				rect.Set(pIn.x, pIn.y, pIn.x, pIn.y);
				rect.InsetBy(-CONTROL_POINT_EXTEND, -CONTROL_POINT_EXTEND);
				break;
			case MOVE_POINT_OUT:
			case TOGGLE_SHARP_OUT:
			case REMOVE_POINT_OUT:
				rect.Set(pOut.x, pOut.y, pOut.x, pOut.y);
				rect.InsetBy(-CONTROL_POINT_EXTEND, -CONTROL_POINT_EXTEND);
				break;
			case SELECT_POINTS:
				rect.Set(min4(p.x, pIn.x, pOut.x, pOut.x),
							min4(p.y, pIn.y, pOut.y, pOut.y),
							max4(p.x, pIn.x, pOut.x, pOut.x),
							max4(p.y, pIn.y, pOut.y, pOut.y));
				rect.InsetBy(-POINT_EXTEND, -POINT_EXTEND);
				break;
		}
	}
	return rect;
}

// _SetModeForMousePos
void
PathManipulator::_SetModeForMousePos(BPoint where)
{
	uint32 mode = UNDEFINED;
	int32 index = -1;

	if (fPath) {
		float zoomLevel = fCanvasView->ZoomLevel();

		// see if we're close enough at a control point
		BPoint point;
		BPoint pointIn;
		BPoint pointOut;
		for (int32 i = 0; fPath->GetPointsAt(i, point, pointIn, pointOut)
						  && mode == UNDEFINED; i++) {

			float distM = dist(point, where) * zoomLevel;
			float distIn = dist(pointIn, where) * zoomLevel;
			float distOut = dist(pointOut, where) * zoomLevel;
			
			if (distM < MOVE_THRESHOLD) {
				if (i == 0 && fClickToClose
					&& !fPath->IsClosed() && fPath->CountPoints() > 1) {
					mode = fCommandDown ? TOGGLE_SHARP :
								(fOptionDown ? REMOVE_POINT : CLOSE_PATH);
					index = i;
				} else {
					mode = fCommandDown ? TOGGLE_SHARP :
								(fOptionDown ? REMOVE_POINT : MOVE_POINT);
					index = i;
				}
			}
			if (distIn < distM && distIn < MOVE_THRESHOLD) {
				mode = fCommandDown ? TOGGLE_SHARP_IN : 
							(fOptionDown ? REMOVE_POINT_IN : MOVE_POINT_IN);
				index = i;
			}
			if (distOut < distIn && distOut < distM && distOut < MOVE_THRESHOLD) {
				mode = fCommandDown ? TOGGLE_SHARP_OUT :
							(fOptionDown ? REMOVE_POINT_OUT : MOVE_POINT_OUT);
				index = i;
			}
		}
		// selection mode overrides any other mode,
		// but we need to check for it after we know
		// the index of the point under the mouse (code above)
		int32 pointCount = fPath->CountPoints();
		if (fShiftDown && pointCount > 0) {
			mode = SELECT_POINTS;
		}

		// see if user wants to start new sub path
		if (fAltDown) {
			mode = NEW_PATH;
			index = -1;
		}

		// see if we're close enough at a line
		if (mode == UNDEFINED) {
			float distance;
			if (fPath->GetDistance(where, &distance, &index)) {
				if (distance < (INSERT_DIST_THRESHOLD / zoomLevel)) {
					mode = INSERT_POINT;
				}
			} else {
				// restore index, since it was changed by call above
				index = fCurrentPathPoint;
			}
		}

		// nope, still undefined mode, last fall back
		if (mode == UNDEFINED) {
			if (fFallBackMode == SELECT_POINTS) {
				if (fPath->IsClosed() && pointCount > 0) {
					mode = SELECT_POINTS;
					index = -1;
				} else {
					mode = ADD_POINT;
					index = pointCount - 1;
				}
			} else {
				// user had clicked "New Path" icon
				mode = fFallBackMode;
			}
		}
	}
	// switch mode if necessary
	if (mode != fMode || index != fCurrentPathPoint) {
		// invalidate path display (to highlight the respective point)
		_InvalidateHighlightPoints(index, mode);
		_SetMode(mode);
		fCurrentPathPoint = index;
	}
}

// _InvalidateHighlightPoints
void
PathManipulator::_InvalidateHighlightPoints(int32 newIndex, uint32 newMode)
{
	BRect oldRect = _ControlPointRect(fCurrentPathPoint, fMode);
	BRect newRect = _ControlPointRect(newIndex, newMode);
	if (oldRect.IsValid())
		_InvalidateCanvasRect(oldRect);
	if (newRect.IsValid())
		_InvalidateCanvasRect(newRect);
}


// _UpdateSelection
void
PathManipulator::_UpdateSelection() const
{
	_InvalidateCanvasRect(_ControlPointRect(), false);
	if (BWindow* window = fCanvasView->Window()) {
		window->PostMessage(MSG_UPDATE_SHAPE_UI);
	}
}

// _Nudge
void
PathManipulator::_Nudge(BPoint direction)
{
	bigtime_t now = system_time();
	if (now - fLastNudgeTime > 500000) {
		_FinishNudging();
	}
	if (fPath) {
		fLastNudgeTime = now;
		fNudgeOffset += direction;

		if (fPath && !fCanvasView->IsBusy()) {
			if (fTransformBox) {
				fTransformBox->NudgeBy(direction);
			} else {
				if (!fNudgeAction) {
	
					bool fromSelection = !fSelection->IsEmpty();
	
					int32 count = fromSelection ? fSelection->CountItems()
												: fPath->CountPoints();
					int32* indices = new int32[count];
					control_point* points = new control_point[count];
	
					// init indices and points
					for (int32 i = 0; i < count; i++) {
						indices[i] = fromSelection ? fSelection->IndexAt(i) : i;
						fPath->GetPointsAt(indices[i],
										   points[i].point,
										   points[i].point_in,
										   points[i].point_out,
										   &points[i].connected);
					}
	
//					fNudgeAction = new NudgePointsAction(this, fPath,
//														 indices, points, count);
//	
//					fNudgeAction->SetNewTranslation(fNudgeOffset);
//					fNudgeAction->Redo(fCanvasView);
	
					delete[] indices;
					delete[] points;
		
				} else {
					fNudgeAction->SetNewTranslation(fNudgeOffset);
					fNudgeAction->Redo(fCanvasView);
				}
	
				if (!fMouseDown)
					_SetModeForMousePos(fLastCanvasPos);
			}
		}
	}
}

// _FinishNudging
void
PathManipulator::_FinishNudging()
{
	fNudgeOffset = BPoint(0.0, 0.0);

	if (fNudgeAction) {
		fCanvasView->Perform(fNudgeAction);
		fNudgeAction = NULL;
	}
	if (fTransformBox)
		fCanvasView->Perform(fTransformBox->FinishNudging());
}

// _Delete
void
PathManipulator::_Delete()
{
	if (fPath && !fMouseDown) {
		// make sure we apply an on-going transformation before we proceed
		if (fTransformBox) {
			fCanvasView->Perform(fTransformBox->Perform());
			_SetPointSelection(NULL);
		}

		if (fSelection->CountItems() == fPath->CountPoints()) {
//			fCanvasView->Perform(new RemovePathAction(this, fPath));
		} else {
//			fCanvasView->Perform(new RemovePointAction(this,
//													   fPath,
//													   fSelection->Items(),
//													   fSelection->CountItems()));
			_RemoveSelection();
//			fShapeStroke->Notify();
		}

		_SetModeForMousePos(fLastCanvasPos);
	}
}

