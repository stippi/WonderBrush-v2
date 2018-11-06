// EdgesFilter.cpp

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>

#include "EdgesFilter.h"

// constructor
EdgesFilter::EdgesFilter()
{
}

// destructor
EdgesFilter::~EdgesFilter()
{
}

// Run
status_t
EdgesFilter::Run(const BBitmap* input, const BBitmap* output) const
{
	status_t status = B_BAD_VALUE;
	if (input && input->IsValid() && output && output->IsValid()) {
		if ((input->ColorSpace() == B_RGB32
			|| input->ColorSpace() == B_RGBA32)
			&& (output->ColorSpace() == B_RGB32
			|| output->ColorSpace() == B_RGBA32)) {
			status = B_MISMATCHED_VALUES;
			uint32 inWidth = input->Bounds().IntegerWidth() + 1;
			uint32 inHeight = input->Bounds().IntegerHeight() + 1;
			uint32 outWidth = output->Bounds().IntegerWidth() + 1;
			uint32 outHeight = output->Bounds().IntegerHeight() + 1;
			if (inWidth == outWidth && inHeight == outHeight) {
	
				uint8* src = (uint8*)input->Bits();
				uint8* dst = (uint8*)output->Bits();
				uint32 dstBPR = output->BytesPerRow();
		
				memset(dst, 0, output->BitsLength());
		
				uint32 left, right, top, bottom, edge;
				dst += dstBPR + 4;
				// TODO: use dstBPR instead of width * 4!
				for (uint32 y = 1; y < inHeight - 1; y++) {
					for (uint32 x = 1; x < inWidth - 1; x++) {
						uint32 pos = (y * inWidth + x - 1) * 4;
						left = (src[pos] + src[pos + 1] * 2 + src[pos + 2]) >> 2;
						pos += 8;
						right = (src[pos] + src[pos + 1] * 2 + src[pos + 2]) >> 2;
						pos = ((y - 1) * inWidth + x) * 4;
						top = (src[pos] + src[pos + 1] * 2 + src[pos + 2]) >> 2;
						pos += (inWidth * 4 * 2);
						bottom = (src[pos] + src[pos + 1] * 2 + src[pos + 2]) >> 2;
						edge = (abs(left - right) + abs(top - bottom)) >> 1;
						
						dst[0] = edge;
						dst[1] = edge;
						dst[2] = edge;
						dst[3] = 255;
						dst += 4;
					}
					dst += 8;
				}
			}
		}
	}
	return status;
}

