// Guide.cpp

#include <math.h>
#include <stdio.h>

#include <Message.h>
#include <View.h>

#include <agg_math.h>

#include "support.h"

#include "Guide.h"

// constructor
Guide::Guide()
	: Observable(),
	  fHPos(0.5),
	  fVPos(0.5),
	  fAngle(0.0)
{
}

// constructor
Guide::Guide(float hPos, float vPos, double angle)
	: Observable(),
	  fHPos(hPos),
	  fVPos(vPos),
	  fAngle(angle)
{
}

// constructor
Guide::Guide(BMessage* archive)
	: Observable(),
	  fHPos(0.5),
	  fVPos(0.5),
	  fAngle(0.0)
{
	if (archive) {
		if (archive->FindFloat("h_pos", &fHPos) < B_OK)
			fHPos = 0.5;
		if (archive->FindFloat("v_pos", &fVPos) < B_OK)
			fVPos = 0.5;

		if (archive->FindDouble("angle", &fAngle) < B_OK)
			fAngle = 0.0;
	}
}

// copy constructor
Guide::Guide(const Guide& other)
	: Observable(),
	  fHPos(other.fHPos),
	  fVPos(other.fVPos),
	  fAngle(other.fAngle)
{
}

// destructor
Guide::~Guide()
{
}

// Archive
status_t
Guide::Archive(BMessage* into) const
{
	status_t ret = B_BAD_VALUE;
	if (into) {
		ret = into->AddFloat("h_pos", fHPos);

		if (ret >= B_OK)
			ret = into->AddFloat("v_pos", fVPos);

		if (ret >= B_OK)
			ret = into->AddDouble("angle", fAngle);
	}
	return ret;
}

// operator==
bool
Guide::operator==(const Guide& other) const
{
	return other.fHPos == fHPos && other.fVPos == fVPos && other.fAngle == fAngle;
}

// operator!=
bool
Guide::operator!=(const Guide& other) const
{
	return other.fHPos != fHPos || other.fVPos != fVPos || other.fAngle != fAngle;
}

// operator=
Guide&
Guide::operator=(const Guide& other)
{
	AutoNotificationSuspender _(this);

	SetHPos(other.fHPos);
	SetVPos(other.fVPos);
	SetAngle(other.fAngle);

	return *this;
}

// SetHPos
void
Guide::SetHPos(float pos)
{
	if (fHPos != pos) {
		fHPos = pos;
		Notify();
	}
}

// SetVPos
void
Guide::SetVPos(float pos)
{
	if (fVPos != pos) {
		fVPos = pos;
		Notify();
	}
}

// SetAngle
void
Guide::SetAngle(double angle)
{
	if (fAngle != angle) {
		fAngle = angle;
		Notify();
	}
}

// MoveBy
void
Guide::MoveBy(BPoint offset, const BRect& canvasBounds)
{
	BPoint center;
	center.x = canvasBounds.left + canvasBounds.Width() * fHPos;
	center.y = canvasBounds.top + canvasBounds.Height() * fVPos;

	center += offset;

	if (canvasBounds.Width() > 0.0)
		fHPos = (center.x - canvasBounds.left) / canvasBounds.Width();
	if (canvasBounds.Height() > 0.0)
		fVPos = (center.y - canvasBounds.top) / canvasBounds.Height();

	Notify();
}

// HitTest
bool
Guide::HitTest(BPoint where, const BRect& canvasBounds,
							 const BRect& viewBounds) const
{
	BPoint start, end;
	_CalcLine(canvasBounds, viewBounds, &start, &end);

	return fabs(calc_point_line_distance(start.x, start.y, end.x, end.y, where.x, where.y)) < 8.0;
}

// Draw
void
Guide::Draw(BView* view, const rgb_color& color,
			const BRect& canvasBounds, const BRect& viewBounds) const
{
	BPoint start, end;
	_CalcLine(canvasBounds, viewBounds, &start, &end);
	view->SetHighColor(color);
	view->StrokeLine(start, end);
}

// _CalcLine
void
Guide::_CalcLine(const BRect& canvasBounds, const BRect& viewBounds,
				 BPoint* start, BPoint* end) const
{
	BPoint center;
	center.x = canvasBounds.left + canvasBounds.Width() * fHPos;
	center.y = canvasBounds.top + canvasBounds.Height() * fVPos;

	double sinAngle = sin(fAngle * M_PI / 180.0);
	double cosAngle = cos(fAngle * M_PI / 180.0);

	// find the intersection of the guide line with the top or right side (start)
	double startX, startY;
	if (!agg::calc_intersection(center.x, center.y, center.x + sinAngle, center.y + cosAngle,
								viewBounds.left, viewBounds.top, viewBounds.right, viewBounds.top,
								&startX, &startY))
		agg::calc_intersection(center.x, center.y, center.x + sinAngle, center.y + cosAngle,
							   viewBounds.right, viewBounds.top, viewBounds.right, viewBounds.bottom,
							   &startX, &startY);

	// find the intersection of the guide line with the bottom or left side (end)
	double endX, endY;
	if (!agg::calc_intersection(center.x, center.y, center.x - sinAngle, center.y - cosAngle,
								viewBounds.left, viewBounds.bottom, viewBounds.right, viewBounds.bottom,
								&endX, &endY))
		agg::calc_intersection(center.x, center.y, center.x - sinAngle, center.y - cosAngle,
							   viewBounds.left, viewBounds.top, viewBounds.left, viewBounds.bottom,
							   &endX, &endY);
	start->x = startX;
	start->y = startY;
	end->x = endX;
	end->y = endY;
}

