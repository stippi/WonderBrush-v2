// Transformable.h

#include <Message.h>

#include "Transformable.h"

// constructor
Transformable::Transformable()
{
}

// copy constructor
Transformable::Transformable(const Transformable& other)
{
	for (int32 i = 0; Transform* trans = other.TransformAt(i); i++) {
		AddTransform(trans->Clone());
	}
}

// constructor
Transformable::Transformable(BMessage* archive)
{
	if (archive) {
	}
}

// destructor
Transformable::~Transformable()
{
	for (int32 i = 0; Transform* trans = TransformAt(i); i++)
		delete trans;
}

// Archive
status_t
Transformable::Archive(BMessage* into) const
{
	status_t ret = B_BAD_VALUE;
	if (into) {
		ret = B_OK;
	}
	return ret;
}

// Transform
void
Transformable::Transform(double* x, double* y) const
{
	for (int32 i = 0; Transform* trans = TransformAt(i); i++) {
		trans->Transform(x, y);
	}
}

// Transform
void
Transformable::Transform(BPoint* point) const
{
	for (int32 i = 0; Transform* trans = TransformAt(i); i++) {
		trans->Transform(point);
	}
}

// InverseTransform
void
Transformable::InverseTransform(double* x, double* y) const
{
	for (int32 i = CountTransforms() - 1; Transform* trans = TransformAt(i); i--) {
		trans->InverseTransform(x, y);
	}
}

// InverseTransform
void
Transformable::InverseTransform(BPoint* point) const
{
	for (int32 i = CountTransforms() - 1; Transform* trans = TransformAt(i); i--) {
		trans->InverseTransform(point);
	}
}

// AddTransform
bool
Transformable::AddTransform(Transformation* transform)
{
	if (transform) {
		if (BList::AddItem((void*)transform)) {
			Update();
			return true;
		}
	}
	return false;
}

// AddTransform
bool
Transformable::AddTransform(Transformation* transform, int32 index)
{
	if (transform) {
		if (BList::AddItem((void*)transform, index)) {
			Update();
			return true;
		}
	}
	return false;
}

// RemoveTransform
bool
Transformable::RemoveTransform(Transformation* transform)
{
	if (transform) {
		if (BList::RemoveItem((void*)transform)) {
			Update();
			return true;
		}
	}
	return false;
}

// RemoveTransform
Transform*
Transformable::RemoveTransform(int32 index)
{
	Transform* ret = (Transform*)BList::RemoveItem(index);
	if (ret) {
		Update();
	}
	return ret;
}

// TransformAt
Transform*
Transformable::TransformAt(int32 index) const
{
	return (Transform*)BList::ItemAt(index);
}

// CountTransforms
int32
Transformable::CountTransforms() const
{
	return BList::CountItems();
}


