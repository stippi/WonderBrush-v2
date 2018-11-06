// BlurFilter.h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>

#include "BlurFilter.h"

// constructor
BlurFilter::BlurFilter()
{
}

// destructor
BlurFilter::~BlurFilter()
{
}

// Run
status_t
BlurFilter::Run(const BBitmap* input, const BBitmap* output) const
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
//				uint32 dstBPR = output->BytesPerRow();
	
				uint8* rows[] = { NULL, NULL, NULL };
				for (uint32 x = 0; x < 3; x++) {
					rows[x] = (uint8*)malloc((inWidth + 2) * 4);
					if (rows[x] == NULL) {
						printf("BlurFilter: could not allocate rows\n");
						if (rows[0] != NULL)
							free(rows[0]);
						if (rows[1] != NULL)
							free(rows[1]);
						return B_NO_MEMORY;
					}
					if (x == 0)
						memset(rows[x], 0, (inWidth + 2) * 4);
				}
				
				int temp;
				for (uint32 y = 0; y < inHeight; y++) {
					if (y != 0) memcpy(rows[0] + 4, src + ((y - 1) * inWidth * 4), inWidth * 4);
					memcpy(rows[1] + 4, src + (y * inWidth * 4), inWidth * 4);
					if (y != inHeight - 1) memcpy(rows[2] + 4, src + ((y + 1) * inWidth * 4), inWidth * 4);
					else memset(rows[2], 0, (inWidth + 2) * 4);
					
					for (uint32 x = 0; x < inWidth; x++) {
						temp = rows[0][x * 4] + rows[0][(x + 1) * 4] + rows[0][(x + 2) * 4] +
							rows[1][x * 4] + rows[1][(x + 1) * 4] + rows[1][(x + 2) * 4] +
							rows[2][x * 4] + rows[2][(x + 1) * 4] + rows[2][(x + 2) * 4];
						temp /= 9;
						dst[0] = temp;
						temp = rows[0][x * 4 + 1] + rows[0][(x + 1) * 4 + 1] + rows[0][(x + 2) * 4 + 1] +
							rows[1][x * 4 + 1] + rows[1][(x + 1) * 4 + 1] + rows[1][(x + 2) * 4 + 1] +
							rows[2][x * 4 + 1] + rows[2][(x + 1) * 4 + 1] + rows[2][(x + 2) * 4 + 1];
						temp /= 9;
						dst[1] = temp;
						temp = rows[0][x * 4 + 2] + rows[0][(x + 1) * 4 + 2] + rows[0][(x + 2) * 4 + 2] +
							rows[1][x * 4 + 2] + rows[1][(x + 1) * 4 + 2] + rows[1][(x + 2) * 4 + 2] +
							rows[2][x * 4 + 2] + rows[2][(x + 1) * 4 + 2] + rows[2][(x + 2) * 4 + 2];
						temp /= 9;
						dst[2] = temp;
						dst[3] = 255;
						dst += 4;
					}
				}
				
				free(rows[0]);
				free(rows[1]);
				free(rows[2]);
			}
		}
	}
	return status;
}

