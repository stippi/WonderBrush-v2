// Gray8Image.cpp

#include <Bitmap.h>
#include <TranslationUtils.h>

#include "Gray8Image.h"

// constructor
Gray8Image::Gray8Image()
	: fWidth(0),
	  fHeight(0),
	  fPixels(NULL)
{
}

// copy constructor
Gray8Image::Gray8Image(Gray8Image& copy)
	: fWidth(0),
	  fHeight(0),
	  fPixels(NULL)
{
	if (copy.IsValid()) {
		fWidth = copy.Width();
		fHeight = copy.Height();
		fPixels = new uint8[CountPixels()];
		if (fPixels) {
			for (uint32 i = 0; i < CountPixels(); i++) {
				fPixels[i] = copy.Pixel(i);
			}
		} else
			MakeEmpty();
	} else
		MakeEmpty();
}
// contructor
Gray8Image::Gray8Image(uint32 width, uint32 height)
	: fWidth(width),
	  fHeight(height),
	  fPixels(new uint8[CountPixels()])
{
	if (!IsValid())
		MakeEmpty();
}

// BBitmap constructor
Gray8Image::Gray8Image(BBitmap* bitmap)
	: fWidth(0),
	  fHeight(0),
	  fPixels(NULL)
{
	SetToBitmap(bitmap);
}

// file constructor
Gray8Image::Gray8Image(const char *fileName)
	: fWidth(0),
	  fHeight(0),
	  fPixels(NULL)
{
	SetToBitmap(BTranslationUtils::GetBitmap(fileName));
}

// destructor
Gray8Image::~Gray8Image()
{
  MakeEmpty();
}

// IsValid
bool
Gray8Image::IsValid()
{
	if (Width() > 0 && Height() > 0 && fPixels)
		return true;
	return false;
}

// MakeEmpty
void
Gray8Image::MakeEmpty()
{
	fWidth = 0;
	fHeight = 0;
	delete[] fPixels;
	fPixels = NULL;
}

// SetAllPixels
void
Gray8Image::SetAllPixels(uint8 value)
{
	if (IsValid()) {
		for (uint32 i = 0; i < CountPixels(); i++)
			fPixels[i] = value;
	}
}

// SetPixel
void
Gray8Image::SetPixel(uint32 x, uint32 y, uint8 value)
{
	if (IsValid() && x < Width() && y < Height()) {
		uint32 index = x + y * Width();
		fPixels[index] = value;
	}
}

// GetBitmap
BBitmap*
Gray8Image::GetBitmap()
{
	BBitmap* bitmap = NULL;
	if (IsValid()) {
		bitmap = new BBitmap(BRect(0.0, 0.0, Width() - 1, Height() - 1), B_RGB32);
		if (bitmap && bitmap->IsValid()) {
			// copy ourselves into BBitmap
			uint8* lineStart = (uint8*)bitmap->Bits();
			int32 bpr = bitmap->BytesPerRow();
			uint8* p = fPixels;
			for (uint32 y = 0; y < Height(); y++) {
				int32* pixel = (int32*)lineStart;
				for (uint32 x = 0; x < Width(); x++) {
					uint8* c = (uint8*)pixel++;
					uint8 color = (uint8)((*p++) * 255);
					c[0] = color;
					c[1] = color;
					c[2] = color;
					c[3] = 255;
				}
				lineStart += bpr;
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
Gray8Image::SetToBitmap(BBitmap* bitmap)
{
	bool success = false;
	if (bitmap && bitmap->IsValid()) {
		if (bitmap->ColorSpace() == B_RGB32 || bitmap->ColorSpace() == B_RGBA32) {
			fWidth = (uint32)(bitmap->Bounds().right - bitmap->Bounds().left) + 1;
			fHeight = (uint32)(bitmap->Bounds().bottom - bitmap->Bounds().top) + 1;
			fPixels = new uint8[CountPixels()];
			if (fPixels) {
				// copy BBitmap into cimage
				uint8* lineStart = (uint8*)bitmap->Bits();
				int32 bpr = bitmap->BytesPerRow();
				uint8* p = fPixels;
				for (uint32 y = 0; y < Height(); y++) {
					int32* pixel = (int32*)lineStart;
					for (uint32 x = 0; x < Width(); x++) {
						uint8* c = (uint8*)pixel++;
						*p++ = (uint8)((c[0] + c[1] + c[2]) / 765.0);
					}
					lineStart += bpr;
				}
				success = true;
			} else
				MakeEmpty();
		} else if (bitmap->ColorSpace() == B_RGB32_BIG || bitmap->ColorSpace() == B_RGBA32_BIG) {
			fWidth = (uint32)(bitmap->Bounds().right - bitmap->Bounds().left) + 1;
			fHeight = (uint32)(bitmap->Bounds().bottom - bitmap->Bounds().top) + 1;
			fPixels = new uint8[CountPixels()];
			if (fPixels) {
				// copy BBitmap into cimage
				uint8* lineStart = (uint8*)bitmap->Bits();
				int32 bpr = bitmap->BytesPerRow();
				uint8* p = fPixels;
				for (uint32 y = 0; y < Height(); y++) {
					int32* pixel = (int32*)lineStart;
					for (uint32 x = 0; x < Width(); x++) {
						uint8* c = (uint8*)pixel++;
						*p++ = (uint8)((c[1] + c[2] + c[3]) / 765.0);
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

