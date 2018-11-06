// IntegerKernel.cpp

#include <stdio.h>

#include "support.h"

#include "Gray8Image.h"

#include "IntegerKernel.h"

// constructor
IntegerKernel::IntegerKernel(kernel_func function, int32 radius, bool horizontal)
	: fElement(NULL),
	  fDiameter(0),
	  fHorizontal(horizontal),
	  fSymetry(KERNEL_SYMETRIC),
	  fScale(0)
{
	if (function && radius >= 0) {
		fDiameter = 2 * radius + 1;
		fElement = new int32[fDiameter];
		if (fElement) {
			fElement[radius] = (int32)function(0);
			for (int32 i = 1; i < radius + 1; i++) {
				fElement[radius + i] = (int32)function(3 * float(i) / radius);
				fElement[radius - i] = (int32)function(3 * float(-i) / radius);
			}
		} else
			fDiameter = 0;
	}
}

// constructor
IntegerKernel::IntegerKernel(int32* elements, int32 diameter,
						 bool horizontal, uint32 symetry, int32 scale)
	: fElement(copy(elements, diameter)),
	  fDiameter(diameter),
	  fHorizontal(horizontal),
      fSymetry(symetry),
      fScale(scale)
{
}

// copy constructor
IntegerKernel::IntegerKernel(IntegerKernel& from)
	: fElement(copy(from.Element(), from.Diameter())),
	  fDiameter(from.Diameter()),
	  fHorizontal(from.IsHorizontal()), 
	  fSymetry(from.Symetry()),
	  fScale(from.Scale())
{
}

// destructor
IntegerKernel::~IntegerKernel()
{
	delete[] fElement;
}

// IsValid
bool
IntegerKernel::IsValid() const
{
	if (fElement && fDiameter > 0)
		return true;
	return false;
}

// =
IntegerKernel&
IntegerKernel::operator=(IntegerKernel& from)
{
	if (this != &from) {
		delete[] fElement;
		fElement = copy(from.Element(), from.Diameter());
		fDiameter = from.Diameter();
		fHorizontal = from.IsHorizontal();
		fSymetry = from.Symetry();
		fScale = from.Scale();
	}
	return *this;
}

// *
Gray8Image*
IntegerKernel::operator*(Gray8Image* from) const
{
	Gray8Image* result = NULL;
	if (IsValid() && from && from->IsValid()) {
		uint32 width = from->Width();
		uint32 height = from->Height();
	    result = new Gray8Image(width, height);
	    if (result && result->IsValid()) {
		    result->SetAllPixels(0);
			uint8* np = result->Pixel();   
			uint32 rad = fDiameter / 2;
			uint32 i, j, k;
			uint8* data = from->Pixel();
			switch (fSymetry) {
				case KERNEL_GENERAL:
					if (fHorizontal) {
						for (j = 0; j < height; j++) 
							for (i = rad; i < width - rad; i++)  
								for (k = -rad; k <= rad; k++) 
									np[i + j * width] += data[(i + k) + j * width]
														 * fElement[k + rad];
					} else {
						for (j = rad; j < height - rad; j++) 
							for (i = 0; i < width; i++) 
								for (k = -rad; k <= rad; k++) 
									np[i + j * width] += data[i + (k + j) * width]
														 * fElement[k + rad];
					}
					break;
				case KERNEL_SYMETRIC:
					if (fHorizontal) {
						for (j = 0; j < height; j++) 
							for (i = rad; i < width - rad; i++) {
								np[i + j * width] = data[i + j * width] * fElement[rad];
								for (k = 1; k <= rad; k++) 
									np[i + j * width] += (data[(i + k) + j * width]
														  + data[(i - k) + j * width])
														 * fElement[k + rad];
							}
					} else {
						for (j = rad; j < height - rad; j++) 
							for (i = 0; i < width; i++) {
								np[i + j * width] = data[i + j * width] * fElement[rad];
								for (k = 1; k <= rad; k++) 
									np[i + j * width] += (data[i + (j + k) * width]
														  + data[i + (j - k) * width])
														 * fElement[k + rad];
							}
					}
					break;      
				case KERNEL_ANTISYMETRIC:
					if (fHorizontal) 
						for (j = 0; j < height; j++) 
							for (i = rad; i < width - rad; i++) {
								for(k = 1; k <= rad; k++) 
									np[i + j * width] += (data[(i + k) + j * width]
														  - data[(i - k) + j * width])
														 * fElement[k + rad];
							}
					else {
						for (j = rad; j < height - rad; j++) 
							for (i = 0; i < width; i++) 
								for (k = 1; k <= rad; k++) 
									np[i + j * width] += (data[i + (j + k) * width]
														  -data[i + (j - k) * width])
														 * fElement[k + rad];
					}
					break;
			}
		} else {
			delete result;
			result = NULL;
		}
	}
	return result;
}

// PrintToStream
void
IntegerKernel::PrintToStream()
{
	int32 r = Radius();
	for (int32 i = -r; i <= r; i++)
		cerr << '(' << i << ", " << fElement[i + r] << ")\n";
}






