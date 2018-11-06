// Gray8Image.h

#ifndef GRAY_8_IMAGE_H
#define GRAY_8_IMAGE_H

#include <GraphicsDefs.h>

#include "warpsharp.h"

class BBitmap;

class IMPEXPLIBWARPSHARP Gray8Image {
 public:
							Gray8Image();
							Gray8Image(Gray8Image& copy);
							Gray8Image(uint32 width, uint32 height);
							Gray8Image(BBitmap* bitmap);
							Gray8Image(const char* fileName);

	virtual					~Gray8Image();

			bool			IsValid();

	inline	uint32			Width() const
								{ return fWidth; }
	inline	uint32			Height() const
								{ return fHeight; }
	inline	uint32			CountPixels() const
								{ return Height() * Width(); }

			void			MakeEmpty();

							// pixel access
			void			SetAllPixels(uint8 value);
			void			SetPixel(uint32 x, uint32 y, uint8 value);

							// byte access
	inline	uint8*			Pixel() const
								{ return fPixels; }
	inline	uint8			Pixel(uint32 x, uint32 y) const
								{ return fPixels[x + y * Width()]; }
	inline	uint8			Pixel(uint32 pixelIndex) const
								{ return fPixels[pixelIndex]; }

							// conversion from/into BBitmap
			BBitmap*		GetBitmap();
			bool			SetToBitmap(BBitmap* bitmap);	

 private:
			uint32			fWidth;
			uint32			fHeight;

			uint8*			fPixels;
};




#endif // GRAY_8_IMAGE_H
