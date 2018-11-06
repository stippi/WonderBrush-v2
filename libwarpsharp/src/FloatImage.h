// FloatImage.h

#ifndef FLOAT_IMAGE_H
#define FLOAT_IMAGE_H

#include <GraphicsDefs.h>

#include "warpsharp.h"

class BBitmap;
class RGB8Image;

class IMPEXPLIBWARPSHARP FloatImage {
 public:
							FloatImage();
							FloatImage(FloatImage& copy);
							FloatImage(uint32 width, uint32 height);
							FloatImage(BBitmap* bitmap);
							FloatImage(RGB8Image* image,
									   bool greenOnly = false);
							FloatImage(const char* fileName);

	virtual					~FloatImage();

			bool			IsValid();

	inline	uint32			Width() const
								{ return fWidth; }
	inline	uint32			Height() const
								{ return fHeight; }
	inline	uint32			CountPixels() const
								{ return Height() * Width(); }

			void			MakeEmpty();

							// pixel access
			void			SetAllPixels(float value);
			void			SetPixel(uint32 x, uint32 y, float value);
			void			SetPixel(uint32 index, float value);

							// float access
	inline	float*			Pixel() const
								{ return fPixels; }
	inline	float			Pixel(uint32 x, uint32 y) const
								{ return fPixels[x + y * Width()]; }
	inline	float			Pixel(uint32 pixelIndex) const
								{ return fPixels[pixelIndex]; }
			float			PixelSave(uint32 pixelIndex) const;

							// conversion from/into BBitmap
			BBitmap*		GetBitmap(bool scale = false);
			bool			SetToBitmap(BBitmap* bitmap);	

 private:
			uint32			fWidth;
			uint32			fHeight;

			float*			fPixels;
};




#endif // FLOAT_IMAGE_H
