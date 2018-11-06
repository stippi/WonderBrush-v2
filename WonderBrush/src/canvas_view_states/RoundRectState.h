// RoundRectState.h

#ifndef ROUND_RECT_STATE_H
#define ROUND_RECT_STATE_H

#include "AutoShapeState.h"

class RoundRectState : public AutoShapeState {
 public:
								RoundRectState(CanvasView* parent);
	virtual						~RoundRectState();

	virtual	bool				MessageReceived(BMessage* message);

			void				SetRoundCornerRadius(float radius,
													 bool action = false,
													 bool notify = true);
			float				RoundCornerRadius() const
									{ return fCornerRadius; }

	virtual	bool				GetPoints(control_point* points,
										  int32 count,
										  double xScale,
										  double yScale) const;
 private:
	virtual	void				_UpdateToolCursor();

	virtual	ShapeStroke*		_MakeAutoShape(BPoint where) const;
			void				_MakeRect(const BPoint& where, VectorPath* path) const;

			float				fCornerRadius;
			mutable BPoint		fStartPoint;
};

#endif	// ROUND_RECT_STATE_H
