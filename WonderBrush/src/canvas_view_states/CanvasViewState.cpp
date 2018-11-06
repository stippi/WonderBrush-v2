// CanvasViewState.cpp

#include <stdio.h>

#include <AppDefs.h>

#include <agg_ellipse.h>
#include <agg_conv_transform.h>
#include <agg_trans_affine.h>

#include "defines.h"
#include "support.h"

#include "CanvasView.h"
#include "Stroke.h"

#include "CanvasViewState.h"

bool
CanvasViewState::fMouseDown = false;

// constructor
CanvasViewState::CanvasViewState(CanvasView* parent)
	: fCanvasView(parent),
	  fPrecise(true)
{
}

// destructor
CanvasViewState::~CanvasViewState()
{
}

// Init
void
CanvasViewState::Init(Canvas* canvas, Layer* layer, BPoint lastMousePosition)
{
	UpdateToolCursor();
	DrawCursor(lastMousePosition);
}

// CleanUp
void
CanvasViewState::CleanUp()
{
	ClearCursor();
}

// SetTool
void
CanvasViewState::SetTool(uint32 tool)
{
}

// EditModifier
bool
CanvasViewState::EditModifier(Stroke* modifier)
{
	printf("CanvasViewState::EditModifier()\n");
	return false;
}

// MouseDown
void
CanvasViewState::MouseDown(BPoint where, Point canvasWhere, bool erasor)
{
	fMouseDown = true;
}

// MouseUp
void
CanvasViewState::MouseUp(BPoint where, Point canvasWhere)
{
	fMouseDown = false;
}

// MouseMoved
void
CanvasViewState::MouseMoved(BPoint where, Point canvasWhere,
							uint32 transit, const BMessage* dragMessage)
{
	// cursor stuff
	if (transit == B_ENTERED_VIEW) {
		UpdateToolCursor();
	} else if (transit == B_INSIDE_VIEW) {
		DrawCursor(where);
	} else if (transit == B_EXITED_VIEW) {
		ClearCursor();
		fCanvasView->SetViewCursor(B_CURSOR_SYSTEM_DEFAULT, true);
	}
}

// ModifierSelectionChanged
void
CanvasViewState::ModifierSelectionChanged(bool itemsSelected)
{
}

// FinishLayer
void
CanvasViewState::FinishLayer(BRect dirtyRect)
{
}

// Draw
void
CanvasViewState::Draw(BView* into, BRect updateRect)
{
}

// ModifyClippingRegion
void
CanvasViewState::ModifyClippingRegion(BView* view, BRegion* region)
{
}

// MessageReceived
bool
CanvasViewState::MessageReceived(BMessage* message)
{
	return false;
}

// ModifiersChanged
void
CanvasViewState::ModifiersChanged()
{
}

// HandleKeyDown
bool
CanvasViewState::HandleKeyDown(uint32 key, uint32 modifiers)
{
	return false;
}

// HandleKeyUp
bool
CanvasViewState::HandleKeyUp(uint32 key, uint32 modifiers)
{
	return false;
}

// UpdateToolCursor
void
CanvasViewState::UpdateToolCursor()
{
}

// DrawCursor
void
CanvasViewState::DrawCursor(BPoint where, bool invisible)
{
}

// ClearCursor
void
CanvasViewState::ClearCursor(BRect* updateRect)
{
}

// SetPrecise
void
CanvasViewState::SetPrecise(bool precise)
{
	fPrecise = precise;
}

// DrawCross cross
void
CanvasViewState::DrawCross(BView* v, BPoint p, float r1, float r2)
{
	v->BeginLineArray(16);
		v->AddLine(BPoint(p.x - r1, p.y),
				   BPoint(p.x - r2, p.y), kBlack);
		v->AddLine(BPoint(p.x + r1, p.y),
				   BPoint(p.x + r2, p.y), kBlack);
		v->AddLine(BPoint(p.x, p.y - r1),
				   BPoint(p.x, p.y - r2), kBlack);
		v->AddLine(BPoint(p.x, p.y + r1),
				   BPoint(p.x, p.y + r2), kBlack);
		// white outline
		v->AddLine(BPoint(p.x - r2, p.y + 1.0),
				   BPoint(p.x - r1, p.y + 1.0), kWhite);
		v->AddLine(BPoint(p.x - r1, p.y - 1.0),
				   BPoint(p.x - r2, p.y - 1.0), kWhite);

		v->AddLine(BPoint(p.x + r2, p.y + 1.0),
				   BPoint(p.x + r1, p.y + 1.0), kWhite);
		v->AddLine(BPoint(p.x + r1, p.y - 1.0),
				   BPoint(p.x + r2, p.y - 1.0), kWhite);

		v->AddLine(BPoint(p.x - 1.0, p.y - r2),
				   BPoint(p.x - 1.0, p.y - r1), kWhite);
		v->AddLine(BPoint(p.x + 1.0, p.y - r1),
				   BPoint(p.x + 1.0, p.y - r2), kWhite);

		v->AddLine(BPoint(p.x - 1.0, p.y + r2),
				   BPoint(p.x - 1.0, p.y + r1), kWhite);
		v->AddLine(BPoint(p.x + 1.0, p.y + r1),
				   BPoint(p.x + 1.0, p.y + r2), kWhite);
	v->EndLineArray();
}

// stroke_agg_ellipse
void
stroke_agg_ellipse(BView* v, BPoint p, float radius,
				   rgb_color high, rgb_color low,
				   float tiltX, float tiltY)
{
	v->SetDrawingMode(B_OP_OVER);
	BPoint coordOffset(B_ORIGIN);
	if ((v->Flags() & B_SUBPIXEL_PRECISE) != 0)
		coordOffset += BPoint(0.5, 0.5);

	agg::ellipse ellipse(0.0, 0.0, radius, radius, ceilf(radius) * 2.0);

	// ellipse transformation
	agg::trans_affine ellipseTransform;

	float invTiltX = 1.0 - fabs(tiltX);
	float invTiltY = 1.0 - fabs(tiltY);
	double xScale = (sqrtf(invTiltX * invTiltX + invTiltY * invTiltY) / sqrtf(2.0));

	double angle = calc_angle(B_ORIGIN, BPoint(tiltX, 0.0), BPoint(tiltX, tiltY), false);
	ellipseTransform *= agg::trans_affine_scaling(xScale, 1.0);
	ellipseTransform *= agg::trans_affine_rotation(angle);
	ellipseTransform *= agg::trans_affine_translation(p.x, p.y);

	// transformed ellipse
	agg::conv_transform<agg::ellipse, agg::trans_affine> e(ellipse, ellipseTransform);

	double x;
	double y;
	unsigned cmd = e.vertex(&x, &y);
	bool h = false;
	while (cmd == agg::path_cmd_move_to || cmd == agg::path_cmd_line_to) {
		if (cmd == agg::path_cmd_move_to) {
			v->MovePenTo(BPoint(x, y) + coordOffset);
		} else {
			v->SetHighColor(h ? high : low);
			v->StrokeLine(BPoint(x, y) + coordOffset);
		}
		h = !h;
		cmd = e.vertex(&x, &y);
	}
	e.rewind(0);
	cmd = e.vertex(&x, &y);
	v->SetHighColor(h ? high : low);
	v->StrokeLine(BPoint(x, y) + coordOffset);
}

// DrawCircle
void
CanvasViewState::DrawCircle(BView* v, BPoint p, float r1, float r2,
							float tiltX, float tiltY)
{
/*	v->SetLowColor(kBlack);
	v->StrokeEllipse(p, r1 - 1.0, r1 - 1.0, B_SOLID_LOW);
	v->SetHighColor(kWhite);
	v->StrokeEllipse(p, r1, r1);*/
	stroke_agg_ellipse(v, p, r1 - 1.0, kBlack, kBlack, tiltX, tiltY);
	stroke_agg_ellipse(v, p, r1, kWhite, kWhite, tiltX, tiltY);

	if (r2 != r1 && r2 > 1.0) {
		/*v->SetHighColor(kWhite);
		v->StrokeArc(p, r2, r2, -10.0, 20.0);
		v->StrokeArc(p, r2, r2, 35.0, 20.0);
		v->StrokeArc(p, r2, r2, 80.0, 20.0);
		v->StrokeArc(p, r2, r2, 125.0, 20.0);
		v->StrokeArc(p, r2, r2, 170.0, 20.0);
		v->StrokeArc(p, r2, r2, 215.0, 20.0);
		v->StrokeArc(p, r2, r2, 260.0, 20.0);
		v->StrokeArc(p, r2, r2, 305.0, 20.0);
		v->SetHighColor(kBlack);
		r2 -= 1.0;
		v->StrokeArc(p, r2, r2, -10.0, 20.0);
		v->StrokeArc(p, r2, r2, 35.0, 20.0);
		v->StrokeArc(p, r2, r2, 80.0, 20.0);
		v->StrokeArc(p, r2, r2, 125.0, 20.0);
		v->StrokeArc(p, r2, r2, 170.0, 20.0);
		v->StrokeArc(p, r2, r2, 215.0, 20.0);
		v->StrokeArc(p, r2, r2, 260.0, 20.0);
		v->StrokeArc(p, r2, r2, 305.0, 20.0);*/

		stroke_agg_ellipse(v, p, r2, kWhite, kBlack, tiltX, tiltY);
	}
}

// _ApplyStroke
void
CanvasViewState::_ApplyStroke(BBitmap* dest, BRect area, Stroke* stroke) const
{
	stroke->MergeWithBitmap(dest, fCanvasView->StrokeBitmap(), area,
							fCanvasView->ColorSpace());
}

// _ApplyStroke
void
CanvasViewState::_ApplyStroke(BBitmap* from, BBitmap* to,
						 BRect area, Stroke* stroke) const
{
	stroke->MergeWithBitmap(from, to, fCanvasView->StrokeBitmap(), area,
							fCanvasView->ColorSpace());
}

// _InvalidateCanvasRect
void
CanvasViewState::_InvalidateCanvasRect(BRect r, bool deep) const
{
	float zoom = fCanvasView->ZoomLevel();
	r.InsetBy(-7.0 / zoom, -7.0 / zoom);
	fCanvasView->_InvalidateCanvas(r, deep);
}
