// alpha.h

#ifndef ALPHA_H
#define ALPHA_H

#include <SupportDefs.h>

// subtract_alpha
inline void
subtract_alpha(uint8* dest, uint8 alpha)
{
	dest[3] = max_c(0, dest[3] - alpha);
}

// subtract_alpha_copy
inline void
subtract_alpha_copy(uint8* from, uint8* dest, uint8 alpha)
{
	dest[0] = from[0];
	dest[1] = from[1];
	dest[2] = from[2];
	dest[3] = max_c(0, from[3] - alpha);
}

// add_alpha
inline void
add_alpha(uint8* dest, float scale)
{
	uint8 alpha = (uint8)(255 * scale);
	dest[0] = min_c(255, dest[0] + alpha);
}

// add_alpha
inline void
add_alpha(uint8* dest, float scale, uint8 maxAlpha)
{
	uint8 alpha = (uint8)(maxAlpha * scale);
	dest[0] = min_c(maxAlpha, dest[0] + alpha);
}

// add_alpha
inline void
add_alpha(uint8* dest, uint8 alpha)
{
	dest[0] = min_c(255, dest[0] + alpha);
}

// add_alpha32
inline void
add_alpha32(uint8* dest, uint8 alpha)
{
	dest[3] = min_c(255, dest[3] + alpha);
}

// add_alpha32_copy
inline void
add_alpha32_copy(uint8* from, uint8* dest, uint8 alpha)
{
	dest[3] = min_c(255, from[3] + alpha);
}

// replace_color
inline void
replace_color(uint8* dest, const rgb_color& color, float scale)
{
	uint8 alpha = (uint8)(255 * scale);
	if (alpha > dest[3]) {
		dest[0] = color.blue;
		dest[1] = color.green;
		dest[2] = color.red;
		dest[3] = alpha;
	}
}

// push_alpha
inline void
push_alpha(uint8* dest, float scale)
{
	uint8 alpha = (uint8)(255 * scale);
	if (alpha > dest[0]) {
		dest[0] = alpha;
	}
}

// push_alpha
inline void
push_alpha(uint8* dest, uint8 alpha)
{
	if (alpha > dest[0]) {
		dest[0] = alpha;
	}
}

// push_alpha_limit
inline void
push_alpha_limit(uint8* dest, uint8 alpha, uint8 limit)
{
/*	if (dest[0] < limit) {
		uint8 add = (alpha * (limit - dest[0])) / limit;
		if (dest[0] + add > limit)
			dest[0] = limit;
		else
			dest[0] += add;
	}*/
	if (dest[0] < limit)
		dest[0] += (alpha * (limit - dest[0])) / limit;
//	else
//		dest[0] -= (alpha * (dest[0] - limit)) / dest[0];
}


# endif // ALPHA_H
