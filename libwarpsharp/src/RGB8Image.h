// RGB8Image.h

#ifndef RGB_8_IMAGE_H
#define RGB_8_IMAGE_H

#include <GraphicsDefs.h>

#include "warpsharp.h"

class BBitmap;

class IMPEXPLIBWARPSHARP RGB8Image {
 public:
							RGB8Image();
							RGB8Image(const RGB8Image& copy);
							RGB8Image(uint32 width, uint32 height,
									  bool clearToBlack = false);
							RGB8Image(const BBitmap* bitmap);
							RGB8Image(const char* fileName);

	virtual					~RGB8Image();

			bool			IsValid() const;

	inline	uint32			Width() const
								{ return fWidth; }
	inline	uint32			Height() const
								{ return fHeight; }
	inline	uint32			CountPixels() const
								{ return Height() * Width(); }

			void			MakeEmpty();
			bool			SetTo(uint32 width, uint32 height,
								  bool clearToBlack = false);

							// pixel access
			void			SetAllPixels(rgb_color color);
			void			SetAllPixels(uint8 red, uint8 green, uint8 blue);
			rgb_color		ColorAt(uint32 x, uint32 y) const;
			void			SetPixel(uint32 x, uint32 y, rgb_color color);

							// byte access
	inline	uint8*			Red() const
								{ return fRed; }
	inline	uint8			Red(uint32 x, uint32 y) const
								{ if (IsValid() &&
									  x < Width() && y < Height())
									  return fRed[x + y * Width()];
								   return 0; }
	inline	uint8			Red(uint32 pixelIndex) const
								{ if (pixelIndex < CountPixels())
									  return fRed[pixelIndex];
								  return 0; }

	inline	uint8*			Green() const
								{ return fGreen; }
	inline	uint8			Green(uint32 x, uint32 y) const
								{ if (IsValid() &&
									  x < Width() && y < Height())
									  return fGreen[x + y * Width()];
								  return 0; }
	inline	uint8			Green(uint32 pixelIndex) const
								{ if (pixelIndex < CountPixels())
									  return fGreen[pixelIndex];
								  return 0; }

	inline	uint8*			Blue() const
								{ return fBlue; }
	inline	uint8			Blue(uint32 x, uint32 y) const
								{ if (IsValid() &&
									  x < Width() && y < Height())
									  return fBlue[x + y * Width()];
								  return 0; }
	inline	uint8			Blue(uint32 pixelIndex) const
								{ if (pixelIndex < CountPixels())
									  return fBlue[pixelIndex];
								  return 0; }

							// conversion from/into BBitmap
			BBitmap*		GetBitmap() const;
			bool			CopyIntoBitmap(const BBitmap* bitmap) const;
			bool			SetToBitmap(const BBitmap* bitmap);

 private:
			uint32			fWidth;
			uint32			fHeight;

			uint8*			fRed;
			uint8*			fGreen;
			uint8*			fBlue;
};

#endif // RGB_8_IMAGE_H
