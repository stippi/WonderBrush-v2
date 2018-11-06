// StrokeState.cpp

//#include <math.h>
#include <stdio.h>

#include <Bitmap.h>
#include <Cursor.h>

#include "bitmap_support.h"
#include "cursors.h"
#include "defines.h"

#include "Brush.h"
#include "BlurStroke.h"
#include "BrushStroke.h"
#include "CanvasView.h"
#include "CloneStroke.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"
#include "PenStroke.h"
#include "Stroke.h"

#include "AddModifierAction.h"

#include "StrokeState.h"

enum {
	CLONE_NEEDS_OFFSET			= 0,
	CLONE_NEEDS_FIRST_POINT		= 1,
	CLONE_HAS_OFFSET			= 2,
};

// constructor
StrokeState::StrokeState(CanvasView* parent)
	: CanvasViewState(parent),
	  fCurrentStroke(NULL),
	  fLayer(NULL),
	  fEraser(false),
	  fMode(MODE_BRUSH),
	  fCloneStep(CLONE_NEEDS_OFFSET),
	  fCloneOffset(-1.0, -1.0),
	  fCursorShowing(false)
{
	// init cursor
	fCursor.position = BPoint(-1.0, -1.0);
	fCursor.mouse = BPoint(-1.0, -1.0);;
	fCursor.tilt_x = 0.0;
	fCursor.tilt_y = 0.0;
	fCursor.radius = fCanvasView->BrushRadius().max;
}

// destructor
StrokeState::~StrokeState()
{
}

// Init
void
StrokeState::Init(Canvas* canvas, Layer* layer, BPoint lastMousePosition)
{
	CanvasViewState::Init(canvas, layer, lastMousePosition);
	fLayer = layer;
}

// CleanUp
void
StrokeState::CleanUp()
{
	CanvasViewState::CleanUp();
	fLayer = NULL;
}

// SetTool
void
StrokeState::SetTool(uint32 tool)
{
	// translate tool IDs into modes of the stroke tool
	uint32 mode;
	switch (tool) {
		case TOOL_BRUSH:
			mode = MODE_BRUSH;
			break;
		case TOOL_CLONE:
			mode = MODE_CLONE;
			break;
		case TOOL_BLUR:
			mode = MODE_BLUR;
			break;
		case TOOL_PEN:
			mode = MODE_PEN;
			break;
		case TOOL_PEN_ERASER:
			mode = MODE_PEN_ERASER;
			break;
		case TOOL_ERASER:
			mode = MODE_ERASER;
			break;
		// do nothing in case of unknown tool id
		default:
			mode = fMode;
	}
	if (fMode != mode) {
		fMode = mode;
		if (mode == MODE_CLONE)
			fCloneStep = CLONE_NEEDS_OFFSET;
		UpdateToolCursor();
	}
}

// MouseDown
void
StrokeState::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	CanvasViewState::MouseDown(where, canvasWhere, eraser);

	DrawCursor(where);

	if (fLayer) {
		if (fMode == MODE_CLONE) {
			if (fCloneStep == CLONE_NEEDS_OFFSET) {
				fCloneOffset = canvasWhere.point;
				fCloneStep = CLONE_NEEDS_FIRST_POINT;
				return;
			} else if (fCloneStep == CLONE_NEEDS_FIRST_POINT) {
				fCloneOffset = canvasWhere.point - fCloneOffset;
				fCloneStep = CLONE_HAS_OFFSET;
			}
		}
	
		// start new stroke
		fCurrentStroke = _NewStroke(eraser);
		if (!fCanvasView->AddStroke(fCurrentStroke)) {
			delete fCurrentStroke;
			fCurrentStroke = NULL;
		}
		if (fCurrentStroke) {
			if (modifiers() & B_SHIFT_KEY) {
				// add last point of previous stroke as first point
				Stroke* previousStroke = NULL;
				if (History* history = fLayer->GetHistory()) {
					int32 count = history->CountModifiers();
					previousStroke = history->ModifierAt(count - 2);
				}
				if (previousStroke) {
					Point point;
					if (previousStroke->GetLastPoint(point))
						fCurrentStroke->AddPoint(point);
				}
			}
			if (fCurrentStroke->AddPoint(canvasWhere))
				_StrokeLastLine();
		}
	}
}

// MouseUp
void
StrokeState::MouseUp(BPoint where, Point canvasWhere)
{
	CanvasViewState::MouseUp(where, canvasWhere);

	if (fCurrentStroke) {
		if (fCurrentStroke->Bounds().Intersects(_LayerBounds())) {
			// let the stroke build its cache
			fCurrentStroke->Finish(fCanvasView->StrokeBitmap());
			// clean up after stroke
			clear_area(fCanvasView->StrokeBitmap(), fCurrentStroke->Bounds());
			// switch buffers
			fCanvasView->SwitchBuffers(fCurrentStroke->Bounds());
			fCanvasView->Perform(new AddModifierAction(fLayer, fCurrentStroke));
		} else {
			// get rid of useless (off bounds) stroke
			if (fLayer) {
				if (History* history = fLayer->GetHistory())
					history->RemoveModifier(fCurrentStroke);
			}
			if (HistoryListView* listView = fCanvasView->GetHistoryListView())
				listView->RemoveModifier();
			delete fCurrentStroke;
		}
		fCurrentStroke = NULL;
	}
	UpdateToolCursor();
	DrawCursor(where);
}

// MouseMoved
void
StrokeState::MouseMoved(BPoint where, Point canvasWhere,
						uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

	if (transit == B_EXITED_VIEW)
		fCursor.position = BPoint(-1.0, -1.0);

	fCursor.tilt_x = canvasWhere.tiltX;
	fCursor.tilt_y = canvasWhere.tiltY;

	// drawing
	if (fMouseDown) {
		if (fCurrentStroke) {
			if (!fCanvasView->IsBusy()) {
				fCanvasView->_SetBusy(true);
				if (modifiers() & B_SHIFT_KEY) {
					if (fCurrentStroke->SetLastPoint(canvasWhere.point, canvasWhere.pressure)) {
						BRect r = fCurrentStroke->Bounds();
						// clear previous stroke
						clear_area(fCanvasView->StrokeBitmap(), r);
						// canvas needs updating of the old stroke bounds
						fCanvasView->_InvalidateCanvas(r, true);
						fCurrentStroke->Reset();
						// do new stroke
						_StrokeLastLine();
					}
				} else {
					if (fCurrentStroke->AddPoint(canvasWhere))
						_StrokeLastLine();
				}
			} else {
				fCanvasView->EventDropped();
			}
		}
	}
}

// FinishLayer
void
StrokeState::FinishLayer(BRect dirtyRect)
{
	if (fCurrentStroke && fLayer) {
		// apply current stroke into layer
		_ApplyStroke(fCanvasView->BackBitmap(), fLayer->Bitmap(),
					 dirtyRect, fCurrentStroke);
	}
}
/*
// draw_ellipse
void
draw_ellipse(int32 x, int32 y, int32 w, int32 h, bool antialias)
{
	int32 i, j;
	int32 x0, x1, x2;
	int32 val, last;
	float a_sqr, b_sqr, aob_sqr;
	float w_sqr, h_sqr;
	float y_sqr;
	float t0, t1;
	float r;
	float cx, cy;
	float rad;
	float dist;

	if (!w || !h)
		return;

	a_sqr = (w * w / 4.0);
	b_sqr = (h * h / 4.0);
	aob_sqr = a_sqr / b_sqr;

	cx = x + w / 2.0;
	cy = y + h / 2.0;

	for (i = y; i < (y + h); i++) {
		if (i >= 0 && i < GIMP_ITEM (mask)->height) {
			//  non-antialiased
			if (!antialias) {
				y_sqr = (i + 0.5 - cy) * (i + 0.5 - cy);
				rad = sqrt (a_sqr - a_sqr * y_sqr / (double) b_sqr);
				x1 = ROUND (cx - rad);
				x2 = ROUND (cx + rad);

				switch (op) {
					case GIMP_CHANNEL_OP_ADD:
					case GIMP_CHANNEL_OP_REPLACE:
						gimp_channel_add_segment (mask, x1, i, (x2 - x1), 255);
						break;

					case GIMP_CHANNEL_OP_SUBTRACT:
						gimp_channel_sub_segment (mask, x1, i, (x2 - x1), 255);
						break;
				}
			} else {
				//  antialiasing
				x0 = x;
				last = 0;
				h_sqr = (i + 0.5 - cy) * (i + 0.5 - cy);
				for (j = x; j < (x + w); j++) {
					w_sqr = (j + 0.5 - cx) * (j + 0.5 - cx);
					
					if (h_sqr != 0) {
						t0 = w_sqr / h_sqr;
						t1 = a_sqr / (t0 + aob_sqr);
						r = sqrt (t1 + t0 * t1);
						rad = sqrt (w_sqr + h_sqr);
						dist = rad - r;
					} else
						dist = -1.0;

					if (dist < -0.5)
						val = 255;
					else if (dist < 0.5)
						val = (int) (255 * (1 - (dist + 0.5)));
					else
						val = 0;

					if (last != val && last) {
						switch (op) {
							case GIMP_CHANNEL_OP_ADD:
							case GIMP_CHANNEL_OP_REPLACE:
								gimp_channel_add_segment (mask, x0, i, j - x0, last);
								break;

							case GIMP_CHANNEL_OP_SUBTRACT:
								gimp_channel_sub_segment (mask, x0, i, j - x0, last);
								break;

						}
					}

					if (last != val) {
						x0 = j;
						last = val;
						// because we are symetric accross the y axis we can
						// skip ahead a bit if we are inside the ellipse
						if (val == 255 && j < cx)
							j = cx + (cx - j) - 1;
					}
				}

				if (last) {
					switch (op) {
						case GIMP_CHANNEL_OP_ADD:
						case GIMP_CHANNEL_OP_REPLACE:
							gimp_channel_add_segment (mask, x0, i, j - x0, last);
							break;

						case GIMP_CHANNEL_OP_SUBTRACT:
							gimp_channel_sub_segment (mask, x0, i, j - x0, last);
							break;

					}
				}
			}
		}
	}
}
*/
/*
void
marker(int32 x, int32 y)
{
}

void
compute(int32& R, int32& delta, int32 co)
{
	// update residuum R with coordinate co and compute delta;
	// k is the number of bits necessary to represent the coordinates
	R = R + 2 * co;//R + 2Aco;
	delta = R >> 32 + 1; //R >> k + 1; !?!!? (32 + 1) ?
	R = R & 0xfffffffe;//(1 << 33 - 1);//(pow(2, k + 1) - 1);
}

void
maxwell_baker_ellipse (int32 c, int32 d, float angle);
{
	int32 x1 = c;
	int32 y1 = round(d * cos(angle));//round(dAcos(angle));
	int32 x2 = 0;
	int32 y2 = round(d * sin(angle));//round(dAsin(angle));
	int32 Rx1 = 1 << 32; //pow(2, k);
	int32 Ry1 = 1 << 32; //pow(2, k);
	int32 Rx2 = 1 << 32; //pow(2, k);
	int32 Ry2 = 1 << 32; //pow(2, k);
	// initialize (x1dt or y1dt) <> 0
	int32 x1dt = 1;
	int32 y1dt = 1;
	// initialize (x2dt or y2dt) <> 0
	int32 x2dt = 1;
	int32 y2dt = 1;
	// estimated number of pixels
	int32 estimate = round(PI * (1 << 31));//pow(2, k - 1));

	for (int32 i = 1; i <= estimate; i++) {
		if (x1dt != 0 || y1dt != 0) {
			marker(x2, y2);
			marker(-x2, -y2);
		}
		if (x2dt != 0 || y2dt != 0) {
			marker(x1, y1);
			marker(-x1, -y1);
		}
		compute(Rx1, x1dt, x1);
		compute(Ry1, y1dt, y1);
		compute(Rx2, x2dt, x2);
		compute(Ry2, y2dt, y2);
		
		x1 = x1 - x2dt;
		y1 = y1 - y2dt;
		x2 = x2 + x1dt;
		y2 = y2 + y1dt;
		Rx1 = Rx1 - x2dt;
		Ry1 = Ry1 - y2dt;
		Rx2 = Rx2 + x1dt;
		Ry2 = Ry2 + y1dt;
	}
}
*/
// Draw
void
StrokeState::Draw(BView* into, BRect updateRect)
{
	if (fCursorShowing) {
		BRect r(fCursor.position, fCursor.position);
		float zoomLevel = fCanvasView->ZoomLevel();
		float crossSize = max_c(6.0, ceilf(1.8 * zoomLevel));

		float cursorSize = (fMode == MODE_PEN
							|| fMode == MODE_PEN_ERASER
							|| fMode == MODE_CLONE
							|| fCursor.radius <= 3.0) ? crossSize : fCursor.radius;

		r.InsetBy(-cursorSize, -cursorSize);
		if (updateRect.Intersects(r)) {
			switch (fMode) {
				case MODE_PEN:
				case MODE_PEN_ERASER: {
					// draw cross
					float cursorSize2 = max_c(2.0, ceilf(0.2 * zoomLevel));
					DrawCross(into, fCursor.position, cursorSize, cursorSize2);
					break;
				}
				case MODE_CLONE: {
					// draw cross
					float cursorSize2 = cursorSize / 2.0;
					BPoint p;
					if (fCloneStep == CLONE_NEEDS_OFFSET)
						p = fCursor.position;
					else if (fCloneStep == CLONE_NEEDS_FIRST_POINT) {
						p = fCloneOffset;
						fCanvasView->ConvertFromCanvas(p);
					} else {
						p.x = fCursor.position.x - fCloneOffset.x * zoomLevel;
						p.y = fCursor.position.y - fCloneOffset.y * zoomLevel;
					}
					DrawCross(into, p, cursorSize, cursorSize2);
					// draw a line from offset to current cursor position
					if (fCloneStep == CLONE_NEEDS_FIRST_POINT) {
						into->SetDrawingMode(B_OP_INVERT);
						into->StrokeLine(p, fCursor.position);
						into->SetDrawingMode(B_OP_COPY);
					}
					if (fCloneStep == CLONE_NEEDS_OFFSET)
						break;
					// else fall through
				}
				default:
					if (fCanvasView->BrushFlags() & FLAG_TILT_CONTROLS_SHAPE) {
						DrawCircle(into, fCursor.position, fCursor.max, fCursor.min,
								   fCursor.tilt_x, fCursor.tilt_y);
					} else {
						DrawCircle(into, fCursor.position, fCursor.max, fCursor.min);
					}
					if (fCursor.radius <= 3.0) {
						// draw an additional cross if brush cursor gets too small
						float cursorSize2 = fCursor.radius + 2;
						DrawCross(into, fCursor.position, cursorSize, cursorSize2);
					}
					break;
			}
		}
	}
}

// ModifiersChanged
void
StrokeState::ModifiersChanged()
{
	if (fMode == MODE_CLONE) {
		if (modifiers() & B_COMMAND_KEY) {
			fCloneStep = CLONE_NEEDS_OFFSET;
			DrawCursor(fCursor.position);
		}
	}
}

// HandleKeyDown
bool
StrokeState::HandleKeyDown(uint32 key, uint32 modifiers)
{
	if (fMode == MODE_CLONE) {
		if (key == B_ESCAPE) {
			fCloneStep = CLONE_NEEDS_OFFSET;
			DrawCursor(fCursor.position);
			return true;
		}
	}
	return false;
}

// HandleKeyUp
bool
StrokeState::HandleKeyUp(uint32 key, uint32 modifiers)
{
	return false;
}

// UpdateToolCursor
void
StrokeState::UpdateToolCursor()
{
	BCursor cursor(kEmptyCursor);
	fCanvasView->SetViewCursor(&cursor, true);
	if (fCanvasView->Bounds().Contains(fCursor.position))
		DrawCursor(fCursor.mouse);
	else
		ClearCursor();
}

// DrawCursor
void
StrokeState::DrawCursor(BPoint where, bool invisible)
{
	fCursor.mouse = where;

	if (fCanvasView->Bounds().Contains(where)) {
		BRect updateRect;
		ClearCursor(&updateRect);

		float pixelWidth2 = 0.5 * (fCanvasView->ZoomLevel() - 1.0);
		fCursor.position.x = floorf(where.x + 0.5 + pixelWidth2);
		fCursor.position.y = floorf(where.y + 0.5 + pixelWidth2);
		fCursor.max = ceilf(fCanvasView->BrushRadius().max * fCanvasView->ZoomLevel());
		if (fCanvasView->BrushFlags() & FLAG_PRESSURE_CONTROLS_RADIUS)
			fCursor.min = ceilf(fCanvasView->BrushRadius().min * fCanvasView->ZoomLevel());
		else
			fCursor.min = fCursor.max;
		if (fabs(fCursor.min - fCursor.max) < 3.0) {
			fCursor.min = max_c(fCursor.min, fCursor.max);
			fCursor.max = fCursor.min;
		}
		fCursor.radius = max_c(fCursor.min, fCursor.max);
	
	
		// invalidate area to show cursor
		float crossSize = max_c(6.0, ceilf(1.8 * fCanvasView->ZoomLevel()));
		float cursorSize = fMode == MODE_PEN
							|| fMode == MODE_PEN_ERASER
							|| fCursor.radius <= 3.0 ? crossSize : fCursor.radius;
		fCursor.bounds.left = fCursor.position.x - cursorSize;
		fCursor.bounds.top = fCursor.position.y - cursorSize;
		fCursor.bounds.right = fCursor.position.x + cursorSize;
		fCursor.bounds.bottom = fCursor.position.y + cursorSize;
	
		if (fMode == MODE_CLONE) {
			BPoint p;
			if (fCloneStep == CLONE_NEEDS_OFFSET)
				p = fCursor.position;
			else if (fCloneStep == CLONE_NEEDS_FIRST_POINT) {
				p = fCloneOffset;
				fCanvasView->ConvertFromCanvas(p);
			} else {
				p.x = fCursor.position.x - fCloneOffset.x * fCanvasView->ZoomLevel();
				p.y = fCursor.position.y - fCloneOffset.y * fCanvasView->ZoomLevel();
			}
	
			BRect r(p, p);
			r.InsetBy(-crossSize , -crossSize);
			fCursor.bounds = fCursor.bounds | r;
		}
		if (!invisible) {
			updateRect = updateRect | fCursor.bounds;
			fCanvasView->Invalidate(updateRect);
	
			fCursorShowing = true;
		}
	}
}

// ClearCursor
void
StrokeState::ClearCursor(BRect* updateRect)
{
	if (updateRect)
		*updateRect = fCursor.bounds;
	else
		fCanvasView->Invalidate(fCursor.bounds);

	fCursorShowing = false;
}

// _LayerBounds
BRect
StrokeState::_LayerBounds() const
{
	BRect r(0.0, 0.0, -1.0, -1.0);
	if (fLayer) {
		if (BBitmap* bitmap = fLayer->Bitmap())
			r = bitmap->Bounds();
	}
	return r;
}

// _NewStroke
Stroke*
StrokeState::_NewStroke(bool eraser) const
{
//printf("StrokeState::_NewStroke(eraser=%d)\n", eraser);
	Stroke* stroke = NULL;
	if (fLayer) {
		switch (fMode) {
			case MODE_PEN: {
				uint32 mode = eraser ? MODE_PEN_ERASER : MODE_PEN;
				stroke = new PenStroke(fCanvasView->Color(),
									   fCanvasView->BrushAlpha(),
									   fCanvasView->BrushFlags(),
									   mode);
				break;
			}
			case MODE_PEN_ERASER:
				stroke = new PenStroke(fCanvasView->Color(),
									   fCanvasView->BrushAlpha(),
									   fCanvasView->BrushFlags(),
									   MODE_PEN_ERASER);
				break;
			case MODE_BRUSH: {
				uint32 mode = eraser ? MODE_ERASER : MODE_BRUSH;
				stroke = new BrushStroke(fCanvasView->BrushRadius(),
										 fCanvasView->BrushHardness(),
										 fCanvasView->BrushAlpha(),
										 fCanvasView->BrushSpacing(),
										 fCanvasView->BrushFlags(),
										 fCanvasView->Color(),
										 mode);
				break;
			}
			case MODE_ERASER:
				stroke = new BrushStroke(fCanvasView->BrushRadius(),
										 fCanvasView->BrushHardness(),
										 fCanvasView->BrushAlpha(),
										 fCanvasView->BrushSpacing(),
										 fCanvasView->BrushFlags(),
										 fCanvasView->Color(),
										 MODE_ERASER);
				break;
			case MODE_CLONE:
				stroke = new CloneStroke(fCanvasView->BrushRadius(),
										 fCanvasView->BrushHardness(),
										 fCanvasView->BrushAlpha(),
										 fCanvasView->BrushSpacing(),
										 fCanvasView->BrushFlags(),
										 fCanvasView->Color(),
										 MODE_CLONE,
										 fCloneOffset);
				break;
			case MODE_BLUR:
				stroke = new BlurStroke(fCanvasView->BrushRadius(),
										fCanvasView->BrushHardness(),
										fCanvasView->BrushAlpha(),
										fCanvasView->BrushSpacing(),
										fCanvasView->BrushFlags(),
										fCanvasView->Color(),
										10.0);
				break;
			default:
				break;
		}
	}
	return stroke;
}

// _StrokeLastLine
void
StrokeState::_StrokeLastLine(bool forceStrokeDrawing)
{
	if (fCurrentStroke && fLayer) {
		BRect r(_LayerBounds());
		if (fCurrentStroke->DrawLastLine(fCanvasView->StrokeBitmap(), r)) {
			fLayer->Touch(r);
			if (forceStrokeDrawing)
				_ApplyStroke(fCanvasView->BackBitmap(),
							 fLayer->Bitmap(), r, fCurrentStroke);
			fCanvasView->_InvalidateCanvas(r, true);
		}
	}
}






