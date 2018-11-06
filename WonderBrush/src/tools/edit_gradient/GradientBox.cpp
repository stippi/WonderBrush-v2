// GradientBox.cpp

#include <stdio.h>
#include <string.h>

#include "Gradient.h"
#include "EditGradientState.h"
#include "Stroke.h"
#include "TransformGradientAction.h"

#include "GradientBox.h"

// constructor
GradientBox::GradientBox(CanvasView* view,
						 EditGradientState* state,
						 Stroke* object)
	: TransformBox(view, BRect(0.0, 0.0, 1.0, 1.0)),
	  fState(state),

	  fGradient(object->GetGradient()),
	  fObjectBounds(object->Bounds()),

	  fOriginals(NULL)
{
	BRect box(0.0, 0.0, -1.0, -1.0);
	if (fGradient) {
		// allocate storage for the current
		// transformations of each object
		fOriginals = new double[9];

		// figure out bounds and init transformations
		box = BRect(-200.0, -200.0, 200.0, 200.0);
		box = fGradient->TransformBounds(box);

		// remember parent transformation for removing
		// it's part of the transformation to get clean
		// numbers for use with the gradient
		fParentTransform = *object;

		fGradient->StoreTo(fOriginals);
	}
	SetBox(box);
}

// copy constructor
GradientBox::GradientBox(const GradientBox& other)
	: TransformBox(other),
	  fState(other.fState),
	  fGradient(other.fGradient),
	  fOriginals(fGradient ? new double[9] : NULL)
{
	if (fOriginals) {
		memcpy(fOriginals, other.fOriginals, sizeof(double) * 9);
	}
}

// destructor
GradientBox::~GradientBox()
{
	delete[] fOriginals;
}

// Bounds
BRect
GradientBox::Bounds() const
{
	return TransformBox::Bounds();
}

// Update
void
GradientBox::Update(bool deep)
{
	// remember current bounds
	BRect r = Bounds();

	TransformBox::Update(deep);

	if (deep) {
		if (fGradient) {
			// reset the objects transformation to the saved state
			fGradient->LoadFrom(fOriginals);
			// combined with the current transformation
			fGradient->Multiply(*this);
		}
		fState->_RebuildLayer(fObjectBounds, true);
	}

	r = r | Bounds();
//printf("GradientBox::Update() - invalidating: BRect(%.1f, %.1f, %.1f, %.1f)\n",
//		r.left, r.top, r.right, r.bottom);
	fState->_InvalidateCanvasRect(r);
}

// Perform
Action*
GradientBox::Perform()
{
	return NULL;
}

// Cancel
Action*
GradientBox::Cancel()
{
	SetTransformation(BPoint(0.0, 0.0), 0.0, 1.0, 1.0);

	return NULL;
}

// TransformFromCanvas
void
GradientBox::TransformFromCanvas(BPoint& point) const
{
	if (fGradient->InheritTransformation())
		fParentTransform.InverseTransform(&point);
}

// TransformToCanvas
void
GradientBox::TransformToCanvas(BPoint& point) const
{
	if (fGradient->InheritTransformation())
		fParentTransform.Transform(&point);
}

// MakeAction
TransformAction*
GradientBox::MakeAction(const char* actionName, uint32 nameIndex) const
{
	TransformAction* action =
//		new TransformGradientAction(this, Translation(),LocalRotation(),
//									LocalXScale(),LocalYScale(),
		new TransformGradientAction(this, *this,
									CenterOffset(), actionName, nameIndex);
	fState->_InitPrivateHistory();

	return action;
}

// InheritedScale
float
GradientBox::InheritedScale() const
{
	if (fGradient->InheritTransformation())
		return fParentTransform.scale();
	return 1.0;
}

// ViewSpaceRotation
double
GradientBox::ViewSpaceRotation() const
{
	Transformable t(*this);
	t.Multiply(fParentTransform);
	return t.rotation() * 180.0 / M_PI;
}

