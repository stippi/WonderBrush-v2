// Gradient.cpp

#include <math.h>
#include <stdio.h>

#include <Message.h>

#include "blending.h"
#include "support.h"

#include "Gradient.h"

// constructor
color_step::color_step(const rgb_color c, float o)
{
	color.red = c.red;
	color.green = c.green;
	color.blue = c.blue;
	color.alpha = c.alpha;
	offset = o;
}

// constructor
color_step::color_step(uint8 r, uint8 g, uint8 b, uint8 a, float o)
{
	color.red = r;
	color.green = g;
	color.blue = b;
	color.alpha = a;
	offset = o;
}

// constructor
color_step::color_step(const color_step& other)
{
	color.red = other.color.red;
	color.green = other.color.green;
	color.blue = other.color.blue;
	color.alpha = other.color.alpha;
	offset = other.offset;
}

// constructor
color_step::color_step()
{
	color.red = 0;
	color.green = 0;
	color.blue = 0;
	color.alpha = 255;
	offset = 0;
}

// operator!=
bool
color_step::operator!=(const color_step& other) const
{
	return color.red != other.color.red ||
		   color.green != other.color.green ||
		   color.blue != other.color.blue ||
		   color.alpha != other.color.alpha ||
		   offset != other.offset;
}

// #pragma mark -

// constructor
Gradient::Gradient(bool empty)
	: ArchivableTransformable(),
	  fColors(4),
	  fType(GRADIENT_LINEAR),
	  fInterpolation(INTERPOLATION_SMOOTH),
	  fInheritTransformation(true)
{
	if (!empty) {
		AddColor(color_step(0, 0, 0, 255, 0.0), 0);
		AddColor(color_step(255, 255, 255, 255, 1.0), 1);
	}
}

// constructor
Gradient::Gradient(const BMessage* archive)
	: ArchivableTransformable(archive),
	  fColors(4),
	  fType(GRADIENT_LINEAR),
	  fInterpolation(INTERPOLATION_SMOOTH),
	  fInheritTransformation(true)
{
	if (archive) {
		color_step step;
		for (int32 i = 0; archive->FindFloat("offset", i, &step.offset) >= B_OK; i++) {
			if (restore_color_from_message(archive, step.color, i) >= B_OK)
				AddColor(step, i);
			else
				break;
		}
		if (archive->FindInt32("type", (int32*)&fType) < B_OK) {
			fType = GRADIENT_LINEAR;
		}
		if (archive->FindInt32("interpolation", (int32*)&fInterpolation) < B_OK) {
			fInterpolation = INTERPOLATION_SMOOTH;
		}
		if (archive->FindBool("inherit transformation", &fInheritTransformation) < B_OK) {
			fInheritTransformation = true;
		}
	}
}

// constructor
Gradient::Gradient(const Gradient& other)
	: ArchivableTransformable(other),
	  fColors(4),
	  fType(other.fType),
	  fInterpolation(other.fInterpolation),
	  fInheritTransformation(other.fInheritTransformation)
{
	for (int32 i = 0; color_step* step = other.ColorAt(i); i++) {
		AddColor(*step, i);
	}
}

// destructor
Gradient::~Gradient()
{
	_MakeEmpty();
}

// Archive
status_t
Gradient::Archive(BMessage* into, bool deep) const
{
	status_t ret = ArchivableTransformable::Archive(into, deep);
	// color steps
	if (ret >= B_OK) {
		for (int32 i = 0; color_step* step = ColorAt(i); i++) {
			ret = store_color_in_message(into, step->color);
			if (ret < B_OK)
				break;
			ret = into->AddFloat("offset", step->offset);
			if (ret < B_OK)
				break;
		}
	}
	// gradient and interpolation type
	if (ret >= B_OK)
		ret = into->AddInt32("type", (int32)fType);
	if (ret >= B_OK)
		ret = into->AddInt32("interpolation", (int32)fInterpolation);
	if (ret >= B_OK)
		ret = into->AddBool("inherit transformation", fInheritTransformation);

	// finish off
	if (ret >= B_OK) {
		ret = into->AddString("class", "Gradient");
	}
	return ret;
}

// operator=
Gradient&
Gradient::operator=(const Gradient& other)
{
	AutoNotificationSuspender _(this);

	SetTransformable(other);
	SetColors(other);
	SetType(other.fType);
	SetInterpolation(other.fInterpolation);
	SetInheritTransformation(other.fInheritTransformation);

	Notify();

	return *this;
}

// operator==
bool
Gradient::operator==(const Gradient& other) const
{
	if (Transformable::operator==(other)) {
		int32 count = CountColors();
		if (count == other.CountColors() &&
			fType == other.fType &&
			fInterpolation == other.fInterpolation &&
			fInheritTransformation == other.fInheritTransformation) {
	
			bool equal = true;
			for (int32 i = 0; i < count; i++) {
				color_step* ourStep = ColorAt(i);
				color_step* otherStep = other.ColorAt(i);
				if (*ourStep != *otherStep) {
					equal = false;
					break;
				}
			}
			return equal;
		}
	}
	return false;
}

// operator!=
bool
Gradient::operator!=(const Gradient& other) const
{
	return !(*this == other);
}

// SetColors
void
Gradient::SetColors(const Gradient& other)
{
	AutoNotificationSuspender _(this);

	_MakeEmpty();
	for (int32 i = 0; color_step* step = other.ColorAt(i); i++) {
		AddColor(*step, i);
	}

	Notify();
}

// AddColor
int32
Gradient::AddColor(const rgb_color& color, float offset)
{
	// find the correct index (sorted by offset)
	color_step* step = new color_step(color, offset);
	int32 index = 0;
	while (color_step* s = ColorAt(index)) {
		if (s->offset <= step->offset)
			index++;
		else
			break;
	}
	if (!fColors.AddItem((void*)step, index)) {
		delete step;
		return -1;
	}
	Notify();
	return index;
}

// AddColor
bool
Gradient::AddColor(const color_step& color, int32 index)
{
	color_step* step = new color_step(color);
	if (!fColors.AddItem((void*)step, index)) {
		delete step;
		return false;
	}
	Notify();
	return true;
}

// RemoveColor
bool
Gradient::RemoveColor(int32 index)
{
	color_step* step = (color_step*)fColors.RemoveItem(index);
	if (!step) {
		return false;
	}
	delete step;
	Notify();
	return true;
}

// SetColor
bool
Gradient::SetColor(int32 index, const color_step& color)
{
	if (color_step* step = ColorAt(index)) {
		step->color = color.color;
		step->offset = color.offset;
		Notify();
		return true;
	}
	return false;
}

// SetColor
bool
Gradient::SetColor(int32 index, const rgb_color& color)
{
	if (color_step* step = ColorAt(index)) {
		step->color = color;
		Notify();
		return true;
	}
	return false;
}

// SetOffset
bool
Gradient::SetOffset(int32 index, float offset)
{
	color_step* step = ColorAt(index);
	if (step && step->offset != offset) {
		step->offset = offset;
		Notify();
		return true;
	}
	return false;
}

// CountColors
int32
Gradient::CountColors() const
{
	return fColors.CountItems();
}

// ColorAt
color_step*
Gradient::ColorAt(int32 index) const
{
	return (color_step*)fColors.ItemAt(index);
}

// SetType
void
Gradient::SetType(gradient_type type)
{
	if (fType != type) {
		fType = type;
		Notify();
	}
}

// SetInterpolation
void
Gradient::SetInterpolation(interpolation_type type)
{
	if (fInterpolation != type) {
		fInterpolation = type;
		Notify();
	}
}

// SetInheritTransformation
void
Gradient::SetInheritTransformation(bool inherit)
{
	if (fInheritTransformation != inherit) {
		fInheritTransformation = inherit;
		Notify();
	}
}

// MakeGradient
void
Gradient::MakeGradient(uint32* colors, int32 count) const
{
	color_step* from = ColorAt(0);
	
	if (from) {
		// find the step with the lowest offset
		for (int32 i = 0; color_step* step = ColorAt(i); i++) {
			if (step->offset < from->offset)
				from = step;
		}

		// current index into "colors" array
		int32 index = (int32)floorf(count * from->offset + 0.5);
		if (index < 0)
			index = 0;
		if (index > count)
			index = count;
		//  make sure we fill the entire array
		if (index > 0) {
			uint8* c = (uint8*)&colors[0];
			for (int32 i = 0; i < index; i++) {
				c[0] = from->color.blue;
				c[1] = from->color.green;
				c[2] = from->color.red;
				c[3] = from->color.alpha;
				c += 4;
			}
		}

		// put all steps that we need to interpolate to into a list
		BList nextSteps(fColors.CountItems() - 1);
		for (int32 i = 0; color_step* step = ColorAt(i); i++) {
			if (step != from)
				nextSteps.AddItem((void*)step);
		}

		// interpolate "from" to "to"
		while (!nextSteps.IsEmpty()) {

			// find the step with the next offset
			color_step* to = NULL;
			float nextOffsetDist = 2.0;
			for (int32 i = 0; color_step* step = (color_step*)nextSteps.ItemAt(i); i++) {
				float d = step->offset - from->offset;
				if (d < nextOffsetDist && d >= 0) {
					to = step;
					nextOffsetDist = d;
				}
			}
			if (!to)
				break;

			nextSteps.RemoveItem((void*)to);

			// interpolate
			int32 offset = (int32)floorf((count - 1) * to->offset + 0.5);
			if (offset >= count)
				offset = count - 1;
			int32 dist = offset - index;
			if (dist >= 0) {
				uint8* c = (uint8*)&colors[index];

/*
				uint32 a = (from->color.alpha << 24) | (from->color.red << 16) |
						   (from->color.green << 8) | (from->color.blue);
				uint32 b = (to->color.alpha << 24) | (to->color.red << 16) |
						   (to->color.green << 8) | (to->color.blue);
				for (int32 i = index; i <= offset; i++) {
					float f = (float)(offset - i) / (float)(dist + 1);
					if (fInterpolation == INTERPOLATION_SMOOTH)
						f = gauss(1.0 - f);
					blend_pixels_copy((uint8*)&a, (uint8*)&b, c, 255 - uint8(f * 255 + 0.5));
					c += 4;
				}
*/
#if GAMMA_BLEND
				uint16 fromRed = kGammaTable[from->color.red];
				uint16 fromGreen = kGammaTable[from->color.green];
				uint16 fromBlue = kGammaTable[from->color.blue];
				uint16 toRed = kGammaTable[to->color.red];
				uint16 toGreen = kGammaTable[to->color.green];
				uint16 toBlue = kGammaTable[to->color.blue];

				for (int32 i = index; i <= offset; i++) {
					float f = (float)(offset - i) / (float)(dist + 1);
					if (fInterpolation == INTERPOLATION_SMOOTH)
						f = gauss(1.0 - f);
					float t = 1.0 - f;
					c[0] = kInverseGammaTable[(uint16)floor(fromBlue * f + toBlue * t + 0.5)];
					c[1] = kInverseGammaTable[(uint16)floor(fromGreen * f + toGreen * t + 0.5)];
					c[2] = kInverseGammaTable[(uint16)floor(fromRed * f + toRed * t + 0.5)];
					c[3] = (uint8)floor(from->color.alpha * f + to->color.alpha * t + 0.5);
					c += 4;
				}
#else // GAMMA_BLEND
				float errorC0 = 0.0;
				float errorC1 = 0.0;
				float errorC2 = 0.0;
				float errorC3 = 0.0;
				for (int32 i = index; i <= offset; i++) {
					float f = (float)(offset - i) / (float)(dist + 1);
					if (fInterpolation == INTERPOLATION_SMOOTH)
						f = gauss(1.0 - f);
					float t = 1.0 - f;
					float c0 = from->color.blue * f + to->color.blue * t;
					float c1 = from->color.green * f + to->color.green * t;
					float c2 = from->color.red * f + to->color.red * t;
					float c3 = from->color.alpha * f + to->color.alpha * t;
					c[0] = (uint8)floor(c0 + errorC0);
					c[1] = (uint8)floor(c1 + errorC1);
					c[2] = (uint8)floor(c2 + errorC2);
					c[3] = (uint8)floor(c3 + errorC3);
//printf("%0*ld: [%0*d, %0*d, %0*d, %0*d], [%3.2f, %3.2f, %3.2f, %3.2f], [%3.2f, %3.2f, %3.2f, %3.2f]\n",
//4, i, 3, c[0], 3, c[1], 3, c[2], 3, c[3],
//c0, c1, c2, c3,
//errorC0, errorC1, errorC2, errorC3);
					errorC0 = fmod(errorC0 + c0 - c[0], 0.5);
					errorC1 = fmod(errorC1 + c1 - c[1], 0.5);
					errorC2 = fmod(errorC2 + c2 - c[2], 0.5);
					errorC3 = fmod(errorC3 + c3 - c[3], 0.5);
					c += 4;
				}
#endif // GAMMA_BLEND
			}
			index = offset + 1;
			// the current "to" will be the "from" in the next interpolation
			from = to;
		}
		//  make sure we fill the entire array
		if (index < count) {
			uint8* c = (uint8*)&colors[index];
			for (int32 i = index; i < count; i++) {
				c[0] = from->color.blue;
				c[1] = from->color.green;
				c[2] = from->color.red;
				c[3] = from->color.alpha;
				c += 4;
			}
		}
	}
}

// FitToBounds
void
Gradient::FitToBounds(BRect bounds)
{
	double parl[6];
	parl[0] = bounds.left;
	parl[1] = bounds.top;
	parl[2] = bounds.right;
	parl[3] = bounds.top;
	parl[4] = bounds.right;
	parl[5] = bounds.bottom;
	agg::trans_affine transform(-200.0, -200.0, 200.0, 200.0, parl);
	multiply(transform);
}

// string_for_type
static const char*
string_for_type(gradient_type type)
{
	switch (type) {
		case GRADIENT_LINEAR:
			return "GRADIENT_LINEAR";
		case GRADIENT_CIRCULAR:
			return "GRADIENT_CIRCULAR";
		case GRADIENT_DIAMONT:
			return "GRADIENT_DIAMONT";
		case GRADIENT_CONIC:
			return "GRADIENT_CONIC";
		case GRADIENT_XY:
			return "GRADIENT_XY";
		case GRADIENT_SQRT_XY:
			return "GRADIENT_SQRT_XY";
	}
	return "<unkown>";
}

static const char*
string_for_interpolation(interpolation_type type)
{
	switch (type) {
		case INTERPOLATION_LINEAR:
			return "INTERPOLATION_LINEAR";
		case INTERPOLATION_SMOOTH:
			return "INTERPOLATION_SMOOTH";
	}
	return "<unkown>";
}

// PrintToStream
void
Gradient::PrintToStream() const
{
	printf("Gradient: type: %s, interpolation: %s, inherits transform: %d\n",
		   string_for_type(fType),
		   string_for_interpolation(fInterpolation),
		   fInheritTransformation);
	for (int32 i = 0; color_step* step = ColorAt(i); i++) {
		printf("  %ld: offset: %.1f -> color(%d, %d, %d, %d)\n",
			   i, step->offset,
			   step->color.red,
			   step->color.green,
			   step->color.blue,
			   step->color.alpha);
	}
}

// _MakeEmpty
void
Gradient::_MakeEmpty()
{
	for (int32 i = 0; color_step* color = ColorAt(i); i++) {
		delete color;
	}
	fColors.MakeEmpty();
}
