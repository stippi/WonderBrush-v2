// Brush.cpp

#include <stdio.h>

#include <Bitmap.h>

#include "alpha.h"
#include "blending.h"

#include "Brush.h"

#define BRUSH_ANGLE_TEST 1

// init_gauss_table
#if TEST_NEW_FUNCTION
float*
init_gauss_table()
{
	float* table = new float[256];
	for (uint32 i = 0; i < 256; i++) {
		float x = sqrtf((float)i / 255.0);
		table[i] = powf(2.0, (-2.0 * ((x * 2.0) * (x * 2.0)))) + x * sinf(x * PI) / 10.0;
	}
	return table;
#else // TEST_NEW_FUNCTION
uint8*
init_gauss_table()
{
	uint8* table = new uint8[256];
	for (uint32 i = 0; i < 256; i++) {
		float x = sqrtf((float)i / 255.0);
		float y = powf(2.0, (-2.0 * ((x * 2.0) * (x * 2.0)))) + x * sinf(x * PI) / 10.0;
		table[i] = (uint8)floorf(y * 255.0/* + 0.5*/);
	}
	return table;
}
#endif // TEST_NEW_FUNCTION

#if TEST_NEW_FUNCTION
//float* Brush::fGaussTable = init_gauss_table();
#else
//uint8* Brush::fGaussTable = init_gauss_table();
#endif

// constructor
Brush::Brush(range radius, range hardness)
	: fRadius(radius),
	  fHardness(hardness)
{
}

// destructor
Brush::~Brush()
{
}

double
gauss(double f)
{ 
	// this aint' a real gauss function
	if (f >= -1.0 && f <= 1.0) {
		if (f < -0.5) {
			f = -1.0 - f;
			return (2.0 * f*f);
		}

		if (f < 0.5)
			return (1.0 - 2.0 * f*f);

		f = 1.0 - f;
		return (2.0 * f*f);
	}
	return 0.0;
}


// alpha_for_distance
float
alpha_for_distance(float dist, float radius, float hardness, float* gammaTable)
{
	if (dist <= radius) {
		float hardCenter = radius * hardness;
		if (dist < hardCenter)
			return 1.0;
		else {
			float x = dist - hardCenter;
			radius -= hardCenter;
//			x /= radius;
//			return 1.0 - x;
//			return cosf(x * PI2);
//			return powf(2.0, (-2.0 * ((x * 2.0) * (x * 2.0))));
			return gauss(x);
//printf("index: %ld\n", uint8((x / radius) * 255));
//			return gammaTable[uint8((x / radius) * 255)];
		}
	}
	return 0.0;
}

// Draw
void
Brush::Draw(BPoint where, float pressure, range alphaRange,
			uint32 flags, uint8* dest, uint32 bpr,
			BRect constrainRect) const
{
//bigtime_t now = system_time();
	float radius = fRadius.max;
	float hardness = fHardness.max;
	if (flags & FLAG_PRESSURE_CONTROLS_RADIUS)
		radius = fRadius.min + (fRadius.max - fRadius.min) * pressure;
	if (flags & FLAG_PRESSURE_CONTROLS_HARDNESS)
		hardness = fHardness.min + (fHardness.max - fHardness.min) * pressure;
	if (flags & FLAG_PRESSURE_CONTROLS_APHLA)
		pressure = alphaRange.min + (alphaRange.max - alphaRange.min) * pressure;
	else
		pressure = alphaRange.max;
	uint8 alphaLimit = (uint8)(pressure * 255);
	if (radius <= 0.5) {
		// special case for when brush fits within single pixel
		if (flags & FLAG_SOLID) {
			int32 x = (int32)floorf(where.x + 0.5);
			int32 y = (int32)floorf(where.y + 0.5);
	
			dest += x + bpr * y;
			if (constrainRect.Contains(BPoint(x, y))) {
				push_alpha(dest, alphaLimit);
			}
		} else {
			pressure *= radius * 2.0;
			if (hardness < 1.0) {
				// TODO: maybe tweak this a bit
				pressure *= (0.1 + 0.9 * hardness);
			}
			float floorX = floorf(where.x);
			float floorY = floorf(where.y);
			float leftX = where.x - floorX;	// part not covered by pixel
			float leftY = where.y - floorY;
			float coverX = 1.0 - leftX;	// part covered by pixel
			float coverY = 1.0 - leftY;
			dest += (int32)floorX + bpr * (int32)floorY;
			if (constrainRect.Contains(BPoint(floorX, floorY))) {
				// offset into top left pixel
				push_alpha(dest, (uint8)(alphaLimit * pressure * (coverX * coverY)));
			}
			dest ++;
			if (constrainRect.Contains(BPoint(floorX + 1.0, floorY))) {
				// offset into top right pixel
				push_alpha(dest, (uint8)(alphaLimit * pressure * (leftX * coverY)));
			}
			dest += bpr - 1;
			if (constrainRect.Contains(BPoint(floorX, floorY + 1.0))) {
				// offset into bottom left pixel
				push_alpha(dest, (uint8)(alphaLimit * pressure * (coverX * leftY)));
			}
			dest ++;
			if (constrainRect.Contains(BPoint(floorX + 1.0, floorY + 1.0))) {
				// offset into bottom right pixel
				push_alpha(dest, (uint8)(alphaLimit * pressure * (leftX * leftY)));
			}
		}
	} else {
		// top left pixel
		float left = max_c(floorf(constrainRect.left), floorf(where.x - radius)); 
		float top = max_c(floorf(constrainRect.top), floorf(where.y - radius));
		// bottom right pixel
		float right = min_c(ceilf(constrainRect.right), ceilf(where.x + radius));
		float bottom = min_c(ceilf(constrainRect.bottom), ceilf(where.y + radius));
		// offset into bitmap (top left pixel)
		dest += (int32)left + bpr * (int32)top;
		// precalc some constant variables
//		float radius2 = radius * radius;
//		float radiusBinom = (radius + 1.0) * (radius + 1.0);
		float yDiff = top - where.y;
		float finalYDiff = bottom - where.y;
		float finalXDiff = right - where.x;
		float radiusBinom = (radius + 0.49) * (radius + 0.49);

#if BRUSH_ANGLE_TEST
float angle = 0.0;
float aspect = .5;
float sinus = sinf(angle);
float cosinus = cosf(angle);
		// iterate over pixels within bounding box of brush
		for (; yDiff <= finalYDiff; yDiff++) {
			uint8* destHandle = dest;
			for (float xDiff = left - where.x; xDiff <= finalXDiff; xDiff++) {
				if (*destHandle < alphaLimit) {
					float tx = cosinus * xDiff - sinus * yDiff;
					float ty = cosinus * yDiff + cosinus * xDiff;
					ty *= aspect;
					float currentDist = tx * tx + ty * ty;
					if (currentDist < radiusBinom) {
						// covered
						float coverage = alpha_for_distance(sqrtf(currentDist), radius,
															hardness, NULL);
						uint8 alpha = (uint8)(pressure * coverage * 255.0);
						push_alpha(destHandle, alpha);
					}
				}
				destHandle++;
			}
			dest += bpr;
		}
#else // BRUSH_ANGLE_TEST
		// iterate over pixels within bounding box of brush
		if (flags & FLAG_SOLID) {
			for (; yDiff <= finalYDiff; yDiff++) {
				uint8* destHandle = dest;
				float yDiff2 = yDiff * yDiff;
				for (float xDiff = left - where.x; xDiff <= finalXDiff; xDiff++) {
					if (*destHandle < alphaLimit) {
						float currentDist = xDiff * xDiff + yDiff2;
						if (currentDist < radiusBinom) {
							// covered
							push_alpha(destHandle, alphaLimit);
						}
					}
					destHandle++;
				}
				dest += bpr;
			}
		} else {
			if (hardness == 1.0) {
				float radius2 = (radius - 0.51) * (radius - 0.51);
				for (; yDiff <= finalYDiff; yDiff++) {
					uint8* destHandle = dest;
					float yDiff2 = yDiff * yDiff;
					for (float xDiff = left - where.x; xDiff <= finalXDiff; xDiff++) {
						if (*destHandle < alphaLimit) {
							float currentDist = xDiff * xDiff + yDiff2;
							if (currentDist < radius2) {
								// fully covered
								push_alpha(destHandle, alphaLimit);
							} else if (currentDist < radiusBinom) {
								// radius hits pixel
								float coverage = 1.0 - (sqrtf(currentDist) - (radius - 0.51));
								uint8 alpha = (uint8)(alphaLimit * coverage);
	//							push_alpha_limit(destHandle, alpha, alphaLimit);
								push_alpha(destHandle, alpha);
							}
						}
						destHandle++;
					}
					dest += bpr;
				}
			} else if (hardness < 1.0 && hardness >= 0.0) {
				float radius2 = (radius - 0.51) * (radius - 0.51);
				float radiusBinom = (radius + 0.49) * (radius + 0.49);
//				float radiusBinom = (radius - 0.51) * (radius - 0.51);
				float radiusHard = (radius - 0.51) * hardness;
				float radiusHard2 = radiusHard * radiusHard;
				float softRadius = radius - radiusHard;
				float softRadius2 = radius2 - radiusHard2;
				for (; yDiff <= finalYDiff; yDiff++) {
					uint8* destHandle = dest;
					float yDiff2 = yDiff * yDiff;
					for (float xDiff = left - where.x; xDiff <= finalXDiff; xDiff++) {
						if (*destHandle < alphaLimit) {
							float currentDist = xDiff * xDiff + yDiff2;
#if TEST_NEW_FUNCTION
							float coverage = alpha_for_distance(sqrtf(currentDist), radius,
																hardness, fGaussTable);
							uint8 alpha = (uint8)(pressure * coverage * 255.0);
							if (alpha)
								push_alpha_limit(destHandle, alpha, alphaLimit);
#else // TEST_NEW_FUNCTION
							if (currentDist < radiusHard2) {
								// fully covered part
								push_alpha(destHandle, alphaLimit);
							} else if (currentDist < radius2) {
								// soft part
								uint32 index = (uint32)(((currentDist - radiusHard2) / softRadius2) * 255.0);
//								uint32 index = (uint32)(((sqrtf(currentDist) - radiusHard) / softRadius) * 255.0);
								if (index < 256) {
									uint8 alpha = (uint8)(pressure * fGaussTable[index]);
									push_alpha_limit(destHandle, alpha, alphaLimit);
								} else {
printf("radius hits pixel: index out of range: %ld \n", index);
								}
							} else if (currentDist < radiusBinom) {
								// anti aliasing (radius hits pixel)
								float tooMuch = currentDist - radius2;
								float coverage = 1.0 - (sqrtf(currentDist) - (radius - 0.51));
//								float coverage = sqrtf(tooMuch);
								// index of the gamma value as if the pixel was hit right on
//								uint32 index = (uint32)(((currentDist - radiusHard2) / softRadius2) * 255.0);
								uint32 index = (uint32)((((currentDist - tooMuch) - radiusHard2) / softRadius2) * 255.0);
								if (index < 256) {
//printf("radius hits pixel: tooMuch: %f, coverage: %f, index: %ld (%d)\n",
//	   tooMuch, coverage, index, fGaussTable[index]);
									uint8 alpha = (uint8)(coverage * pressure * fGaussTable[index]);
									push_alpha_limit(destHandle, alpha, alphaLimit);
								} else {
printf("radius hits pixel: tooMuch: %f, coverage: %f, index out of range: %ld \n",
	   tooMuch, coverage, index);
								}
							}
#endif // TEST_NEW_FUNCTION
						}
						destHandle++;
					}
					dest += bpr;
				}
			}
		}
#endif // BRUSH_ANGLE_TEST
	}
//printf("Brush::Draw() - %lld (radius: %f, hardness: %f)\n", system_time() - now, radius, hardness);
}
