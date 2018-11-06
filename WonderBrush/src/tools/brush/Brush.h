// Brush.h

#ifndef BRUSH_H
#define BRUSH_H

#include "defines.h"

#include "Transformable.h"

// some of these flags are mutually exclusive,
// for example, a solid brush ignores the hardness
// completely

enum {
	FLAG_PRESSURE_CONTROLS_APHLA		= 0x01,
	FLAG_PRESSURE_CONTROLS_RADIUS		= 0x02,
	FLAG_PRESSURE_CONTROLS_HARDNESS		= 0x04,
	FLAG_PRESSURE_CONTROLS_SPACING		= 0x08,
	FLAG_SOLID							= 0x10,
	FLAG_TILT_CONTROLS_SHAPE			= 0x20,
};

class Brush {
 public:
								Brush(range radius, range hardness);
	virtual						~Brush();

			void				SetRadius(range radius)
									{ fRadius = radius; }
	inline	range				Radius() const
									{ return fRadius; }
			void				SetHardness(range hardness)
									{ fHardness = hardness; }
	inline	range				Hardness() const
									{ return fHardness; }

			void				Draw(BPoint where, float pressure,
									 range alpha, uint32 flags,
									 uint8* dest, uint32 bpr,
									 BRect constrainRect) const;

			void				Draw(BPoint where, float pressure,
									 float tiltX, float tiltY,
									 range alpha, uint32 flags,
									 uint8* dest, uint32 bpr,
									 const Transformable& transform,
									 BRect constrainRect) const;

 private:
			range				fRadius;
			range				fHardness;

	static	uint8*				fGaussTable;
};

#endif // BRUSH_H
