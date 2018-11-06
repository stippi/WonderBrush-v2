// TransformBox.h

#include <stdio.h>

#include <agg_trans_affine.h>
#include <agg_math.h>

#include "support.h"

#include "CanvasView.h"
#include "DragStates.h"
#include "TransformAction.h"

#include "TransformBox.h"

#define INSET 8.0

// constructor
TransformBox::TransformBox(CanvasView* canvasView, BRect box)
	: AdvancedTransformable(),
	  fCanvasView(canvasView),
	  fOriginalBox(box),
	  fLeftTop(box.LeftTop()),
	  fRightTop(box.RightTop()),
	  fLeftBottom(box.LeftBottom()),
	  fRightBottom(box.RightBottom()),
	  fOrigin((fLeftTop.x + fRightBottom.x) / 2.0,
	  		  (fLeftTop.y + fRightBottom.y) / 2.0),
	  fCenterOffset(0.0, 0.0),

	  fCurrentAction(NULL),
	  fCurrentState(NULL),
	  fDragging(false),
	  fMousePos(-10000.0, -10000.0),
	  fModifiers(0),

	  fNudging(false),

//	  fDragLTState(new DragCornerState(this, DragCornerState::LEFT_TOP_CORNER)),
//	  fDragRTState(new DragCornerState(this, DragCornerState::RIGHT_TOP_CORNER)),
//	  fDragLBState(new DragCornerState(this, DragCornerState::LEFT_BOTTOM_CORNER)),
//	  fDragRBState(new DragCornerState(this, DragCornerState::RIGHT_BOTTOM_CORNER)),
//	  fDragLTState(new ScaleCornerState2(this, ScaleCornerState2::LEFT_TOP_CORNER)),
//	  fDragRTState(new ScaleCornerState2(this, ScaleCornerState2::RIGHT_TOP_CORNER)),
//	  fDragLBState(new ScaleCornerState2(this, ScaleCornerState2::LEFT_BOTTOM_CORNER)),
//	  fDragRBState(new ScaleCornerState2(this, ScaleCornerState2::RIGHT_BOTTOM_CORNER)),
	  fDragLTState(new ScaleCornerState3(this, ScaleCornerState3::LEFT_TOP_CORNER)),
	  fDragRTState(new ScaleCornerState3(this, ScaleCornerState3::RIGHT_TOP_CORNER)),
	  fDragLBState(new ScaleCornerState3(this, ScaleCornerState3::LEFT_BOTTOM_CORNER)),
	  fDragRBState(new ScaleCornerState3(this, ScaleCornerState3::RIGHT_BOTTOM_CORNER)),

	  fDragLTPerspState(new OffsetCornerState(this, OffsetCornerState::LEFT_TOP_CORNER)),
	  fDragRTPerspState(new OffsetCornerState(this, OffsetCornerState::RIGHT_TOP_CORNER)),
	  fDragLBPerspState(new OffsetCornerState(this, OffsetCornerState::LEFT_BOTTOM_CORNER)),
	  fDragRBPerspState(new OffsetCornerState(this, OffsetCornerState::RIGHT_BOTTOM_CORNER)),

//	  fDragLState(new DragSideState(this, DragSideState::LEFT_SIDE)),
//	  fDragRState(new DragSideState(this, DragSideState::RIGHT_SIDE)),
//	  fDragTState(new DragSideState(this, DragSideState::TOP_SIDE)),
//	  fDragBState(new DragSideState(this, DragSideState::BOTTOM_SIDE)),
	  fDragLState(new ScaleSideState(this, ScaleSideState::LEFT_SIDE)),
	  fDragRState(new ScaleSideState(this, ScaleSideState::RIGHT_SIDE)),
	  fDragTState(new ScaleSideState(this, ScaleSideState::TOP_SIDE)),
	  fDragBState(new ScaleSideState(this, ScaleSideState::BOTTOM_SIDE)),

//	  fRotateState(new RotateBoxState(this)),
	  fRotateState(new RotateBoxState2(this)),
//	  fTranslateState(new DragBoxState(this)),
	  fTranslateState(new DragBoxState2(this)),
	  fOffsetCenterState(new OffsetCenterState(this))
{
/*	BPoint offset(box.left + box.Width() / 2.0, box.top + box.Height() / 2.0);
	fOriginalBox.OffsetBy(-offset.x, -offset.y);
	TranslateBy(offset);*/
}

// constructor
TransformBox::TransformBox(const TransformBox& other)
	: AdvancedTransformable(other),
	  fCanvasView(other.fCanvasView),
	  fOriginalBox(other.fOriginalBox),

	  fLeftTop(other.fLeftTop),
	  fRightTop(other.fRightTop),
	  fLeftBottom(other.fLeftBottom),
	  fRightBottom(other.fRightBottom),

	  fOrigin(other.fOrigin),
	  fCenterOffset(other.fCenterOffset),

	  fCurrentAction(NULL),
	  fCurrentState(NULL),
	  fDragging(false),
	  fMousePos(other.fMousePos),
	  fModifiers(other.fModifiers),

//	  fDragLTState(new DragCornerState(this, DragCornerState::LEFT_TOP_CORNER)),
//	  fDragRTState(new DragCornerState(this, DragCornerState::RIGHT_TOP_CORNER)),
//	  fDragLBState(new DragCornerState(this, DragCornerState::LEFT_BOTTOM_CORNER)),
//	  fDragRBState(new DragCornerState(this, DragCornerState::RIGHT_BOTTOM_CORNER)),
//	  fDragLTState(new ScaleCornerState2(this, ScaleCornerState2::LEFT_TOP_CORNER)),
//	  fDragRTState(new ScaleCornerState2(this, ScaleCornerState2::RIGHT_TOP_CORNER)),
//	  fDragLBState(new ScaleCornerState2(this, ScaleCornerState2::LEFT_BOTTOM_CORNER)),
//	  fDragRBState(new ScaleCornerState2(this, ScaleCornerState2::RIGHT_BOTTOM_CORNER)),
	  fDragLTState(new ScaleCornerState3(this, ScaleCornerState3::LEFT_TOP_CORNER)),
	  fDragRTState(new ScaleCornerState3(this, ScaleCornerState3::RIGHT_TOP_CORNER)),
	  fDragLBState(new ScaleCornerState3(this, ScaleCornerState3::LEFT_BOTTOM_CORNER)),
	  fDragRBState(new ScaleCornerState3(this, ScaleCornerState3::RIGHT_BOTTOM_CORNER)),

	  fDragLTPerspState(new OffsetCornerState(this, OffsetCornerState::LEFT_TOP_CORNER)),
	  fDragRTPerspState(new OffsetCornerState(this, OffsetCornerState::RIGHT_TOP_CORNER)),
	  fDragLBPerspState(new OffsetCornerState(this, OffsetCornerState::LEFT_BOTTOM_CORNER)),
	  fDragRBPerspState(new OffsetCornerState(this, OffsetCornerState::RIGHT_BOTTOM_CORNER)),

//	  fDragLState(new DragSideState(this, DragSideState::LEFT_SIDE)),
//	  fDragRState(new DragSideState(this, DragSideState::RIGHT_SIDE)),
//	  fDragTState(new DragSideState(this, DragSideState::TOP_SIDE)),
//	  fDragBState(new DragSideState(this, DragSideState::BOTTOM_SIDE)),
	  fDragLState(new ScaleSideState(this, ScaleSideState::LEFT_SIDE)),
	  fDragRState(new ScaleSideState(this, ScaleSideState::RIGHT_SIDE)),
	  fDragTState(new ScaleSideState(this, ScaleSideState::TOP_SIDE)),
	  fDragBState(new ScaleSideState(this, ScaleSideState::BOTTOM_SIDE)),

//	  fRotateState(new RotateBoxState(this)),
	  fRotateState(new RotateBoxState2(this)),
//	  fTranslateState(new DragBoxState(this)),
	  fTranslateState(new DragBoxState2(this)),
	  fOffsetCenterState(new OffsetCenterState(this))
{
}

// destructor
TransformBox::~TransformBox()
{
	delete fCurrentAction;

	delete fDragLTState;
	delete fDragRTState;
	delete fDragLBState;
	delete fDragRBState;

	delete fDragLState;
	delete fDragRState;
	delete fDragTState;
	delete fDragBState;

	delete fDragLTPerspState;
	delete fDragRTPerspState;
	delete fDragLBPerspState;
	delete fDragRBPerspState;

	delete fRotateState;
	delete fTranslateState;
	delete fOffsetCenterState;
}

// Set
void
TransformBox::Set(const Transformable& t, BPoint centerOffset)
{
	SetTransformable(t);

	_UpdateAffine();

	// adopt center offset
	fCenterOffset += centerOffset;

	// call hook function
	Update();
}

// Bounds
BRect
TransformBox::Bounds() const
{
	// convert to canvas view coordinates
	BPoint lt = fLeftTop;
	BPoint rt = fRightTop;
	BPoint lb = fLeftBottom;
	BPoint rb = fRightBottom;
	BPoint c = fOrigin;

	TransformToCanvas(lt);
	TransformToCanvas(rt);
	TransformToCanvas(lb);
	TransformToCanvas(rb);
	TransformToCanvas(c);

	BRect r;
	r.left = min5(lt.x, rt.x, lb.x, rb.x, c.x);
	r.top = min5(lt.y, rt.y, lb.y, rb.y, c.y);
	r.right = max5(lt.x, rt.x, lb.x, rb.x, c.x);
	r.bottom = max5(lt.y, rt.y, lb.y, rb.y, c.y);
	return r;
}

// Update
void
TransformBox::Update(bool deep)
{
	// recalculate the points from the original box
	fLeftTop = fOriginalBox.LeftTop();
	fRightTop = fOriginalBox.RightTop();
	fLeftBottom = fOriginalBox.LeftBottom();
	fRightBottom = fOriginalBox.RightBottom();

	fOrigin.x = (fLeftTop.x + fRightBottom.x) / 2.0;
	fOrigin.y = (fLeftTop.y + fRightBottom.y) / 2.0;

	fOrigin += fCenterOffset;

	// transform the points
	Transform(&fLeftTop);
	Transform(&fRightTop);
	Transform(&fLeftBottom);
	Transform(&fRightBottom);

	Transform(&fOrigin);
}

// Draw
void
TransformBox::Draw(BView* into)
{
	// convert to canvas view coordinates
	BPoint lt = fLeftTop;
	BPoint rt = fRightTop;
	BPoint lb = fLeftBottom;
	BPoint rb = fRightBottom;
	BPoint c = fOrigin;

	TransformToCanvas(lt);
	TransformToCanvas(rt);
	TransformToCanvas(lb);
	TransformToCanvas(rb);
	TransformToCanvas(c);

	fCanvasView->ConvertFromCanvas(lt);	
	fCanvasView->ConvertFromCanvas(rt);	
	fCanvasView->ConvertFromCanvas(lb);	
	fCanvasView->ConvertFromCanvas(rb);	
	fCanvasView->ConvertFromCanvas(c);	

	into->SetDrawingMode(B_OP_COPY);
	into->SetHighColor(255, 255, 255, 255);
	into->SetLowColor(0, 0, 0, 255);
	_StrokeBWLine(into, lt, rt);
	_StrokeBWLine(into, rt, rb);
	_StrokeBWLine(into, rb, lb);
	_StrokeBWLine(into, lb, lt);

	double rotation = ViewSpaceRotation();
	_StrokeBWPoint(into, lt, rotation);
	_StrokeBWPoint(into, rt, rotation + 90.0);
	_StrokeBWPoint(into, rb, rotation + 180.0);
	_StrokeBWPoint(into, lb, rotation + 270.0);


	BRect cr(c, c);
	cr.InsetBy(-3.0, -3.0);
	into->StrokeEllipse(cr, B_SOLID_HIGH);
	cr.InsetBy(1.0, 1.0);
	into->StrokeEllipse(cr, B_SOLID_LOW);
	into->SetDrawingMode(B_OP_COPY);
}

// OffsetOrigin
void
TransformBox::OffsetOrigin(BPoint offset)
{
	if (offset != BPoint(0.0, 0.0)) {
		fCenterOffset += offset;
		Update(false);
	}
}

// Origin
BPoint
TransformBox::Origin() const
{
	return fOrigin;
}

// SetBox
void
TransformBox::SetBox(BRect box)
{
	if (fOriginalBox != box) {
		fOriginalBox = box;
		Update(false);
	}
}

// FinishTransaction
Action*
TransformBox::FinishTransaction()
{
	Action* action = fCurrentAction;
	if (fCurrentAction) {
//		fCurrentAction->SetNewTransformation(Translation(),
//											 LocalRotation(),
//											 LocalXScale(),
//											 LocalYScale(),
		fCurrentAction->SetNewTransformation(*this,
											 CenterOffset());
		fCurrentAction = NULL;
	}
	return action;
}

// MouseDown
void
TransformBox::MouseDown(BPoint where)
{
	TransformFromCanvas(where);

	fDragging = true;
	if (fCurrentState) {
		fCurrentState->SetOrigin(where);

		delete fCurrentAction;
		fCurrentAction = MakeAction(fCurrentState->ActionName(),
									fCurrentState->ActionNameIndex());
	}
}

// MouseMoved
void
TransformBox::MouseMoved(BPoint where, float zoom)
{
	TransformFromCanvas(where);

	if (fMousePos != where) {
		fMousePos = where;
		if (fDragging) {
			if (fCurrentState) {
				fCurrentState->DragTo(fMousePos, fModifiers);
				fCurrentState->UpdateViewCursor(fCanvasView, fMousePos);
			}
		} else {
			_SetState(_DragStateFor(where, zoom));
			if (fCurrentState) {
				fCurrentState->UpdateViewCursor(fCanvasView, fMousePos);
			}
		}
	}
}

// MouseUp
Action*
TransformBox::MouseUp()
{
	fDragging = false;
	return FinishTransaction();
}

// NudgeBy
void
TransformBox::NudgeBy(BPoint offset)
{
	if (!fNudging && !fCurrentAction) {
		fCurrentAction = MakeAction("Move", MOVE);
		fNudging = true;
	}
	if (fNudging) {
		TranslateBy(offset);
	}
}

// FinishNudging
Action*
TransformBox::FinishNudging()
{
	fNudging = false;
	return FinishTransaction();
}

// TransformFromCanvas
void
TransformBox::TransformFromCanvas(BPoint& point) const
{
}

// TransformToCanvas
void
TransformBox::TransformToCanvas(BPoint& point) const
{
}

// ModifiersChanged
void
TransformBox::ModifiersChanged(uint32 modifiers)
{
	uint32 oldControl = fModifiers & B_CONTROL_KEY;
	uint32 newControl = modifiers & B_CONTROL_KEY;

	fModifiers = modifiers;

	// switch state if necessary
	// (in case B_CONTROL_KEY changed, but not right during a drag)
	if (!fDragging && oldControl != newControl) {
		_SetState(_DragStateFor(fMousePos, fCanvasView->ZoomLevel()));
	}

	if (fCurrentState) {
		// give the chance to update the view cursor
		fCurrentState->UpdateViewCursor(fCanvasView, fMousePos);
		// fake a mouse moved event with new modifiers
		if (fDragging)
			fCurrentState->DragTo(fMousePos, fModifiers);
	}
}

// UpdateToolCursor
void
TransformBox::UpdateToolCursor()
{
	if (fCurrentState)
		fCurrentState->UpdateViewCursor(fCanvasView, fMousePos);
}

// ConvertToLocal
void
TransformBox::ConvertFromLocal(BPoint* newLocal, double rotation,
							   double xScale, double yScale) const
{
	// duplicate our own matrix
	agg::trans_affine m;
	m.multiply(agg::trans_affine_scaling(xScale, yScale));
	m.multiply(agg::trans_affine_rotation(rotation * M_PI / 180.0));
	m.multiply(agg::trans_affine_translation(newLocal->x,
											 newLocal->y));

	double x = (fOriginalBox.left + fOriginalBox.right) / 2.0 + fCenterOffset.x;
	double y = (fOriginalBox.top + fOriginalBox.bottom) / 2.0 + fCenterOffset.y;
	m.transform(&x, &y);

	double xDiff = x - fOrigin.x;
	double yDiff = y - fOrigin.y;

	// translation (convert to world)
	BPoint currentLocal = LocalTranslation();
	newLocal->x += (newLocal->x - currentLocal.x) - xDiff;
	newLocal->y += (newLocal->y - currentLocal.y) - yDiff;
}

// LocalTranslation
BPoint
TransformBox::LocalTranslation() const
{
	BPoint center;
	center.x = (fOriginalBox.left + fOriginalBox.right) / 2.0;
	center.y = (fOriginalBox.top + fOriginalBox.bottom) / 2.0;
	return LocalTranslation(center, *this);
}

// LocalTranslation
BPoint
TransformBox::LocalTranslation(BPoint origin,
							   const Transformable& transform)
{
	BPoint center = origin;
	transform.Transform(&origin);
	center.x = origin.x - center.x;
	center.y = origin.y - center.y;
	return center;
}

// ViewSpaceRotation
double
TransformBox::ViewSpaceRotation() const
{
	// assume no inherited transformation
	return LocalRotation();
}

// point_line_dist
float
point_line_dist(BPoint start, BPoint end, BPoint p, float radius)
{
	BRect r(min_c(start.x, end.x),
			min_c(start.y, end.y),
			max_c(start.x, end.x),
			max_c(start.y, end.y));
	r.InsetBy(-radius, -radius);
	if (r.Contains(p)) {
		return fabs(agg::calc_line_point_distance(start.x, start.y,
												  end.x, end.y,
												  p.x, p.y));
	}
	return min_c(dist(start, p), dist(end, p));
}

// #pragma mark -

// OffsetQuadLeftTop
void
TransformBox::OffsetQuadLeftTop(BPoint offset)
{
	if (offset == B_ORIGIN)
		return;
	_UpdateFromQuad(fLeftTop + offset, fRightTop, fLeftBottom, fRightBottom);
}

// OffsetQuadRightTop
void
TransformBox::OffsetQuadRightTop(BPoint offset)
{
	if (offset == B_ORIGIN)
		return;
	_UpdateFromQuad(fLeftTop, fRightTop + offset, fLeftBottom, fRightBottom);
}

// OffsetQuadLeftBottom
void
TransformBox::OffsetQuadLeftBottom(BPoint offset)
{
	if (offset == B_ORIGIN)
		return;
	_UpdateFromQuad(fLeftTop, fRightTop, fLeftBottom + offset, fRightBottom);
}

// OffsetQuadRightBottom
void
TransformBox::OffsetQuadRightBottom(BPoint offset)
{
	if (offset == B_ORIGIN)
		return;
	_UpdateFromQuad(fLeftTop, fRightTop, fLeftBottom, fRightBottom + offset);
}

// OffsetQuadRightBottom
void
TransformBox::SetQuad(const BPoint& lt, const BPoint& rt,
					  const BPoint& lb, const BPoint& rb)
{
	_UpdateFromQuad(lt, rt, lb, rb);
}

// _UpdateFromQuad
void
TransformBox::_UpdateFromQuad(const BPoint& lt, const BPoint& rt,
							  const BPoint& lb, const BPoint& rb)
{
	// modify matrix for perspective transformation
	double quad[8];
	quad[0] = lt.x;
	quad[1] = lt.y;

	quad[2] = rt.x;
	quad[3] = rt.y;

	quad[4] = rb.x;
	quad[5] = rb.y;

	quad[6] = lb.x;
	quad[7] = lb.y;

	rect_to_quad(fOriginalBox.left, fOriginalBox.top, 
				 fOriginalBox.right, fOriginalBox.bottom,
				 quad);

	_UpdateAffine();

	// call hook function
	Update();
}

// _UpdateAffine
void
TransformBox::_UpdateAffine()
{
	// adopt the affine parameters to the matrix
	fRotation = agg::rad2deg(rotation());

	double translationX;
	double translationY;
	translation(&translationX, &translationY);
	fTranslation.x = translationX;
	fTranslation.y = translationY;

	scaling(&fXScale, &fYScale);
}

// #pragma mark -

// _DragStateFor
//
// where is expected in canvas view coordinates
DragState*
TransformBox::_DragStateFor(BPoint where, float canvasZoom)
{
	DragState* state = NULL;
	// convert to canvas zoom level
	//
	// the conversion is necessary, because the "hot regions"
	// around a point should be the same size no matter what
	// zoom level the canvas is displayed at

	float inset = INSET / canvasZoom;

	// priorities:
	// transformation center point has highest priority ?!?
	if (dist(where, fOrigin) < inset)
		state = fOffsetCenterState;

	if (!state) {
		// next, the inner area of the box

		// for the following calculations
		// we can apply the inverse transformation to all points
		// this way we have to consider BRects only, not transformed polygons
		BPoint lt = fLeftTop;
		BPoint rb = fRightBottom;
		BPoint w = where;

		InverseTransform(&w);
		InverseTransform(&lt);
		InverseTransform(&rb);

		// next priority has the inside of the box
		BRect iR(lt, rb);
		float hInset = min_c(inset, max_c(0, (iR.Width() - inset) / 2.0));
		float vInset = min_c(inset, max_c(0, (iR.Height() - inset) / 2.0));
//printf("width: %.3f, hInset: %.3f, height: %.3f, vInset: %.3f, proposed: %.3f\n", iR.Width(), hInset,
//																  iR.Height(), vInset, inset);
		iR.InsetBy(hInset, vInset);
		if (iR.Contains(w))
			state = fTranslateState;
	}

	if (!state) {
		// next priority have the corners
		float dLT = dist(fLeftTop, where);
		float dRT = dist(fRightTop, where);
		float dLB = dist(fLeftBottom, where);
		float dRB = dist(fRightBottom, where);
		float d = min4(dLT, dRT, dLB, dRB);
		if (d < inset) {
			if (fModifiers & B_CONTROL_KEY) {
				if (d == dLT)
					state = fDragLTPerspState;
				else if (d == dRT)
					state = fDragRTPerspState;
				else if (d == dLB)
					state = fDragLBPerspState;
				else if (d == dRB)
					state = fDragRBPerspState;
			} else {
				if (d == dLT)
					state = fDragLTState;
				else if (d == dRT)
					state = fDragRTState;
				else if (d == dLB)
					state = fDragLBState;
				else if (d == dRB)
					state = fDragRBState;
			}
		}
	}

	if (!state) {
		// next priority have the sides
		float dL = point_line_dist(fLeftTop, fLeftBottom, where, inset);
		float dR = point_line_dist(fRightTop, fRightBottom, where, inset);
		float dT = point_line_dist(fLeftTop, fRightTop, where, inset);
		float dB = point_line_dist(fLeftBottom, fRightBottom, where, inset);
		float d = min4(dL, dR, dT, dB);
		if (d < inset) {
			if (d == dL)
				state = fDragLState;
			else if (d == dR)
				state = fDragRState;
			else if (d == dT)
				state = fDragTState;
			else if (d == dB)
				state = fDragBState;
		}
	}

	if (!state) {
		BPoint lt = fLeftTop;
		BPoint rb = fRightBottom;
		BPoint w = where;

		InverseTransform(&w);
		InverseTransform(&lt);
		InverseTransform(&rb);

		// check inside of the box again
		BRect iR(lt, rb);
		if (iR.Contains(w)) {
			state = fTranslateState;
		} else {
			// last priority has the rotate state
			state = fRotateState;
		}
	}

/*	// update the state
	if (state) {
		fCanvasView->ConvertToCanvas(where);
		state->SetOrigin(where);
	}*/
	return state;
}

// _StrokeBWLine
void
TransformBox::_StrokeBWLine(BView* into, BPoint from, BPoint to) const
{
	// find out how to offset the second line optimally
	BPoint offset(0.0, 0.0);
	// first, do we have a more horizontal line or a more vertical line?
	float xDiff = to.x - from.x;
	float yDiff = to.y - from.y;
	if (fabs(xDiff) > fabs(yDiff)) {
		// horizontal
		if (xDiff > 0.0) {
			offset.y = -1.0;
		} else {
			offset.y = 1.0;
		}
	} else {
		// vertical
		if (yDiff < 0.0) {
			offset.x = -1.0;
		} else {
			offset.x = 1.0;
		}
	}
	// stroke two lines in high and low color of the view
	into->StrokeLine(from, to, B_SOLID_LOW);
	from += offset;
	to += offset;
	into->StrokeLine(from, to, B_SOLID_HIGH);
}

// _StrokeBWPoint
void
TransformBox::_StrokeBWPoint(BView* into, BPoint point, double angle) const
{
	double x = point.x;
	double y = point.y;

	double x1 = x;
	double y1 = y - 5.0;

	double x2 = x - 5.0;
	double y2 = y - 5.0;

	double x3 = x - 5.0;
	double y3 = y;

	agg::trans_affine m;

	double xOffset = -x;
	double yOffset = -y;

	agg::trans_affine_rotation r(angle * M_PI / 180.0);

	r.transform(&xOffset, &yOffset);
	xOffset = x + xOffset;
	yOffset = y + yOffset;

	m.multiply(r);
	m.multiply(agg::trans_affine_translation(xOffset, yOffset));

	m.transform(&x, &y);
	m.transform(&x1, &y1);
	m.transform(&x2, &y2);
	m.transform(&x3, &y3);

	BPoint p[4];
	p[0] = BPoint(x, y);
	p[1] = BPoint(x1, y1);
	p[2] = BPoint(x2, y2);
	p[3] = BPoint(x3, y3);

	into->FillPolygon(p, 4, B_SOLID_HIGH);

	into->StrokeLine(p[0], p[1], B_SOLID_LOW);
	into->StrokeLine(p[1], p[2], B_SOLID_LOW);
	into->StrokeLine(p[2], p[3], B_SOLID_LOW);
	into->StrokeLine(p[3], p[0], B_SOLID_LOW);
}

// _SetState
void
TransformBox::_SetState(DragState* state)
{
	if (state != fCurrentState) {
		fCurrentState = state;
		fCurrentState->UpdateViewCursor(fCanvasView, fMousePos);
	}
}


