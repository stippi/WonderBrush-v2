// support.h

#ifndef SUPPORT_H
#define SUPPORT_H

#include <GraphicsDefs.h>
#include <Rect.h>
#include <agg_math_stroke.h>

class BBitmap;
class BDataIO;
class BMessage;
class BPositionIO;
class BString;
class BView;
class BWindow;
class Space;

// constrain
inline void
constrain(float& value, float min, float max)
{
	if (value < min)
		value = min;
	if (value > max)
		value = max;
}

// constrain_int32_0_255_asm
inline int32
constrain_int32_0_255_asm(int32 value) {
    asm("movl  $0,    %%ecx; \
         movl  $255,  %%edx; \
         cmpl  %%ecx, %%eax; \
         cmovl %%ecx, %%eax; \
         cmpl  %%edx, %%eax; \
         cmovg %%edx, %%eax"
       : "=a" (value)
       : "a" (value) 
       : "%ecx", "%edx" );
    return value;
}

inline int32
constrain_int32_0_255_c(int32 value) {
    return max_c(0, min_c(255, value));
}

#define constrain_int32_0_255 constrain_int32_0_255_asm

// rect_to_int
inline void
rect_to_int(BRect r,
			int32& left, int32& top, int32& right, int32& bottom)
{
	left = (int32)floorf(r.left);
	top = (int32)floorf(r.top);
	right = (int32)ceilf(r.right);
	bottom = (int32)ceilf(r.bottom);
}

// dist
inline float
dist(BPoint a, BPoint b)
{
	float xDiff = b.x - a.x;
	float yDiff = b.y - a.y;
	return sqrtf(xDiff * xDiff + yDiff * yDiff);
}

// calc_point_line_distance
double
calc_point_line_distance(double x1, double y1,
						 double x2, double y2,
						 double x,  double y);

// calc_angle
double
calc_angle(BPoint origin, BPoint from, BPoint to, bool degree = true);

/*
template <class T>
T min4(const T a, const T b, const T c, const T d)
{
	T e = a < b ? a : b;
	T f = c < d ? c : d;
	return e < f ? e : f;
} 
template <class T>
T max4(const T a, const T b, const T c, const T d)
{
	T e = a > b ? a : b;
	T f = c > d ? c : d;
	return e > f ? e : f;
} 
*/
inline float
min4(float a, float b, float c, float d)
{
	return min_c(a, min_c(b, min_c(c, d)));
} 

inline float
max4(float a, float b, float c, float d)
{
	return max_c(a, max_c(b, max_c(c, d)));
} 

inline float
min5(float v1, float v2, float v3, float v4, float v5)
{
	return min_c(min4(v1, v2, v3, v4), v5);
}

inline float
max5(float v1, float v2, float v3, float v4, float v5)
{
	return max_c(max4(v1, v2, v3, v4), v5);
}

inline float
roundf(float v)
{
	if (v >= 0.0)
		return floorf(v + 0.5);
	return ceilf(v - 0.5);
}



status_t load_settings(BMessage* message, const char* fileName,
					   const char* folder = NULL);

status_t save_settings(BMessage* message, const char* fileName,
					   const char* folder = NULL);

status_t store_color_in_message(BMessage* message, rgb_color color);

status_t restore_color_from_message(const BMessage* message, rgb_color& color, int32 index = 0);

// looper of view must be locked!
void stroke_frame(BView* view, BRect frame,
				  rgb_color left, rgb_color top,
				  rgb_color right, rgb_color bottom);


Space* vertical_space();
Space* horizontal_space();

BMessage make_color_drop_message(rgb_color color, BBitmap* bitmap);

void print_modifiers();

status_t push_data(BDataIO* object, const void* data,
				   ssize_t size, ssize_t& total);

status_t push_bitmap(BDataIO* object, BBitmap* bitmap, ssize_t& total);
status_t pull_bitmap(BDataIO* object, BBitmap** bitmap, ssize_t tagSize);

status_t write_string(BPositionIO* stream, BString& string);
void append_float(BString& string, float n, int32 maxDigits = 4);

double gauss(double f);

agg::line_cap_e convert_cap_mode(uint32 mode);
agg::line_join_e convert_join_mode(uint32 mode);

# endif // SUPPORT_H
