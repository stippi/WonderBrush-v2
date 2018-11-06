// RLEBuffer.h

#ifndef RLE_BUFFER_H
#define RLE_BUFFER_H

#include <Rect.h>

class BBitmap;
class BMessage;

class RLEScanline {
 public:
								RLEScanline();
								RLEScanline(BMessage* archive);
								RLEScanline(uint8* bits,
										 	uint32 bytes);
								RLEScanline(const RLEScanline& other);
	virtual						~RLEScanline();

			status_t			SetTo(uint8* bits,
									  uint32 bytes);
			status_t			Fill(uint8* bits,
									 uint32 bytes,
									 uint32 skipBytes);

			uint32				Size() const
									{ return fBytes; }

			status_t			Archive(BMessage* into) const;

 private:
			uint8*				fData;
			uint32				fBytes;
			bool				fEncoded;
};


class RLEBuffer {
 public:
								RLEBuffer();
								RLEBuffer(BMessage* archive);
								RLEBuffer(BBitmap* source,
										  BRect frame);
								RLEBuffer(const RLEBuffer& other);

	virtual						~RLEBuffer();

			status_t			SetTo(BBitmap* source,
									  BRect frame);
			status_t			Fill(BBitmap* dest,
									 int32 xOffset, int32 yOffset,
									 BRect constrainFrame);

			uint32				Size() const;

			status_t			Archive(BMessage* into) const;
			BRect				Bounds() const;
			bool				IsValid() const
									{ return fCount > 0 && fBytes > 0; }

 private:
			void				_MakeEmpty();

			int32				fXOffset;
			int32				fYOffset;
			RLEScanline**		fLines;
			uint32				fCount;
			uint32				fBytes;
};

#endif // RLE_BUFFER_H
