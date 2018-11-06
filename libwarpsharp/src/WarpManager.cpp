// WarpManager.cpp

#include <stdio.h>

#include <Bitmap.h>

#include "image_operations.h"
#include "support.h"

#include "FloatImage.h"
#include "MapImage.h"
#include "RGB8Image.h"
#include "SparseMatrix.h"

#include "WarpManager.h"

// constructor
WarpManager::WarpManager()
	: fEdgeRadius(2),
	  fLambda(10.0),
	  fMu(0.01),
	  fScale(1.0),
	  fUseTwoComponents(false),
	  fUseNMS(false),
	  fInputImage(NULL),
	  fInputFloatImage(NULL),
	  fEdgeImages(NULL),
	  fMapImage(NULL),
	  fGX(NULL),
	  fGY(NULL),
	  fDeleteSharpMap(true)
{
}

// destructor
WarpManager::~WarpManager()
{
	MakeEmpty();
}

// IsValid
bool
WarpManager::IsValid() const
{
	if (fEdgeRadius >= 1)
		return true;
	return false;
}

// MakeEmpty
void
WarpManager::MakeEmpty()
{
	fInputImage = NULL;
	delete fInputFloatImage;
	fInputFloatImage = NULL;
	_DeleteEdgeImages();
	_DeleteMapImage();
	_DeleteSparseMatrices();
}

// SetEdgeRadius
void
WarpManager::SetEdgeRadius(uint32 radius)
{
	if (radius >= 1 && radius != fEdgeRadius) {
		fEdgeRadius = radius;
		_DeleteEdgeImages();
		_DeleteMapImage();
	}
}

// SetLambda
void
WarpManager::SetLambda(float lambda)
{
	if (lambda != fLambda) {
		fLambda = lambda;
		_DeleteMapImage();
		_DeleteEdgeImages();
	}
}

// SetMu
void
WarpManager::SetMu(float mu)
{
	if (mu != fMu) {
		fMu = mu;
		_DeleteMapImage();
		_DeleteSparseMatrices();
	}
}

// SetScale
void
WarpManager::SetScale(float scale)
{
	fScale = scale;
}

// SetUseTwoComponents
void
WarpManager::SetUseTwoComponents(bool two)
{
	if (two != fUseTwoComponents) {
		fUseTwoComponents = two;
		_DeleteMapImage();
	}
}

// SetUseNonMaximalSupression
void
WarpManager::SetUseNonMaximalSupression(bool nms)
{
	if (nms != fUseNMS) {
		fUseNMS = nms;
		_DeleteMapImage();
	}
}

// SetSharpMap
bool
WarpManager::SetSharpMap(MapImage* map)
{
	bool success = false;
	if (map && map->IsValid()) {
		if (fInputImage && fInputImage->IsValid()) {
			if (map->Width() == fInputImage->Width()
				&& map->Height() == fInputImage->Height())
				success = true;
		} else
			success = true;
	}
	if (success) {
		_DeleteMapImage();
		fDeleteSharpMap = false;
		fMapImage = map;
	}
	return success;
}

// SetInputImage
bool
WarpManager::SetInputImage(RGB8Image* input)
{
	bool success = false;
	if (input && input->IsValid()) {
		delete fInputFloatImage;
		fInputImage = input;
		fInputFloatImage = new FloatImage(fInputImage);
		success = fInputFloatImage->IsValid();
		if (!success && fDeleteSharpMap)
			MakeEmpty();
	}
	return success;
}

// UnsetInputImage
void
WarpManager::UnsetInputImage()
{
	fInputImage = NULL;
}

// GetWarpedImage
RGB8Image*
WarpManager::GetWarpedImage()
{
	if (!fMapImage)
		_ComputeSharpMap(fInputFloatImage);
	RGB8Image* result = canonical_warp_b(fInputImage, *fMapImage, fScale);
	return result;
}

// GetEdgeImage
FloatImage*
WarpManager::GetEdgeImage()
{
//	FloatImage* edges = edge_data(fInputFloatImage, fEdgeRadius);
//	return edges;
	FloatImage* normed = NULL;
	if (fEdgeImages)
		normed = norm(fEdgeImages[0], fEdgeImages[1]);
	return normed;
}

// _ComputeInitial
void
WarpManager::_ComputeInitial(float* xInit, float* yInit,
						 uint32 width, uint32 height)
{
	for (uint32 j = 0; j < height; j++)
		for (uint32 i = 0; i < width; i++) {
			xInit[i + j * width] = i;
			yInit[i + j * width] = j;
		}
}

// _ComputeRightHand
void
WarpManager::_ComputeRightHand(float* bx, float* by, float mu,
							   FloatImage* xClamp, FloatImage* yClamp)
{
	uint32 dx = xClamp->Width();
	uint32 dy = xClamp->Height();
	float* gx = xClamp->Pixel();
	float* gy = yClamp->Pixel();
/*	for (uint32 j = 0; j < dy; j++)
		for (uint32 i = 0; i < dx; i++) {
			if ((i == 0) || (i == dx - 1)
				|| (j == 0) || (j == dy - 1)) {
				bx[i + j * dx] = i;
				by[i + j * dx] = j;
			} else {
				bx[i + j * dx] = 4 * mu * gx[i + j * dx] * i;
				by[i + j * dx] = 4 * mu * gy[i + j * dx] * j;
			}
		}*/
	// unrolled version:
	for (uint32 i = 0; i < dx; i++) {
		bx[i] = i;
		by[i] = 0;
	}
	for (uint32 i = 0; i < dx; i++) {
		bx[i + (dy - 1) * dx] = i;
		by[i + (dy - 1) * dx] = dy - 1;
	}
	for (uint32 j = 1; j < dy - 1; j++) {
		bx[j * dx] = 0;
		by[j * dx] = j;
		bx[(dx - 1) + j * dx] = dx - 1;
		by[(dx - 1) + j * dx] = j;
		for (uint32 i = 1; i < dx - 1; i++) {
			bx[i + j * dx] = 4 * mu * gx[i + j * dx] * i;
			by[i + j * dx] = 4 * mu * gy[i + j * dx] * j;
		}
	}
}

// _ComputeSharpMap
void
WarpManager::_ComputeSharpMap(FloatImage* input)
{
	if (input && input->IsValid()) {
		_DeleteMapImage();
	
		uint32 dx = input->Width();
		uint32 dy = input->Height();
		uint32 size = dx * dy;
	
		uint32 maxIterations = uint32(8 * (fLambda / 10 + 1));
		uint32 iterations = 1;
	
		float* xInit = new float[size];
		float* yInit = new float[size];
		float* bx = new float[size];
		float* by = new float[size];
/*		float* xInit = clear_float(size);
		float* yInit = clear_float(size);
		float* bx = clear_float(size);
		float* by = clear_float(size);*/
	
		if (!fGX || !fGY || !fEdgeImages)
			_ComputeSparseMatrices(input);

		if (fGX && fGY && fEdgeImages && xInit && yInit && bx && by) {
			_ComputeInitial(xInit, yInit, dx, dy);
			_ComputeRightHand(bx, by, fMu, fEdgeImages[0], fEdgeImages[1]);
	
			SparseMatrix::Solve(*fGX, xInit, bx, maxIterations, iterations);
			SparseMatrix::Solve(*fGY, yInit, by, maxIterations, iterations);
		
			fDeleteSharpMap = true;
			fMapImage = new MapImage(dx, dy, xInit, yInit);
		}
		delete[] xInit;
		delete[] yInit;
		delete[] bx;
		delete[] by;
	}
}

// _ComputeEdgeImages
void
WarpManager::_ComputeEdgeImages(FloatImage* input)
{
	if (input && input->IsValid()) {
		_DeleteEdgeImages();
		if (fUseNMS)
			fEdgeImages = nms_edge_data(input, fLambda, fEdgeRadius);
		else
			fEdgeImages = edge_data(input, fLambda, fEdgeRadius);
		if (fEdgeImages)
			_ComputeSparseMatrices(input);
	}
}

// _ComputeSparseMatrices
void
WarpManager::_ComputeSparseMatrices(FloatImage* input)
{
	if (!fEdgeImages) {
		_ComputeEdgeImages(input);
		return;
	}
	if (fEdgeImages) {
		_DeleteSparseMatrices();
		fGX = SparseMatrix::AssembleSharpMatrix(fEdgeImages[0], fMu);
		fGY = SparseMatrix::AssembleSharpMatrix(fEdgeImages[1], fMu);
	}
}

// _DeleteEdgeImages
void
WarpManager::_DeleteEdgeImages()
{
	if (fEdgeImages) {
		delete fEdgeImages[0];
		delete fEdgeImages[1];
	}
	delete[] fEdgeImages;
	fEdgeImages = NULL;
}

// _DeleteMapImage
void
WarpManager::_DeleteMapImage()
{
	if (fDeleteSharpMap)
		delete fMapImage;
	fMapImage = NULL;
}

// _DeleteSparseMatrices
void
WarpManager::_DeleteSparseMatrices()
{
	delete fGX;
	fGX = NULL;
	delete fGY;
	fGY = NULL;
}
