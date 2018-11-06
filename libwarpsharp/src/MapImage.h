// MapImage.h

#ifndef MAP_IMAGE_H
#define MAP_IMAGE_H

#include <SupportDefs.h>

#include "complex.h"
#include "warpsharp.h"

class FloatImage;

class IMPEXPLIBWARPSHARP MapImage {
 public:
	enum {
		DIRECTION_FORWARD	= 0,
		DIRECTION_BACKWARD	= 1,
	};
								MapImage();
								MapImage(uint32 width,
										 uint32 height,
										 complex* vectors);
								MapImage(uint32 width,
										 uint32 height,
										 double* r,
										 double* im); 
								MapImage(uint32 width,
										 uint32 height,
										 float* r,
										 float* im); 
								MapImage(const MapImage&);
								MapImage(const char* filename);
	virtual						~MapImage();

			bool				IsValid() const;

	inline	complex*			Vector() const
									{ return fVector; }
	inline	uint32				Width() const
									{ return fWidth; }
	inline	uint32				Height() const
									{ return fHeight; }
	inline	uint32				CountVectors() const
									{ return Width() * Height(); }


			MapImage&			operator=(const MapImage& copy);
			complex&			operator()(uint32 x, uint32 y) const
									{ return fVector[x + y * fWidth]; }
			complex				operator()(const complex& z);
			FloatImage*			GetFloatImage(bool imaginary) const;
			FloatImage*			ABS() const;
			bool				SaveAs(const char* filename,
									   bool overwrite = false) const;
			void				Scale(int32 factor, uint32 direction);
			void				Movement();

 private:
			uint32				fWidth;
			uint32				fHeight;
			complex*			fVector;
};

#endif // MAP_IMAGE_H





