// image_operations.cpp

#include <math.h>
#include <stdio.h>

#include "support.h"

#include "FloatImage.h"
#include "FloatKernel.h"
#include "Gray8Image.h"
#include "IntegerKernel.h"
#include "MapImage.h"
#include "RGB8Image.h"

#include "image_operations.h"

const float MAX = 0.15; // this parameter was empirically found to 
                        // be the maximal value of the edge detector.

// dx_op
FloatImage*
dx_op(FloatImage* source, uint32 radius)
{
	FloatImage* result = NULL;
	if (source && source->IsValid()) {
		FloatKernel g1(Dgaussian, radius, true);
		g1.SetSymetry(FloatKernel::KERNEL_ANTISYMETRIC);
		FloatKernel g2(gaussian, radius, false);
		g2.SetSymetry(FloatKernel::KERNEL_SYMETRIC);
		FloatImage* temp = g1 * source;
		result = g2 * temp;
		delete temp;
	}
	return result;
}

// dy_op
FloatImage*
dy_op(FloatImage* source, uint32 radius)
{
	FloatImage* result = NULL;
	if (source && source->IsValid()) {
		FloatKernel g1(gaussian, radius, true);
		g1.SetSymetry(FloatKernel::KERNEL_SYMETRIC);
		FloatKernel g2(Dgaussian, radius, false);
		g2.SetSymetry(FloatKernel::KERNEL_ANTISYMETRIC);
		FloatImage* temp = g1 * source;
		result = g2 * temp;
		delete temp;
	}
	return result;
}

// box
FloatImage*
box(FloatImage* source, uint32 x0, uint32 x1, uint32 y0, uint32 y1)
{
	FloatImage* result = NULL;
	if (source && source->IsValid() && x0 <= x1 && y0 <= y1) {
		uint32 width = source->Width();
		uint32 height = source->Height();
		// don't allow box to be out of bounds
		if (x1 < width && y1 < height) {
			result = new FloatImage(x1 - x0, y1 - y0);
			if (result && result->IsValid()) {
				float* output = result->Pixel();
				float* input = source->Pixel();
				for (uint32 i = y0; i < y1; i++)
					for(uint32 j = x0; j< x1; j++)
						*output++ = input[i * width + j];
			} else {
				delete result;
				result = NULL;
			}
		}
	}
	return result;
}

// add_margin
FloatImage*
add_margin(FloatImage* source, bool deleteSource)
{
	FloatImage* result = NULL;
	if (source && source->IsValid()) {
		uint32 width = source->Width();
		uint32 height = source->Height();
	    uint32 marginWidth = width + 2;
	    uint32 marginHeight = height + 2;
	    result = new FloatImage(marginWidth, marginHeight);
	    if (result && result->IsValid()) {
			float* output = result->Pixel();
			float* input = source->Pixel();
			for (uint32 i = 0; i < width; i++)
				for (uint32 j = 0; j < height; j++)
					output[1 + i + (j + 1) * marginWidth] = input[i + j * width];
			for (uint32 i = 1; i < 1 + width; i++)
				output[i] = output[i + marginWidth];
			for (uint32 i = 1; i < 1 + width; i++)
				output[i + marginWidth * (marginHeight - 1)] =
					output[i + marginWidth * (marginHeight - 2)];
			for (uint32 j = 1; j < 1 + height; j++)
				output[j * marginWidth] = output[j * marginWidth + 1];
			for (uint32 j = 1; j < 1 + height; j++)
				output[j * marginWidth + marginWidth - 1] =
					output[j * marginWidth + marginWidth - 2];
			// corners
			output[0] = output[1];
			output[marginWidth - 1] = output[marginWidth - 2];
			output[marginWidth * (marginHeight - 1)] = output[marginWidth * (marginHeight - 1) + 1];
			output[marginWidth * marginHeight - 1] = output[marginWidth * marginHeight - 2];
		
			if (deleteSource)
				delete source;
	    } else {
	    	delete result;
	    	result = NULL;
	    }
	}
	return result;
}

// -------------------------------------
// functions for edge detection
// -------------------------------------

static double normFactor = 512;

// default_norm_threshold
inline float
default_norm_threshold (float maxWidth, float maxHeight)
{
	return (maxWidth + maxHeight) / normFactor;
}

static float (*norm_thresh)(float mx, float my) = default_norm_threshold;

// norm
FloatImage*
norm(FloatImage* dX, FloatImage* dY)
{
	FloatImage* normed = NULL;
	if (dX && dY && dX->IsValid() && dY->IsValid()) {
		register int32 size = dX->CountPixels();
		register double threshold = norm_thresh(fmax(dX->Pixel(), size),
												fmax(dY->Pixel(), size));
	
		normed = new FloatImage(dX->Width(), dX->Height());
		if (normed && normed->IsValid()) {
			register float* x = dX->Pixel();
			register float* y = dY->Pixel();
			register float* n = normed->Pixel();
			do {
				double m = *x * *x + *y * *y;
				*(n++) = m > threshold ? sqrt(m) : 0;
				x++;
				y++;
			} while (--size);
		} else {
			delete normed;
			normed = NULL;
		}
	}
	return normed;
}

// derive
FloatImage*
derive(FloatImage* source, uint32 direction, uint32 radius)
{
	FloatImage* result = direction == X_DIR ? dx_op(source, radius)
											: dy_op(source, radius);
	return result;
}

// non_maximal_suppression
FloatImage*
non_maximal_suppression(FloatImage* Dx, FloatImage* Dy, FloatImage* mag)
{
	FloatImage* thin = NULL;
	if (Dx && Dy && mag && Dx->IsValid() && Dy->IsValid() && mag->IsValid()) {
		register double F = tan(M_PI / 8.0);
		register uint32 width = Dx->Width();
		register uint32 size = width * Dx->Height();
		thin = new FloatImage(width, Dx->Height());
		if (thin && thin->IsValid()) {
			float* X = Dx->Pixel();
			float* Y = Dy->Pixel();
			float* M = mag->Pixel();
			float* T = thin->Pixel();
			do {
				*(T++) = *M && (fabs(*Y) < F * fabs(*X) ?
					(*M >= M[1] && *M >= M[-1]) :
						fabs(*X) < F * fabs(*Y) ? (*M >= M[width] && *M >= M[-width]) :
							*X * *Y  > 0.0 ? (*M >= M[1 + width] && *M >= M[-1 - width]) :
								(*M >= M[width - 1] && *M >= M[1 - width])) ? *M : 0.0;
		        X++; Y++; M++;
		   } while (--size);
		} else {
			delete thin;
			thin = NULL;
		}
	}
	return thin;
}

// nms_edge_data
FloatImage**
nms_edge_data(FloatImage* source, float lambda, uint32 radius)
{
	FloatImage** output = NULL;
	if (source && source->IsValid()) {
		uint32 width = source->Width();
		uint32 height = source->Height();
		uint32 size = source->CountPixels();
		FloatImage* xInfo = derive(source, X_DIR, radius);
		FloatImage* yInfo = derive(source, Y_DIR, radius);
		FloatImage* normed = norm(xInfo, yInfo);
		FloatImage* nms = non_maximal_suppression(xInfo, yInfo, normed);
		if (nms && nms->IsValid()) {
			output = new FloatImage*[2];
			if (output) {
				output[0] = new FloatImage(width, height);
				output[1] = new FloatImage(width, height);
				if (output[0] && output[0]->IsValid()
					&& output[1] && output[1]->IsValid()) {
					float* out0 = output[0]->Pixel();
					float* out1 = output[1]->Pixel();
					float* x = xInfo->Pixel();
					float* y = yInfo->Pixel();
					float* N = normed->Pixel();
					float* n = nms->Pixel();
					for (uint32 i = 0; i < size; i++) {
						if (n[i] == 0.0)
							out0[i] = out1[i] = 0.0;
						else {
							out0[i] = n[i] * fabs(x[i]) / N[i];
					         out1[i] = n[i] * fabs(y[i]) / N[i];
						}
					}
					float min, max;
					float* p = output[0]->Pixel();
					min_max(p, min, max, size);
					while (size--) {
						*p = 1.0 + (*p) * lambda / max;
						p++;
					}
					p = output[1]->Pixel();
					size = source->CountPixels();
					min_max(p, min, max, size);
					while (size--) {
						*p = 1.0 + (*p) * lambda / max;
						p++;
					}
				} else {
					delete output[0];
					delete output[1];
					delete[] output;
					output = NULL;
				}
			}
		}
		delete xInfo;
		delete yInfo;
		delete normed;
		delete nms;
	}
	return output;
}

// edge_data
FloatImage**
edge_data(FloatImage* source, float lambda, uint32 radius)
{
	FloatImage** output = NULL;
	if (source && source->IsValid()) {
		uint32 width = source->Width();
		uint32 height =source->Height();
		uint32 size = source->CountPixels();
		FloatImage* xInfo = derive(source, X_DIR, radius);
		FloatImage* yInfo = derive(source, Y_DIR, radius);
		FloatImage* normed = norm(xInfo, yInfo);
		if (xInfo && yInfo && normed
			&& xInfo->IsValid() && yInfo->IsValid() && normed->IsValid()) {
			output = new FloatImage*[2];
			if (output) {
				output[0] = new FloatImage(width, height);
				output[1] = new FloatImage(width, height);
				if (output[0] && output[0]->IsValid()
					&& output[1] && output[1]->IsValid()) {
					float* out0 = output[0]->Pixel();
					float* out1 = output[1]->Pixel();
					float* x = xInfo->Pixel();
					float* y = yInfo->Pixel();
					float* N = normed->Pixel();
					for (uint32 i = 0; i < size; i++) {
						if (N[i] == 0.0)
							*out0++ = *out1++ = 0.0;
						else {
							*out0++ = N[i] * fabs(x[i]) / N[i];
							*out1++ = N[i] * fabs(y[i]) / N[i];
						}
					}
					float min, max;
					float* p = output[0]->Pixel();
					min_max(p, min, max, size);
					while(size--) {
						*p = 1.0 + (*p) * lambda / max;
						p++;
					}
					p = output[1]->Pixel();
					size = source->CountPixels();
					min_max(p, min, max, size);
					while (size--) {
						*p = 1.0 + (*p) * lambda / max;
						p++;
					}
				} else {
					delete output[0];
					delete output[1];
					delete[] output;
					output = NULL;
				}
			}
		}
		delete xInfo;
		delete yInfo;
		delete normed;
	}
	return output;
}

// nms_edge_map
MapImage*
nms_edge_map(FloatImage *source, float lambda, uint32 radius)
{
	MapImage* map = NULL;
	if (source && source->IsValid()) {
		uint32 width = source->Width();
		uint32 height = source->Height();
		uint32 size = source->CountPixels();
		FloatImage* xInfo = derive(source, X_DIR, radius);
		FloatImage* yInfo = derive(source, Y_DIR, radius);
		FloatImage* normed = norm(xInfo, yInfo);
		FloatImage* nms = non_maximal_suppression(xInfo, yInfo, normed);
		if (nms && nms->IsValid()) {
			float* out0 = new float[size];
			float* out1 = new float[size];
			if (out0 && out1) {
				float* x = xInfo->Pixel();
				float* y = yInfo->Pixel();
				float* N = normed->Pixel();
				float* n = nms->Pixel();
				for (uint32 i = 0; i < size; i++) {
					if (n[i] == 0.0)
						out0[i] = out1[i] = 0.0;
					else {
						out0[i] = n[i] * fabs(x[i]) / N[i];
						out1[i] = n[i] * fabs(y[i]) / N[i];
					}
				}
				float max = fmax(out0, size);
				float *p = out0;
				while(size--) {
					*p = 1 + (*p) * lambda / max;
					p++;
				}
				size = source->CountPixels();
				p = out1;
				max = fmax(out1, size);
				while(size--) {
					*p = 1 + (*p) * lambda / max;
					p++;
				}
				map = new MapImage(width, height, out0, out1);
			}
			delete [] out0;
			delete [] out1;
		}
		delete xInfo;
		delete yInfo;
		delete normed;
		delete nms;
	}
	return map;
}

// edge_map
MapImage*
edge_map(FloatImage* source, float lambda, uint32 radius)
{
	MapImage* map = NULL;
	if (source && source->IsValid()) {
		uint32 width = source->Width();
		uint32 height = source->Height();
		uint32 size = source->CountPixels();
		FloatImage* xInfo = derive(source, X_DIR, radius);
		FloatImage* yInfo = derive(source, Y_DIR, radius);
		FloatImage* normed = norm(xInfo, yInfo);
		if (normed && normed->IsValid()) {
			float* out0 = new float[size];
			float* out1 = new float[size];
			if (out0 && out1) {
				float* x = xInfo->Pixel();
				float* y = yInfo->Pixel();
				float* N = normed->Pixel();
				for (uint32 i = 0; i < size; i++) {
					if (N[i] == 0)
						out0[i] = out1[i] = 0.0;
					else {
						out0[i] = N[i] * fabs(x[i]) / N[i];
						out1[i] = N[i] * fabs(y[i]) / N[i];
					}
				}
				float max = fmax(out0, size);
				float *p = out0;
				while(size--) {
					*p = 1 + (*p)*lambda/max;
					p++;
				}
				size = source->CountPixels();
				p = out1;
				max = fmax(out1, size);
				while(size--) {
					*p = 1 + (*p)*lambda/max;
					p++;
				}
				map = new MapImage(width, height, out0, out1);
			}
			delete[] out0;
			delete[] out1;
		}
		delete xInfo;
		delete yInfo;
		delete normed;
	}
	return map;
}

// nms_scan_edge_map
MapImage*
nms_scan_edge_map(FloatImage* source, float lambda, uint32 radius)
{
	MapImage* map = NULL;
	if (source && source->IsValid()) {
		complex a(1.0, 1.0);
		uint32 width = source->Width();
		uint32 height = source->Height();
		uint32 size = source->CountPixels();
		FloatImage* xInfo = derive(source, X_DIR, radius);
		FloatImage* yInfo = derive(source, Y_DIR, radius);
		FloatImage* normed = norm(xInfo, yInfo);
		FloatImage* nms = non_maximal_suppression(xInfo, yInfo, normed);
		if (nms && nms->IsValid()) {
			complex* out = new complex[size];
			if (out) {
				float* x = xInfo->Pixel();
				float* y = yInfo->Pixel();
				float* N = normed->Pixel();
				float* n = nms->Pixel();
				for (uint32 i = 0; i < size; i++)
					if (n[i] == 0)
						out[i] = complex(0,0);
					else
						out[i] = n[i] * complex(fabs(x[i]) / N[i], fabs(y[i])/N[i]);
				complex *p = out;
				while(size--)
					*p++ = (a + (*p) * (lambda / MAX)); 
				map = new MapImage(width, height, out);
			}
		}
		delete xInfo;
		delete yInfo;
		delete normed;
		delete nms;
	}
	return map;
}

// scan_edge_map
MapImage*
scan_edge_map(FloatImage* source, float lambda, uint32 radius)
{
	MapImage* map = NULL;
	if (source && source->IsValid()) {
		uint32 width = source->Width();
		uint32 height = source->Height();
		uint32 size = source->CountPixels();
		FloatImage* xInfo = derive(source, X_DIR, radius);
		FloatImage* yInfo = derive(source, Y_DIR, radius);
		FloatImage* normed = norm(xInfo, yInfo);
		if (normed && normed->IsValid()) {
			complex *out = new complex[size];
			if (out) {
				float* x = xInfo->Pixel();
				float* y = yInfo->Pixel();
				float* N = normed->Pixel();
				for (uint32 i = 0; i < size; i++)
					if (N[i] == 0)
						out[i] = complex(0.0, 0.0);
					else
						out[i] = complex(fabs(x[i]), fabs(y[i]));
				complex *p = out;
				while (size--)
					*p++ = (1 + (*p) * (lambda / MAX));
				map = new MapImage(width, height, out);
			}
		}
		delete xInfo;
		delete yInfo;
		delete normed;
	}
	return map;
}

// nms_edge_data
FloatImage*
nms_edge_data(FloatImage* source, uint32 radius)
{
	FloatImage* nms = NULL;
	if (source && source->IsValid()) {
		FloatImage* xInfo = derive(source, X_DIR, radius);
		FloatImage* yInfo = derive(source, Y_DIR, radius);
		FloatImage* normed = norm(xInfo, yInfo);
		nms = non_maximal_suppression(xInfo, yInfo, normed);
		delete xInfo;
		delete yInfo;
		delete normed;
	}
	return nms;
}

// edge_data
FloatImage*
edge_data(FloatImage* source, uint32 radius)
{
	FloatImage* normed = NULL;
	if (source && source->IsValid()) {
		FloatImage* xInfo = derive(source, X_DIR, radius);
		FloatImage* yInfo = derive(source, Y_DIR, radius);
		normed = norm(xInfo, yInfo);
		delete xInfo;
		delete yInfo;
	}
	return normed;
}

// mapdiff
void
mapdiff(MapImage& map1, const MapImage& map2)
{
	if ((!map1.IsValid() || !map2.IsValid())
		|| (map1.Width() != map2.Width())
		|| (map1.Height() != map2.Height()))
		return;
	uint32 size = map1.CountVectors();
	uint32 i;
	complex* p1 = map1.Vector();
	complex* p2 = map2.Vector();
	float maxx = 0.0;
	float maxy = 0.0;
	for (i = 0; i < size; i++) {
		float tx = fabs(p1[i].real()-p2[i].real());
		float ty = fabs(p1[i].imag()-p2[i].imag());
		if (tx > maxx)
			maxx = tx;
		if (ty > maxy)
			maxy = ty;
		p1[i] = complex(tx, ty);
	}
	if (maxx > 0.0) {
		float factor = 255.0 / maxx;
		for (i = 0; i < size; i++)
			p1[i] = complex(p1[i].real()*factor, p1[i].imag());
	}
	if (maxy > 0.0) {
		float factor = 255.0 / maxy;
		for (i = 0; i < size; i++)
			p1[i] = complex(p1[i].real(), p1[i].imag() * factor);
	}
}

// -----------------------------------
// general functions
// -----------------------------------

// combine
RGB8Image*
combine(RGB8Image* original, RGB8Image* blurred, float theta)
{
	RGB8Image* result = NULL;
	if (original && blurred && original->IsValid() && blurred->IsValid()) {
		uint32 size = original->CountPixels();
		if (size == blurred->CountPixels()) {
			result = new RGB8Image(original->Width(), original->Height());
			if (result && result->IsValid()) {
				uint8* r = result->Red();
				uint8* g = result->Green();
				uint8* b = result->Blue();
				uint8* ro = original->Red();
				uint8* go = original->Green();
				uint8* bo = original->Blue();
				uint8* rb = blurred->Red();
				uint8* gb = blurred->Green();
				uint8* bb = blurred->Blue();
				while(size--) {
					*r++ = clip((1.0 + theta) * (*ro++) - theta * (*rb++));
					*g++ = clip((1.0 + theta) * (*go++) - theta * (*gb++));
					*b++ = clip((1.0 + theta) * (*bo++) - theta * (*bb++));
				}
			} else {
				delete result;
				result = NULL;
			}
		}
	}
	return result;
}

// combine
RGB8Image*
combine(RGB8Image* original, RGB8Image* blurred, RGB8Image* sharpen, float theta)
{
	RGB8Image* result = NULL;
	if (original && original->IsValid()
		&& blurred && blurred->IsValid()
		&& sharpen && sharpen->IsValid()) {
		uint32 size = original->CountPixels();
		if (size == blurred->CountPixels() && size == sharpen->CountPixels()) {
			result = new RGB8Image(original->Width(), original->Height());
			if (result && result->IsValid()) {
				uint8* r = result->Red();
				uint8* g = result->Green();
				uint8* b = result->Blue();
				uint8* ro = original->Red();
				uint8* go = original->Green();
				uint8* bo = original->Blue();
				uint8* rb = blurred->Red();
				uint8* gb = blurred->Green();
				uint8* bb = blurred->Blue();
				uint8* rs = sharpen->Red();
				uint8* gs = sharpen->Green();
				uint8* bs = sharpen->Blue();
				while (size--) {
					*r++ = clip(*ro++ + theta*(*rs++ - *rb++));
					*g++ = clip(*go++ + theta*(*gs++ - *gb++));
					*b++ = clip(*bo++ + theta*(*bs++ - *bb++));
				}
			} else {
				delete result;
				result = NULL;
			}
		}
	}
	return result;
}

// scale_bilinear
RGB8Image*
scale_bilinear(RGB8Image* source, float factor)
{
	RGB8Image* result = NULL;
	if (source && source->IsValid() && factor > 0.0) {
		uint32 sourceWidth = source->Width();
		uint32 sourceHeight = source->Height();
		uint32 destWidth = uint32(float(sourceWidth) * factor);
		uint32 destHeight = uint32(float(sourceHeight) * factor);

		result = new RGB8Image(destWidth, destHeight);
		if (result && result->IsValid()) {
			register uint32 destSize = result->CountPixels();
			register int32 t = -1;
			result->SetAllPixels(0, 0, 0);
			uint8* rh = result->Red();
			uint8* gh = result->Green();
  			uint8* bh = result->Blue();
			uint8* fr = source->Red();
			uint8* fg = source->Green();
			uint8* fb = source->Blue();
			float weights[4];
			uint32 index[4];
			while (++t < (int32)destSize) {
				point out = point(float(t % destWidth) / factor,
								  float(t / destWidth) / factor);
				int32 x = (int32)out.real();
				int32 y = (int32)out.imag();
				float a = out.real() - x;
				float c = out.imag() - y;
				int32 mx = min(x + 1, (int32)sourceWidth - 1);
				int32 my = min(y + 1, (int32)sourceHeight - 1);
	
				weights[0] = (1.0 - a) * (1 - c);
				weights[1] = (1.0 - c) * a;
				weights[2] = (1.0 - a) * c;
				weights[3] = a * c;
	
				index[0] = x + y * sourceWidth; 
				index[1] = mx + y * sourceWidth; 
				index[2] = x + my * sourceWidth;
				index[3] = mx + my * sourceWidth;
	
				float tr = 0.0;
				float tg = 0.0;
				float tb = 0.0;
	
				for (uint32 i = 0; i < 4; i++) {
					tr += weights[i] * fr[index[i]];
					tg += weights[i] * fg[index[i]];
					tb += weights[i] * fb[index[i]];
				}
				*(rh++) = clip(tr);
				*(gh++) = clip(tg); 
				*(bh++) = clip(tb);
			}
		} else {
			delete result;
			result = NULL;
		}
	}
	return result;
}

// canonical_warp_b
RGB8Image*
canonical_warp_b(RGB8Image* source, MapImage& map, float scale, bool invert)
{
	RGB8Image* result = NULL;
	if (source && source->IsValid() && map.IsValid() && scale > 0.0
		&& map.Width() == source->Width()
		&& map.Height() == source->Height()) {

		uint32 sourceWidth = source->Width();
		uint32 sourceHeight = source->Height();
		uint32 destWidth = uint32(map.Width() * scale);
		uint32 destHeight = uint32(map.Height() * scale);

		result = new RGB8Image(destWidth, destHeight);
		if (result && result->IsValid()) {
			register uint32 destSize = result->CountPixels();
			register int32 t = -1;
			result->SetAllPixels(0, 0, 0);
			uint8* rr = result->Red();
			uint8* rg = result->Green();
			uint8* rb = result->Blue();

			uint8* fr = source->Red();
			uint8* fg = source->Green();
			uint8* fb = source->Blue();

			float weights[4];
			int32 index[4];
			float s = 1.0 / scale;
			while (++t < (int32)destSize) {
				int32 xx = t % destWidth;
				int32 yy = t / destWidth;
				point out = scale ==1 ? map(xx, yy) : map(point(xx, yy) * s);
				if (invert)
					out = 2 * point(min(xx * s, sourceWidth - 1),
									min(yy * s, sourceHeight - 1)) - out;
				int32 i = int32(out.real());
				int32 j = int32(out.imag());
				if (i >=0 && i < (int32)sourceWidth && j >= 0 && j < (int32)sourceHeight) { 
					float x = out.real() - i;
					float y = out.imag() - j;

					weights[0] = (1 - x) * (1 - y);
					weights[1] = x * (1 - y);
					weights[2] = (1 - x) * y;
					weights[3] = x * y;

					index[0] = i + j * sourceWidth;
					index[1] = min(i + 1, (int32)sourceWidth - 1) + j * sourceWidth; 
					index[2] = i + min((int32)sourceHeight - 1, j + 1) * sourceWidth; 
					index[3] = min(i + 1, (int32)sourceWidth - 1)
								+ min(j + 1, (int32)sourceHeight - 1) * sourceWidth;

					float tr = 0;
					float tg = 0;
					float tb = 0;

					for (uint32 ii = 0; ii < 4; ii++) {
						tr += weights[ii] * fr[index[ii]];
						tg += weights[ii] * fg[index[ii]];
						tb += weights[ii] * fb[index[ii]];
					}
					*(rr++) = clip(tr);
					*(rg++) = clip(tg); 
					*(rb++) = clip(tb);
  
				} else {
					rr++;
					rg++;
					rb++;
				}
			}
		} else {
			delete result;
			result = NULL;
		}
	}
	return result;
}

// canonical_warp_f
RGB8Image*
canonical_warp_f(RGB8Image* source, MapImage& map,
				 uint32 destWidth, uint32 destHeight, uint32 res)
{
	RGB8Image* result = NULL;
	if (source && source->IsValid() && map.IsValid()
		&& source->Width() == map.Width() && map.Height() == source->Height()) {
		result = new RGB8Image(destWidth, destHeight);
		if (result && result->IsValid()) {
			uint8* r = result->Red();
			uint8* g = result->Green();
			uint8* b = result->Blue();

			uint32 width = source->Width();
			uint32 height = source->Height();
			uint32 size = source->CountPixels();

			float* dists = new float[size];
			if (dists) {
				for (uint32 u = 0; u < size; u++)
					dists[u] = 1.0;
				point P[4];
				int32 IND[4];
				for (float j = 0.0; j <= height - 1.0; j += 1.0 / res) {
					for (float i = 0.0; i <= width - 1.0; i += 1.0 / res) {
						uint8 rv = source->Red(i, j);
						uint8 gv = source->Green(i, j);
						uint8 bv = source->Blue(i, j);
		
						point z = map(point(i, j));
						point p00 = point(floor(z.real()), floor(z.imag()));
						P[0] = p00;
						P[1] = p00 + 1;
						P[2] = p00 + I;
						P[3] = p00 + I + 1;
		
						int32 i00 = int32(floor(z.real()))
									+ int32(floor(z.imag())) * destWidth;
						IND[0] = i00;
						IND[1] = i00 + 1;
						IND[2] = i00 + destWidth;
						IND[3] = i00 + destWidth + 1;
		
						uint32 rad = 0;
						float dist = 0.0;
						while (rad < 4) {
							if (is_legal(IND[rad], size) && ((dist = norm(z - P[rad])) <= 0.25)) {
								if(dist < dists[IND[rad]]) {
									dists[IND[rad]] = dist;
									r[IND[rad]] = rv;
									g[IND[rad]] = gv;
									b[IND[rad]] = bv;
								}
								rad = 4;
							} else rad++;
						}
					}
				}
			}
			delete[] dists;
		} else {
			delete result;
			result = NULL;
		}
	}
	return result;
}



