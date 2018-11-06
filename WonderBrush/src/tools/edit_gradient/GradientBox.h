// GradientBox.h

#ifndef GRADIENT_BOX_H
#define GRADIENT_BOX_H

#include "TransformBox.h"

class Action;
class CanvasView;
class Gradient;
class Stroke;
class EditGradientState;

class GradientBox : public TransformBox {
 public:
								GradientBox(CanvasView* view,
											EditGradientState* state,
											Stroke* object);
								GradientBox(const GradientBox& other);
	virtual						~GradientBox();

	virtual	BRect				Bounds() const;
	virtual	void				Update(bool deep = true);

								// convinience function
			Action*				Perform();
			Action*				Cancel();

	virtual	void				TransformFromCanvas(BPoint& point) const;
	virtual	void				TransformToCanvas(BPoint& point) const;

	virtual	TransformAction*	MakeAction(const char* actionName,
										   uint32 nameIndex) const;

			Gradient*			GetGradient() const
									{ return fGradient; }

			float				InheritedScale() const;
	virtual	double				ViewSpaceRotation() const;

 private:
			EditGradientState*	fState;

			Gradient*			fGradient;
			BRect				fObjectBounds;

			// saves the transformable objects transformation states
			// prior to this transformation
			double*				fOriginals;

			Transformable		fParentTransform;
};

#endif // GRADIENT_BOX_H

