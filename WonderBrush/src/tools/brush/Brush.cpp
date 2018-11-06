// Brush.cpp

#include <stdio.h>

#include <Bitmap.h>

#include <agg_alpha_mask_u8.h>
#include <agg_pixfmt_gray.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_u.h>
#include <agg_scanline_p.h>

#include <agg_ellipse.h>
#include <agg_conv_transform.h>
#include <agg_span_gradient.h>
#include <agg_span_interpolator_trans.h>

#include "agg_pixfmt_brush8.h"
#include "alpha.h"
#include "blending.h"
#include "support.h"

#include "Brush.h"

#define BRUSH_ANGLE_TEST 1

// init_gauss_table
uint8*
init_gauss_table()
{
	uint8* table = new uint8[256];
	for (uint32 i = 0; i < 256; i++) {
//		float x = sqrtf((float)i / 255.0);
//		float y = powf(2.0, (-2.0 * ((x * 2.0) * (x * 2.0)))) + x * sinf(x * PI) / 10.0;
//		table[i] = (uint8)floorf(y * 255.0/* + 0.5*/);
		table[i] = (uint8)(255.0 * (gauss(i / 255.0)));
	}
//	table[0] = 255;
//	table[255] = 0;
	return table;
}



uint8* Brush::fGaussTable = init_gauss_table();

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

// Draw
void
Brush::Draw(BPoint where, float pressure, range alphaRange,
			uint32 flags, uint8* dest, uint32 bpr,
			BRect constrainRect) const
{
	Transformable transform;
	Draw(where, pressure, 0.0, 0.0, alphaRange, flags, dest, bpr, transform, constrainRect);
//bigtime_t now = system_time();
/*	float radius = fRadius.max;
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
//				float softRadius = radius - radiusHard;
				float softRadius2 = radius2 - radiusHard2;
				for (; yDiff <= finalYDiff; yDiff++) {
					uint8* destHandle = dest;
					float yDiff2 = yDiff * yDiff;
					for (float xDiff = left - where.x; xDiff <= finalXDiff; xDiff++) {
						if (*destHandle < alphaLimit) {
							float currentDist = xDiff * xDiff + yDiff2;
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
						}
						destHandle++;
					}
					dest += bpr;
				}
			}
		}
	}*/
//printf("Brush::Draw() - %lld (radius: %f, hardness: %f)\n", system_time() - now, radius, hardness);
}


typedef agg::renderer_base<agg::pixfmt_brush8>					renderer_base;
//typedef agg::renderer_scanline_u_solid<renderer_base>			renderer_type;
typedef agg::renderer_scanline_aa_solid<renderer_base>			renderer_type;
typedef agg::gradient_circle									gradient_function;
typedef agg::span_interpolator_trans<Transformable>				interpolator_type;

typedef agg::pod_auto_array<agg::gray8, 256>					color_array_type;
typedef agg::span_gradient<agg::gray8,
						   interpolator_type,
						   gradient_function,
						   color_array_type>					gradient_generator;

typedef agg::span_allocator<agg::gray8>							gradient_allocator;

typedef agg::renderer_scanline_aa<renderer_base,
								  gradient_generator>			gradient_renderer;
//typedef agg::renderer_scanline_u<renderer_base,
//								  gradient_generator>			gradient_renderer;



// Draw
void
Brush::Draw(BPoint where, float pressure, float tiltX, float tiltY,
			range alpha, uint32 flags,
			uint8* bits, uint32 bpr,
			const Transformable& transform,
			BRect constrainRect) const
{
//printf("Brush::Draw()\n");
//bigtime_t startTime = system_time();
	// NOTE: I have no idea why I had the second and third check here!!!
	if (!constrainRect.IsValid()/* || constrainRect.Width() < 1.0 || constrainRect.Height() < 1.0*/) {
//printf("invalid constrain rect\n");
		return;
	}

	// radius
	double radius;
	if (flags & FLAG_PRESSURE_CONTROLS_RADIUS)
		radius = fRadius.min + (fRadius.max - fRadius.min) * pressure;
	else
		radius = fRadius.max;

	// check clipping here
	BRect clipTest(where.x - radius, where.y - radius, where.x + radius, where.y + radius);
	clipTest = transform.TransformBounds(clipTest);
	if (!constrainRect.Intersects(clipTest)) {
//printf("brush shape outside clipping\n");
		return;
	}
//printf("drawing brush (%f, %f)\n", where.x, where.y);

	// hardness
	double hardness;
	if (flags & FLAG_PRESSURE_CONTROLS_HARDNESS)
		hardness = fHardness.min + (fHardness.max - fHardness.min) * pressure;
	else
		hardness = fHardness.max;


	agg::rasterizer_scanline_aa<> rasterizer;

	// attach the AGG buffer to the bitmap
	agg::rendering_buffer buffer;
	int width = constrainRect.IntegerWidth() + 1;
	int height = constrainRect.IntegerHeight() + 1;
	bits += (int32)constrainRect.left + (int32)constrainRect.top * bpr;
	buffer.attach(bits, width, height, bpr);

	rasterizer.clip_box(0, 0, width, height);

	agg::pixfmt_brush8 pixelFormat(buffer);

	renderer_base rendererBase(pixelFormat);
	renderer_type renderer(rendererBase);
	agg::scanline_u8 scanlineU;

	uint8 alphaScale;
	if (flags & FLAG_PRESSURE_CONTROLS_APHLA)
		alphaScale = uint8(255 * (alpha.min + (alpha.max - alpha.min) * pressure));
	else
		alphaScale = uint8(255 * alpha.max);

	renderer.color(agg::gray8(alphaScale));

	// ellipse
	agg::ellipse ellipse(0.0, 0.0, radius, radius, 64);

	// ellipse transformation
	Transformable ellipseTransform;

	// calculate tilt deformation and rotation
	if (flags & FLAG_TILT_CONTROLS_SHAPE) {
		float invTiltX = 1.0 - fabs(tiltX);
		float invTiltY = 1.0 - fabs(tiltY);
		double xScale = (sqrtf(invTiltX * invTiltX + invTiltY * invTiltY) / sqrtf(2.0));
	
		double angle = calc_angle(B_ORIGIN, BPoint(tiltX, 0.0), BPoint(tiltX, tiltY), false);
		ellipseTransform *= agg::trans_affine_scaling(xScale, 1.0);
		ellipseTransform *= agg::trans_affine_rotation(angle);
	}

	// calculate transformation
	ellipseTransform *= agg::trans_affine_translation(where.x, where.y);
	ellipseTransform *= transform;
	ellipseTransform *= agg::trans_affine_translation(-constrainRect.left, -constrainRect.top);

	// configure pixel format
	pixelFormat.cover_scale(alphaScale);
	pixelFormat.solid(flags & FLAG_SOLID);

	// actually rendering the gradient in the ellipse
	agg::conv_transform<agg::ellipse, Transformable> transformedEllipse(ellipse, ellipseTransform);
	rasterizer.add_path(transformedEllipse);

//bigtime_t renderTime = system_time();

	// special case for hardness = 1.0
	if (hardness == 1.0) {
		agg::render_scanlines(rasterizer, scanlineU, renderer);
	} else {
		// gradient transformation
		Transformable gradientTransform = ellipseTransform;
		gradientTransform.invert();
	
		// defining the gradient
		gradient_function	gradientFunction;
		interpolator_type	interpolator(gradientTransform);
		gradient_allocator	spanAllocator;
		color_array_type	array((agg::gray8*)fGaussTable);
		gradient_generator	gradientGenerator(spanAllocator, interpolator,
											  gradientFunction, array,
											  hardness * radius, radius * 2 - hardness * radius + 1.0);
		gradient_renderer	gradientRenderer(rendererBase, gradientGenerator);
	
		agg::render_scanlines(rasterizer, scanlineU, gradientRenderer);
	}

//bigtime_t finishTime = system_time();
//printf("init time: %lld, render time: %lld, total: %lld  (radius: %f, hardness: %f)\n",
//	   renderTime - startTime, finishTime - renderTime, finishTime - startTime, radius, hardness);
}

