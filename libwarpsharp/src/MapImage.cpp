// MapImage.cpp

#include <math.h>
#include <stdio.h>

#include <StorageDefs.h>

#include "complex.h"
#include "support.h"

#include "FloatImage.h"

#include "MapImage.h"

// contstructor
MapImage::MapImage()
	: fWidth(0),
	  fHeight(0),
	  fVector(NULL)
{
}

// contstructor
MapImage::MapImage(uint32 width, uint32 height, complex *vectors)
	: fWidth(width),
	  fHeight(height),
	  fVector(vectors)
{
}

// contstructor
MapImage::MapImage(const MapImage& copy)
	: fWidth(copy.Width()),
	  fHeight(copy.Height()), 
	  fVector(new complex[CountVectors()])
{
	// copy contents of other map image
	for (uint32 i = 0; i < CountVectors(); i++)
		fVector[i] = copy.fVector[i];
}

// contstructor
MapImage::MapImage(uint32 width, uint32 height, double *re, double *im)
	: fWidth(width),
	  fHeight(height),
	  fVector(new complex[CountVectors()])
{
	for (uint32 i = 0; i < CountVectors(); i++)
		fVector[i] = complex(re[i], im[i]);
}
 
// contstructor
MapImage::MapImage(uint32 width, uint32 height, float *re, float *im)
	: fWidth(width),
	  fHeight(height),
	  fVector(new complex[CountVectors()])
{
	for (uint32 i = 0; i < CountVectors(); i++)
		fVector[i] = complex(re[i], im[i]);
}
 
const char* mapdir = "";

// contstructor
MapImage::MapImage(const char* filename)
	: fWidth(0),
	  fHeight(0),
	  fVector(NULL)
{
	//load a gziped map file from disc
	char full[B_PATH_NAME_LENGTH];
	sprintf(full, "%s%s", mapdir, filename);
	FILE* f = fopen(full, "r");
	if (f) { 
		fclose(f);
		char compress[B_PATH_NAME_LENGTH + 50];
		sprintf(compress, "gunzip -c %s", full);    
		FILE* command = popen(compress, "r");
		if (command) {
			fscanf(command, "%ld %ld\n", &fWidth, &fHeight);
			fVector = new point[CountVectors()];
			fread(fVector, sizeof(point), CountVectors(), command);
			pclose(command);
		}
	}
}

// destructor
MapImage::~MapImage()
{
	delete[] fVector;
}

// IsValid
bool
MapImage::IsValid() const
{
	if (fWidth > 0 && fHeight > 0 && fVector)
		return true;
	return false;
}

// SaveAs      
bool
MapImage::SaveAs(const char* filename, bool overwrite) const
{
	bool success = false;
	if (IsValid() && filename) {
		char full[B_PATH_NAME_LENGTH];
		sprintf(full, "%s%s", mapdir, filename);
		if ((fopen(full, "r")) && (!overwrite)) {
			sprintf(full, "mapdir%s", filename);
		}
		char compress[200];
		sprintf(compress, "gzip -c >%s", full);
		FILE* file = popen(compress, "w");
		fprintf(file, "%ld %ld\n", fWidth, fHeight);
		fwrite(fVector, sizeof(point), CountVectors(), file);
		fclose(file);
		success = true;
	}
	return success;
}

// =
MapImage&
MapImage::operator=(const MapImage& copy)
{
	if (this != &copy && copy.IsValid()) {
		delete[] fVector;
		fWidth = 0;
		fHeight = 0;
		fVector = new complex[copy.CountVectors()];
		if (fVector) {
			fWidth = copy.Width();
			fHeight = copy.Height();
			complex* copyVector = copy.Vector();
			for (uint32 i = 0; i < CountVectors(); i++)
				fVector[i] = copyVector[i];
		}
	}
	return (*this);
}

// ()
complex
MapImage::operator()(const complex& z)
{
	uint32 i = uint32(floor(z.real())),
					  j = int(floor(z.imag()));
	double x = z.real()-i, y = z.imag()-j;
	complex* p = fVector;
	return (1 - x) * (1 - y) * p[i + j * fWidth]
			+ x * (1 - y) * p[min(i + 1, fWidth - 1) + j * fWidth]
			+ (1 - x) * y * p[i + min(fHeight - 1, j + 1) * fWidth]
			+ x * y * p[min(i + 1, fWidth - 1) + min(j + 1, fHeight - 1) * fWidth];
}

// GetFloatImage
FloatImage*
MapImage::GetFloatImage(bool imaginary) const
{
	FloatImage* result = NULL;
	if (IsValid()) {
		result = new FloatImage(Width(), Height());
		if (result && result->IsValid()) {
			float* out = result->Pixel();
			complex* val = fVector;
			uint32 index = CountVectors();
			do {
				*(out)++ = imaginary == IMAGINARY ? imag(*(val)++) : real(*(val)++);
			} while (--index);
		} else {
			delete result;
			result = NULL;
		}
	}
	return result;
}  

// ABS
FloatImage*
MapImage::ABS() const
{
	FloatImage* result = NULL;
	if (IsValid()) {
		result = new FloatImage(Width(), Height());
		if (result && result->IsValid()) {
			float* out = result->Pixel();
			complex* val = fVector;
			uint32 index = CountVectors();
			while (index--) { 
				complex t(*val++);
				*out++ = sqrt(sqr(real(t)) + sqr(imag(t)));
			}
		} else {
			delete result;
			result = NULL;
		}
	}
	return result;
}  

// Scale
void
MapImage::Scale(int32 factor, uint32 direction)
{
	if (factor == 1 || !IsValid())
		return;
	switch (direction) {
		case DIRECTION_FORWARD: {
			uint32 count = CountVectors();
			for (uint32 i = 0; i < count; i++)
				fVector[i] *= factor;
			break;
		}
		case DIRECTION_BACKWARD: {
			uint32 width = fWidth;
			uint32 height = fHeight;
			float inc = 1.0 / factor;
			fWidth *= factor;
			fHeight *= factor;
			complex* vector1 = new complex[CountVectors()];
			for (uint32 i = 0; i < CountVectors(); i++)
				vector1[i] = complex(0.0, 0.0);
			complex* p1 = vector1;
			for (uint32 j = 0; j < fHeight; j++)
				for (uint32 i = 0; i < fWidth; i++, p1++) {
					int32 ii = i % factor;
					int32 jj = j % factor;
					int32 ifr = i / factor;
					int32 jfr = j / factor;
					float t = ii * inc;
					float s = jj * inc;
					int32 index[4];
					index[0] = ifr + jfr * width;
					index[1] = min(ifr + 1, (int32)width - 1) + jfr * width;
					index[2] = ifr + min(jfr + 1, (int32)height - 1) * width;
					index[3] = min(ifr + 1, (int32)width - 1)
							   + min(jfr + 1, (int32)height - 1) * width;
					float coef[4];
					coef[0] = (1 - t) * (1 - s);
					coef[1] = t * (1 - s);
					coef[2] = (1 - t) * s;
					coef[3] = t * s;
					for (uint32 k = 0; k < 4; k++) 
						if (coef[k] != 0)
							*p1 += coef[k] * fVector[index[k]];
				}
			delete [] fVector;
			fVector = vector1;
			break;
		}
	}
}

// Movement
void
MapImage::Movement()
{
	if (IsValid()) {
		for (uint32 j = 0; j < fHeight; j++)
			for(uint32 i = 0; i < fWidth; i++) 
				fVector[i + j * fWidth] -= complex(i, j);
	}
}

