// support.cpp

#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <BitmapStream.h>
#include <Entry.h>
#include <File.h>
#include <NodeInfo.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>

#include "support.h"

static const char* kDefaultTranslatorName = "TGA image";
static int32 imageFormat = 0;
static uint32 translator = 0;

// new_dmatrix
double**
new_dmatrix(uint32 entries, uint32 entrySize)
{
	double** matrix = NULL;
	if (entries > 0 && entrySize > 0) {
		matrix  = new double*[entries];
		for (uint32 i = 0; i < entries; i++)
			matrix[i] = new double[entrySize];
	}
	return matrix;
}

// clear_dmatrix
double**
clear_dmatrix(uint32 entries, uint32 entrySize)
{
	double** matrix = new_dmatrix(entries, entrySize);
	if (matrix) {
		for (uint32 i = 0; i < entries; i++)
			for (uint32 j = 0; i < entrySize; j++)
				matrix[i][j] = 0.0;
	}
	return matrix;
}

// free_dmatrix
void
free_dmatrix(double** matrix, uint32 entries)
{
	if (matrix) {
		for (uint32 i = 0; i < entries; i++)
			delete[] matrix[i];
		delete[] matrix;
	}
}

// new_matrix
float**
new_matrix(uint32 entries, uint32 entrySize)
{
	float** matrix = NULL;
	if (entries > 0 && entrySize > 0) {
		matrix = new float*[entries];
		float* b = new float[entries * entrySize];
		float** c = matrix;
		while (entries--) {
			*(c++) = b; 
			b += entrySize;
		}
	}
	return matrix;
}

// clear_matrix
float**
clear_matrix(uint32 entries, uint32 entrySize)
{
	float** matrix = new_matrix(entries, entrySize);
	if (matrix) {
		uint32 matrixSize = entries * entrySize;
		float* p = *matrix;
		do *(p++) = 0.0; while(--matrixSize);
	}
	return matrix;
}

// free_martrix
void
free_matrix(float** matrix)
{
	delete[] *matrix;
	delete[] matrix;
}

// copy
float*
copy(float* src, uint32 size)
{
	float* dst = NULL;
	if (src && size > 0) {
		dst = new float[size];
		memcpy((char*)dst, (char*)src, size * sizeof(float));
	}
	return dst;
}

// copy
int32*
copy(int32* src, uint32 size)
{
	int32* dst = NULL;
	if (src && size > 0) {
		dst = new int32[size];
		memcpy((char*)dst, (char*) src, size * sizeof(int32));
	}
	return dst;
}

// new_fmatrix
float**
new_fmatrix(uint32 entries, uint32 entrySize)
{
	float** matrix = NULL;
	if (entries > 0 && entrySize > 0) {
		matrix = new float*[entries];
		for (uint32 i = 0; i < entries; i++)
			matrix[i] = new float[entrySize];
	}
	return matrix;
}

// free_fmatrix
void
free_fmatrix(float** matrix, uint32 entries)
{
	if (matrix) {
		for (uint32 i = 0; i < entries; i++)
			delete[] matrix[i];
		delete[] matrix;
	}
}

// new_imatrix
int32**
new_imatrix(uint32 entries, uint32 entrySize)
{
	int32** matrix = NULL;
	if (entries > 0 && entrySize > 0) {
		matrix = new int32*[entries];
		for (uint32 i = 0; i < entries; i++)
			matrix[i] = new int32[entrySize];
	}
	return matrix;
}

// free_imatrix
void
free_imatrix(int32** matrix, uint32 entries)
{
	if (matrix) {
		for (uint32 i = 0; i < entries; i++)
			delete[] matrix[i];
		delete[] matrix;
	}
}

// complex2float
void
complex2float(complex *z, float *x, float *y, uint32 size)
{
	if (z && x && y) {
		for (uint32 i = 0; i < size; i++) {
			x[i] = z[i].real();
			y[i] = z[i].imag();
		}
	}
}

// min_max
void
min_max(float* data, float& min, float& max, uint32 size)
{
	min = FLT_MAX;
	max = -min;
	if (data) {
		float *p = data;
		while (size--) {
			if (*p < min )
				min = *p;
			if (*p > max )
				max = *p;
			p++;
		}
	}
}

// fmax
float
fmax(float* area, uint32 size)
{
	register float maximum = *(area++);
	--size;
	do {
		if (maximum < *area) maximum = *area;
	} while (area++, --size);

	return maximum;
}

// imax
int32
imax(int32* area, uint32 size)
{
	register int32 maximum = *(area++);
	--size;
	do {
		if (maximum < *area) maximum = *area;
	} while (area++, --size);

	return maximum;
}

// clear_float
float*
clear_float(uint32 size)
{
	float* output = NULL;
	if (size > 0) {
		output = new float[size];
		if (output) {
			float* hook = output;
			while (--size)
				*hook++ = 0.0;
		}
	}
	return output;
}

// set_file_type
status_t
set_file_type(BFile *file, int32 translator, uint32 type) 
{ 
	translation_format * formats; 
	int32 count;

	status_t err = BTranslatorRoster::Default()->GetOutputFormats(translator,
																  (const translation_format **) &formats,
																  &count);
	if (err < B_OK)
		return err;

	const char * mime = NULL; 
	for (int ix=0; ix<count; ix++) {
		if (formats[ix].type == type) {
			mime = formats[ix].MIME;
			break;
		}
	} 
	if (mime == NULL)
		// this should not happen, but being defensive might be prudent
		return B_ERROR; 

	// use BNodeInfo to set the file type
	BNodeInfo ninfo(file);
	return ninfo.SetType(mime);
} 

// set_translator
//static
bool
set_translator(const char* name)
{
	const char* translatorName = kDefaultTranslatorName;
	if (name)
		translatorName = name;

	BTranslatorRoster * use = BTranslatorRoster::Default();
	translator_id * ids = NULL;
	int32 count = 0;

	bool found = false;

	status_t err = use->GetAllTranslators(&ids, &count); 
	if (err >= B_OK) {
		for (int tix = 0; tix < count; tix++) { 
			const translation_format *formats = NULL; 
			int32 num_formats = 0; 
			bool ok = false; 
			err = use->GetInputFormats(ids[tix], &formats, &num_formats); 
			if (err >= B_OK) {
				for (int iix = 0; iix < num_formats; iix++) { 
					if (formats[iix].type == B_TRANSLATOR_BITMAP) { 
						ok = true; 
						break; 
					}
				}
			}
			if (!ok)
				continue; 
			err = use->GetOutputFormats(ids[tix], &formats, &num_formats); 
			if (err >= B_OK) {
				for (int32 oix = 0; oix < num_formats; oix++) {
		 			if (formats[oix].type != B_TRANSLATOR_BITMAP) {
	 					if (strcmp(formats[oix].name, translatorName) == 0) {
	 						found = true;
		 					imageFormat = formats[oix].type;
		 					translator = ids[tix];
		 				}
		 				break;
		 			}
				}
			}
		}
	}
	delete[] ids;
	return found;
}

// save_bitmap
void
save_bitmap(BBitmap* bitmap, const char* fileName)
{
	// construct file name
	BFile output(fileName, B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	status_t err = output.InitCheck();
	if (err >= B_OK) {
		BBitmapStream input(bitmap);
		err = BTranslatorRoster::Default()->Translate(&input, NULL, NULL, &output, imageFormat);
		if (err == B_OK) {
			err = set_file_type(&output, translator, imageFormat);
			entry_ref ref;
        	if (err != B_OK)
        		printf("error setting output filetype: %s\n", strerror(err));
        } else
        	printf("error writing output file \"%s\": %s\n", fileName, strerror(err));

        input.DetachBitmap(&bitmap);
        output.Unset();
    } else
		printf("error creating output file \"%s\": %s\n", fileName, strerror(err));
}


