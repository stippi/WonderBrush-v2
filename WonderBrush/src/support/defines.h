// defines.h

#ifndef DEFINES_H
#define DEFINES_H

#include <GraphicsDefs.h>
#include <Point.h>

// brush stroke modes
enum {
	MODE_BRUSH					= 0,
	MODE_CLONE					= 1,
	MODE_PEN					= 2,
	MODE_ERASER					= 3,
	MODE_PEN_ERASER				= 4,
	MODE_RESTORE				= 5,
	MODE_BLUR					= 6,
	MODE_GRADIENT				= 7,

	MODE_UNDEFINED				= 9,
};

// tools
enum {
#if SLIM_GUI
	TOOL_UNDEFINED				= -1,
	TOOL_CROP					= 0,
	TOOL_TRANSLATE,
	TOOL_BRUSH,
	TOOL_ERASER,
	TOOL_PEN,
	TOOL_PEN_ERASER,
	TOOL_CLONE,
	TOOL_BUCKET_FILL,
	TOOL_BLUR,
	TOOL_COLOR_PICKER,
	TOOL_TEXT,
	TOOL_SHAPE,
	TOOL_SELECT,
	TOOL_ELLIPSE,
	TOOL_ROUND_RECT,
	TOOL_EDIT_GRADIENT,
	TOOL_GUIDES,
	TOOL_PICK,
#else
	TOOL_UNDEFINED				= -1,
	TOOL_CROP					= 0,
	TOOL_TRANSLATE,
	TOOL_BRUSH,
	TOOL_PEN,
	TOOL_ERASER,
	TOOL_PEN_ERASER,
	TOOL_CLONE,
	TOOL_BLUR,
	TOOL_BUCKET_FILL,
	TOOL_COLOR_PICKER,
	TOOL_TEXT,
	TOOL_SHAPE,
	TOOL_SELECT,
	TOOL_ELLIPSE,
	TOOL_ROUND_RECT,
	TOOL_EDIT_GRADIENT,
	TOOL_GUIDES,
	TOOL_PICK,
#endif
	TOOL_LAST,
};

enum {
	PICK_MASK_BRUSH				= 1,
	PICK_MASK_ERASER			= 1 << 1,
	PICK_MASK_PEN				= 1 << 2,
	PICK_MASK_PEN_ERASER		= 1 << 3,
	PICK_MASK_TEXT				= 1 << 4,
	PICK_MASK_SHAPE				= 1 << 5,
	PICK_MASK_BITMAP			= 1 << 6,
	PICK_MASK_CLONE_BRUSH		= 1 << 7,
	PICK_MASK_BLUR_BRUSH		= 1 << 8,
	PICK_MASK_FILL				= 1 << 9,
};

static const uint32 PICK_MASK_ALL = PICK_MASK_BRUSH |
									PICK_MASK_ERASER |
									PICK_MASK_PEN |
									PICK_MASK_PEN_ERASER |
									PICK_MASK_TEXT |
									PICK_MASK_SHAPE |
									PICK_MASK_BITMAP |
									PICK_MASK_CLONE_BRUSH |
									PICK_MASK_BLUR_BRUSH |
									PICK_MASK_FILL;

inline bool
is_pickable(uint32 toolID, uint32 pickMask)
{
	switch (toolID) {
		case TOOL_BRUSH:
			return pickMask & PICK_MASK_BRUSH;
		case TOOL_PEN:
			return pickMask & PICK_MASK_PEN;
		case TOOL_ERASER:
			return pickMask & PICK_MASK_ERASER;
		case TOOL_PEN_ERASER:
			return pickMask & PICK_MASK_PEN_ERASER;
		case TOOL_CLONE:
			return pickMask & PICK_MASK_CLONE_BRUSH;
		case TOOL_BLUR:
			return pickMask & PICK_MASK_BLUR_BRUSH;
		case TOOL_BUCKET_FILL:
			return pickMask & PICK_MASK_FILL;
		case TOOL_TEXT:
			return pickMask & PICK_MASK_TEXT;
		case TOOL_SHAPE:
			return pickMask & PICK_MASK_SHAPE;
	}
	return false;
}

// color spaces
enum {
	COLOR_SPACE_NONLINEAR_RGB	= 0,
	COLOR_SPACE_LINEAR_RGB		= 1,
	COLOR_SPACE_LAB				= 2,
	COLOR_SPACE_UNKNOWN			= 3,
};

// line cap modes (Shapes in outline mode)
enum {
	CAP_MODE_BUTT = 0,
	CAP_MODE_SQUARE,
	CAP_MODE_ROUND,
};

// line join modes (Shapes in outline mode)
enum {
	JOIN_MODE_MITER = 0,
	JOIN_MODE_ROUND,
	JOIN_MODE_BEVEL,
};

enum {
	GUIDE_UNITS_PERCENT = 0,
	GUIDE_UNITS_PIXELS,
	GUIDE_UNITS_MM,
	GUIDE_UNITS_INCH,
};

enum {
	ALIGN_LEFT		= 0,
	ALIGN_CENTER,
	ALIGN_RIGHT,
	ALIGN_JUSTIFY,
};


const pattern kStripes = (pattern){ { 0xc7, 0x8f, 0x1f, 0x3e, 0x7c, 0xf8, 0xf1, 0xe3 } };
const pattern kDotted = (pattern){ { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa } };
const pattern kDottedBigger = (pattern){ { 0x33, 0x33, 0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc } };

const rgb_color kWhite = (rgb_color){ 255, 255, 255, 255 };
const rgb_color kBlack = (rgb_color){ 0, 0, 0, 255 };
const rgb_color kRed = (rgb_color){ 255, 0, 0, 255 };
const rgb_color kGreen	= (rgb_color){ 0, 255, 0, 255 };
const rgb_color kBlue	= (rgb_color){ 0, 0, 255, 255 };
const rgb_color kAlphaLow	= (rgb_color){ 0xbb, 0xbb, 0xbb, 0xff };
const rgb_color kAlphaHigh	= (rgb_color){ 0xe0, 0xe0, 0xe0, 0xff };

struct range {
	float	min;
	float	max;
};

struct Point {
	Point()
		: point(0.0, 0.0),
		  pressure(1.0),
		  tiltX(0.0),
		  tiltY(0.0)
	{
	}
	Point(BPoint p)
		: point(p),
		  pressure(1.0),
		  tiltX(0.0),
		  tiltY(0.0)
	{
	}
	Point(BPoint p, float pressure)
		: point(p),
		  pressure(pressure),
		  tiltX(0.0),
		  tiltY(0.0)
	{
	}
	Point(const Point& other)
		: point(other.point),
		  pressure(other.pressure),
		  tiltX(other.tiltX),
		  tiltY(other.tiltY)
	{
	}
	Point& operator=(const Point& other)
	{
		point = other.point;
		pressure = other.pressure;
		tiltX = other.tiltX;
		tiltY = other.tiltY;
		return *this;
	}
	bool operator==(const Point& other) const
	{
		return point == other.point &&
			   pressure == other.pressure &&
			   tiltX == other.tiltX &&
			   tiltY == other.tiltY;
	}
	bool operator!=(const Point& other) const
	{
		return point != other.point ||
			   pressure != other.pressure ||
			   tiltX != other.tiltX ||
			   tiltY != other.tiltY;
	}

	BPoint				point;
	float				pressure;
	float				tiltX;
	float				tiltY;
};


#endif // DEFINES_H
