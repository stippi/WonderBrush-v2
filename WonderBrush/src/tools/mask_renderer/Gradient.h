// Gradient.h

#ifndef WB_GRADIENT_H
#define WB_GRADIENT_H

#include <GraphicsDefs.h>
#include <List.h>

#include "ArchivableTransformable.h"
#include "Observable.h"

class BMessage;

enum gradient_type {
	GRADIENT_LINEAR,
	GRADIENT_CIRCULAR,
	GRADIENT_DIAMONT,
	GRADIENT_CONIC,
	GRADIENT_XY,
	GRADIENT_SQRT_XY,
};

enum interpolation_type {
	INTERPOLATION_LINEAR,
	INTERPOLATION_SMOOTH,
};

struct color_step {
					color_step(const rgb_color c, float o);
					color_step(uint8 r, uint8 g, uint8 b, uint8 a, float o);
					color_step(const color_step& other);
					color_step();

			bool	operator!=(const color_step& other) const;

	rgb_color		color;
	float			offset;
};

class Gradient : public ArchivableTransformable,
				 public Observable {
 public:
								Gradient(bool empty = false);
								Gradient(const BMessage* archive);
								Gradient(const Gradient& other);
	virtual						~Gradient();

			status_t			Archive(BMessage* into, bool deep = true) const;

			Gradient&			operator=(const Gradient& other);

			bool				operator==(const Gradient& other) const;
			bool				operator!=(const Gradient& other) const;

			void				SetColors(const Gradient& other);


			int32				AddColor(const rgb_color& color, float offset);
			bool				AddColor(const color_step& color, int32 index);

			bool				RemoveColor(int32 index);

			bool				SetColor(int32 index, const color_step& step);
			bool				SetColor(int32 index, const rgb_color& color);
			bool				SetOffset(int32 index, float offset);

			int32				CountColors() const;
			color_step*			ColorAt(int32 index) const;

			void				SetType(gradient_type type);
			gradient_type		Type() const
									{ return fType; }

			void				SetInterpolation(interpolation_type type);
			interpolation_type		Interpolation() const
									{ return fInterpolation; }

			void				SetInheritTransformation(bool inherit);
			bool				InheritTransformation() const
									{ return fInheritTransformation; }

			void				MakeGradient(uint32* colors,
											 int32 count) const;

			void				FitToBounds(BRect bounds);

			void				PrintToStream() const;

 private:
			void				_MakeEmpty();

			BList				fColors;
			gradient_type		fType;
			interpolation_type	fInterpolation;
			bool				fInheritTransformation;
};


#endif // WB_GRADIENT_H
