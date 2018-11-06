// Transformation.h

#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

#include <SupportDefs.h>

class BMessage;
class TransformEditor;

class Transformation {
 public:
								Transformation();
								Transformation(const Transformation& other);
								Transformation(BMessage* archive);
	virtual						~Transformation();

	virtual	status_t			Archive(BMessage* into) const = 0;

	virtual	void				Transform(double* x, double* y) const = 0;
	virtual	void				Transform(BPoint* point) const = 0;

	virtual	void				InverseTransform(double* x, double* y) const = 0;
	virtual	void				InverseTransform(BPoint* point) const = 0;

	virtual	TransformEditor*	Editor() const = 0;
};

#endif // TRANSFORMATION_H

