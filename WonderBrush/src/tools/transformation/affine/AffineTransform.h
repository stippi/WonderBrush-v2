// AffineTransformation.h

#ifndef AFFINE_TRANSFORMATION_H
#define AFFINE_TRANSFORMATION_H

#include <agg_trans_affine.h>

#include <Rect.h>

#include "Transformation.h"

class AffineTransformation : public Transformation,
							 private agg::trans_affine {
 public:
								AffineTransformation();
								AffineTransformation(const AffineTransformation& other);
								AffineTransformation(BMessage* archive);
	virtual						~AffineTransformation();

								// Transformation
	virtual	status_t			Archive(BMessage* into) const;

	virtual	void				Transform(double* x, double* y) const;
	virtual	void				Transform(BPoint* point) const;

	virtual	void				InverseTransform(double* x, double* y) const;
	virtual	void				InverseTransform(BPoint* point) const;

								// AffineTransformation
			void				SetTransformation(BPoint translation,
												  double rotation,
												  double xScale,
												  double yScale);

			void				TranslateBy(BPoint offset);
			void				RotateBy(BPoint origin, double degrees);
			void				ScaleBy(BPoint origin, double xScale, double yScale);

			void				SetTranslationAndScale(BPoint offset,
													   double xScale, double yScale);

			BPoint				Translation() const
									{ return fTranslation; }
			double				LocalRotation() const
									{ return fRotation; }
			double				LocalXScale() const
									{ return fXScale; }
			double				LocalYScale() const
									{ return fYScale; }

 private:
			void				_UpdateMatrix();

			BPoint				fTranslation;
			double				fRotation;
			double				fXScale;
			double				fYScale;
};

#endif // AFFINE_TRANSFORMATION_H

