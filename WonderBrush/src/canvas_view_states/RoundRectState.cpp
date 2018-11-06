// RoundRectState.cpp

#include <stdio.h>

#include <Cursor.h>
#include <Message.h>
#include <Window.h>

#include "cursors.h"
#include "support.h"

#include "AutoShapeBox.h"
#include "CanvasView.h"
#include "RoundRectRadiusAction.h"
#include "ShapeStroke.h"
#include "VectorPath.h"

#include "RoundRectState.h"

// constructor
RoundRectState::RoundRectState(CanvasView* parent)
	: AutoShapeState(parent,
					 MSG_SET_RECT_OPACITY,
					 MSG_SET_RECT_OUTLINE,
					 MSG_SET_RECT_OUTLINE_WIDTH),
	  fCornerRadius(0.0),
	  fStartPoint(0.0, 0.0)
{
}

// destructor
RoundRectState::~RoundRectState()
{
}

// MessageReceived
bool
RoundRectState::MessageReceived(BMessage* message)
{
	if (message->what == MSG_SET_RECT_CORNER_RADIUS) {
		float value;
		if (message->FindFloat("max value", &value) >= B_OK) {
			if (!fCanvasView->IsBusy())
				SetRoundCornerRadius(value, true, false);
			else
				fCanvasView->EventDropped();
		}
		return true;
	} else
		return AutoShapeState::MessageReceived(message);
}

// SetRoundCornerRadius
void
RoundRectState::SetRoundCornerRadius(float radius, bool action, bool notify)
{
	constrain(radius, 0.0, 1.0);
	if (fCornerRadius != radius) {

		float oldRadius = fCornerRadius;
		fCornerRadius = radius;

		if (fShapeStroke && fBox) {
			// action
			if (action) {
				fCanvasView->Perform(new RoundRectRadiusAction(this, oldRadius));
			}

			if (oldRadius == 0.0 || radius == 0.0) {
				// switch from 4 to 8 points or reverse
				if (VectorPath* path = fShapeStroke->Path()) {
					_MakeRect(fStartPoint, path);
					fBox->SetToShape(fShapeStroke);
				}
			}
			fBox->Update(true);
		}

		if (notify) {
			BMessage message(MSG_SET_RECT_CORNER_RADIUS);
			message.AddFloat("value", fCornerRadius);
			fCanvasView->Window()->PostMessage(&message);
		}
	}
}

static const double kappla = 1.0 - 0.552284;

// GetPoints
bool
RoundRectState::GetPoints(control_point* points, int32 count,
						  double xScale, double yScale) const
{
	if (count == 8 && xScale != 0.0 && yScale != 0.0) {
		double left = points[0].point.x;
		double top = points[7].point.y;
		double right = points[5].point.x;
		double bottom = points[2].point.y;

		double width = fabs((right - left) * xScale);
		double height = fabs((bottom - top) * yScale);

		double r = (min_c(width, height) * fCornerRadius) / 2.0;

		double xRadius = r / fabs(xScale);
		double yRadius = r / fabs(yScale);

		points[0].point.y = top + yRadius;
		points[0].point_in.y = top + kappla * yRadius;

		points[1].point.y = bottom - yRadius;
		points[1].point_out.y = bottom - kappla * yRadius;

		points[2].point.x = left + xRadius;
		points[2].point_in.x = left + kappla * xRadius;

		points[3].point.x = right - xRadius;
		points[3].point_out.x = right - kappla * xRadius;

		points[4].point.y = bottom - yRadius;
		points[4].point_in.y = bottom - kappla * yRadius;

		points[5].point.y = top + yRadius;
		points[5].point_out.y = top + kappla * yRadius;

		points[6].point.x = right - xRadius;
		points[6].point_in.x = right - kappla * xRadius;

		points[7].point.x = left + xRadius;
		points[7].point_out.x = left + kappla * xRadius;

		double ySpace = min_c((points[1].point.y - points[0].point.y) / 3.0,
							  points[0].point.y - points[0].point_in.y);
		double xSpace = min_c((points[3].point.x - points[2].point.x) / 3.0,
							  points[2].point.x - points[2].point_in.x);

		points[0].point_out.y = points[0].point.y + ySpace;
		points[1].point_in.y = points[1].point.y - ySpace;
		points[2].point_out.x = points[2].point.x + xSpace;
		points[3].point_in.x = points[3].point.x - xSpace;
		points[4].point_out.y = points[4].point.y - ySpace;
		points[5].point_in.y = points[5].point.y + ySpace;
		points[6].point_out.x = points[6].point.x - xSpace;
		points[7].point_in.x = points[7].point.x + xSpace;
		return true;
	}
	return false;
}

// _UpdateToolCursor
void
RoundRectState::_UpdateToolCursor()
{
	BCursor cursor(kRectCursor);
	fCanvasView->SetViewCursor(&cursor, true);
}

static const double kDefaultRadius = 1.0;

// _MakeAutoShape
ShapeStroke*
RoundRectState::_MakeAutoShape(BPoint where) const
{
	// cache startpoint, because we might need to rebuild the path later
	fStartPoint = where;
	ShapeStroke* rect = new ShapeStroke(fCanvasView->Color());
	if (VectorPath* path = rect->Path()) {
		_MakeRect(where, path);
	} else {
		fprintf(stderr, "Error creating rect: no path!\n");
		delete rect;
		rect = NULL;
	}
	return rect;
}

// _MakeRect
void
RoundRectState::_MakeRect(const BPoint& where, VectorPath* path) const
{
	path->MakeEmpty();

	if (fCornerRadius == 0.0) {
		// add four control points defining the rect:
		// a   d 
		// 
		// b   c
		BPoint a(where.x, where.y);
		BPoint b(where.x, where.y + kDefaultRadius);
		BPoint c(where.x + kDefaultRadius, where.y + kDefaultRadius);
		BPoint d(where.x + kDefaultRadius, where.y);
		
		path->AddPoint(a);
		path->AddPoint(b);
		path->AddPoint(c);
		path->AddPoint(d);
	} else {
		// add eight control points defining the round rect:
		//  h   g
		// a     f 
		// 
		// b     e
		//  c   d
		double r = fCornerRadius / 2.0;

		BPoint a(where.x, where.y + r);
		BPoint b(where.x, where.y + kDefaultRadius - r);

		BPoint c(where.x + r, where.y + kDefaultRadius);
		BPoint d(where.x + kDefaultRadius - r, where.y + kDefaultRadius);

		BPoint e(where.x + kDefaultRadius, where.y + kDefaultRadius - r);
		BPoint f(where.x + kDefaultRadius, where.y + r);

		BPoint g(where.x + kDefaultRadius - r, where.y);
		BPoint h(where.x + r, where.y);
		
		path->AddPoint(a);
		path->AddPoint(b);
		path->AddPoint(c);
		path->AddPoint(d);
		path->AddPoint(e);
		path->AddPoint(f);
		path->AddPoint(g);
		path->AddPoint(h);
	}

	path->SetClosed(true);
}
