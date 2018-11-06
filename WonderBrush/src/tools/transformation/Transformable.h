// Transformable.h

#ifndef TRANSFORMABLE_H
#define TRANSFORMABLE_H

#include <List.h>
#include <Rect.h>

class BMessage;

class Transformable : public Transform, private BList {
 public:
								Transformable();
								Transformable(const Transformable& other);
								Transformable(BMessage* archive);
	virtual						~Transformable();

	virtual	status_t			Archive(BMessage* into) const;

	virtual	BRect				Bounds() = 0;
	virtual	void				Update() = 0;

								// Transform
			void				Transform(double* x, double* y) const;
			void				Transform(BPoint* point) const;

			void				InverseTransform(double* x, double* y) const;
			void				InverseTransform(BPoint* point) const;

								// Transformable
			bool				AddTransform(Transformation* transform);
			bool				AddTransform(Transformation* transform, int32 index);
			bool				RemoveTransform(Transformation* transform);
			Transform*			RemoveTransform(int32 index);
			Transform*			TransformAt(int32 index) const;
			int32				CountTransforms() const;

 private:
};

#endif // TRANSFORMABLE_H

