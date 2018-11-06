// RGB8Image.cpp

#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <TranslationUtils.h>

#include "RGB8Image.h"

// constructor
RGB8Image::RGB8Image()
	: fWidth(0),
	  fHeight(0),
	  fRed(NULL),
	  fGreen(NULL),
	  fBlue(NULL)
{
}

// contructor
RGB8Image::RGB8Image(uint32 width, uint32 height, bool clearToBlack)
	: fWidth(width),
	  fHeight(height),
	  fRed(new uint8[CountPixels()]),
	  fGreen(new uint8[CountPixels()]),
	  fBlue(new uint8[CountPixels()])
{
	if (!IsValid())
		MakeEmpty();
	else if (clearToBlack)
		SetAllPixels(0, 0, 0);
}

// copy constructor
RGB8Image::RGB8Image(const RGB8Image& copy)
	: fWidth(0),
	  fHeight(0),
	  fRed(NULL),
	  fGreen(NULL),
	  fBlue(NULL)
{
	if (copy.IsValid()) {
		fWidth = copy.Width();
		fHeight = copy.Height();
		uint32 size = CountPixels();
		fRed = new uint8[size];
		fGreen = new uint8[size];
		fBlue = new uint8[size];
		if (fRed && fGreen && fBlue) {
			memcpy(fRed, copy.fRed, size);
			memcpy(fGreen, copy.fGreen, size);
			memcpy(fBlue, copy.fBlue, size);
		} else
			MakeEmpty();
	}
}

// BBitmap constructor
RGB8Image::RGB8Image(const BBitmap* bitmap)
	: fWidth(0),
	  fHeight(0),
	  fRed(NULL),
	  fGreen(NULL),
	  fBlue(NULL)
{
	SetToBitmap(bitmap);
}

// file constructor
RGB8Image::RGB8Image(const char *fileName)
	: fWidth(0),
	  fHeight(0),
	  fRed(NULL),
	  fGreen(NULL),
	  fBlue(NULL)
{
	BBitmap* bitmap = BTranslationUtils::GetBitmap(fileName);
	SetToBitmap(bitmap);
	delete bitmap;
}

// destructor
RGB8Image::~RGB8Image()
{
  MakeEmpty();
}

// IsValid
bool
RGB8Image::IsValid() const
{
	if (Width() > 0 && Height() > 0
		&& fRed && fGreen && fBlue)
		return true;
	return false;
}

// MakeEmpty
void
RGB8Image::MakeEmpty()
{
	fWidth = 0;
	fHeight = 0;
	delete[] fRed;
	delete[] fGreen;
	delete[] fBlue;
	fRed = NULL;
	fGreen = NULL;
	fBlue = NULL;
}

// SetTo
bool
RGB8Image::SetTo(uint32 width, uint32 height, bool clearToBlack)
{
	MakeEmpty();
	fWidth = width;
	fHeight = height;
	fRed = new uint8[CountPixels()];
	fGreen = new uint8[CountPixels()];
	fBlue = new uint8[CountPixels()];
	bool success = IsValid();
	if (clearToBlack && success) {
		SetAllPixels(0, 0, 0);
	}
	return success;
}

// SetAllPixels
void
RGB8Image::SetAllPixels(rgb_color color)
{
	if (IsValid()) {
		memset(fRed, color.red, CountPixels());
		memset(fGreen, color.green, CountPixels());
		memset(fBlue, color.blue, CountPixels());
	}
}

// SetAllPixels
void
RGB8Image::SetAllPixels(uint8 red, uint8 green, uint8 blue)
{
	SetAllPixels((rgb_color){red, green, blue, 255});
}

// ColorAt
rgb_color
RGB8Image::ColorAt(uint32 x, uint32 y) const
{
	rgb_color color = { 0, 0, 0, 255 };
	if (IsValid() && x < Width() && y < Height()) {
		color.red = Red(x, y);
		color.green = Green(x, y);
		color.blue = Blue(x, y);
	}
	return color;
}

// SetPixel
void
RGB8Image::SetPixel(uint32 x, uint32 y, rgb_color color)
{
	if (IsValid() && x < Width() && y < Height()) {
		uint32 index = x + y * Width();
		fRed[index] = color.red;
		fGreen[index] = color.green;
		fBlue[index] = color.blue;
	}
}

// GetBitmap
BBitmap*
RGB8Image::GetBitmap() const
{
	BBitmap* bitmap = NULL;
	if (IsValid()) {
		bitmap = new BBitmap(BRect(0.0, 0.0, Width() - 1, Height() - 1), B_RGB32);
		if (!CopyIntoBitmap(bitmap)) {
			delete bitmap;
			bitmap = NULL;
		}
	}
	return bitmap;
}

// CopyIntoBitmap
bool
RGB8Image::CopyIntoBitmap(const BBitmap* bitmap) const
{
	bool success = false;
	if (bitmap && bitmap->IsValid()) {
		uint32 width = bitmap->Bounds().IntegerWidth() + 1;
		uint32 height = bitmap->Bounds().IntegerHeight() + 1;
		if (width == fWidth && height == fHeight) {
			// copy ourselves into BBitmap
			uint8* lineStart = (uint8*)bitmap->Bits();
			int32 bpr = bitmap->BytesPerRow();
			uint8* r = fRed;
			uint8* g = fGreen;
			uint8* b = fBlue;
			for (uint32 y = 0; y < Height(); y++) {
				int32* pixel = (int32*)lineStart;
				for (uint32 x = 0; x < Width(); x++) {
					uint8* c = (uint8*)pixel++;
					c[0] = *b++;
					c[1] = *g++;
					c[2] = *r++;
//					c[3] = 255;
				}
				lineStart += bpr;
			}
			success = true;
		}
	}
	return success;
}

// SetToBitmap
bool
RGB8Image::SetToBitmap(const BBitmap* bitmap)
{
	bool success = false;
	if (bitmap && bitmap->IsValid()) {
		if (bitmap->ColorSpace() == B_RGB32 || bitmap->ColorSpace() == B_RGBA32) {
			fWidth = (uint32)(bitmap->Bounds().right - bitmap->Bounds().left) + 1;
			fHeight = (uint32)(bitmap->Bounds().bottom - bitmap->Bounds().top) + 1;
			fRed = new uint8[CountPixels()];
			fGreen = new uint8[CountPixels()];
			fBlue = new uint8[CountPixels()];
			if (fRed && fGreen && fBlue) {
				// copy BBitmap into our data
				uint8* lineStart = (uint8*)bitmap->Bits();
				int32 bpr = bitmap->BytesPerRow();
				uint8* r = fRed;
				uint8* g = fGreen;
				uint8* b = fBlue;
				for (uint32 y = 0; y < Height(); y++) {
					int32* pixel = (int32*)lineStart;
					for (uint32 x = 0; x < Width(); x++) {
						uint8* c = (uint8*)pixel++;
						*b++ = c[0];
						*g++ = c[1];
						*r++ = c[2];
					}
					lineStart += bpr;
				}
				success = true;
			} else
				MakeEmpty();
		} else if (bitmap->ColorSpace() == B_RGB32_BIG || bitmap->ColorSpace() == B_RGBA32_BIG) {
			fWidth = (uint32)(bitmap->Bounds().right - bitmap->Bounds().left) + 1;
			fHeight = (uint32)(bitmap->Bounds().bottom - bitmap->Bounds().top) + 1;
			fRed = new uint8[CountPixels()];
			fGreen = new uint8[CountPixels()];
			fBlue = new uint8[CountPixels()];
			if (fRed && fGreen && fBlue) {
				// copy BBitmap into our data
				uint8* lineStart = (uint8*)bitmap->Bits();
				int32 bpr = bitmap->BytesPerRow();
				uint8* r = fRed;
				uint8* g = fGreen;
				uint8* b = fBlue;
				for (uint32 y = 0; y < Height(); y++) {
					int32* pixel = (int32*)lineStart;
					for (uint32 x = 0; x < Width(); x++) {
						uint8* c = (uint8*)pixel++;
						*r++ = c[1];
						*g++ = c[2];
						*b++ = c[3];
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

