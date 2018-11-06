// Guide.h

#ifndef GUIDE_H
#define GUIDE_H

#include <Rect.h>

#include "Observable.h"

class BMessage;
class BView;

class Guide : public Observable {
 public:
								Guide();
								Guide(float hPos, float vPos, double angle);
								Guide(BMessage* archive);
								Guide(const Guide& other);

	virtual						~Guide();

			status_t			Archive(BMessage* into) const;

			bool				operator==(const Guide& other) const;
			bool				operator!=(const Guide& other) const;

			Guide&				operator=(const Guide& other);

			void				SetHPos(float pos);
	inline	float				HPos() const
									{ return fHPos; }
			void				SetVPos(float pos);
	inline	float				VPos() const
									{ return fVPos; }
			void				SetAngle(double angle);
	inline	double				Angle() const
									{ return fAngle; }

			void				MoveBy(BPoint offset,
									   const BRect& canvasBounds);

	virtual	bool				HitTest(BPoint where, const BRect& canvasBoundsInView,
										const BRect& viewBounds) const;
	virtual	void				Draw(BView* view,
									 const rgb_color& color,
									 const BRect& canvasBoundsInView,
									 const BRect& viewBounds) const;

 private:
			void				_CalcLine(const BRect& canvasBoundsInView,
										  const BRect& viewBounds,
										  BPoint* start,
										  BPoint* end) const;
			float				fHPos;
			float				fVPos;
			double				fAngle;
};

#endif // GUIDE_H
