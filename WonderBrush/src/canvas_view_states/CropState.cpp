// CropState.cpp

#include <math.h>
#include <stdio.h>

#include <AppDefs.h>
#include <Cursor.h>
#include <Message.h>
#include <Region.h>
#include <Window.h>

#include "cursors.h"

#include "CanvasView.h"
#include "Canvas.h"
#include "ChangeCropRectAction.h"
#include "CropAction.h"
#include "HistoryManager.h"
#include "MainWindow.h"

#include "CropState.h"

// constructor
CropState::CropState(CanvasView* parent)
	: CanvasViewState(parent),
	  fTrackingStart(0.0, 0.0),
	  fCropRect(0.0, 0.0, -1.0, -1.0),
	  fLastCropRect(0.0, 0.0, -1.0, -1.0),
	  fCanvas(NULL),
	  fPrivateHistory(new HistoryManager),
	  fShiftDown(false),
	  fDraggingMode(DRAGGING_NONE)
{
}

// destructor
CropState::~CropState()
{
	delete fPrivateHistory;
}

// Init
void
CropState::Init(Canvas* canvas, Layer* layer, BPoint currentMousePos)
{
	CanvasViewState::Init(canvas, layer, currentMousePos);

	// in any case: reset cropping rect if the canvas is switched
//	if (fCanvas != canvas)
//		_SetRect(BRect(0.0, 0.0, -1.0, -1.0), false, true);

	fCanvas = canvas;
//	if (fCanvas) {
//		_SetRect(fCanvas->Bounds());
//	} else 
	_SetRect(BRect(0.0, 0.0, -1.0, -1.0), false, true);

	fShiftDown = modifiers() & B_SHIFT_KEY;
	fDraggingMode = DRAGGING_NONE;
}

// CleanUp
void
CropState::CleanUp()
{
	CanvasViewState::CleanUp();

	_Cancel();
	fCanvas = NULL;
}

#define EXTENT 20.0

// MouseDown
void
CropState::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	CanvasViewState::MouseDown(where, canvasWhere, eraser);

	fTrackingStart = canvasWhere.point;
	fLeftTopStart = fCropRect.LeftTop();
	fLastCanvasPos = canvasWhere.point;

	if (fCropRect.IsValid()) {
		fDraggingMode = _DragModeFor(where);
		_SetRect(fCropRect, true);
	} else {
		_SetRect(BRect(canvasWhere.point, canvasWhere.point), true);
		fDraggingMode = DRAGGING_NONE; // yet
	}

	// figure out aspect ratio of current crop rect
	if (fCropRect.Width() > 0.0 && fCropRect.Height() > 0.0)
		fAspect = fCropRect.Width() / fCropRect.Height();
	else
		fAspect = -1.0;

	switch (fDraggingMode) {
		case DRAGGING_LEFT_TOP:
			fTrackingOffset = canvasWhere.point - fCropRect.LeftTop();
			break;
		case DRAGGING_RIGHT_TOP:
			fTrackingOffset = canvasWhere.point - fCropRect.RightTop();
			break;
		case DRAGGING_LEFT_BOTTOM:
			fTrackingOffset = canvasWhere.point - fCropRect.LeftBottom();
			break;
		case DRAGGING_RIGHT_BOTTOM:
			fTrackingOffset = canvasWhere.point - fCropRect.RightBottom();
			break;

		case DRAGGING_LEFT_SIDE:
			fTrackingOffset.x = canvasWhere.point.x - fCropRect.left;
			break;
		case DRAGGING_TOP_SIDE:
			fTrackingOffset.y = canvasWhere.point.y - fCropRect.top;
			break;
		case DRAGGING_RIGHT_SIDE:
			fTrackingOffset.x = canvasWhere.point.x - fCropRect.right;
			break;
		case DRAGGING_BOTTOM_SIDE:
			fTrackingOffset.y = canvasWhere.point.y - fCropRect.bottom;
			break;
		case DRAGGING_NONE:
			fTrackingOffset.x = 0.0;
			fTrackingOffset.y = 0.0;
			break;
	}
	
	fCanvasView->SetAutoScrolling(true);
	
	UpdateToolCursor();
}

// MouseUp
void
CropState::MouseUp(BPoint where, Point canvasWhere)
{
	CanvasViewState::MouseUp(where, canvasWhere);

	uint32 mode = _DragModeFor(where);
	if (mode != fDraggingMode) {
		fDraggingMode = mode;
		UpdateToolCursor();
	}
}

// MouseMoved
void
CropState::MouseMoved(BPoint where, Point canvasWhere,
					  uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

	if (fMouseDown) {
		// since the tablet is generating mouse moved messages
		// even if only the pressure changes (and not the actual mouse position)
		// we insert this additional check to prevent too much calculation
		if (fLastCanvasPos != canvasWhere.point) {
			fLastCanvasPos = canvasWhere.point;
//			BRect invalidRect = fCropRect;
			BRect newBounds;
//			BRect oldRect = fCropRect;
//			BRegion invalidRegion;

			if (fDraggingMode != DRAGGING_ALL)
				canvasWhere.point -= fTrackingOffset;

			if (fDraggingMode == DRAGGING_NONE) {
				// if the cropping rect was undefined when the user
				// clicked, the corner the user is dragging is
				// yet unkown and will depend on the direction the mouse
				// is moved. In any case will the user drag a
				// corner, not an egde. To simplify the second if block,
				// the dragging mode is specified to the
				// left/right sides first and then the specific corners
				if (canvasWhere.point.x <=  fCropRect.left) {
					newBounds.left = canvasWhere.point.x;
					newBounds.right = fCropRect.left;
					fDraggingMode = DRAGGING_LEFT_SIDE;
				} else {
					newBounds.left = fCropRect.left;
					newBounds.right = canvasWhere.point.x;
					fDraggingMode = DRAGGING_RIGHT_SIDE;
				}
				if (canvasWhere.point.y <= fCropRect.top) {
					newBounds.top = canvasWhere.point.y;
					newBounds.bottom = fCropRect.top;
					if (fDraggingMode == DRAGGING_LEFT_SIDE)
						_SetDragMode(DRAGGING_LEFT_TOP);
					else
						_SetDragMode(DRAGGING_RIGHT_TOP);
				} else {
					newBounds.top = fCropRect.top;
					newBounds.bottom = canvasWhere.point.y;
					if (fDraggingMode == DRAGGING_LEFT_SIDE)
						_SetDragMode(DRAGGING_LEFT_BOTTOM);
					else
						_SetDragMode(DRAGGING_RIGHT_BOTTOM);
				}
			}

			switch (fDraggingMode) {
				default:
				case DRAGGING_NONE:
					break;
				case DRAGGING_LEFT_TOP:
					newBounds.Set(canvasWhere.point.x,
								  canvasWhere.point.y,
								  fCropRect.right,
								  fCropRect.bottom);
					if (fShiftDown && fAspect > 0.0) {
						// prevent flipping over
						newBounds.left = newBounds.left < newBounds.right ?
										 newBounds.left : newBounds.right;
						newBounds.top = newBounds.top < newBounds.bottom ?
										newBounds.top : newBounds.bottom;
						// adjust rect to keep aspect
						float aspect = newBounds.Width() / newBounds.Height();
						if (aspect < fAspect)
							newBounds.left = newBounds.right - fAspect * newBounds.Height();
						else if (aspect > fAspect)
							newBounds.top = newBounds.bottom - newBounds.Width() / fAspect;
					} else {
						// don't flip over, just change what is being dragged
						if (newBounds.left > newBounds.right) {
							// swap the sides
							newBounds.left = fCropRect.right;
							newBounds.right = canvasWhere.point.x;
							if (newBounds.top > newBounds.bottom) {
								_SetDragMode(DRAGGING_RIGHT_BOTTOM);
								// swap the sides
								newBounds.top = fCropRect.bottom;
								newBounds.bottom = canvasWhere.point.y;
							} else {
								_SetDragMode(DRAGGING_RIGHT_TOP);
							}
						} else {
							if (newBounds.top > newBounds.bottom) {
								_SetDragMode(DRAGGING_LEFT_BOTTOM);
								// swap the sides
								newBounds.top = fCropRect.bottom;
								newBounds.bottom = canvasWhere.point.y;
							}
						}
					}
					break;
				case DRAGGING_RIGHT_TOP:
					newBounds.Set(fCropRect.left,
								  canvasWhere.point.y,
								  canvasWhere.point.x,
								  fCropRect.bottom);
					if (fShiftDown && fAspect > 0.0) {
						// prevent flipping over
						newBounds.top = newBounds.top < newBounds.bottom ?
										newBounds.top : newBounds.bottom;
						newBounds.right = newBounds.right > newBounds.left ?
										  newBounds.right : newBounds.left;
						// adjust rect to keep aspect
						float aspect = newBounds.Width() / newBounds.Height();
						if (aspect < fAspect)
							newBounds.right = newBounds.left + fAspect * newBounds.Height();
						else if (aspect > fAspect)
							newBounds.top = newBounds.bottom - newBounds.Width() / fAspect;
					} else {
						// don't flip over, just change what is being dragged
						if (newBounds.left > newBounds.right) {
							// swap the sides
							newBounds.left = canvasWhere.point.x;
							newBounds.right = fCropRect.left;
							if (newBounds.top > newBounds.bottom) {
								_SetDragMode(DRAGGING_LEFT_BOTTOM);
								// swap the sides
								newBounds.top = fCropRect.bottom;
								newBounds.bottom = canvasWhere.point.y;
							} else {
								_SetDragMode(DRAGGING_LEFT_TOP);
							}
						} else {
							if (newBounds.top > newBounds.bottom) {
								_SetDragMode(DRAGGING_RIGHT_BOTTOM);
								// swap the sides
								newBounds.top = fCropRect.bottom;
								newBounds.bottom = canvasWhere.point.y;
							}
						}
					}
					break;
				case DRAGGING_LEFT_BOTTOM:
					newBounds.Set(canvasWhere.point.x,
								  fCropRect.top,
								  fCropRect.right,
								  canvasWhere.point.y);
					if (fShiftDown && fAspect > 0.0) {
						// prevent flipping over
						newBounds.left = newBounds.left < newBounds.right ?
										 newBounds.left : newBounds.right;
						newBounds.bottom = newBounds.bottom > newBounds.top ?
										   newBounds.bottom : newBounds.top;
						// adjust rect to keep aspect
						float aspect = newBounds.Width() / newBounds.Height();
						if (aspect < fAspect)
							newBounds.left = newBounds.right - fAspect * newBounds.Height();
						else if (aspect > fAspect)
							newBounds.bottom = newBounds.top + newBounds.Width() / fAspect;
					} else {
						// don't flip over, just change what is being dragged
						if (newBounds.left > newBounds.right) {
							// swap the sides
							newBounds.left = fCropRect.right;
							newBounds.right = canvasWhere.point.x;
							if (newBounds.top > newBounds.bottom) {
								_SetDragMode(DRAGGING_RIGHT_TOP);
								// swap the sides
								newBounds.top = canvasWhere.point.y;
								newBounds.bottom = fCropRect.top;
							} else {
								_SetDragMode(DRAGGING_RIGHT_BOTTOM);
							}
						} else {
							if (newBounds.top > newBounds.bottom) {
								_SetDragMode(DRAGGING_LEFT_TOP);
								// swap the sides
								newBounds.top = canvasWhere.point.y;
								newBounds.bottom = fCropRect.top;
							}
						}
					}
					break;
				case DRAGGING_RIGHT_BOTTOM:
					newBounds.Set(fCropRect.left,
								  fCropRect.top,
								  canvasWhere.point.x,
								  canvasWhere.point.y);
					if (fShiftDown && fAspect > 0.0) {
						// prevent flipping over
						newBounds.right = newBounds.right > newBounds.left ?
										  newBounds.right : newBounds.left;
						newBounds.bottom = newBounds.bottom > newBounds.top ?
										   newBounds.bottom : newBounds.top;
						// adjust rect to keep aspect
						float aspect = newBounds.Width() / newBounds.Height();
						if (aspect < fAspect)
							newBounds.right = newBounds.left + fAspect * newBounds.Height();
						else if (aspect > fAspect)
							newBounds.bottom = newBounds.top + newBounds.Width() / fAspect;
					} else {
						// don't flip over, just change what is being dragged
						if (newBounds.left > newBounds.right) {
							// swap the sides
							newBounds.left = canvasWhere.point.x;
							newBounds.right = fCropRect.left;
							if (newBounds.top > newBounds.bottom) {
								_SetDragMode(DRAGGING_LEFT_TOP);
								// swap the sides
								newBounds.top = canvasWhere.point.y;
								newBounds.bottom = fCropRect.top;
							} else {
								_SetDragMode(DRAGGING_LEFT_BOTTOM);
							}
						} else {
							if (newBounds.top > newBounds.bottom) {
								_SetDragMode(DRAGGING_RIGHT_TOP);
								// swap the sides
								newBounds.top = canvasWhere.point.y;
								newBounds.bottom = fCropRect.top;
							}
						}
					}
					break;
				case DRAGGING_LEFT_SIDE:
					newBounds.Set(canvasWhere.point.x,
								  fCropRect.top,
								  fCropRect.right,
								  fCropRect.bottom);
					// don't flip over, just change what is being dragged
					if (newBounds.left > newBounds.right) {
						newBounds.left = fCropRect.right;
						newBounds.right = canvasWhere.point.x;
						// swap the sides
						_SetDragMode(DRAGGING_RIGHT_SIDE);
					}
					if (fShiftDown && fAspect > 0.0) {
						float aspect = newBounds.Width() / newBounds.Height();
						if (aspect != fAspect) {
							float middle = (newBounds.top + newBounds.bottom) / 2.0;
							float height = newBounds.Width() / fAspect;
							newBounds.top = middle - height / 2.0;
							newBounds.bottom = newBounds.top + height;
						}
					}
					break;
				case DRAGGING_TOP_SIDE:
					newBounds.Set(fCropRect.left,
								  canvasWhere.point.y,
								  fCropRect.right,
								  fCropRect.bottom);
					// don't flip over, just change what is being dragged
					if (newBounds.top > newBounds.bottom) {
						newBounds.top = fCropRect.bottom;
						newBounds.bottom = canvasWhere.point.y;
						// swap the sides
						_SetDragMode(DRAGGING_BOTTOM_SIDE);
					}
					if (fShiftDown && fAspect > 0.0) {
						float aspect = newBounds.Width() / newBounds.Height();
						if (aspect != fAspect) {
							float middle = (newBounds.left + newBounds.right) / 2.0;
							float width = fAspect * newBounds.Height();
							newBounds.left = middle - width / 2.0;
							newBounds.right = newBounds.left + width;
						}
					}
					break;
				case DRAGGING_RIGHT_SIDE:
					newBounds.Set(fCropRect.left,
								  fCropRect.top,
								  canvasWhere.point.x,
								  fCropRect.bottom);
					// don't flip over, just change what is being dragged
					if (newBounds.left > newBounds.right) {
						newBounds.left = canvasWhere.point.x;
						newBounds.right = fCropRect.left;
						// swap the sides
						_SetDragMode(DRAGGING_LEFT_SIDE);
					}
					if (fShiftDown && fAspect > 0.0) {
						float aspect = newBounds.Width() / newBounds.Height();
						if (aspect != fAspect) {
							float middle = (newBounds.top + newBounds.bottom) / 2.0;
							float height = newBounds.Width() / fAspect;
							newBounds.top = middle - height / 2.0;
							newBounds.bottom = newBounds.top + height;
						}
					}
					break;
				case DRAGGING_BOTTOM_SIDE:
					newBounds.Set(fCropRect.left,
								  fCropRect.top,
								  fCropRect.right,
								  canvasWhere.point.y);
					// don't flip over, just change what is being dragged
					if (newBounds.top > newBounds.bottom) {
						newBounds.top = canvasWhere.point.y;
						newBounds.bottom = fCropRect.top;
						// swap the sides
						_SetDragMode(DRAGGING_TOP_SIDE);
					}
					if (fShiftDown && fAspect > 0.0) {
						float aspect = newBounds.Width() / newBounds.Height();
						if (aspect != fAspect) {
							float middle = (newBounds.left + newBounds.right) / 2.0;
							float width = fAspect * newBounds.Height();
							newBounds.left = middle - width / 2.0;
							newBounds.right = newBounds.left + width;
						}
					}
					break;
				case DRAGGING_ALL:
					newBounds = fCropRect;
					newBounds.OffsetTo(fLeftTopStart + (canvasWhere.point - fTrackingStart));
					break;
			}
/*			if (oldRect != fCropRect) {
//				invalidRegion.Include(_ViewRect(fCropRect));
//				invalidRegion.Exclude(_ViewRect(oldRect & fCropRect));
				int32 count = invalidRegion.CountRects();
				for (int32 i = 0; i < count; i++) {
					fCanvasView->Invalidate(invalidRegion.RectAt(i));
				}
			}*/
			if (newBounds.IsValid())
				_SetRect(newBounds);
/*			if (invalidRect.IsValid()) {
				invalidRect = invalidRect | fCropRect;
				_InvalidateCanvasView(invalidRect);
			}*/
		}
	} else {
		_SetDragMode(_DragModeFor(where));
	}
}

// Draw
void
CropState::Draw(BView* into, BRect updateRect)
{
	if (fCanvas && fCropRect.IsValid()) {
		BRect viewCropRect = _ViewRect(fCropRect);

//		into->SetDrawingMode(B_OP_ALPHA);
//		into->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);
		into->SetHighColor(0, 0, 0, 100);
/*
		BRect t(updateRect.left,
				updateRect.top,
				updateRect.right,
				viewCropRect.top - 1.0);
		if (t.IsValid())
			into->FillRect(t);

		BRect l(updateRect.left,
				viewCropRect.top,
				viewCropRect.left - 1.0,
				viewCropRect.bottom);
		if (l.IsValid())
			into->FillRect(l);

		BRect r(viewCropRect.right + 1.0,
				viewCropRect.top,
				updateRect.right,
				viewCropRect.bottom);
		if (r.IsValid())
			into->FillRect(r);

		BRect b(updateRect.left,
				viewCropRect.bottom + 1.0,
				updateRect.right,
				updateRect.bottom);
		if (b.IsValid())
			into->FillRect(b);
*/
		into->SetDrawingMode(B_OP_COPY);
		into->StrokeRect(viewCropRect);
		viewCropRect.InsetBy(-1.0, -1.0);
		into->SetHighColor(255, 255, 255, 255);
		into->StrokeRect(viewCropRect);
		viewCropRect.InsetBy(1.0, 1.0);
		_DrawDragPoint(into, BRect(viewCropRect.left - 6.0, viewCropRect.top - 6.0,
								   viewCropRect.left, viewCropRect.top));
		_DrawDragPoint(into, BRect(viewCropRect.right, viewCropRect.top - 6.0,
								   viewCropRect.right + 6.0, viewCropRect.top));
		_DrawDragPoint(into, BRect(viewCropRect.right, viewCropRect.bottom,
								   viewCropRect.right + 6.0, viewCropRect.bottom + 6.0));
		_DrawDragPoint(into, BRect(viewCropRect.left - 6.0, viewCropRect.bottom,
								   viewCropRect.left, viewCropRect.bottom + 6.0));
	}
}

// ModifyClippingRegion
void
CropState::ModifyClippingRegion(BView* view, BRegion* region)
{

	if (fCanvas && fCropRect.IsValid()) {
		BRect viewCropRect = _ViewRect(fCropRect);
	
		viewCropRect.InsetBy(-1.0, -1.0);
		region->Exclude(viewCropRect);

		viewCropRect.InsetBy(2.0, 2.0);
		region->Include(viewCropRect);

		viewCropRect.InsetBy(-1.0, -1.0);

		region->Exclude(BRect(viewCropRect.left - 6.0, viewCropRect.top - 6.0,
							  viewCropRect.left, viewCropRect.top));
		region->Exclude(BRect(viewCropRect.right, viewCropRect.top - 6.0,
							  viewCropRect.right + 6.0, viewCropRect.top));
		region->Exclude(BRect(viewCropRect.right, viewCropRect.bottom,
							  viewCropRect.right + 6.0, viewCropRect.bottom + 6.0));
		region->Exclude(BRect(viewCropRect.left - 6.0, viewCropRect.bottom,
							  viewCropRect.left, viewCropRect.bottom + 6.0));

		region->Include(BRect(viewCropRect.left - 4.0, viewCropRect.top - 4.0,
							  viewCropRect.left - 2.0, viewCropRect.top - 2.0));
		region->Include(BRect(viewCropRect.right + 2.0, viewCropRect.top - 4.0,
							  viewCropRect.right + 4.0, viewCropRect.top - 2.0));
		region->Include(BRect(viewCropRect.right + 2.0, viewCropRect.bottom + 2.0,
							  viewCropRect.right + 4.0, viewCropRect.bottom + 4.0));
		region->Include(BRect(viewCropRect.left - 4.0, viewCropRect.bottom + 2.0,
							  viewCropRect.left - 2.0, viewCropRect.bottom + 4.0));

		view->ConstrainClippingRegion(region);
	}
}

// MessageReceived
bool
CropState::MessageReceived(BMessage* message)
{
	bool handled = true;
	switch (message->what) {
		case MSG_CROPPING: {
			BRect rect;
			if (message->FindRect("rect", &rect) >= B_OK) {
				_SetRect(rect, true);
			}
			break;
		}
		case MSG_SUB_PIXEL_PRECISION:
			_SetRect(BRect(floorf(fCropRect.left + 0.5),
						   floorf(fCropRect.top + 0.5),
						   floorf(fCropRect.right + 0.5),
						   floorf(fCropRect.bottom + 0.5)), true);
			// do not return true, since CanvasView needs to handle this message itself
			handled = false;
			break;
		case MSG_CROP_ENTIRE_CANVAS:
			if (fCanvas && fCanvas->Bounds() != fCropRect)
				_SetRect(fCanvas->Bounds(), true);
			break;
		case MSG_CONFIRM_TOOL:
			_Perform(fCropRect);
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
CropState::ModifiersChanged()
{
	fShiftDown = modifiers() & B_SHIFT_KEY;
	if (fMouseDown) {
		// fake a mouse moved event to force visual update
		BPoint where = fLastCanvasPos;
		fCanvasView->ConvertFromCanvas(where);
		Point canvasWhere(fLastCanvasPos, 1.0);
		fLastCanvasPos.x--;	// make it look like the mouse actually moved
		MouseMoved(where, canvasWhere, B_INSIDE_VIEW, NULL);
	}
}

// HandleKeyDown
bool
CropState::HandleKeyDown(uint32 key, uint32 modifiers)
{
	bool handled = true;
	switch (key) {
		case B_RETURN:
			_Perform(fCropRect);
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
CropState::UpdateToolCursor()
{
	const uchar* cursorData = NULL;
	switch (fDraggingMode) {
		case DRAGGING_LEFT_TOP:
		case DRAGGING_RIGHT_BOTTOM:
			cursorData = kLeftTopRightBottomCursor;
			break;
		case DRAGGING_RIGHT_TOP:
		case DRAGGING_LEFT_BOTTOM:
			cursorData = kLeftBottomRightTopCursor;
			break;
		case DRAGGING_LEFT_SIDE:
		case DRAGGING_RIGHT_SIDE:
			cursorData = kLeftRightCursor;
			break;
		case DRAGGING_TOP_SIDE:
		case DRAGGING_BOTTOM_SIDE:
			cursorData = kUpDownCursor;
			break;
		case DRAGGING_ALL:
			cursorData = kMoveCursor;
			break;
	}

	if (cursorData) {
		BCursor cursor(cursorData);
		fCanvasView->SetViewCursor(&cursor, true);
	} else
		fCanvasView->SetViewCursor(B_CURSOR_SYSTEM_DEFAULT, true);
}

// _SetRect
void
CropState::_SetRect(BRect newBounds, bool action, bool notify)
{
	if (!fPrecise) {
		newBounds.left = floorf(newBounds.left + 0.5);
		newBounds.top = floorf(newBounds.top + 0.5);
		newBounds.right = floorf(newBounds.right + 0.5);
		newBounds.bottom = floorf(newBounds.bottom + 0.5);
	}
	BRect oldCropRect = fCropRect;
	if (fCropRect != newBounds) {
		// calculate the area that needs to get invalidated
		if (newBounds.IsValid() && fCropRect.IsValid())
			_InvalidateCanvasView(fCropRect | newBounds);
		else if (newBounds.IsValid() || fCropRect.IsValid())
			fCanvasView->Invalidate();

		if (!newBounds.IsValid() && fCropRect.IsValid())
			fLastCropRect = fCropRect;

		// init private undo stack
		if (newBounds.IsValid()) {
			_SetConfirmationEnabled(true);
			if (fCanvas)
				fCanvas->SetTemporaryHistory(fPrivateHistory);
		}

		fCropRect = newBounds;
		notify = true;
	}
	// create action if necessary
	if (action) {
		fCanvasView->Perform(new ChangeCropRectAction(this, oldCropRect));
	}
	if (notify) {
		// send message
		if (BWindow* window = fCanvasView->Window()) {
			BMessage message(MSG_CROPPING);
			message.AddRect("rect", fCropRect);
			window->PostMessage(&message);
		}
	}
}

// _Perform
void
CropState::_Perform(BRect newBounds)
{
	_SetRect(BRect(0.0, 0.0, -1.0, -1.0));

	if (fCanvas)
		fCanvas->SetTemporaryHistory(NULL);
	fPrivateHistory->Clear();

	_SetConfirmationEnabled(false);

	fCanvasView->Perform(new CropAction(fCanvas, newBounds));
	// just in case newBounds was invalid
	fCanvasView->UpdateHistoryItems();
}

// _Cancel
void
CropState::_Cancel()
{
	if (fCanvas)
		fCanvas->SetTemporaryHistory(NULL);
	fPrivateHistory->Clear();

	_SetConfirmationEnabled(false);
	fCanvasView->UpdateHistoryItems();

	_SetRect(BRect(0.0, 0.0, -1.0, -1.0));
}

// _SetConfirmationEnabled
void
CropState::_SetConfirmationEnabled(bool enable) const
{
	if (MainWindow* window = dynamic_cast<MainWindow*>(fCanvasView->Window()))
		window->SetConfirmationEnabled(enable);
}

// _DrawDragPoint
void
CropState::_DrawDragPoint(BView* into, BRect r) const
{
	into->SetHighColor(0, 0, 0, 255);
	into->StrokeRect(r);
	r.InsetBy(1.0, 1.0);
	into->SetHighColor(255, 255, 255, 255);
	into->FillRect(r);
}

// _InvalidateCanvasView
void
CropState::_InvalidateCanvasView(BRect r) const
{
	r = _ViewRect(r);
	r.InsetBy(-6.0, -6.0);
	fCanvasView->Invalidate(r);
}

// _ViewRect
BRect
CropState::_ViewRect(BRect r) const
{
	// figure out where crop rect lies within view coordinates
	BPoint lt = r.LeftTop();
	BPoint rb = r.RightBottom();
	// take the coordinate of the next canvas x/y
	rb.x++;
	rb.y++;
	fCanvasView->ConvertFromCanvas(lt);
	fCanvasView->ConvertFromCanvas(rb);
	// after converting to view coordinates
	// move left/top border to the next pixel/column
	// before canvas cropping rect
	// the right coordinate is already on the desired position
	lt.x--;
	lt.y--;

	r.Set(floorf(lt.x), floorf(lt.y),
		  ceilf(rb.x), ceilf(rb.y));
	return r;
}

// _DragModeFor
uint32
CropState::_DragModeFor(BPoint where)
{
	if (!fCropRect.IsValid())
		return DRAGGING_NONE;

	uint32 mode = DRAGGING_NONE;

	BRect r = _ViewRect(fCropRect);

/*	BRect lt(r.LeftTop(), r.LeftTop());
	BRect rt(r.RightTop(), r.RightTop());
	BRect lb(r.LeftBottom(), r.LeftBottom());
	BRect rb(r.RightBottom(), r.RightBottom());

	lt.InsetBy(-EXTENT, -EXTENT);
	lt.OffsetBy(-EXTENT, -EXTENT);

	rt.InsetBy(-EXTENT, -EXTENT);
	rt.OffsetBy(EXTENT, -EXTENT);

	lb.InsetBy(-EXTENT, -EXTENT);
	lb.OffsetBy(-EXTENT, EXTENT);

	rb.InsetBy(-EXTENT, -EXTENT);
	rb.OffsetBy(EXTENT, EXTENT);

	if (lt.Contains(where)) {
		mode = DRAGGING_LEFT_TOP;
	} else if (rt.Contains(where)) {
		mode = DRAGGING_RIGHT_TOP;
	} else if (lb.Contains(where)) {
		mode = DRAGGING_LEFT_BOTTOM;
	} else if (rb.Contains(where)) {
		mode = DRAGGING_RIGHT_BOTTOM;
	} else {
		BRect ls(lt.LeftBottom(), lb.RightTop());
		BRect ts(lt.RightTop(), rt.LeftBottom());
		BRect rs(rt.LeftBottom(), rb.RightTop());
		BRect bs(lb.RightTop(), rb.LeftBottom());
		if (ls.Contains(where)) {
			mode = DRAGGING_LEFT_SIDE;
		} else if (ts.Contains(where)) {
			mode = DRAGGING_TOP_SIDE;
		} else if (rs.Contains(where)) {
			mode = DRAGGING_RIGHT_SIDE;
		} else if (bs.Contains(where)) {
			mode = DRAGGING_BOTTOM_SIDE;
		} else {
			BRect i(lt.RightBottom(), rb.LeftTop());
			if (i.Contains(where))
				mode = DRAGGING_ALL;
			else
				mode = DRAGGING_NONE;
		}
	}*/
	if (r.Contains(where)) {
		mode = DRAGGING_ALL;
	} else {
		// we're not decided yet, we approach the matter step by step
		if (r.top > where.y)
			mode = DRAGGING_TOP_SIDE;
		else if (r.bottom < where.y)
			mode = DRAGGING_BOTTOM_SIDE;
		else if (r.left > where.x)
			mode = DRAGGING_LEFT_SIDE;
		else if (r.right < where.x)
			mode = DRAGGING_RIGHT_SIDE;
		switch (mode) {
			case DRAGGING_TOP_SIDE:
				if (r.left > where.x)
					mode = DRAGGING_LEFT_TOP;
				else if (r.right < where.x)
					mode = DRAGGING_RIGHT_TOP;
				break;
			case DRAGGING_BOTTOM_SIDE:
				if (r.left > where.x)
					mode = DRAGGING_LEFT_BOTTOM;
				else if (r.right < where.x)
					mode = DRAGGING_RIGHT_BOTTOM;
				break;
			case DRAGGING_LEFT_SIDE:
				if (r.top > where.y)
					mode = DRAGGING_LEFT_TOP;
				else if (r.bottom < where.y)
					mode = DRAGGING_LEFT_BOTTOM;
				break;
			case DRAGGING_RIGHT_SIDE:
				if (r.top > where.y)
					mode = DRAGGING_RIGHT_TOP;
				else if (r.bottom < where.y)
					mode = DRAGGING_RIGHT_BOTTOM;
				break;
		}
	}
	return mode;
}

// _SetDragMode
void
CropState::_SetDragMode(uint32 mode)
{
	if (mode != fDraggingMode) {
		fDraggingMode = mode;
		UpdateToolCursor();
	}
}
