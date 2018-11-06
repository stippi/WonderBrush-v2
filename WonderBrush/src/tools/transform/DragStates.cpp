// DragStates.cpp

#include <math.h>
#include <stdio.h>

#include <Cursor.h>
#include <InterfaceDefs.h>
#include <View.h>

#include <agg_math.h>

#include "cursors.h"
#include "support.h"

#include "TransformBox.h"
#include "Strings.h"

#include "DragStates.h"

// TODO: change all these to use rect_to_quad() matrix calculation...

//------------------------------------------------------------------
// DragState

// constructor
DragState::DragState(TransformBox* parent)
	: fOrigin(0.0, 0.0),
	  fParent(parent)
{
}

// SetOrigin
void
DragState::SetOrigin(BPoint origin)
{
	fOrigin = origin;
}

// ActionName
const char*
DragState::ActionName() const
{
	return "Transformation";
}

// ActionNameIndex
uint32
DragState::ActionNameIndex() const
{
	return TRANSFORMATION;
}

// _SetViewCursor
void
DragState::_SetViewCursor(BView* view, const uchar* cursorData) const
{
	BCursor cursor(cursorData);
	view->SetViewCursor(&cursor);
}

// #pragma mark - DragCornerState

// constructor
DragCornerState::DragCornerState(TransformBox* parent, uint32 corner)
	: DragState(parent),
	  fCorner(corner)
{
}

// SetOrigin
void
DragCornerState::SetOrigin(BPoint origin)
{
	fOldXScale = fParent->LocalXScale();
	fOldYScale = fParent->LocalYScale();

	fOldOffset = fParent->Translation();

	// copy the matrix at the start of the drag procedure
	fMatrix.reset();
	fMatrix.multiply(agg::trans_affine_scaling(fOldXScale, fOldYScale));
	fMatrix.multiply(agg::trans_affine_rotation(fParent->LocalRotation()
		* M_PI / 180.0));
	fMatrix.multiply(agg::trans_affine_translation(fParent->Translation().x,
												   fParent->Translation().y));

	double x = origin.x;
	double y = origin.y;
	fMatrix.inverse_transform(&x, &y);
	origin.x = x;
	origin.y = y;

	BRect box = fParent->Box();
	switch (fCorner) {
		case LEFT_TOP_CORNER:
			fXOffsetFromCorner = origin.x - box.left;
			fYOffsetFromCorner = origin.y - box.top;
			fOldWidth = box.left - box.right;
			fOldHeight = box.top - box.bottom;
			origin.x = box.right;
			origin.y = box.bottom;
			break;
		case RIGHT_TOP_CORNER:
			fXOffsetFromCorner = origin.x - box.right;
			fYOffsetFromCorner = origin.y - box.top;
			fOldWidth = box.right - box.left;
			fOldHeight = box.top - box.bottom;
			origin.x = box.left;
			origin.y = box.bottom;
			break;
		case LEFT_BOTTOM_CORNER:
			fXOffsetFromCorner = origin.x - box.left;
			fYOffsetFromCorner = origin.y - box.bottom;
			fOldWidth = box.left - box.right;
			fOldHeight = box.bottom - box.top;
			origin.x = box.right;
			origin.y = box.top;
			break;
		case RIGHT_BOTTOM_CORNER:
			fXOffsetFromCorner = origin.x - box.right;
			fYOffsetFromCorner = origin.y - box.bottom;
			fOldWidth = box.right - box.left;
			fOldHeight = box.bottom - box.top;
			origin.x = box.left;
			origin.y = box.top;
			break;
	}
	DragState::SetOrigin(origin);
}
// DragTo
void
DragCornerState::DragTo(BPoint current, uint32 modifiers)
{
	double x = current.x;
	double y = current.y;
	fMatrix.inverse_transform(&x, &y);

	double xScale = 1.0;
	double yScale = 1.0;
	BPoint translation(0.0, 0.0);
	switch (fCorner) {
		case LEFT_TOP_CORNER:
		case RIGHT_TOP_CORNER:
		case LEFT_BOTTOM_CORNER:
		case RIGHT_BOTTOM_CORNER:
			x -= fOrigin.x;
			y -= fOrigin.y;
			if (fOldWidth != 0.0)
				xScale = (x - fXOffsetFromCorner) / (fOldWidth);
			if (fOldHeight != 0.0)
				yScale = (y - fYOffsetFromCorner) / (fOldHeight);
			// constrain aspect ratio if shift is pressed
			if (modifiers & B_SHIFT_KEY) {
				if (fabs(xScale) > fabs(yScale))
					yScale = yScale > 0.0 ? fabs(xScale) : -fabs(xScale);
				else
					xScale = xScale > 0.0 ? fabs(yScale) : -fabs(yScale);
			}
			translation.x = fOrigin.x - fOrigin.x * xScale;
			translation.y = fOrigin.y - fOrigin.y * yScale;
			break;
	}
	x = translation.x;
	y = translation.y;
	fMatrix.transform(&x, &y);
	translation.x = x;
	translation.y = y;

	fParent->SetTranslationAndScale(translation,
									xScale * fOldXScale,
									yScale * fOldYScale);
}

// UpdateViewCursor
void
DragCornerState::UpdateViewCursor(BView* view, BPoint current) const
{
	float rotation = fmod(360.0 - fParent->ViewSpaceRotation() + 22.5, 180.0);
	bool flipX = fParent->LocalXScale() < 0.0;
	bool flipY = fParent->LocalYScale() < 0.0;
	if (rotation < 45.0) {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				else
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				else
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				break;
		}
	} else if (rotation < 90.0) {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				else
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				else
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				break;
		}
	} else if (rotation < 135.0) {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				else
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				break;
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				else
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				break;
				break;
		}
	} else {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				else
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				else
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				break;
		}
	}
}

// ActionName
const char*
DragCornerState::ActionName() const
{
	return "Scale";
}

// ActionNameIndex
uint32
DragCornerState::ActionNameIndex() const
{
	return SCALE;
}


// #pragma mark - ScaleCornerState

// constructor
ScaleCornerState::ScaleCornerState(TransformBox* parent, uint32 corner)
	: DragState(parent),
	  fCorner(corner)
{
}

// SetOrigin
void
ScaleCornerState::SetOrigin(BPoint origin)
{
	DragState::SetOrigin(origin);

	fLtX = fParent->LeftTop().x;
	fLtY = fParent->LeftTop().y;
	fRtX = fParent->RightTop().x;
	fRtY = fParent->RightTop().y;
	fLbX = fParent->LeftBottom().x;
	fLbY = fParent->LeftBottom().y;
	fRbX = fParent->RightBottom().x;
	fRbY = fParent->RightBottom().y;

	// init vector from opposite to dragged corner
	switch (fCorner) {
		case LEFT_TOP_CORNER:
			fVectorX = fLtX - fRbX;
			fVectorY = fLtY - fRbY;
			break;
		case RIGHT_TOP_CORNER:
			fVectorX = fRtX - fLbX;
			fVectorY = fRtY - fLbY;
			break;
		case LEFT_BOTTOM_CORNER:
			fVectorX = fLbX - fRtX;
			fVectorY = fLbY - fRtY;
			break;
		case RIGHT_BOTTOM_CORNER:
			fVectorX = fRbX - fLtX;
			fVectorY = fRbY - fLtY;
			break;
	}

	printf("box: (%.1f, %.1f)->(%.1f, %.1f)->(%.1f, %.1f)->(%.1f, %.1f)\n",
		fLtX, fLtY, fRtX, fRtY, fRbX, fRbY, fLbX, fLbY);

	printf("vector: (%.1f, %.1f)\n", fVectorX, fVectorY);
}

// DragTo
void
ScaleCornerState::DragTo(BPoint current, uint32 modifiers)
{
	BPoint offset = current - fOrigin;

	double vectorX = 0.0;
	double vectorY = 0.0;

	double anchorX = 0.0;
	double anchorY = 0.0;

	switch (fCorner) {
		case LEFT_TOP_CORNER:
			vectorX = (fLtX + offset.x) - fRbX;
			vectorY = (fLtY + offset.y) - fRbY;
			anchorX = fRbX;
			anchorY = fRbY;
			break;
		case RIGHT_TOP_CORNER:
			vectorX = (fRtX + offset.x) - fLbX;
			vectorY = (fRtY + offset.y) - fLbY;
			anchorX = fLbX;
			anchorY = fLbY;
			break;
		case LEFT_BOTTOM_CORNER:
			vectorX = (fLbX + offset.x) - fRtX;
			vectorY = (fLbY + offset.y) - fRtY;
			anchorX = fRtX;
			anchorY = fRtY;
			break;
		case RIGHT_BOTTOM_CORNER:
			vectorX = (fRbX + offset.x) - fLtX;
			vectorY = (fRbY + offset.y) - fLtY;
			anchorX = fLtX;
			anchorY = fLtY;
			break;
	}

	printf("new vector: (%.1f, %.1f)\n", vectorX, vectorY);
	printf("    anchor: (%.1f, %.1f)\n", anchorX, anchorY);

	double scaleX = vectorX / fVectorX;
	double scaleY = vectorY / fVectorY;
printf("Scale (%.3f, %.3f)\n", scaleX, scaleX);

	if (fabs(scaleX) > 10000.0 || fabs(scaleX) > 10000.0) {
		printf("scale too large...\n");
		return;
	}

	agg::trans_affine m;
	m *= agg::trans_affine_translation(-anchorX, -anchorY);
	m *= agg::trans_affine_scaling(scaleX, scaleY);
	m *= agg::trans_affine_translation(anchorX, anchorY);

	double ltX = fLtX;
	double ltY = fLtY;
	double rtX = fRtX;
	double rtY = fRtY;
	double lbX = fLbX;
	double lbY = fLbY;
	double rbX = fRbX;
	double rbY = fRbY;

	m.transform(&ltX, &ltY);
	m.transform(&rtX, &rtY);
	m.transform(&lbX, &lbY);
	m.transform(&rbX, &rbY);

	fParent->SetQuad(BPoint(ltX, ltY), BPoint(rtX, rtY),
					 BPoint(lbX, lbY), BPoint(rbX, rbY));
}

// UpdateViewCursor
void
ScaleCornerState::UpdateViewCursor(BView* view, BPoint current) const
{
	_SetViewCursor(view, kPathMoveCursor);
}

// ActionName
const char*
ScaleCornerState::ActionName() const
{
	return "Scale";
}

// ActionNameIndex
uint32
ScaleCornerState::ActionNameIndex() const
{
	return SCALE;
}


// #pragma mark - ScaleCornerState2

//This implementation is the most "precise" in that it doesn't
//distort the angles of the lines, it just drags the corner.
//The problem with it is that I didn't know how to implement
//shift-dragging, which is supposed to keep the x and y scale
//the same. When the rectangle shape is distorted, I don't really
//now what that would mean anyways.

// constructor
ScaleCornerState2::ScaleCornerState2(TransformBox* parent, uint32 corner)
	: DragState(parent),
	  fCorner(corner)
{
}

// SetOrigin
void
ScaleCornerState2::SetOrigin(BPoint origin)
{
	DragState::SetOrigin(origin);

	// init vectors
	switch (fCorner) {
		case LEFT_TOP_CORNER:
			// A  B
			// D  C
			fAnchor = fParent->RightBottom();
			fCD = vector(fAnchor, fParent->LeftBottom());
			fCB = vector(fAnchor, fParent->RightTop());

			fAD = vector(fParent->LeftTop(), fParent->LeftBottom());
			fAB = vector(fParent->LeftTop(), fParent->RightTop());
			break;
		case RIGHT_TOP_CORNER:
			// D  A
			// C  B
			fAnchor = fParent->LeftBottom();
			fCD = vector(fAnchor, fParent->LeftTop());
			fCB = vector(fAnchor, fParent->RightBottom());

			fAD = vector(fParent->RightTop(), fParent->LeftTop());
			fAB = vector(fParent->RightTop(), fParent->RightBottom());
			break;
		case LEFT_BOTTOM_CORNER:
			// B  C
			// A  D
			fAnchor = fParent->RightTop();
			fCD = vector(fAnchor, fParent->RightBottom());
			fCB = vector(fAnchor, fParent->LeftTop());

			fAD = vector(fParent->LeftBottom(), fParent->RightBottom());
			fAB = vector(fParent->LeftBottom(), fParent->LeftTop());
			break;
		case RIGHT_BOTTOM_CORNER:
			// C  D
			// B  A
			fAnchor = fParent->LeftTop();
			fCD = vector(fAnchor, fParent->RightTop());
			fCB = vector(fAnchor, fParent->LeftBottom());

			fAD = vector(fParent->RightBottom(), fParent->RightTop());
			fAB = vector(fParent->RightBottom(), fParent->LeftBottom());
			break;
	}
}

// DragTo
void
ScaleCornerState2::DragTo(BPoint current, uint32 modifiers)
{
	BPoint offset = current - fOrigin;

	BPoint lt = fParent->LeftTop();
	BPoint rt = fParent->RightTop();
	BPoint lb = fParent->LeftBottom();
	BPoint rb = fParent->RightBottom();

	BPoint a;
	double bX;
	double bY;
	double dX;
	double dY;
	BPoint c = fAnchor;

	switch (fCorner) {
		case LEFT_TOP_CORNER:
			a = lt + offset;
			break;
		case RIGHT_TOP_CORNER:
			a = rt + offset;
			break;
		case LEFT_BOTTOM_CORNER:
			a = lb + offset;
			break;
		case RIGHT_BOTTOM_CORNER:
			a = rb + offset;
			break;
	}

	// TODO: returns bool -> use to detect invalid scales!
	agg::calc_intersection(c.x, c.y, c.x + fCD.x, c.y + fCD.y,
						   a.x, a.y, a.x + fAD.x, a.y + fAD.y,
						   &dX, &dY);
	agg::calc_intersection(c.x, c.y, c.x + fCB.x, c.y + fCB.y,
						   a.x, a.y, a.x + fAB.x, a.y + fAB.y,
						   &bX, &bY);

	switch (fCorner) {
		case LEFT_TOP_CORNER:
			// A  B
			// D  C
			lt = a;
			rt.x = bX;
			rt.y = bY;
			lb.x = dX;
			lb.y = dY;
			rb = c;
			break;
		case RIGHT_TOP_CORNER:
			// D  A
			// C  B
			rt = a;
			rb.x = bX;
			rb.y = bY;
			lt.x = dX;
			lt.y = dY;
			lb = c;
			break;
		case LEFT_BOTTOM_CORNER:
			// B  C
			// A  D
			lb = a;
			lt.x = bX;
			lt.y = bY;
			rb.x = dX;
			rb.y = dY;
			rt = c;
			break;
		case RIGHT_BOTTOM_CORNER:
			// C  D
			// B  A
			rb = a;
			lb.x = bX;
			lb.y = bY;
			rt.x = dX;
			rt.y = dY;
			lt = c;
			break;
	}

	DragState::SetOrigin(current);
	fParent->SetQuad(lt, rt, lb, rb);
}

// UpdateViewCursor
void
ScaleCornerState2::UpdateViewCursor(BView* view, BPoint current) const
{
	// TODO: doesn't work well for perspective transformations
	float rotation = fmod(360.0 - fParent->ViewSpaceRotation() + 22.5, 180.0);
	bool flipX = fParent->LocalXScale() < 0.0;
	bool flipY = fParent->LocalYScale() < 0.0;
	if (rotation < 45.0) {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				else
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				else
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				break;
		}
	} else if (rotation < 90.0) {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				else
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				else
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				break;
		}
	} else if (rotation < 135.0) {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				else
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				break;
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				else
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				break;
				break;
		}
	} else {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				else
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				else
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				break;
		}
	}
}

// ActionName
const char*
ScaleCornerState2::ActionName() const
{
	return "Scale";
}

// ActionNameIndex
uint32
ScaleCornerState2::ActionNameIndex() const
{
	return SCALE;
}


// #pragma mark - ScaleCornerState3

// This one works by transforming everything inversely
// then creating a transformation on the original box
// which is then applied *before* the already given
// transformation that was current when the drag started.

ScaleCornerState3::ScaleCornerState3(TransformBox* parent, uint32 corner)
	: DragState(parent),
	  fCorner(corner),
	  fOffsetFromCorner(B_ORIGIN),
	  fOldTransform()
{
}

// SetOrigin
void
ScaleCornerState3::SetOrigin(BPoint origin)
{
	fOldTransform = *fParent;

	fOldTransform.InverseTransform(&origin);

	BRect box = fParent->Box();
	switch (fCorner) {
		case LEFT_TOP_CORNER:
			fOffsetFromCorner = origin - box.LeftTop();
			break;
		case RIGHT_TOP_CORNER:
			fOffsetFromCorner = origin - box.RightTop();
			break;
		case LEFT_BOTTOM_CORNER:
			fOffsetFromCorner = origin - box.LeftBottom();
			break;
		case RIGHT_BOTTOM_CORNER:
			fOffsetFromCorner = origin - box.RightBottom();
			break;
	}

	DragState::SetOrigin(origin);
}

// DragTo
void
ScaleCornerState3::DragTo(BPoint current, uint32 modifiers)
{
	fOldTransform.InverseTransform(&current);
	
	BRect oldBox = fParent->Box();
	BRect newBox = oldBox;

	switch (fCorner) {
		case LEFT_TOP_CORNER:
			newBox.left = current.x - fOffsetFromCorner.x;
			newBox.top = current.y - fOffsetFromCorner.y;
			break;
		case RIGHT_TOP_CORNER:
			newBox.right = current.x - fOffsetFromCorner.x;
			newBox.top = current.y - fOffsetFromCorner.y;
			break;
		case LEFT_BOTTOM_CORNER:
			newBox.left = current.x - fOffsetFromCorner.x;
			newBox.bottom = current.y - fOffsetFromCorner.y;
			break;
		case RIGHT_BOTTOM_CORNER:
			newBox.right = current.x - fOffsetFromCorner.x;
			newBox.bottom = current.y - fOffsetFromCorner.y;
			break;
	}

	if ((modifiers & B_SHIFT_KEY)
		&& oldBox.Width() != 0.0 && oldBox.Height() != 0.0) {

		double xScale = newBox.Width() / oldBox.Width();
		double yScale = newBox.Height() / oldBox.Height();

		if (fabs(xScale) > fabs(yScale))
			yScale = yScale > 0.0 ? fabs(xScale) : -fabs(xScale);
		else
			xScale = xScale > 0.0 ? fabs(yScale) : -fabs(yScale);

		switch (fCorner) {
			case LEFT_TOP_CORNER:
				newBox.left = oldBox.right - oldBox.Width() * xScale;
				newBox.top = oldBox.bottom - oldBox.Height() * yScale;
				break;
			case RIGHT_TOP_CORNER:
				newBox.right = oldBox.left + oldBox.Width() * xScale;
				newBox.top = oldBox.bottom - oldBox.Height() * yScale;
				break;
			case LEFT_BOTTOM_CORNER:
				newBox.left = oldBox.right - oldBox.Width() * xScale;
				newBox.bottom = oldBox.top + oldBox.Height() * yScale;
				break;
			case RIGHT_BOTTOM_CORNER:
				newBox.right = oldBox.left + oldBox.Width() * xScale;
				newBox.bottom = oldBox.top + oldBox.Height() * yScale;
				break;
		}
	}

	Transformable t;
	t.rect_to_rect(oldBox.left, oldBox.top, oldBox.right, oldBox.bottom,
				   newBox.left, newBox.top, newBox.right, newBox.bottom);

	t.Multiply(fOldTransform);
	fParent->Set(t, B_ORIGIN);
}

// UpdateViewCursor
void
ScaleCornerState3::UpdateViewCursor(BView* view, BPoint current) const
{
	// TODO: doesn't work well for perspective transformations
	float rotation = fmod(360.0 - fParent->ViewSpaceRotation() + 22.5, 180.0);
	bool flipX = fParent->LocalXScale() < 0.0;
	bool flipY = fParent->LocalYScale() < 0.0;
	if (rotation < 45.0) {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				else
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				else
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				break;
		}
	} else if (rotation < 90.0) {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				else
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				else
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				break;
		}
	} else if (rotation < 135.0) {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				else
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				break;
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftTopRightBottomCursor
											   : kLeftBottomRightTopCursor);
				else
					_SetViewCursor(view, flipY ? kLeftBottomRightTopCursor
											   : kLeftTopRightBottomCursor);
				break;
				break;
		}
	} else {
		switch (fCorner) {
			case LEFT_TOP_CORNER:
			case RIGHT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				else
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				break;
			case RIGHT_TOP_CORNER:
			case LEFT_BOTTOM_CORNER:
				if (flipX)
					_SetViewCursor(view, flipY ? kLeftRightCursor
											   : kUpDownCursor);
				else
					_SetViewCursor(view, flipY ? kUpDownCursor
											   : kLeftRightCursor);
				break;
		}
	}
}

// ActionName
const char*
ScaleCornerState3::ActionName() const
{
	return "Scale";
}

// ActionNameIndex
uint32
ScaleCornerState3::ActionNameIndex() const
{
	return SCALE;
}

// #pragma mark - DragSideState


DragSideState::DragSideState(TransformBox* parent, uint32 side)
	: DragState(parent),
	  fSide(side)
{
}

// SetOrigin
void
DragSideState::SetOrigin(BPoint origin)
{
	fOldXScale = fParent->LocalXScale();
	fOldYScale = fParent->LocalYScale();

	fOldOffset = fParent->Translation();

	// copy the matrix at the start of the drag procedure
	fMatrix.reset();
	fMatrix.multiply(agg::trans_affine_scaling(fOldXScale, fOldYScale));
	fMatrix.multiply(agg::trans_affine_rotation(fParent->LocalRotation()
		* M_PI / 180.0));
	fMatrix.multiply(agg::trans_affine_translation(fParent->Translation().x,
												   fParent->Translation().y));

	double x = origin.x;
	double y = origin.y;
	fMatrix.inverse_transform(&x, &y);
	origin.x = x;
	origin.y = y;

	BRect box = fParent->Box();
	switch (fSide) {
		case LEFT_SIDE:
			fOffsetFromSide = origin.x - box.left;
			fOldSideDist = box.left - box.right;
			origin.x = box.right;
			break;
		case RIGHT_SIDE:
			fOffsetFromSide = origin.x - box.right;
			fOldSideDist = box.right - box.left;
			origin.x = box.left;
			break;
		case TOP_SIDE:
			fOffsetFromSide = origin.y - box.top;
			fOldSideDist = box.top - box.bottom;
			origin.y = box.bottom;
			break;
		case BOTTOM_SIDE:
			fOffsetFromSide = origin.y - box.bottom;
			fOldSideDist = box.bottom - box.top;
			origin.y = box.top;
			break;
	}
	DragState::SetOrigin(origin);
}

// DragTo
void
DragSideState::DragTo(BPoint current, uint32 modifiers)
{
	double x = current.x;
	double y = current.y;
	fMatrix.inverse_transform(&x, &y);

	double xScale = 1.0;
	double yScale = 1.0;
	BPoint translation(0.0, 0.0);
	switch (fSide) {
		case LEFT_SIDE:
		case RIGHT_SIDE:
			x -= fOrigin.x;
			if (fOldSideDist != 0.0)
				xScale = (x - fOffsetFromSide) / (fOldSideDist);
			translation.x = fOrigin.x - fOrigin.x * xScale;
			break;
		case TOP_SIDE:
		case BOTTOM_SIDE:
			y -= fOrigin.y;
			if (fOldSideDist != 0.0)
				yScale = (y - fOffsetFromSide) / (fOldSideDist);
			translation.y = fOrigin.y - fOrigin.y * yScale;
			break;
	}
	x = translation.x;
	y = translation.y;
	fMatrix.transform(&x, &y);
	translation.x = x;
	translation.y = y;

	fParent->SetTranslationAndScale(translation,
									xScale * fOldXScale,
									yScale * fOldYScale);
}

// UpdateViewCursor
void
DragSideState::UpdateViewCursor(BView* view, BPoint current) const
{
	float rotation = fmod(360.0 - fParent->ViewSpaceRotation() + 22.5, 180.0);
	if (rotation < 45.0) {
		switch (fSide) {
			case LEFT_SIDE:
			case RIGHT_SIDE:
				_SetViewCursor(view, kLeftRightCursor);
				break;
			case TOP_SIDE:
			case BOTTOM_SIDE:
				_SetViewCursor(view, kUpDownCursor);
				break;
		}
	} else if (rotation < 90.0) {
		switch (fSide) {
			case LEFT_SIDE:
			case RIGHT_SIDE:
				_SetViewCursor(view, kLeftBottomRightTopCursor);
				break;
			case TOP_SIDE:
			case BOTTOM_SIDE:
				_SetViewCursor(view, kLeftTopRightBottomCursor);
				break;
		}
	} else if (rotation < 135.0) {
		switch (fSide) {
			case LEFT_SIDE:
			case RIGHT_SIDE:
				_SetViewCursor(view, kUpDownCursor);
				break;
			case TOP_SIDE:
			case BOTTOM_SIDE:
				_SetViewCursor(view, kLeftRightCursor);
				break;
		}
	} else {
		switch (fSide) {
			case LEFT_SIDE:
			case RIGHT_SIDE:
				_SetViewCursor(view, kLeftTopRightBottomCursor);
				break;
			case TOP_SIDE:
			case BOTTOM_SIDE:
				_SetViewCursor(view, kLeftBottomRightTopCursor);
				break;
		}
	}
}

// ActionName
const char*
DragSideState::ActionName() const
{
	return "Scale";
}

// ActionNameIndex
uint32
DragSideState::ActionNameIndex() const
{
	return SCALE;
}


// #pragma mark - ScaleSideState


ScaleSideState::ScaleSideState(TransformBox* parent, uint32 side)
	: DragState(parent),
	  fSide(side)
{
}

// SetOrigin
void
ScaleSideState::SetOrigin(BPoint origin)
{
	fOldTransform = *fParent;

	fOldTransform.InverseTransform(&origin);

	BRect box = fParent->Box();
	switch (fSide) {
		case LEFT_SIDE:
			fOffsetFromSide = origin.x - box.left;
			break;
		case RIGHT_SIDE:
			fOffsetFromSide = origin.x - box.right;
			break;
		case TOP_SIDE:
			fOffsetFromSide = origin.y - box.top;
			break;
		case BOTTOM_SIDE:
			fOffsetFromSide = origin.y - box.bottom;
			break;
	}

	DragState::SetOrigin(origin);
}

// DragTo
void
ScaleSideState::DragTo(BPoint current, uint32 modifiers)
{
	fOldTransform.InverseTransform(&current);
	
	BRect oldBox = fParent->Box();
	BRect newBox = oldBox;

	switch (fSide) {
		case LEFT_SIDE:
			newBox.left = current.x - fOffsetFromSide;
			break;
		case RIGHT_SIDE:
			newBox.right = current.x - fOffsetFromSide;
			break;
		case TOP_SIDE:
			newBox.top = current.y - fOffsetFromSide;
			break;
		case BOTTOM_SIDE:
			newBox.bottom = current.y - fOffsetFromSide;
			break;
	}

	Transformable t;
	t.rect_to_rect(oldBox.left, oldBox.top, oldBox.right, oldBox.bottom,
				   newBox.left, newBox.top, newBox.right, newBox.bottom);

	t.Multiply(fOldTransform);
	fParent->Set(t, B_ORIGIN);
}

// UpdateViewCursor
void
ScaleSideState::UpdateViewCursor(BView* view, BPoint current) const
{
	float rotation = fmod(360.0 - fParent->ViewSpaceRotation() + 22.5, 180.0);
	if (rotation < 45.0) {
		switch (fSide) {
			case LEFT_SIDE:
			case RIGHT_SIDE:
				_SetViewCursor(view, kLeftRightCursor);
				break;
			case TOP_SIDE:
			case BOTTOM_SIDE:
				_SetViewCursor(view, kUpDownCursor);
				break;
		}
	} else if (rotation < 90.0) {
		switch (fSide) {
			case LEFT_SIDE:
			case RIGHT_SIDE:
				_SetViewCursor(view, kLeftBottomRightTopCursor);
				break;
			case TOP_SIDE:
			case BOTTOM_SIDE:
				_SetViewCursor(view, kLeftTopRightBottomCursor);
				break;
		}
	} else if (rotation < 135.0) {
		switch (fSide) {
			case LEFT_SIDE:
			case RIGHT_SIDE:
				_SetViewCursor(view, kUpDownCursor);
				break;
			case TOP_SIDE:
			case BOTTOM_SIDE:
				_SetViewCursor(view, kLeftRightCursor);
				break;
		}
	} else {
		switch (fSide) {
			case LEFT_SIDE:
			case RIGHT_SIDE:
				_SetViewCursor(view, kLeftTopRightBottomCursor);
				break;
			case TOP_SIDE:
			case BOTTOM_SIDE:
				_SetViewCursor(view, kLeftBottomRightTopCursor);
				break;
		}
	}
}

// ActionName
const char*
ScaleSideState::ActionName() const
{
	return "Scale";
}

// ActionNameIndex
uint32
ScaleSideState::ActionNameIndex() const
{
	return SCALE;
}


// #pragma mark - DragBoxState


// SetOrigin
void
DragBoxState::SetOrigin(BPoint origin)
{
	fOldTranslation = fParent->Translation();
	DragState::SetOrigin(origin);
}

// DragTo
void
DragBoxState::DragTo(BPoint current, uint32 modifiers)
{
	BPoint offset = current - fOrigin;
	BPoint newTranslation = fOldTranslation + offset;
	if (modifiers & B_SHIFT_KEY) {
		if (fabs(offset.x) > fabs(offset.y))
			newTranslation.y = fOldTranslation.y;
		else
			newTranslation.x = fOldTranslation.x;
	}
	fParent->TranslateBy(newTranslation - fParent->Translation());
}

// UpdateViewCursor
void
DragBoxState::UpdateViewCursor(BView* view, BPoint current) const
{
	_SetViewCursor(view, kMoveCursor);
}

// ActionName
const char*
DragBoxState::ActionName() const
{
	return "Move";
}

// ActionNameIndex
uint32
DragBoxState::ActionNameIndex() const
{
	return MOVE;
}


// #pragma mark - DragBoxState2


// SetOrigin
void
DragBoxState2::SetOrigin(BPoint origin)
{
	fOldTransform = *fParent;
	DragState::SetOrigin(origin);
}

// DragTo
void
DragBoxState2::DragTo(BPoint current, uint32 modifiers)
{
	BPoint offset = current - fOrigin;
	if (modifiers & B_SHIFT_KEY) {
		if (fabs(offset.x) > fabs(offset.y))
			offset.y = 0.0;
		else
			offset.x = 0.0;
	}
	Transformable newTransform(fOldTransform);
	newTransform.TranslateBy(offset);
	fParent->Set(newTransform, B_ORIGIN);
}

// UpdateViewCursor
void
DragBoxState2::UpdateViewCursor(BView* view, BPoint current) const
{
	_SetViewCursor(view, kMoveCursor);
}

// ActionName
const char*
DragBoxState2::ActionName() const
{
	return "Move";
}

// ActionNameIndex
uint32
DragBoxState2::ActionNameIndex() const
{
	return MOVE;
}


// #pragma mark - RotateBoxState


// constructor
RotateBoxState::RotateBoxState(TransformBox* parent)
	: DragState(parent),
	  fOldAngle(0.0)
{
}

// SetOrigin
void
RotateBoxState::SetOrigin(BPoint origin)
{
	DragState::SetOrigin(origin);
	fOldAngle = fParent->LocalRotation();
}

// DragTo
void
RotateBoxState::DragTo(BPoint current, uint32 modifiers)
{
	double angle = calc_angle(fParent->Origin(), fOrigin, current);

	if (modifiers & B_SHIFT_KEY) {
		if (angle < 0.0)
			angle -= 22.5;
		else
			angle += 22.5;
		angle = 45.0 * ((int32)angle / 45);
	}

	double newAngle = fOldAngle + angle;

	fParent->RotateBy(fParent->Origin(), newAngle - fParent->LocalRotation());
}

// UpdateViewCursor
void
RotateBoxState::UpdateViewCursor(BView* view, BPoint current) const
{
	BPoint origin(fParent->Origin());
	fParent->TransformToCanvas(origin);
	fParent->TransformToCanvas(current);
	BPoint from = origin + BPoint(sinf(22.5 * 180.0 / M_PI) * 50.0,
								  -cosf(22.5 * 180.0 / M_PI) * 50.0);

	float rotation = calc_angle(origin, from, current) + 180.0;

	if (rotation < 45.0) {
		_SetViewCursor(view, kRotateLCursor);
	} else if (rotation < 90.0) {
		_SetViewCursor(view, kRotateLTCursor);
	} else if (rotation < 135.0) {
		_SetViewCursor(view, kRotateTCursor);
	} else if (rotation < 180.0) {
		_SetViewCursor(view, kRotateRTCursor);
	} else if (rotation < 225.0) {
		_SetViewCursor(view, kRotateRCursor);
	} else if (rotation < 270.0) {
		_SetViewCursor(view, kRotateRBCursor);
	} else if (rotation < 315.0) {
		_SetViewCursor(view, kRotateBCursor);
	} else {
		_SetViewCursor(view, kRotateLBCursor);
	}
}

// ActionName
const char*
RotateBoxState::ActionName() const
{
	return "Rotate";
}

// ActionNameIndex
uint32
RotateBoxState::ActionNameIndex() const
{
	return ROTATE;
}


// #pragma mark - RotateBoxState2


// constructor
RotateBoxState2::RotateBoxState2(TransformBox* parent)
	: DragState(parent),
	  fOldTransform()
{
}

// SetOrigin
void
RotateBoxState2::SetOrigin(BPoint origin)
{
	DragState::SetOrigin(origin);
	fOldTransform = *fParent;
}

// DragTo
void
RotateBoxState2::DragTo(BPoint current, uint32 modifiers)
{
	double angle = calc_angle(fParent->Origin(), fOrigin, current);

	if (modifiers & B_SHIFT_KEY) {
		if (angle < 0.0)
			angle -= 22.5;
		else
			angle += 22.5;
		angle = 45.0 * ((int32)angle / 45);
	}

	Transformable newTransform(fOldTransform);
	newTransform.RotateBy(fParent->Origin(), angle);

	fParent->Set(newTransform, B_ORIGIN);
}

// UpdateViewCursor
void
RotateBoxState2::UpdateViewCursor(BView* view, BPoint current) const
{
	BPoint origin(fParent->Origin());
	fParent->TransformToCanvas(origin);
	fParent->TransformToCanvas(current);
	BPoint from = origin + BPoint(sinf(22.5 * 180.0 / M_PI) * 50.0,
								  -cosf(22.5 * 180.0 / M_PI) * 50.0);

	float rotation = calc_angle(origin, from, current) + 180.0;

	if (rotation < 45.0) {
		_SetViewCursor(view, kRotateLCursor);
	} else if (rotation < 90.0) {
		_SetViewCursor(view, kRotateLTCursor);
	} else if (rotation < 135.0) {
		_SetViewCursor(view, kRotateTCursor);
	} else if (rotation < 180.0) {
		_SetViewCursor(view, kRotateRTCursor);
	} else if (rotation < 225.0) {
		_SetViewCursor(view, kRotateRCursor);
	} else if (rotation < 270.0) {
		_SetViewCursor(view, kRotateRBCursor);
	} else if (rotation < 315.0) {
		_SetViewCursor(view, kRotateBCursor);
	} else {
		_SetViewCursor(view, kRotateLBCursor);
	}
}

// ActionName
const char*
RotateBoxState2::ActionName() const
{
	return "Rotate";
}

// ActionNameIndex
uint32
RotateBoxState2::ActionNameIndex() const
{
	return ROTATE;
}


// #pragma mark - OffsetCenterState


// SetOrigin
void
OffsetCenterState::SetOrigin(BPoint origin)
{
	fParent->InverseTransform(&origin);
	DragState::SetOrigin(origin);
}


// DragTo
void
OffsetCenterState::DragTo(BPoint current, uint32 modifiers)
{
	fParent->InverseTransform(&current);
	fParent->OffsetOrigin(current - fOrigin);
	fOrigin = current;
//	printf("OffsetCenterState::DragTo()\n");
}

// UpdateViewCursor
void
OffsetCenterState::UpdateViewCursor(BView* view, BPoint current) const
{
	_SetViewCursor(view, kPathMoveCursor);
}

// ActionName
const char*
OffsetCenterState::ActionName() const
{
	return "Move Pivot";
}

// ActionNameIndex
uint32
OffsetCenterState::ActionNameIndex() const
{
	return MOVE_PIVOT;
}


// #pragma mark - OffsetCornerState


// constructor
OffsetCornerState::OffsetCornerState(TransformBox* parent, uint32 corner)
	: DragState(parent),
	  fCorner(corner)
{
}

// SetOrigin
void
OffsetCornerState::SetOrigin(BPoint origin)
{
	DragState::SetOrigin(origin);
}

// DragTo
void
OffsetCornerState::DragTo(BPoint current, uint32 modifiers)
{
	BPoint offset(current.x - fOrigin.x, current.y - fOrigin.y);
	switch (fCorner) {
		case LEFT_TOP_CORNER:
			fParent->OffsetQuadLeftTop(offset);
			break;
		case RIGHT_TOP_CORNER:
			fParent->OffsetQuadRightTop(offset);
			break;
		case LEFT_BOTTOM_CORNER:
			fParent->OffsetQuadLeftBottom(offset);
			break;
		case RIGHT_BOTTOM_CORNER:
			fParent->OffsetQuadRightBottom(offset);
			break;
	}
	SetOrigin(current);
}

// UpdateViewCursor
void
OffsetCornerState::UpdateViewCursor(BView* view, BPoint current) const
{
	_SetViewCursor(view, kMoveCursor);
}

// ActionName
const char*
OffsetCornerState::ActionName() const
{
	return "Drag Corner";
}

// ActionNameIndex
uint32
OffsetCornerState::ActionNameIndex() const
{
	return 0;//DRAG_CORNER;
}



