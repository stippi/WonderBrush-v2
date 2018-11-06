// AdvancedTransformable.h

#include <stdio.h>

#include "AdvancedTransformable.h"

// constructor
AdvancedTransformable::AdvancedTransformable()
	: Transformable(),
	  fTranslation(0.0, 0.0),
	  fRotation(0.0),
	  fXScale(1.0),
	  fYScale(1.0)
{
}

// copy constructor
AdvancedTransformable::AdvancedTransformable(const AdvancedTransformable& other)
	: Transformable(other),
	  fTranslation(other.fTranslation),
	  fRotation(other.fRotation),
	  fXScale(other.fXScale),
	  fYScale(other.fYScale)
{
}

// destructor
AdvancedTransformable::~AdvancedTransformable()
{
}

// SetTransformation
void
AdvancedTransformable::SetTransformation(BPoint translation,
										 double rotation,
										 double xScale,
										 double yScale)
{
	if (xScale == 0.0)
		xScale = 0.000001;
	if (yScale == 0.0)
		yScale = 0.000001;

	if (fTranslation != translation ||
		fRotation != rotation ||
		fXScale != xScale ||
		fYScale != yScale) {

		fTranslation = translation;
		fRotation = rotation;
		fXScale = xScale;
		fYScale = yScale;
	
		_UpdateMatrix();
	}
}

// TranslateBy
void
AdvancedTransformable::TranslateBy(BPoint offset)
{
	if (offset.x != 0.0 || offset.y != 0.0) {
		fTranslation += offset;
	
		_UpdateMatrix();
	}
}

// RotateBy
//
// converts a rotation in world coordinates into
// a combined local rotation and a translation
void
AdvancedTransformable::RotateBy(BPoint origin, double degrees)
{
	if (degrees != 0.0) {
		fRotation += degrees;
	
		// rotate fTranslation
		double xOffset = fTranslation.x - origin.x;
		double yOffset = fTranslation.y - origin.y;
	
		agg::trans_affine_rotation m(degrees * M_PI / 180.0);
		m.transform(&xOffset, &yOffset);
	
		fTranslation.x = origin.x + xOffset;
		fTranslation.y = origin.y + yOffset;
	
		_UpdateMatrix();
	}
}

/*
// ScaleBy
//
// converts a scalation in world coordinates into
// a combined local scalation and a translation
void
AdvancedTransformable::ScaleBy(BPoint origin, double xScale, double yScale)
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
*/
// SetTranslationAndScale
void
AdvancedTransformable::SetTranslationAndScale(BPoint offset,
											  double xScale, double yScale)
{
	if (xScale == 0.0)
		xScale = 0.000001;
	if (yScale == 0.0)
		yScale = 0.000001;

	if (fTranslation != offset || fXScale != xScale || fYScale != yScale) {
		fTranslation = offset;
	
		fXScale = xScale;
		fYScale = yScale;
	
		_UpdateMatrix();
	}
}

// operator=
AdvancedTransformable&
AdvancedTransformable::operator=(const AdvancedTransformable& other)
{
	fTranslation = other.fTranslation;
	fRotation = other.fRotation;
	fXScale = other.fXScale;
	fYScale = other.fYScale;

	reset();
	multiply(other);
	Update();
	return *this;
}

// _UpdateMatrix
void
AdvancedTransformable::_UpdateMatrix()
{
//	reset();
	sx = 1.0;
	shy = 0.0;
	shx = 0.0;
	sy = 1.0;
	tx = 0.0;
	ty = 0.0;
// -

	multiply(agg::trans_affine_scaling(fXScale, fYScale));
	multiply(agg::trans_affine_rotation(fRotation * M_PI / 180.0));
	multiply(agg::trans_affine_translation(fTranslation.x, fTranslation.y));

	// call hook function
	Update();
}

