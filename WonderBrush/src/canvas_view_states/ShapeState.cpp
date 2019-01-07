// ShapeState.cpp

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

#include "ShapeState.h"

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
		{ return (int32)(intptr_t)BList::ItemAt(index); }

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
ShapeState::ShapeState(CanvasView* parent)
	: CanvasViewState(parent),
	  fPrivateHistory(new HistoryManager()),
	  fShapeStroke(NULL),
	  fEditAction(NULL),
	  fCanceling(false),

	  fCommandDown(false),
	  fOptionDown(false),
	  fShiftDown(false),
	  fAltDown(false),

	  fLayer(NULL),
	  fMode(NEW_PATH),
	  fFallBackMode(SELECT_POINTS),
	  fAlpha(255),
	  fOutline(false),
	  fOutlineWidth(1.0),
	  fClosed(false),
	  fCapMode(CAP_MODE_BUTT),
	  fJoinMode(JOIN_MODE_MITER),
	  fFillingRule(FILL_MODE_EVEN_ODD),
	  fSolid(false),

	  fPath(NULL),
	  fPathUnderMouse(NULL),
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
ShapeState::~ShapeState()
{
	delete fPrivateHistory;
	delete fChangePointAction;
	delete fInsertPointAction;
	delete fAddPointAction;
	delete fSelection;
	delete fOldSelection;
	delete fTransformBox;
	delete fNudgeAction;
}

// Init
void
ShapeState::Init(Canvas* canvas, Layer* layer, BPoint lastMousePosition)
{
	CanvasViewState::Init(canvas, layer, lastMousePosition);
	fLayer = layer;
	fCanvas = canvas;

	// just to be sure
	fFallBackMode = SELECT_POINTS;
	_SetMode(NEW_PATH);

	SetClosed(false);
}

// CleanUp
void
ShapeState::CleanUp()
{
	_Perform();
	CanvasViewState::CleanUp();
	fLayer = NULL;
	fCanvas = NULL;
}

// EditModifier
bool
ShapeState::EditModifier(Stroke* modifier)
{
	ShapeStroke* shapeModifier = dynamic_cast<ShapeStroke*>(modifier);
	if (shapeModifier && shapeModifier != fShapeStroke) {
		_SetModifier(shapeModifier, true);
		return true;
	}
	return false;
}

// MouseDown
void
ShapeState::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	CanvasViewState::MouseDown(where, canvasWhere, eraser);

	if (fLayer) {

		if (fMode == TRANSFORM_POINTS) {
			if (fTransformBox) {
				fTransformBox->MouseDown(canvasWhere.point);

				if (!fTransformBox->IsRotating())
					fCanvasView->SetAutoScrolling(true);
			}
			return;
		}

		if (fOutline && !fPrecise) {
			float offset = fmod(fOutlineWidth, 2.0) / 2.0;
			canvasWhere.point += BPoint(offset, offset);
		}


		// start new stroke
		if (!fShapeStroke) {
			ShapeStroke* modifier = new ShapeStroke(fCanvasView->Color());
			if (!fCanvasView->AddStroke(modifier)) {
				delete modifier;
			} else {
				_SetModifier(modifier);

				fCurrentPathPoint = -1;
				fMode = ADD_POINT;
				fFallBackMode = SELECT_POINTS;
				_AddPoint(canvasWhere);
			}
		} else {
			// don't do anything stupid if there is no path
			if (!fPath)
				fMode = NEW_PATH;

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
					fChangePointAction = new ChangePointAction(this,
															   fShapeStroke,
															   fCurrentPathPoint,
															   fSelection->Items(),
															   fSelection->CountItems());
					_Select(fCurrentPathPoint, fShiftDown);
					break;
			}

			// at this point we init doing something
			switch (fMode) {
				case NEW_PATH: {
					fCanvasView->Perform(new NewPathAction(this,
														   fShapeStroke,
														   canvasWhere.point,
														   fSelection->Items(),
														   fSelection->CountItems()));
					// drag the new out control point from here on
					fMode = ADD_POINT;
					fFallBackMode = SELECT_POINTS;
					fCurrentPathPoint = 0;
					_Select(fCurrentPathPoint, fShiftDown);
					break;
				}
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
				case CLOSE_PATH: {
					SetClosed(true, true);
					break;
				}
				case REMOVE_POINT:
					if (fPath->CountPoints() == 1) {
						fCanvasView->Perform(new RemovePathAction(this,
																  fShapeStroke));
					} else {
						fCanvasView->Perform(new RemovePointAction(this,
																   fShapeStroke,
																   fCurrentPathPoint,
																   fSelection->Items(),
																   fSelection->CountItems()));
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
				case SELECT_SUB_PATH:
					fCanvasView->Perform(new SelectPathAction(this,
															  fShapeStroke,
															  fPathUnderMouse,
															  fSelection->Items(),
															  fSelection->CountItems()));
					break;
			}
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
}

// MouseUp
void
ShapeState::MouseUp(BPoint where, Point canvasWhere)
{
	// prevent carrying out actions more than once by only
	// doing it if "fMouseDown" is true at the point of
	// entering this function
	bool mouseDown = fMouseDown;
	CanvasViewState::MouseUp(where, canvasWhere);

	// this next call prevents the need for notification
	// messages being send for each MouseMoved() call
	if (fShapeStroke)
		fShapeStroke->Notify();

	if (fMode == TRANSFORM_POINTS) {
		if (fTransformBox) {
			fCanvasView->Perform(fTransformBox->MouseUp());
		}
		return;
	}

	if (fShapeStroke && mouseDown) {
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
					fCanvasView->Perform(new SelectPointsAction(this, fShapeStroke,
																fOldSelection->Items(),
																fOldSelection->CountItems(),
																fSelection->Items(),
																fSelection->CountItems()));
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
						fCanvasView->Perform(new SelectPointsAction(this, fShapeStroke,
																	fOldSelection->Items(),
																	fOldSelection->CountItems(),
																	fSelection->Items(),
																	fSelection->CountItems()));
					}
				} else {
					_FinishNudging();
				}
				break;
		}
	}
}

// MouseMoved
void
ShapeState::MouseMoved(BPoint where, Point canvasWhere,
					   uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

	// since the tablet is generating mouse moved messages
	// even if only the pressure changes (and not the actual mouse position)
	// we insert this additional check to prevent too much calculation
	if (fMouseDown && fLastCanvasPos == canvasWhere.point)
		return;

	fLastCanvasPos = canvasWhere.point;

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

	if (fMouseDown && fShapeStroke) {

		if (fCanvasView->IsBusy()) {
			if (fMouseDown)
				fCanvasView->EventDropped();
			return;
		}

		BRect r(fShapeStroke->Bounds());
		BRect cr(_ControlPointRect());

		bool redraw = true;

		if (fMode == CLOSE_PATH) {
			// continue by moving the point
			_SetMode(MOVE_POINT);
			delete fChangePointAction;
			fChangePointAction = new ChangePointAction(this,
													   fShapeStroke,
													   fCurrentPathPoint,
													   fSelection->Items(),
													   fSelection->CountItems());
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
			_RedrawStroke(r, cr, true);
	} else {
		// use a subpixel mouse pos for hit testing
		fCanvasView->ConvertToCanvas(where);
		_SetModeForMousePos(where);
	}
}

// ModifierSelectionChanged
void
ShapeState::ModifierSelectionChanged(bool itemsSelected)
{
	_Perform();
}

/*
// FinishLayer
void
ShapeState::FinishLayer(BRect dirtyRect)
{
	if (fShapeStroke && fLayer) {
		// apply current stroke into layer
		_ApplyStroke(fCanvasView->BackBitmap(), fLayer->Bitmap(),
					 dirtyRect, fShapeStroke);
	}
}*/

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
						fSkip = false;
						fDrawingView->SetHighColor(0, 0, 0, 255);

						fCanvasView->ConvertFromCanvas(point);
						fDrawingView->MovePenTo(point);
					}
	virtual	void	LineTo(BPoint point)
					{
						fCanvasView->ConvertFromCanvas(point);
						if (!fSkip) {
							if (fBlack)
								fDrawingView->SetHighColor(255, 255, 255, 255);
							else
								fDrawingView->SetHighColor(0, 0, 0, 255);
							fBlack = !fBlack;

							fDrawingView->StrokeLine(point);
						} else {
							fDrawingView->MovePenTo(point);
						}
						fSkip = !fSkip;
					}

 private:
	CanvasView*		fCanvasView;
	BView*			fDrawingView;
	bool			fBlack;
	bool			fSkip;
};


// Draw
void
ShapeState::Draw(BView* into, BRect updateRect)
{
	if (fShapeStroke) {

		for (int32 i = 0; VectorPath* path = fShapeStroke->PathAt(i); i++) {
			// draw the Bezier curve, but only if editing
			// if not "editing", the path is actually on top all other modifiers
			// TODO: make this customizable in the GUI
			CanvasViewIterator iterator(fCanvasView, into);
			path->Iterate(&iterator, fCanvasView->ZoomLevel()/* / 2.0*/);

			if (path == fPath) {
				into->SetLowColor(0, 0, 0, 255);
				BPoint point;
				BPoint pointIn;
				BPoint pointOut;
				rgb_color focus_color = (rgb_color){ 255, 0, 0, 255 }; //ui_color(B_KEYBOARD_NAVIGATION_COLOR);
				rgb_color highlight_color = (rgb_color){ 60, 60, 255, 255 }; //ui_color(B_KEYBOARD_NAVIGATION_COLOR);
				for (int32 i = 0; path->GetPointsAt(i, point, pointIn, pointOut); i++) {
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
		}
		if (fTransformBox) {
			fTransformBox->Draw(into);
		}
	}
}

// MessageReceived
bool
ShapeState::MessageReceived(BMessage* message)
{
	bool result = true;
	bool sliderAction = message->HasBool("begin");
	switch (message->what) {
		case MSG_SET_COLOR: {
			rgb_color color;
			if (restore_color_from_message(message, color) == B_OK) {
				if (!fCanvasView->IsBusy() || sliderAction)
					SetColor(color, sliderAction, false);
				else
					fCanvasView->EventDropped();
			}
			result = false;
			break;
		}
		case MSG_SET_SHAPE_OPACITY: {
			float value;
			if (message->FindFloat("max value", &value) >= B_OK) {
				if (!fCanvasView->IsBusy() || sliderAction)
					SetAlpha((uint8)floorf(value * 255.0 + 0.5),
							 sliderAction, false);
				else
					fCanvasView->EventDropped();
			}
			break;
		}
		case MSG_SET_SHAPE_OUTLINE: {
			int32 outline;
			if (message->FindInt32("be:value", &outline) == B_OK) {
				SetOutline(outline == B_CONTROL_ON, true, false);
			}
			break;
		}
		case MSG_SET_SHAPE_OUTLINE_WIDTH: {
			float outlineWidth;
			if (message->FindFloat("max value", &outlineWidth) == B_OK) {
				if (!fCanvasView->IsBusy() || sliderAction)
					SetOutlineWidth((outlineWidth * outlineWidth) * 100.0,
									sliderAction, false);
				else
					fCanvasView->EventDropped();
			}
			break;
		}
		case MSG_SET_CAP_MODE: {
			uint32 mode;
			if (message->FindInt32("mode", (int32*)&mode) >= B_OK) {
				SetCapMode(mode, true, false);
			}
			break;
		}
		case MSG_SET_JOIN_MODE: {
			uint32 mode;
			if (message->FindInt32("mode", (int32*)&mode) >= B_OK) {
				SetJoinMode(mode, true, false);
			}
			break;
		}
		case MSG_CONFIRM_TOOL:
			if (fTransformBox) {
				fCanvasView->Perform(fTransformBox->Perform());
				_SetModeForMousePos(fLastCanvasPos);
			} else
				_Perform();
			break;
		case MSG_CANCEL_TOOL:
			if (fTransformBox) {
				fCanvasView->Perform(fTransformBox->Cancel());
				_SetModeForMousePos(fLastCanvasPos);
			} else
				_Cancel();
			break;
		case MSG_SHAPE_TRANSFORM:
			if (fShapeStroke && !fSelection->IsEmpty())
				_SetMode(TRANSFORM_POINTS);
			break;
		case MSG_SHAPE_NEW_PATH:
			if (fShapeStroke && fPath) {
				fFallBackMode = NEW_PATH;
				_SetModeForMousePos(fLastCanvasPos);
			}
			break;
		case MSG_SHAPE_REMOVE_POINTS:
			_Delete();
			break;
		case MSG_SET_SHAPE_CLOSED: {
			int32 closed;
			if (message->FindInt32("be:value", &closed) >= B_OK) {
				SetClosed(closed == B_CONTROL_ON, true);
			}
			break;
		}
		case MSG_REVERSE_PATH:
			if (fPath) {
				fCanvasView->Perform(new ReversePathAction(this, fShapeStroke));
			}
			break;
		case B_SELECT_ALL:
			if (fPath) {
				*fOldSelection = *fSelection;
				fSelection->MakeEmpty();
				int32 count = fPath->CountPoints();
				for (int32 i = 0; i < count; i++)
					fSelection->Add(i);
				if (*fOldSelection != *fSelection) {
					fCanvasView->Perform(new SelectPointsAction(this, fShapeStroke,
																fOldSelection->Items(),
																fOldSelection->CountItems(),
																fSelection->Items(),
																fSelection->CountItems()));
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
ShapeState::ModifiersChanged()
{
	fCommandDown = modifiers() & B_COMMAND_KEY;
	fOptionDown = modifiers() & B_CONTROL_KEY;
	fShiftDown = modifiers() & B_SHIFT_KEY;
	fAltDown = modifiers() & B_OPTION_KEY;

	if (fTransformBox) {
		fTransformBox->ModifiersChanged(modifiers());
		return;
	}
	// reevaluate mode
	if (!fMouseDown)
		_SetModeForMousePos(fLastCanvasPos);
}

// HandleKeyDown
bool
ShapeState::HandleKeyDown(uint32 key, uint32 modifiers)
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
				_Perform();
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
				_Cancel();
			break;
		case 't':
		case 'T':
			if (fShapeStroke && !fSelection->IsEmpty())
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
			if (fShapeStroke && !fSelection->IsEmpty())
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
ShapeState::HandleKeyUp(uint32 key, uint32 modifiers)
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
ShapeState::UpdateToolCursor()
{
	if (fTransformBox) {
		fTransformBox->UpdateToolCursor();
		return;
	}
	const unsigned char* cursorData = kPathNewCursor;
	if (fShapeStroke) {
		switch (fMode) {
			case NEW_PATH:
				cursorData = kPathNewCursor;
				break;
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

// RebuildLayer
bool
ShapeState::RebuildLayer(Layer* layer, BRect area) const
{
	if (fLayer && fShapeStroke && layer == fLayer) {
		_RedrawStroke(area, area, true, true);
		return true;
	}
	return false;
}

// PrepareForObjectPropertyChange
void
ShapeState::PrepareForObjectPropertyChange()
{
	if (fTransformBox) {
		fCanvasView->Perform(fTransformBox->Perform());
		_SetModeForMousePos(fLastCanvasPos);
	} else if (fShapeStroke && fPath) {
		_InvalidateCanvasRect(fPath->ControlPointBounds());
	}
}

// ObjectChanged
void
ShapeState::ObjectChanged(const Observable* object)
{
	const ShapeStroke* shapeObject = dynamic_cast<const ShapeStroke*>(object);
	if (shapeObject && shapeObject == fShapeStroke) {

		// update all controls

		// opacity
		if (fShapeStroke->Alpha() != fAlpha) {

			fAlpha = fShapeStroke->Alpha();
//printf("opacity changed: %d\n", fAlpha);

			if (fCanvasView->Window()) {
				BMessage opacity(MSG_SET_SHAPE_OPACITY);
				opacity.AddFloat("value", fAlpha / 255.0);
				fCanvasView->Window()->PostMessage(&opacity);
			}
		}

		// outline
		if (fShapeStroke->IsOutline() != fOutline) {

			fOutline = fShapeStroke->IsOutline();
//printf("outline changed: %d\n", fOutline);

			if (fCanvasView->Window()) {
				BMessage outline(MSG_SET_SHAPE_OUTLINE);
				outline.AddBool("outline", fOutline);
				fCanvasView->Window()->PostMessage(&outline);
			}
		}

		// outline width
		if (fShapeStroke->OutlineWidth() != fOutlineWidth) {

			fOutlineWidth = fShapeStroke->OutlineWidth();
//printf("outline width changed: %f\n", fOutlineWidth);

			if (fCanvasView->Window()) {
				BMessage outlineWidth(MSG_SET_SHAPE_OUTLINE_WIDTH);
				outlineWidth.AddFloat("value", sqrtf(fOutlineWidth / 100.0));
				fCanvasView->Window()->PostMessage(&outlineWidth);
			}
		}

		// cap mode
		if (fShapeStroke->CapMode() != fCapMode) {

			fCapMode = fShapeStroke->CapMode();
//printf("cap mode changed: %ld\n", fCapMode);

			if (fCanvasView->Window()) {
				BMessage capMode(MSG_SET_CAP_MODE);
				capMode.AddInt32("mode", fCapMode);
				fCanvasView->Window()->PostMessage(&capMode);
			}
		}

		// join mode
		if (fShapeStroke->JoinMode() != fJoinMode) {

			fJoinMode = fShapeStroke->JoinMode();
//printf("join mode changed: %ld\n", fJoinMode);

			if (fCanvasView->Window()) {
				BMessage joinMode(MSG_SET_JOIN_MODE);
				joinMode.AddInt32("mode", fJoinMode);
				fCanvasView->Window()->PostMessage(&joinMode);
			}
		}

		// closed
		if (fShapeStroke->IsClosed() != fClosed) {

			fClosed = fShapeStroke->IsClosed();
//printf("closed changed: %d\n", fClosed);

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
		// filling rule
		if (fFillingRule != fShapeStroke->FillingRule()) {
			fFillingRule = fShapeStroke->FillingRule();
		}
		// solid
		if (fSolid != fShapeStroke->Solid()) {
			fSolid = fShapeStroke->Solid();
		}

		// the current path might have changed
		_SetPath(fShapeStroke->Path());
		if (BWindow* window = fCanvasView->Window()) {
			window->PostMessage(MSG_UPDATE_SHAPE_UI);
		}
	}
}

// StoreNonGUISettings
void
ShapeState::StoreNonGUISettings(BMessage* message) const
{
	if (message) {
		if (message->ReplaceInt32("shape filling rule", fFillingRule) < B_OK)
			message->AddInt32("shape filling rule", fFillingRule);
		if (message->ReplaceBool("shape solid", fSolid) < B_OK)
			message->AddBool("shape solid", fSolid);
	}
}

// RestoreNonGUISettings
void
ShapeState::RestoreNonGUISettings(const BMessage* message)
{
	if (message) {
		if (message->FindInt32("shape filling rule", (int32*)&fFillingRule) < B_OK)
			fFillingRule = FILL_MODE_EVEN_ODD;
		if (message->FindBool("shape solid", &fSolid) < B_OK)
			fSolid = false;
	}
}

// SetSomething
template <class V, class F>
void
ShapeState::SetSomething(V value, V control, F func, bool ignorBusy)
{
	// handle invalidation
	if (fShapeStroke) {
		BRect r(fShapeStroke->Bounds());
		BRect cr(_ControlPointRect());
		(fShapeStroke->*func)(value);
		_RedrawStroke(r, cr, true, ignorBusy);
	}
}

// SetAlpha
void
ShapeState::SetAlpha(uint8 opacity, bool action, bool notify)
{
	if (fAlpha != opacity) {
		if (!fShapeStroke || !notify) {
			fAlpha = opacity;
		}

		if (fShapeStroke) {
			if (action)
				fCanvasView->Perform(new ChangeAlphaAction(this, fShapeStroke));

			SetSomething(opacity, fAlpha, &ShapeStroke::SetAlpha, notify);
		}
	}
}

// SetOutline
void
ShapeState::SetOutline(bool outline, bool action, bool notify)
{
	if (fOutline != outline) {
		if (!fShapeStroke || !notify) {
			fOutline = outline;
		}

		if (fShapeStroke) {
			if (action)
				fCanvasView->Perform(new ChangeOutlineAction(this, fShapeStroke));

			SetSomething(outline, fOutline, &ShapeStroke::SetOutline, notify);
		}
	}
}

// SetOutlineWidth
void
ShapeState::SetOutlineWidth(float width, bool action, bool notify)
{
	if (fOutlineWidth != width) {
		if (!fShapeStroke || !notify) {
			fOutlineWidth = width;
		}

		if (fShapeStroke) {
			if (action)
				fCanvasView->Perform(new ChangeOutlineWidthAction(this, fShapeStroke));

			SetSomething(width, fOutlineWidth, &ShapeStroke::SetOutlineWidth, notify);
		}
	}
}

// SetCapMode
void
ShapeState::SetCapMode(uint32 mode, bool action, bool notify)
{
	if (fCapMode != mode) {
		if (!fShapeStroke || !notify) {
			fCapMode = mode;
		}

		if (fShapeStroke) {
			if (action)
				fCanvasView->Perform(new ChangeCapModeAction(this, fShapeStroke));

			SetSomething(mode, fCapMode, &ShapeStroke::SetCapMode, notify);
		}
	}
}

// SetJoinMode
void
ShapeState::SetJoinMode(uint32 mode, bool action, bool notify)
{
	if (fJoinMode != mode) {
		if (!fShapeStroke || !notify) {
			fJoinMode = mode;
		}

		if (fShapeStroke) {
			if (action)
				fCanvasView->Perform(new ChangeJoinModeAction(this, fShapeStroke));

			SetSomething(mode, fJoinMode, &ShapeStroke::SetJoinMode, notify);
		}
	}
}

// SetColor
void
ShapeState::SetColor(rgb_color color, bool action, bool notify)
{
	if (fColor.red != color.red ||
		fColor.green != color.green ||
		fColor.blue != color.blue) {

		if (!fShapeStroke || !notify) {
			fColor = color;
		}

		if (fShapeStroke) {
			if (action)
				fCanvasView->Perform(new SetShapeColorAction(this, fShapeStroke));

			// handle invalidation
			BRect r(fShapeStroke->Bounds());
			BRect cr(_ControlPointRect());
			fShapeStroke->SetColor(color);
			_RedrawStroke(r, cr, true, notify);

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

// SetClosed
void
ShapeState::SetClosed(bool closed, bool action, bool notify)
{
	bool newClosed = closed;

	if (fShapeStroke) {
		if (closed && fPath->CountPoints() <= 1) {
			notify = true;
			newClosed = false;
			closed = false;
		}
		if (fShapeStroke->IsClosed() != closed) {
			BRect r = fShapeStroke->Bounds();
			BRect cr = _ControlPointRect();
			fShapeStroke->SetClosed(closed);
			_RedrawStroke(r, cr, true, true);

			if (action) {
				fCanvasView->Perform(new CloseAction(this, fShapeStroke));
			}
		}
	} else {
		notify = true;
		newClosed = false;
	}
	fClosed = newClosed;

	if (notify) {
		if (BWindow* window = fCanvasView->Window()) {
			window->PostMessage(MSG_UPDATE_SHAPE_UI);
		}
	}
}

/*
// WidthEnabled
bool
ShapeState::WidthEnabled() const
{
	bool result = false;
	if (fShapeStroke) {
		result = fShapeStroke->IsOutline();
	}
	return result;
}

// TransformEnabled
bool
ShapeState::TransformEnabled() const
{
	bool result = false;
	if (fShapeStroke) {
		result = !fSelection->IsEmpty();
	}
	return result;
}
*/

// ControlFlags
uint32
ShapeState::ControlFlags() const
{
	uint32 flags = 0;
	if (fShapeStroke) {
		flags |= SHAPE_UI_FLAGS_CAN_CREATE_PATH;
		if (fPath && !fSelection->IsEmpty())
			flags |= SHAPE_UI_FLAGS_HAS_SELECTION;
		if (fPath && fPath->CountPoints() > 1)
			flags |= SHAPE_UI_FLAGS_CAN_CLOSE_PATH;
		if (fPath && fPath->IsClosed())
			flags |= SHAPE_UI_FLAGS_PATH_IS_CLOSED;
		if (fTransformBox)
			flags |= SHAPE_UI_FLAGS_IS_TRANSFORMING;
		if (fFallBackMode == NEW_PATH)
			flags |= SHAPE_UI_FLAGS_IS_CREATING_PATH;
		if (fPath)
			flags |= SHAPE_UI_FLAGS_CAN_REVERSE_PATH;
	} else {
		if (fOutline)
			flags |= SHAPE_UI_FLAGS_IS_OUTLINE;
	}
	return flags;
}

// _SetModifier
void
ShapeState::_SetModifier(ShapeStroke* modifier, bool edit)
{
	if (fShapeStroke)
		_Perform();
	if (modifier) {
		fShapeStroke = modifier;
		fShapeStroke->AddObserver(this);
		fSelection->MakeEmpty();
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

		if (edit)
			fEditAction = new EditModifierAction(fLayer, fShapeStroke);
		if (fEditAction) {
			// customize ourself to the shape stroke
			// and trigger the notifications
			ObjectChanged(fShapeStroke);
		} else {
			// customize the modifier
			fShapeStroke->SuspendNotifications(true);

			fShapeStroke->SetAlpha(fAlpha);
			fShapeStroke->SetOutline(fOutline);
			fShapeStroke->SetOutlineWidth(fOutlineWidth);
			fShapeStroke->SetCapMode(fCapMode);
			fShapeStroke->SetJoinMode(fJoinMode);
			fShapeStroke->SetFillingRule(fFillingRule);
			fShapeStroke->SetSolid(fSolid);

			fShapeStroke->SuspendNotifications(false);
		}

		if (fShapeStroke->HasPath(fShapeStroke->Path()))
			_SetPath(fShapeStroke->Path());
		else
			_SetPath(fShapeStroke->PathAt(0));
		_SetConfirmationEnabled(true);
	}
}

// _SetPath
void
ShapeState::_SetPath(VectorPath* path)
{
	if (fShapeStroke && fPath != path) {
		BRect r = _ControlPointRect();
		fPath = path;
		fShapeStroke->SetCurrentPath(path);
		fSelection->MakeEmpty();
		r = r | _ControlPointRect();
		_InvalidateCanvasRect(r, false);
		if (!fMouseDown)
			_SetModeForMousePos(fLastCanvasPos);
	}
}

// _LayerBounds
BRect
ShapeState::_LayerBounds() const
{
	BRect r(0.0, 0.0, -1.0, -1.0);
	if (fLayer) {
		if (BBitmap* bitmap = fLayer->Bitmap())
			r = bitmap->Bounds();
	}
	return r;
}

// _RedrawStroke
void
ShapeState::_RedrawStroke(BRect oldStrokeBounds, BRect oldControlPointsBounds,
						  bool forceStrokeDrawing, bool ignorBusy) const
{
	if (fShapeStroke && fLayer) {
		if (!fCanvasView->IsBusy() || ignorBusy) {

			fCanvasView->_SetBusy(true);

			if (fEditAction) {
				fShapeStroke->UpdateBounds();

				BRect r(fShapeStroke->Bounds());
				fLayer->Touch(r);

				r = r | oldStrokeBounds;

				fCanvasView->RebuildBitmap(r, fLayer);
				_InvalidateCanvasRect(oldControlPointsBounds | _ControlPointRect(), false);
			} else {
				fShapeStroke->UpdateBounds();

				BRect r(fShapeStroke->Bounds());
				fLayer->Touch(r);

				clear_area(fCanvasView->StrokeBitmap(), oldStrokeBounds);
				fShapeStroke->Draw(fCanvasView->StrokeBitmap(), r);

				r = r | oldStrokeBounds;

				if (forceStrokeDrawing)
					_ApplyStroke(fCanvasView->BackBitmap(),
								 fLayer->Bitmap(), r, fShapeStroke);

				_InvalidateCanvasRect(r, true);
				_InvalidateCanvasRect(oldControlPointsBounds | _ControlPointRect(), false);
			}
		} else {
			fCanvasView->EventDropped();
		}
	}
}

// _Perform
void
ShapeState::_Perform()
{
	if (!fCanceling) {
		_SetMode(UNDEFINED);
		fFallBackMode = SELECT_POINTS;
		if (fShapeStroke) {
			fShapeStroke->RemoveObserver(this);
			// prevent adding paths with zero control points
			if (fPath && fPath->CountPoints() == 0) {
				_Cancel();
				return;
			}
			// remove private history from canvas
			// this needs to be done before any action is performed, since
			// we don't want to add these actions to the private history
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
				if (!fCanvasView->Perform(new AddModifierAction(fLayer, fShapeStroke))) {
					delete fShapeStroke;
				} else {
					BRect r(fShapeStroke->Bounds());
					clear_area(fCanvasView->StrokeBitmap(), r);
					fCanvasView->SwitchBuffers(r);
					_InvalidateCanvasRect(r);
				}
			}
			// we will stop drawing control points
			_InvalidateCanvasRect(_ControlPointRect(), false);
			// history needs to be empty
			fPrivateHistory->Clear();
			// unfocus the text stroke item (we're now done editing it)
			HistoryListView* listView = fCanvasView->GetHistoryListView();
			if (listView)
				listView->SetItemFocused(-1);
			// this is not ours anymore
			fShapeStroke = NULL;
			fPath = NULL;
			fPathUnderMouse = NULL;
			_SetConfirmationEnabled(false);
		}
		SetClosed(false);

		UpdateToolCursor();
		_UpdateSelection();
	}
}

// _Cancel
void
ShapeState::_Cancel()
{
	fCanceling = true;
	_SetMode(UNDEFINED);
	fFallBackMode = SELECT_POINTS;
	// clean up
	if (fShapeStroke) {
		fShapeStroke->RemoveObserver(this);
		// unfocus the shape modifier item, remove it
		// remove modifier from layers history
		HistoryListView* listView = fCanvasView->GetHistoryListView();
		History* history = fLayer->GetHistory();
		if (history && listView) {
			listView->SetItemFocused(-1);
			if (fEditAction) {
				// we have only edited this modifier
				// undo all the changes
				BRect r = _ControlPointRect();
				_InvalidateCanvasRect(r, false);
				fEditAction->Undo(fCanvasView);
				delete fEditAction;
				fEditAction = NULL;
			} else {
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
		}
		fShapeStroke = NULL;
		fPath = NULL;
		fPathUnderMouse = NULL;
		if (fCanvas) {
			fCanvas->SetTemporaryHistory(NULL);
			fCanvasView->UpdateHistoryItems();
		}
		fPrivateHistory->Clear();
		fCanvasView->_UpdateNavigatorView();
	}
	_SetConfirmationEnabled(false);

	SetClosed(false);

	UpdateToolCursor();
	_UpdateSelection();
	fCanceling = false;
}

// _SetConfirmationEnabled
void
ShapeState::_SetConfirmationEnabled(bool enable) const
{
	if (MainWindow* window = dynamic_cast<MainWindow*>(fCanvasView->Window()))
		window->SetConfirmationEnabled(enable);
}

// _SetMode
void
ShapeState::_SetMode(uint32 mode)
{
	if (fMode != mode) {
//printf("switching mode: %s -> %s\n", string_for_mode(fMode), string_for_mode(mode));
		fMode = mode;

		if (fMode == TRANSFORM_POINTS && fShapeStroke) {
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
		}
		if (BWindow* window = fCanvasView->Window()) {
			window->PostMessage(MSG_UPDATE_SHAPE_UI);
		}
		UpdateToolCursor();
	}
}

// _SetPointSelection
void
ShapeState::_SetPointSelection(PointSelection* selection)
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
ShapeState::_AddPoint(Point where)
{
	if (fShapeStroke && fPath) {
		BRect r(fShapeStroke->Bounds());
		BRect cr(_ControlPointRect());

		if (fPath->AddPoint(where.point)) {
			fCurrentPathPoint = fPath->CountPoints() - 1;

			delete fAddPointAction;
			fAddPointAction = new AddPointAction(this, fShapeStroke, fCurrentPathPoint,
												 fSelection->Items(),
												 fSelection->CountItems());

			_Select(fCurrentPathPoint, fShiftDown);

			_RedrawStroke(r, cr, true);
		}
	}
}

BPoint
scale_point(BPoint a, BPoint b, float scale)
{
	return BPoint(a.x + (b.x - a.x) * scale,
				  a.y + (b.y - a.y) * scale);
}

// _InsertPoint
void
ShapeState::_InsertPoint(Point where, int32 index)
{
	if (fShapeStroke && fPath) {
		BRect r(fShapeStroke->Bounds());
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
				fInsertPointAction = new InsertPointAction(this, fShapeStroke, index,
														   fSelection->Items(),
														   fSelection->CountItems());

				fPath->SetPointOut(index - 1, previousOut);
				fPath->SetPointIn(index + 1, nextIn);

				fCurrentPathPoint = index;
				_ShiftSelection(fCurrentPathPoint, 1);
				_Select(fCurrentPathPoint, fShiftDown);
				_RedrawStroke(r, cr, true);
			}
		}
	}
}

// _SetInOutConnected
void
ShapeState::_SetInOutConnected(int32 index, bool connected)
{
	if (fShapeStroke && fPath) {
		BRect r(fShapeStroke->Bounds());
		BRect cr(_ControlPointRect());

		if (fPath->SetInOutConnected(index, connected)) {
			_RedrawStroke(r, cr, true);
		}
	}
}

// _SetSharp
void
ShapeState::_SetSharp(int32 index)
{
	if (fShapeStroke && fPath) {
		BRect r(fShapeStroke->Bounds());
		BRect cr(_ControlPointRect());

		BPoint p;
		fPath->GetPointAt(index, p);
		if (fPath->SetPoint(index, p, p, p, true)) {
			_RedrawStroke(r, cr, true);
		}
	}
}

// _RemoveSelection
void
ShapeState::_RemoveSelection()
{
	if (fShapeStroke && fPath) {
		BRect r(fShapeStroke->Bounds());
		BRect cr(_ControlPointRect());

		int32 count = fSelection->CountItems();
		for (int32 i = 0; i < count; i++) {
			if (!fPath->RemovePoint(fSelection->IndexAt(i) - i))
				break;
		}

		_RedrawStroke(r, cr, true);

		SetClosed(fPath->IsClosed() &&
				  fPath->CountPoints() > 1);

		fSelection->MakeEmpty();
	}
}


// _RemovePoint
void
ShapeState::_RemovePoint(int32 index)
{
	if (fShapeStroke && fPath) {
		BRect r(fShapeStroke->Bounds());
		BRect cr(_ControlPointRect());

		if (fPath->RemovePoint(index)) {
			_RedrawStroke(r, cr, true);
			_Deselect(index);
			_ShiftSelection(index + 1, -1);
		}
	}
}

// _RemovePointIn
void
ShapeState::_RemovePointIn(int32 index)
{
	if (fShapeStroke && fPath) {
		BRect r(fShapeStroke->Bounds());
		BRect cr(_ControlPointRect());

		BPoint p;
		if (fPath->GetPointAt(index, p)) {
			fPath->SetPointIn(index, p);
			fPath->SetInOutConnected(index, false);

			_RedrawStroke(r, cr, true);
		}
	}
}

// _RemovePointOut
void
ShapeState::_RemovePointOut(int32 index)
{
	if (fShapeStroke && fPath) {
		BRect r(fShapeStroke->Bounds());
		BRect cr(_ControlPointRect());

		BPoint p;
		if (fPath->GetPointAt(index, p)) {
			fPath->SetPointOut(index, p);
			fPath->SetInOutConnected(index, false);

			_RedrawStroke(r, cr, true);
		}
	}
}

// _ReversePath
void
ShapeState::_ReversePath()
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
		_RedrawStroke(fShapeStroke->Bounds(), _ControlPointRect(), true, true);
	}
}

// _Select
void
ShapeState::_Select(BRect r)
{
	if (fShapeStroke && fPath) {
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
}

// _Select
void
ShapeState::_Select(int32 index, bool extend)
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
ShapeState::_Select(const int32* indices, int32 count, bool extend)
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
ShapeState::_Deselect(int32 index)
{
	if (fSelection->Contains(index)) {
		fSelection->Remove(index);
		_UpdateSelection();
	}
}

// _ShiftSelection
void
ShapeState::_ShiftSelection(int32 startIndex, int32 direction)
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
ShapeState::_IsSelected(int32 index) const
{
	return fSelection->Contains(index);
}

// _ControlPointRect
BRect
ShapeState::_ControlPointRect() const
{
	BRect r(0.0, 0.0, -1.0, -1.0);
	if (fShapeStroke && fPath) {
		r = fPath->ControlPointBounds();
		r = r | fShapeStroke->Bounds();
		r.InsetBy(-POINT_EXTEND, -POINT_EXTEND);
	}
	return r;
}

// _SetModeForMousePos
void
ShapeState::_SetModeForMousePos(BPoint where)
{
	if (fShapeStroke) {

		uint32 mode = UNDEFINED;
		int32 index = -1;
		fPathUnderMouse = NULL;

		if (!fPath) {
			mode = NEW_PATH;
			index = 0;
		} else {
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
					if (i == 0 && !fShapeStroke->IsOutline()
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

			// see if mouse is over another subpath
			if (mode == UNDEFINED) {
				float distance;
				float hitTestDist = INSERT_DIST_THRESHOLD / zoomLevel;
				for (int32 i = 0; VectorPath* path = fShapeStroke->PathAt(i); i++) {
					int32 dummy;
					if (path != fPath &&
						path->Bounds().InsetByCopy(-hitTestDist, -hitTestDist).Contains(where) &&
						path->GetDistance(where, &distance, &dummy) &&
						distance < hitTestDist) {

						mode = SELECT_SUB_PATH;
						fPathUnderMouse = path;
						break;
					}
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
//	_SetMode(NEW_PATH);
}

// _ControlPointRect
BRect
ShapeState::_ControlPointRect(int32 index, uint32 mode) const
{
	BRect rect(0.0, 0.0, -1.0, -1.0);
	if (fShapeStroke && fPath && index >= 0) {
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

// _InvalidateHighlightPoints
void
ShapeState::_InvalidateHighlightPoints(int32 newIndex, uint32 newMode)
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
ShapeState::_UpdateSelection() const
{
	_InvalidateCanvasRect(_ControlPointRect(), false);
	if (BWindow* window = fCanvasView->Window()) {
		window->PostMessage(MSG_UPDATE_SHAPE_UI);
	}
}

// _Nudge
void
ShapeState::_Nudge(BPoint direction)
{
	bigtime_t now = system_time();
	if (now - fLastNudgeTime > 500000) {
		_FinishNudging();
	}
	if (fShapeStroke) {
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

					fNudgeAction = new NudgePointsAction(this, fShapeStroke,
														 indices, points, count);

					fNudgeAction->SetNewTranslation(fNudgeOffset);
					fNudgeAction->Redo(fCanvasView);

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
ShapeState::_FinishNudging()
{
	fNudgeOffset = BPoint(0.0, 0.0);

	if (fNudgeAction) {
		fCanvasView->Perform(fNudgeAction);
		fNudgeAction = NULL;
	}
	if (fTransformBox)
		fCanvasView->Perform(fTransformBox->FinishNudging());

	if (fShapeStroke)
		fShapeStroke->Notify();
}

// _Delete
void
ShapeState::_Delete()
{
	if (fShapeStroke && fPath && !fMouseDown) {
		// make sure we apply an on-going transformation before we proceed
		if (fTransformBox) {
			fCanvasView->Perform(fTransformBox->Perform());
			_SetPointSelection(NULL);
		}

		if (fSelection->CountItems() == fPath->CountPoints()) {
			fCanvasView->Perform(new RemovePathAction(this, fShapeStroke));
		} else {
			fCanvasView->Perform(new RemovePointAction(this,
													   fShapeStroke,
													   fSelection->Items(),
													   fSelection->CountItems()));
			_RemoveSelection();
			fShapeStroke->Notify();
		}

		_SetModeForMousePos(fLastCanvasPos);
	}
}

