// FloatKernal.h

#include <stdio.h>

#include "support.h"

#include "FloatImage.h"

#include "FloatKernel.h"

// constructor
FloatKernel::FloatKernel(kernel_func function, int32 radius, bool horizontal)
	: fElement(NULL),
	  fDiameter(0),
	  fHorizontal(horizontal),
	  fSymetry(KERNEL_SYMETRIC)
{
	if (function && radius >= 0) {
		fDiameter = 2 * radius + 1;
		fElement = new float[fDiameter];
		if (fElement) {
			fElement[radius] = function(0);
			for (int32 i = 1; i < radius + 1; i++) {
				fElement[radius + i] = function(3 * float(i) / radius);
				fElement[radius - i] = function(3 * float(-i) / radius);
			}
		}
	}
}

// constructor
FloatKernel::FloatKernel(float* elements, int32 diameter,
						 bool horizontal, uint32 symetry)
	: fElement(copy(elements, diameter)),
	  fDiameter(diameter),
	  fHorizontal(horizontal), 
      fSymetry(symetry)
{
}

// copy contructor
FloatKernel::FloatKernel(FloatKernel& from)
	: fElement(copy(from.Element(), from.Diameter())),
	  fDiameter(from.Diameter()),
	  fHorizontal(from.IsHorizontal()),
	  fSymetry(from.Symetry())
{
}

// destructor
FloatKernel::~FloatKernel()
{
	delete[] fElement;
}

// IsValid
bool
FloatKernel::IsValid() const
{
	if (fElement && fDiameter > 0)
		return true;
	return false;
}

// =
FloatKernel&
FloatKernel::operator=(FloatKernel& from)
{
	if (this != &from) {
		delete[] fElement;
		fElement = copy(from.Element(), from.Diameter()); 
		fDiameter = from.Diameter(); 
		fHorizontal = from.IsHorizontal();
		fSymetry = from.Symetry();
	}
	return *this;
}

// *
FloatImage*
FloatKernel::operator*(FloatImage* from) const
{
	FloatImage* result = NULL;
	if (IsValid() && from && from->IsValid()) {
		int32 rad = Radius();
		uint32 i = 0;
		uint32 j = 0;
		int32 k = 0;
		uint32 width = from->Width();
		uint32 height = from->Height();
		float *data  = from->Pixel();
		result = new FloatImage(width, height);
		if (result && result->IsValid()) {
			result->SetAllPixels(0.0);
			float *np = result->Pixel();
			switch (fSymetry) {
				case KERNEL_GENERAL: 
					if (fHorizontal) {
						for (j = 0; j < height; j++) 
							for (i = rad; i < width - rad; i++)  
						for (k = -rad; k <= rad; k++) 
							np[i + j * width] += data[(i + k) + j * width] * fElement[k + rad];
					} else {
						for (j = rad; j < height - rad; j++) {
							for (i = 0; i < width; i++) {
								for (k = -rad; k <= rad; k++) {
									np[i + j * width] += data[i + (k + j) * width]
														 * fElement[k + rad];
								}
							}
						}
					}
					break;
				case KERNEL_SYMETRIC:
					if (fHorizontal) {
						for (j = 0; j < height; j++) {
							for (i = rad; i < width - rad; i++) {
								np[i + j * width] = data[i + j * width] * fElement[rad];
								for (k = 1; k <= rad; k++) {
									np[i + j * width] += (data[(i + k) + j * width]
														  + data[(i - k) + j * width])
														 * fElement[k + rad];
								}
							}
						}
					} else {
						for (j = rad; j < height - rad; j++) {
							for (i = 0; i < width; i++) {
								np[i + j * width] = data[i + j * width] * fElement[rad];
								for (k = 1; k <= rad; k++)  {
									np[i + j * width] += (data[i + (j + k) * width]
														  + data[i + (j - k) * width])
														 * fElement[k + rad];
								}
							}
						}
					}
					break;      
				case KERNEL_ANTISYMETRIC:
					if (fHorizontal) {
						for (j = 0; j < height; j++) {
							for (i = rad; i < width - rad; i++) {
								for (k = 1; k <= rad; k++) {
									np[i + j * width] += (data[(i + k) + j * width]
														  - data[(i - k) + j * width])
														 * fElement[k + rad];
								}
							}
						}
					} else {
						for (j = rad; j < height - rad; j++) {
							for (i = 0; i < width; i++) {
								for (k = 1; k <= rad; k++) {
									np[i + j * width] += (data[i + (j + k) * width]
														  -data[i + (j - k) * width])
														 * fElement[k + rad];
								}
							}
						}
					}
					break;
			}
		}
	}
	return result;
}

// PrintToStream
void
FloatKernel::PrintToStream() const
{
	int32 r = Radius();
	for (int32 i = -r; i <= r; i++)
		cerr << '(' << i << ", " << fElement[i + r] << ")\n";
}
