// EllipseState.cpp

#include <stdio.h>

#include <Cursor.h>

#include "cursors.h"

#include "CanvasView.h"
#include "ShapeStroke.h"
#include "VectorPath.h"

#include "EllipseState.h"

// constructor
EllipseState::EllipseState(CanvasView* parent)
	: AutoShapeState(parent,
					 MSG_SET_ELLIPSE_OPACITY,
					 MSG_SET_ELLIPSE_OUTLINE,
					 MSG_SET_ELLIPSE_OUTLINE_WIDTH)
{
}

// destructor
EllipseState::~EllipseState()
{
}

// _UpdateToolCursor
void
EllipseState::_UpdateToolCursor()
{
	BCursor cursor(kEllipseCursor);
	fCanvasView->SetViewCursor(&cursor, true);
}

static float kDefaultRadius = 1.0;

// _MakeAutoShape
ShapeStroke*
EllipseState::_MakeAutoShape(BPoint where) const
{
	ShapeStroke* ellipse = new ShapeStroke(fCanvasView->Color());
	if (VectorPath* path = ellipse->Path()) {
		// add four control points defining an ellipse:
		//   a 
		// b   d
		//   c
		BPoint a(where.x + kDefaultRadius, where.y);
		BPoint b(where.x, where.y + kDefaultRadius);
		BPoint c(where.x + kDefaultRadius, where.y + 2.0 * kDefaultRadius);
		BPoint d(where.x + 2.0 * kDefaultRadius, where.y + kDefaultRadius);
		
		path->AddPoint(a);
		path->AddPoint(b);
		path->AddPoint(c);
		path->AddPoint(d);

		path->SetClosed(true);

		float controlDist = 0.552284 * kDefaultRadius;
		path->SetPoint(0, a, a + BPoint(controlDist, 0.0),
							 a + BPoint(-controlDist, 0.0), true);
		path->SetPoint(1, b, b + BPoint(0.0, -controlDist),
							 b + BPoint(0.0, controlDist), true);
		path->SetPoint(2, c, c + BPoint(-controlDist, 0.0),
							 c + BPoint(controlDist, 0.0), true);
		path->SetPoint(3, d, d + BPoint(0.0, controlDist),
							 d + BPoint(0.0, -controlDist), true);

		// this looks better for very slim ellipses
		ellipse->SetJoinMode(JOIN_MODE_ROUND);
	} else {
		fprintf(stderr, "Error creating ellipse: no path!\n");
		delete ellipse;
		ellipse = NULL;
	}
	return ellipse;
}
