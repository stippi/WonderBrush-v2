// image_operations.h

#ifndef IOMAGE_OPERATIONS_H
#define IOMAGE_OPERATIONS_H

class FloatImage;
class MapImage;
class RGB8Image;

FloatImage*		dx_op(FloatImage* source, uint32 radius);

FloatImage*		dy_op(FloatImage* source, uint32 radius);

FloatImage*		box(FloatImage* source,
					uint32 x0, uint32 x1, uint32 y0, uint32 y1);

FloatImage*		add_margin(FloatImage* source,
						   bool deleteSource = false);

FloatImage*		norm(FloatImage* dX, FloatImage* dY);

FloatImage*		derive(FloatImage* source,
					   uint32 direction, uint32 radius);

FloatImage*		non_maximal_suppression(FloatImage* Dx,
										FloatImage* Dy,
										FloatImage* mag);

FloatImage**	nms_edge_data(FloatImage* source,
							  float lambda, uint32 radius);

FloatImage**	edge_data(FloatImage* source,
						  float lambda, uint32 radius);

MapImage*		nms_edge_map(FloatImage *source,
							 float lambda, uint32 radius);

MapImage*		edge_map(FloatImage* source,
						 float lambda, uint32 radius);

MapImage*		nms_scan_edge_map(FloatImage* source,
								  float lambda, uint32 radius);

MapImage*		scan_edge_map(FloatImage* source,
							  float lambda, uint32 radius);

FloatImage*		nms_edge_data(FloatImage* source, uint32 radius);

FloatImage*		edge_data(FloatImage* source, uint32 radius);

void			mapdiff(MapImage& map1, const MapImage& map2);

// -----------------------------------
// general functions
// -----------------------------------

RGB8Image*		combine(RGB8Image* original,
						RGB8Image* blurred, float theta);

RGB8Image*		combine(RGB8Image* original,
						RGB8Image* blurred,
						RGB8Image* sharpen, float theta);

RGB8Image*		scale_bilinear(RGB8Image* source, float factor);

RGB8Image*		canonical_warp_b(RGB8Image* source,
								 MapImage& map,
								 float scale, bool invert = false);

RGB8Image*		canonical_warp_f(RGB8Image* source,
								 MapImage& map,
								 uint32 destWidth,
								 uint32 destHeight, uint32 res);


#endif // IOMAGE_OPERATIONS_H
