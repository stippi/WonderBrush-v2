// AffineTransformation.h

#include <Message.h>

#include "AffineTransformation.h"

// constructor
AffineTransformation::AffineTransformation()
	: agg::trans_affine(),
	  fTranslation(0.0, 0.0),
	  fRotation(0.0),
	  fXScale(1.0),
	  fYScale(1.0)
{
}

// copy constructor
AffineTransformation::AffineTransformation(const AffineTransformation& other)
	: agg::trans_affine(other),
	  fTranslation(other.fTranslation),
	  fRotation(other.fRotation),
	  fXScale(other.fXScale),
	  fYScale(other.fYScale)
{
}

// constructor
AffineTransformation::AffineTransformation(BMessage* archive)
	: agg::trans_affine(),
	  fTranslation(0.0, 0.0),
	  fRotation(0.0),
	  fXScale(1.0),
	  fYScale(1.0)
{
	if (archive) {
		if (archive->FindPoint("translation", &fTranslation) < B_OK)
			fTranslation = BPoint(0.0, 0.0);
		if (archive->FindDouble("rotation", &fRotation) < B_OK)
			fRotation = 0.0;
		if (archive->FindDouble("x scale", &fXScale) < B_OK)
			fXScale = 1.0;
		if (archive->FindDouble("y scale", &fYScale) < B_OK)
			fYScale = 1.0;

		_UpdateMatrix();
	}
}

// destructor
AffineTransformation::~AffineTransformation()
{
}

// Archive
status_t
AffineTransformation::Archive(BMessage* into) const
{
	status_t ret = B_BAD_VALUE;
	if (into) {
		ret = into->AddPoint("translation", fTranslation);
		if (ret >= B_OK)
			ret = into->AddDouble("rotation", fRotation);
		if (ret >= B_OK)
			ret = into->AddDouble("x scale", fXScale);
		if (ret >= B_OK)
			ret = into->AddDouble("y scale", fYScale);
	}
	return ret;
}

// Transform
void
AffineTransformation::Transform(double* x, double* y) const
{
	transform(x, y);
}

// Transform
void
AffineTransformation::Transform(BPoint* point) const
{
	if (point) {
		double x = point->x;
		double y = point->y;

		transform(&x, &y);
	
		point->x = x;
		point->y = y;
	}
}

// InverseTransform
void
AffineTransformation::InverseTransform(double* x, double* y) const
{
	inverse_transform(x, y);
}

// InverseTransform
void
AffineTransformation::InverseTransform(BPoint* point) const
{
	if (point) {
		double x = point->x;
		double y = point->y;

		inverse_transform(&x, &y);
	
		point->x = x;
		point->y = y;
	}
}

// SetTransformation
void
AffineTransformation::SetTransformation(BPoint translation,
								 double rotation,
								 double xScale,
								 double yScale)
{
	fTranslation = translation;
	fRotation = rotation;
	fXScale = xScale;
	fYScale = yScale;

	_UpdateMatrix();
}

// TranslateBy
void
AffineTransformation::TranslateBy(BPoint offset)
{
	fTranslation += offset;

	_UpdateMatrix();
}

// RotateBy
//
// converts a rotation in world coordinates into
// a combined local rotation and a translation
void
AffineTransformation::RotateBy(BPoint origin, double degrees)
{
	fRotation += degrees;

	// rotate fTranslation
	double xOffset = fTranslation.x - origin.x;
	double yOffset = fTranslation.y - origin.y;

	agg::trans_affine_rotation m(degrees * PI / 180.0);
	m.transform(&xOffset, &yOffset);

	fTranslation.x = origin.x + xOffset;
	fTranslation.y = origin.y + yOffset;

	_UpdateMatrix();
}

// ScaleBy
//
// converts a scalation in world coordinates into
// a combined local scalation and a translation
void
AffineTransformation::ScaleBy(BPoint origin, double xScale, double yScale)
{
	fXScale *= xScale;
	fYScale *= yScale;

	// scale fTranslation
	double xOffset = fTranslation.x - origin.x;
	double yOffset = fTranslation.y - origin.y;

	fTranslation.x = origin.x + (xOffset * xScale);
	fTranslation.y = origin.y + (yOffset * yScale);

	_UpdateMatrix();
}

// SetTranslationAndScale
void
AffineTransformation::SetTranslationAndScale(BPoint offset,
									  double xScale, double yScale)
{
	fTranslation = offset;

	fXScale = xScale;
	fYScale = yScale;

	_UpdateMatrix();
}

// _UpdateMatrix
void
AffineTransformation::_UpdateMatrix()
{
	reset();
	multiply(agg::trans_affine_scaling(fXScale, fYScale));
	multiply(agg::trans_affine_rotation(fRotation * PI / 180.0));
	multiply(agg::trans_affine_translation(fTranslation.x, fTranslation.y));
}

