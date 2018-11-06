// TransformEditor.h

#include <stdio.h>

#include <agg_trans_affine.h>

#include "support.h"

#include "CanvasView.h"

#include "TransformEditor.h"

float
min5(float v1, float v2, float v3, float v4, float v5)
{
	return min_c(min_c(min_c(min_c(v1, v2), v3), v4), v5);
}

float
max5(float v1, float v2, float v3, float v4, float v5)
{
	return max_c(max_c(max_c(max_c(v1, v2), v3), v4), v5);
}

// constructor
TransformEditor::TransformEditor(CanvasView* canvasView, BRect box)
	: Transformable(),
	  fCanvasView(canvasView)
{
/*	BPoint offset(box.left + box.Width() / 2.0, box.top + box.Height() / 2.0);
	fOriginalBox.OffsetBy(-offset.x, -offset.y);
	TranslateBy(offset);*/
}

// destructor
TransformEditor::~TransformEditor()
{
	delete fDragLTState;
	delete fDragRTState;
	delete fDragLBState;
	delete fDragRBState;

	delete fDragLState;
	delete fDragRState;
	delete fDragTState;
	delete fDragBState;

	delete fRotateState;
	delete fTranslateState;
	delete fOffsetCenterState;
}

// Bounds
BRect
TransformEditor::Bounds()
{
	BRect r;
	r.left = min5(fLeftTop.x, fRightTop.x, fLeftBottom.x, fRightBottom.x, fOrigin.x);
	r.top = min5(fLeftTop.y, fRightTop.y, fLeftBottom.y, fRightBottom.y, fOrigin.y);
	r.right = max5(fLeftTop.x, fRightTop.x, fLeftBottom.x, fRightBottom.x, fOrigin.x);
	r.bottom = max5(fLeftTop.y, fRightTop.y, fLeftBottom.y, fRightBottom.y, fOrigin.y);
	r.InsetBy(-3.0, -3.0);
	return r;
}

// Update
void
TransformEditor::Update()
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
TransformEditor::Draw(BView* into)
{
	// convert to canvas view coordinates
	BPoint lt = fLeftTop;
	BPoint rt = fRightTop;
	BPoint lb = fLeftBottom;
	BPoint rb = fRightBottom;
	BPoint c = fOrigin;

	fCanvasView->ConvertFromCanvas(lt);	
	fCanvasView->ConvertFromCanvas(rt);	
	fCanvasView->ConvertFromCanvas(lb);	
	fCanvasView->ConvertFromCanvas(rb);	
	fCanvasView->ConvertFromCanvas(c);	
	
	into->SetHighColor(255, 255, 255, 255);
	into->SetLowColor(0, 0, 0, 255);
	_StrokeBWLine(into, lt, rt);
	_StrokeBWLine(into, rt, rb);
	_StrokeBWLine(into, rb, lb);
	_StrokeBWLine(into, lb, lt);

	double rotation = LocalRotation();
	_StrokeBWPoint(into, lt, rotation);
	_StrokeBWPoint(into, rt, rotation + 90.0);
	_StrokeBWPoint(into, rb, rotation + 180.0);
	_StrokeBWPoint(into, lb, rotation + 270.0);


	BRect cr(c, c);
	cr.InsetBy(-3.0, -3.0);
	into->StrokeLine(cr.LeftTop(), cr.RightBottom());
	into->StrokeLine(cr.LeftBottom(), cr.RightTop());
}

// OffsetOrigin
void
TransformEditor::OffsetOrigin(BPoint offset)
{
	fCenterOffset += offset;
	Update();
}

// Origin
BPoint
TransformEditor::Origin() const
{
	return fOrigin;
}

// SetBox
void
TransformEditor::SetBox(BRect box)
{
	fOriginalBox = box;
	Update();
}

// Width
double
TransformEditor::Width() const
{
	return fOriginalBox.Width();
}

// Height
double
TransformEditor::Height() const
{
	return fOriginalBox.Height();
}

// ScaledWidth
double
TransformEditor::ScaledWidth() const
{
	return fOriginalBox.Width() * LocalXScale();
}

// ScaledHeight
double
TransformEditor::ScaledHeight() const
{
	return fOriginalBox.Height() * LocalYScale();
}

// MouseDown
void
TransformEditor::MouseDown(BPoint where)
{
//printf("TransformEditor::MouseDown()\n");
	fDragging = true;
	if (fCurrentState) {
		fCanvasView->ConvertToCanvas(where);
		fCurrentState->SetOrigin(where);
	}
}

// MouseMoved
void
TransformEditor::MouseMoved(BPoint where)
{
//printf("TransformEditor::MouseMoved()\n");
	fMousePos = where;
	fCanvasView->ConvertToCanvas(fMousePos);
	if (fDragging) {
		if (fCurrentState) {
			fCanvasView->ConvertToCanvas(where);
			fCurrentState->DragTo(where, fModifiers);
			fCurrentState->UpdateViewCursor(fCanvasView, where);
		}
	} else {
		_SetState(_DragStateFor(where));
		if (fCurrentState) {
			fCurrentState->UpdateViewCursor(fCanvasView, fMousePos);
		}
	}
}

// MouseUp
void
TransformEditor::MouseUp()
{
//printf("TransformEditor::MouseUp()\n");
	fDragging = false;
}

// ModifiersChanged
void
TransformEditor::ModifiersChanged(uint32 modifiers)
{
	fModifiers = modifiers;
	if (fDragging && fCurrentState) {
		fCurrentState->DragTo(fMousePos, fModifiers);
	}
}

// UpdateToolCursor
void
TransformEditor::UpdateToolCursor()
{
	if (fCurrentState)
		fCurrentState->UpdateViewCursor(fCanvasView, fMousePos);
}

// _DragStateFor
//
// where is expected in canvas view coordinates
DragState*
TransformEditor::_DragStateFor(BPoint where)
{
	DragState* state = NULL;
	// convert to canvas view coordinates
	//
	// the conversion is necessary, because the "hot regions"
	// around a point should be the same size no matter what
	// zoom level the canvas is displayed at
	BPoint lt = fLeftTop;
	BPoint rt = fRightTop;
	BPoint lb = fLeftBottom;
	BPoint rb = fRightBottom;
	BPoint c = fOrigin;

	fCanvasView->ConvertFromCanvas(lt);	
	fCanvasView->ConvertFromCanvas(rt);	
	fCanvasView->ConvertFromCanvas(lb);	
	fCanvasView->ConvertFromCanvas(rb);	
	fCanvasView->ConvertFromCanvas(c);	

	// priorities:
	// transformation center point has highest priority ?!?
	if (!state) {
		BRect r(c, c);
		r.InsetBy(-INSET, -INSET);
		if (r.Contains(where))
			state = fOffsetCenterState;
	}	

	// next, the inner area of the box
	if (!state) {
		// for the following calculations
		// we can apply the inverse transformation to all points
		// this way we have to consider BRects only, not transformed polygons
		InverseTransform(&where);
		InverseTransform(&lt);
		InverseTransform(&rt);
		InverseTransform(&lb);
		InverseTransform(&rb);

		// next priority has the inside of the box
		BRect iR(lt, rb);
		if (iR.Width() < INSET * 2.0) {
			float extend = (INSET * 2.0 - iR.Width()) / 2.0;
			iR.InsetBy(-extend, 0.0);
		}
		if (iR.Height() < INSET * 2.0) {
			float extend = (INSET * 2.0 - iR.Height()) / 2.0;
			iR.InsetBy(0.0, -extend);
		}
		float widthInset = (iR.Width() - 2.0 * INSET) / 2.0;
		constrain(widthInset, 0.0, INSET);
		float heightInset = (iR.Height() - 2.0 * INSET) / 2.0;
		constrain(heightInset, 0.0, INSET);
		iR.InsetBy(widthInset, heightInset);
		if (iR.Contains(where))
			state = fTranslateState;

		if (!state) {
			// next priority have the corners
			BRect oR(iR);
			oR.InsetBy(-INSET * 2.0, -INSET * 2.0);

			BRect ltR(oR.left, oR.top, iR.left, iR.top);
			BRect rtR(iR.right, oR.top, oR.right, iR.top);
			BRect lbR(oR.left, iR.bottom, iR.left, oR.bottom);
			BRect rbR(iR.right, iR.bottom, oR.right, oR.bottom);

			if (ltR.Contains(where))
				state = fDragLTState;
			else if (rtR.Contains(where))
				state = fDragRTState;
			else if (lbR.Contains(where))
				state = fDragLBState;
			else if (rbR.Contains(where))
				state = fDragRBState;

			if (!state) {
				// next priority have the sides
				BRect lR(oR.left, iR.top, iR.left, iR.bottom);
				BRect rR(iR.right, iR.top, oR.right, iR.bottom);
				BRect tR(iR.left, oR.top, iR.right, iR.top);
				BRect bR(iR.left, iR.bottom, iR.right, oR.bottom);

				if (lR.Contains(where))
					state = fDragLState;
				else if (rR.Contains(where))
					state = fDragRState;
				else if (tR.Contains(where))
					state = fDragTState;
				else if (bR.Contains(where))
					state = fDragBState;

				if (!state) {
					// last priority has the rotate state
					state = fRotateState;
				}
			}
		}
	}

/*	// update the state
	if (state) {
		fCanvasView->ConvertToCanvas(where);
		state->SetOrigin(where);
	}*/
	return state;
}

// _SetState
void
TransformEditor::_SetState(DragState* state)
{
	if (state != fCurrentState) {
		fCurrentState = state;
		fCurrentState->UpdateViewCursor(fCanvasView, fMousePos);
	}
}

// _StrokeBWLine
void
TransformEditor::_StrokeBWLine(BView* into, BPoint from, BPoint to) const
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
TransformEditor::_StrokeBWPoint(BView* into, BPoint point, double angle) const
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

	agg::trans_affine_rotation r(angle * PI / 180.0);

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


