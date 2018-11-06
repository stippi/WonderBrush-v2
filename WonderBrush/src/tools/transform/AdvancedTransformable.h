// AdvancedTransformable.h

#ifndef ADVANCED_TRANSFORMABLE_H
#define ADVANCED_TRANSFORMABLE_H

#include "Transformable.h"

class AdvancedTransformable : public Transformable {
 public:
								AdvancedTransformable();
								AdvancedTransformable(const AdvancedTransformable& other);
	virtual						~AdvancedTransformable();

	virtual	void				Update(bool deep = true) {}

			void				SetTransformation(BPoint translation,
												  double rotation,
												  double xScale,
												  double yScale);

	virtual	void				TranslateBy(BPoint offset);
	virtual	void				RotateBy(BPoint origin, double degrees);
//	virtual	void				ScaleBy(BPoint origin, double xScale, double yScale);

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

			AdvancedTransformable&		operator=(const AdvancedTransformable& other);

 protected:
			void				_UpdateMatrix();

			BPoint				fTranslation;
			double				fRotation;
			double				fXScale;
			double				fYScale;
};

#endif // ADVANCED_TRANSFORMABLE_H

