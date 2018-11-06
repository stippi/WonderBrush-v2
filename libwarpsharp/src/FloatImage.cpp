// FloatImage.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <TranslationUtils.h>

#include "support.h"

#include "RGB8Image.h"

#include "FloatImage.h"

// constructor
FloatImage::FloatImage()
	: fWidth(0),
	  fHeight(0),
	  fPixels(NULL)
{
}

// contructor
FloatImage::FloatImage(uint32 width, uint32 height) 
	: fWidth(width),
	  fHeight(height),
	  fPixels(new float[CountPixels()])
{
	if (!IsValid())
		MakeEmpty();
}

// copy constructor
FloatImage::FloatImage(FloatImage& copy)
	: fWidth(0),
	  fHeight(0),
	  fPixels(NULL)
{
	if (copy.IsValid()) {
		fWidth = copy.Width();
		fHeight = copy.Height();
		fPixels = new float[CountPixels()];
		if (fPixels) {
			for (uint32 i = 0; i < CountPixels(); i++)
				fPixels[i] = copy.Pixel(i);
		} else
			MakeEmpty();
	} else
		MakeEmpty();
}

// BBitmap constructor
FloatImage::FloatImage(BBitmap* bitmap)
	: fWidth(0),
	  fHeight(0),
	  fPixels(NULL)
{
	SetToBitmap(bitmap);
}

// RGB8Image constructor
FloatImage::FloatImage(RGB8Image* image, bool greenOnly)
	: fWidth(0),
	  fHeight(0),
	  fPixels(NULL)
{
	if (image && image->IsValid()) {
		uint32 size = image->CountPixels();
		fWidth = image->Width();
		fHeight = image->Height();
		fPixels = new float[size];
		float* p = fPixels;
		uint8* r = image->Red();
		uint8* g = image->Green();
		uint8* b = image->Blue();
		if (!greenOnly)
			while (size--)
				*p++ = 0.299 * (float)(*r++)
					   + 0.587 * (float)(*g++)
					   + 0.114 * (float)(*b++);
		else
			while (size--)
				*p++ = (float)(*g++);
	}
}


// file constructor
FloatImage::FloatImage(const char *fileName)
	: fWidth(0),
	  fHeight(0),
	  fPixels(NULL)
{
	SetToBitmap(BTranslationUtils::GetBitmap(fileName));
}

// destructor
FloatImage::~FloatImage()
{
  MakeEmpty();
}

// IsValid
bool
FloatImage::IsValid()
{
	if (Width() > 0 && Height() > 0 && fPixels)
		return true;
	return false;
}

// MakeEmpty
void
FloatImage::MakeEmpty()
{
	fWidth = 0;
	fHeight = 0;
	delete[] fPixels;
	fPixels = NULL;
}

// SetAllPixels
void
FloatImage::SetAllPixels(float value)
{
	if (IsValid()) {
		uint32 count = CountPixels();
		for (uint32 i = 0; i < count; i++)
			fPixels[i] = value;
	}
}

// SetPixel
void
FloatImage::SetPixel(uint32 x, uint32 y, float value)
{
	if (IsValid() && x < Width() && y < Height()) {
		uint32 index = x + y * Width();
		fPixels[index] = value;
	} else {
fprintf(stderr, "SetPixel(%ld, %ld) - index out of bounds!()", x, y);
	}
}

// SetPixel
void
FloatImage::SetPixel(uint32 index, float value)
{
	if (IsValid() && index < CountPixels()) {
		fPixels[index] = value;
	} else {
fprintf(stderr, "SetPixel(%ld) - index out of bounds!()", index);
	}
}

// PixelSave
float
FloatImage::PixelSave(uint32 pixelIndex) const
{
	if (pixelIndex < CountPixels())
		return fPixels[pixelIndex];
	else {
fprintf(stderr, "PixelSave() - index out of bounds!()");
		return 0.0;
	}
}


// GetBitmap
BBitmap*
FloatImage::GetBitmap(bool scale)
{
	BBitmap* bitmap = NULL;
	if (IsValid()) {
		bitmap = new BBitmap(BRect(0.0, 0.0, Width() - 1, Height() - 1), B_RGB32);
		if (bitmap && bitmap->IsValid()) {
			// copy ourselves into BBitmap
			uint8* lineStart = (uint8*)bitmap->Bits();
			int32 bpr = bitmap->BytesPerRow();
			float* p = fPixels;
			if (scale) {
				float min, max;
				min_max(p, min, max, CountPixels());
				float factor  = max > min ? (max - min) : 1.0;
				for (uint32 y = 0; y < Height(); y++) {
					int32* pixel = (int32*)lineStart;
					for (uint32 x = 0; x < Width(); x++) {
						uint8* c = (uint8*)pixel++;
						uint8 color = (uint8)(((*p++) - min) * 255 / factor);;
						c[0] = color;
						c[1] = color;
						c[2] = color;
						c[3] = 255;
					}
					lineStart += bpr;
				}
			} else {
				for (uint32 y = 0; y < Height(); y++) {
					int32* pixel = (int32*)lineStart;
					for (uint32 x = 0; x < Width(); x++) {
						uint8* c = (uint8*)pixel++;
						uint8 color = intclip((int32)*p++);
						c[0] = color;
						c[1] = color;
						c[2] = color;
						c[3] = 255;
					}
					lineStart += bpr;
				}
			}
		} else {
			delete bitmap;
			bitmap = NULL;
		}
	}
	return bitmap;
}

// SetToBitmap
bool
FloatImage::SetToBitmap(BBitmap* bitmap)
{
	bool success = false;
	if (bitmap && bitmap->IsValid()) {
		if (bitmap->ColorSpace() == B_RGB32 || bitmap->ColorSpace() == B_RGBA32) {
			fWidth = (uint32)(bitmap->Bounds().right - bitmap->Bounds().left) + 1;
			fHeight = (uint32)(bitmap->Bounds().bottom - bitmap->Bounds().top) + 1;
			fPixels = new float[CountPixels()];
			if (fPixels) {
				// copy BBitmap into cimage
				uint8* lineStart = (uint8*)bitmap->Bits();
				int32 bpr = bitmap->BytesPerRow();
				float* p = fPixels;
				for (uint32 y = 0; y < Height(); y++) {
					int32* pixel = (int32*)lineStart;
					for (uint32 x = 0; x < Width(); x++) {
						uint8* c = (uint8*)pixel++;
						*p++ = 0.299 * (float)c[2]
							   + 0.587 * (float)c[1]
							   + 0.114 * (float)c[0];
					}
					lineStart += bpr;
				}
				success = true;
			} else
				MakeEmpty();
		} else if (bitmap->ColorSpace() == B_RGB32_BIG || bitmap->ColorSpace() == B_RGBA32_BIG) {
			fWidth = (uint32)(bitmap->Bounds().right - bitmap->Bounds().left) + 1;
			fHeight = (uint32)(bitmap->Bounds().bottom - bitmap->Bounds().top) + 1;
			fPixels = new float[CountPixels()];
			if (fPixels) {
				// copy BBitmap into cimage
				uint8* lineStart = (uint8*)bitmap->Bits();
				int32 bpr = bitmap->BytesPerRow();
				float* p = fPixels;
				for (uint32 y = 0; y < Height(); y++) {
					int32* pixel = (int32*)lineStart;
					for (uint32 x = 0; x < Width(); x++) {
						uint8* c = (uint8*)pixel++;
						*p++ = 0.299 * (float)c[1]
							   + 0.587 * (float)c[2]
							   + 0.114 * (float)c[3];
					}
					lineStart += bpr;
				}
				success = true;
			} else
				MakeEmpty();
		}
	}
	return success;
}

