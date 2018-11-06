// RLEBuffer.cpp

#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Message.h>
#include <TypeConstants.h>

#include "support.h"

#include "RLEBuffer.h"

// constructor
RLEScanline::RLEScanline()
	: fData(NULL),
	  fBytes(0),
	  fEncoded(true)
{
}

// constructor
RLEScanline::RLEScanline(BMessage* archive)
	: fData(NULL),
	  fBytes(0),
	  fEncoded(true)
{
	if (archive && archive->FindBool("encoded", &fEncoded) >= B_OK) {
		const void* data;
		ssize_t size;
		status_t status = archive->FindData("data", B_RAW_TYPE, &data, &size);
		if (status >= B_OK) {
			fData = (uint8*)malloc(size);
			if (fData) {
				memcpy(fData, data, size);
				fBytes = size;
			}
		}
	}
}

// constructor
RLEScanline::RLEScanline(uint8* bits, uint32 bytes)
	: fData(NULL),
	  fBytes(0),
	  fEncoded(true)
{
	SetTo(bits, bytes);
}

// private constructor
RLEScanline::RLEScanline(const RLEScanline& other)
	: fData((uint8*)malloc(other.fBytes)),
	  fBytes(other.fBytes),
	  fEncoded(other.fEncoded)
{
	memcpy(fData, other.fData, fBytes);
}

// destructor
RLEScanline::~RLEScanline()
{
	if (fData)
		free(fData);
}

// SetTo
status_t
RLEScanline::SetTo(uint8* bits, uint32 bytes)
{
	status_t status = B_BAD_VALUE;
	if (bits && bytes > 0) {
		if (fData)
			free(fData);
		fBytes = 0;
		fEncoded = true;
		// allocate worst case ammount of memory
		fData = (uint8*)malloc(bytes * 2);
		if (fData) {
			uint8* handle = fData;
			handle[0] = 1;		// count
			handle[1] = *bits;	// value
			fBytes = 2;
			bits++;
			for (uint32 i = 1; i < bytes; i++) {
				if (handle[1] != *bits || handle[0] == 255) {
					// write to data
					handle += 2;
					// keep track of how many bytes of fData we used
					fBytes += 2;
					// next value
					handle[0] = 1;
					handle[1] = *bits;
				} else {
					handle[0]++;
				}
				// check if the RLE compression is too inefficient
				if (fBytes > bytes) {
					fBytes = bytes;
					memcpy(fData, bits - i, bytes);
					fEncoded = false;
					break;
				}
				bits++;
			}
			// reallocate data to how much we actually need
			fData = (uint8*)realloc((void*)fData, fBytes);
			// final check
			if (!fData)
				status = B_NO_MEMORY;
			else
				status = B_OK;
		} else
			status = B_NO_MEMORY;
	}
	return status;
}

// Fill
// bits already points to the correct location
// skipBytes tells us how many bytes we should skip from *our* data
status_t
RLEScanline::Fill(uint8* bits, uint32 bytes, uint32 skipBytes)
{
	status_t status = B_NO_INIT;
	if (fData) {
		status = B_BAD_VALUE;
		if (bits && bytes > 0) {
			if (fEncoded) {
				uint8* handle = fData;
				// decompress to bits
				uint8 count = handle[0];
				uint8 value = handle[1];
				for (uint32 i = 0; i < bytes + skipBytes; i++) {
					if (i >= skipBytes) {
						*bits = value;
						bits++;
					}
					count--;
					if (count == 0 && i < bytes + skipBytes - 1) {
						handle += 2;
						count = handle[0];
						value = handle[1];
					}
				}
			} else {
				memcpy(bits, fData + skipBytes, bytes);
			}
			// done
			status = B_OK;
		}
	}
	return status;
}

// Archive
status_t
RLEScanline::Archive(BMessage* into) const
{
	status_t status = fData ? B_BAD_VALUE : B_NO_INIT;
	if (into && fData) {
		status = into->AddData("data", B_RAW_TYPE, fData, fBytes);
		if (status >= B_OK)
			status = into->AddBool("encoded", fEncoded);
	}
	return status;
}

// constructor
RLEBuffer::RLEBuffer()
	: fXOffset(0),
	  fYOffset(0),
	  fLines(NULL),
	  fCount(0),
	  fBytes(0)
{
}

// constructor
RLEBuffer::RLEBuffer(BMessage* archive)
	: fXOffset(0),
	  fYOffset(0),
	  fLines(NULL),
	  fCount(0),
	  fBytes(0)
{
	if (archive && archive->FindInt32("bytes", (int32*)&fBytes) >= B_OK
		&& archive->FindInt32("x offset", &fXOffset) >= B_OK
		&& archive->FindInt32("y offset", &fYOffset) >= B_OK) {
		BMessage lineArchive;
		type_code type;
		if (archive->GetInfo("line", &type, (int32*)&fCount) >= B_OK
			&& type == B_MESSAGE_TYPE) {
			fLines = new RLEScanline*[fCount];
			for (uint32 i = 0; i < fCount; i++) {
				if (archive->FindMessage("line", i, &lineArchive) >= B_OK)
					fLines[i] = new RLEScanline(&lineArchive);
				else {
					// clean up
					for (int32 j = i - 1; j >= 0; j--)
						delete fLines[j];
					delete[] fLines;
					fLines = NULL;
					break;
				}
			}
		}
	}
}

// constructor
RLEBuffer::RLEBuffer(BBitmap* source, BRect frame)
	: fXOffset(0),
	  fYOffset(0),
	  fLines(NULL),
	  fCount(0),
	  fBytes(0)
{
	SetTo(source, frame);
}

// copy constructor
RLEBuffer::RLEBuffer(const RLEBuffer& other)
	: fXOffset(other.fXOffset),
	  fYOffset(other.fXOffset),
	  fLines(new RLEScanline*[other.fCount]),
	  fCount(other.fCount),
	  fBytes(other.fBytes)
{
	for (uint32 i = 0; i < fCount; i++) {
		fLines[i] = new RLEScanline(*other.fLines[i]);
	}
}

// destructor
RLEBuffer::~RLEBuffer()
{
	_MakeEmpty();
}

// SetTo
status_t
RLEBuffer::SetTo(BBitmap* source, BRect frame)
{
	status_t status = B_BAD_VALUE;
	// the frame passed is the original stroke frame
	BRect original = frame;
	if (source && source->IsValid() && source->ColorSpace() == B_GRAY8
		&& frame.IsValid() && source->Bounds().Intersects(frame)) {
		// make sure all of frame lies within bitmap 
		frame = frame & source->Bounds();

		_MakeEmpty();

		int32 left, top, right, bottom;
		rect_to_int(frame, left, top, right, bottom);
		fBytes = (uint32)(right - left) + 1;
		fCount = (uint32)(bottom - top) + 1;
		uint8* bits = (uint8*)source->Bits();
		uint32 bpr = source->BytesPerRow();
		bits += left + top * bpr;
		// allocate the scanlines
		fLines = new RLEScanline*[fCount];
		for (uint32 i = 0; i < fCount; i++) {
			fLines[i] = new RLEScanline(bits, fBytes);
			bits += bpr;
		}
		// remember the offset of the buffer
		fXOffset = left - (int32)floorf(original.left);
		fYOffset = top - (int32)floorf(original.top);
		status = B_OK;
	}
	return status;
}

// Fill
// the RLEBuffer itself knows nothing about its original placement in a bitmap
// this is what xOffset and yOffset is for
// constrainFrame defines any additional clipping
status_t
RLEBuffer::Fill(BBitmap* dest, int32 xOffset, int32 yOffset, BRect constrainFrame)
{
	status_t status = B_NO_INIT;
	if (fLines) {
		status = B_BAD_VALUE;
		if (dest && dest->IsValid() && dest->ColorSpace() == B_GRAY8
			&& constrainFrame.IsValid() && dest->Bounds().Intersects(constrainFrame)) {
			// compensate for our own offset
			xOffset += fXOffset;
			yOffset += fYOffset;
			// frame is the entire area that this buffer could possibly fill
			BRect frame(xOffset, yOffset, xOffset + fBytes - 1, yOffset + fCount - 1);
			// make sure all of constrainFrame lies within bitmap
			constrainFrame = constrainFrame & dest->Bounds();
			if (constrainFrame.Intersects(frame)) {
				// make sure constrainFrame is clipped to the area we touch
				constrainFrame = constrainFrame & frame;
	
				int32 x = (int32)floorf(constrainFrame.left);
				int32 y = (int32)floorf(constrainFrame.top);
	
				uint8* bits = (uint8*)dest->Bits();
				uint32 bpr = dest->BytesPerRow();
				bits += x + y * bpr;
	
				uint32 bytes = constrainFrame.IntegerWidth() + 1;
				uint32 lines = constrainFrame.IntegerHeight() + 1;
				uint32 skipBytes = 0;
				uint32 skipLines = 0;
				if (x > xOffset)
					skipBytes = x - xOffset;
				if (y > yOffset)
					skipLines = y - yOffset;

				for (uint32 i = 0; i < lines; i++) {
					fLines[i + skipLines]->Fill(bits, bytes, skipBytes);
					bits += bpr;
				}
				status = B_OK;
			}
		}
	}
//printf("RLEBuffer::Fill(): %s\n", strerror(status));
	return status;
}

// Size
uint32
RLEBuffer::Size() const
{
	uint32 size = 0;
	if (fLines) {
		for (uint32 i = 0; i < fCount; i++)
			size += fLines[i]->Size();
	}
	return size;
}

// Archive
status_t
RLEBuffer::Archive(BMessage* into) const
{
	status_t status = fLines ? B_BAD_VALUE : B_NO_INIT;
	if (into && fLines) {
		for (uint32 i = 0; i < fCount; i++) {
			BMessage lineArchive;
			status = fLines[i]->Archive(&lineArchive);
			if (status >= B_OK)
				status = into->AddMessage("line", &lineArchive);
			if (status < B_OK)
				break;
		}
		if (status >= B_OK) {
			status = into->AddInt32("bytes", fBytes);
			if (status >= B_OK) {
				status = into->AddInt32("x offset", fXOffset);
				if (status >= B_OK) {
					status = into->AddInt32("y offset", fYOffset);
				}
			}
		}
	}
	return status;
}

// Bounds
BRect
RLEBuffer::Bounds() const
{
	return BRect(fXOffset, fYOffset, fXOffset + fBytes - 1, fYOffset + fCount - 1);
}

// _MakeEmpty
void
RLEBuffer::_MakeEmpty()
{
	for (uint32 i = 0; i < fCount; i++) {
		delete fLines[i];
	}
	delete[] fLines;
	fLines = NULL;
}
