// EllipseState.h

#ifndef ELLIPSE_STATE_H
#define ELLIPSE_STATE_H

#include "AutoShapeState.h"

class EllipseState : public AutoShapeState {
 public:
								EllipseState(CanvasView* parent);
	virtual						~EllipseState();

 private:
	virtual	void				_UpdateToolCursor();

	virtual	ShapeStroke*		_MakeAutoShape(BPoint where) const;
};

#endif	// ELLIPSE_STATE_H
