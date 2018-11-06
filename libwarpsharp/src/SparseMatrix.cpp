// SparseMatrix.cpp

#include <stdio.h>
#include <string.h>

#include "support.h"

#include "FloatImage.h"
#include "RGB8Image.h"

#include "SparseMatrix.h"

const uint32 TRESHOLD = 0;

// constructor
SparseMatrix::SparseMatrix()
	: fOrder(0), 
	  fEntriesPerRow(NULL),
	  fEntryCount(0),
	  fEntries(NULL),
	  fValues(NULL)
{
}

// copy constructor
SparseMatrix::SparseMatrix(const SparseMatrix& copy)
	: fOrder(0), 
	  fEntriesPerRow(NULL),
	  fEntryCount(0),
	  fEntries(NULL),
	  fValues(NULL)
{
	if (copy.IsValid()) {
		fOrder = copy.Order();
		fEntriesPerRow = new uint32[fOrder];
		fEntryCount = copy.CountEntries();
		fEntries = new int32[fEntryCount];
		fValues = new float[fEntryCount];
		for (uint32 i = 0; i < fOrder; i++)
			fEntriesPerRow[i] = copy.EntriesPerRow(i);
		for (uint32 i = 0; i < fEntryCount; i++) {
			fEntries[i] = copy.EntryAt(i);
			fValues[i] = copy.ValueAt(i);
		}
	}
}

// constructor
SparseMatrix::SparseMatrix(uint32 order, uint32 entryCount,
						   uint32* entriesPerRow, int32* entries,
						   float* values)
	: fOrder(0), 
	  fEntriesPerRow(NULL),
	  fEntryCount(0),
	  fEntries(NULL),
	  fValues(NULL)
{
	if (order > 0 && entriesPerRow && entries && values) {
		fOrder = order;
		fEntriesPerRow = new uint32[fOrder];
		fEntryCount = entryCount;
		fEntries = new int32[fEntryCount];
		fValues = new float[fEntryCount];
		for(uint32 i = 0; i < fOrder; i++)
			fEntriesPerRow[i] = entriesPerRow[i];
		for (uint32 i = 0; i < fEntryCount; i++) {
			fEntries[i] = entries[i];
			fValues[i] = values[i];
		}
	}
}

// constructor
SparseMatrix::SparseMatrix(uint32* entriesPerRow, int32* entries,
						   float* values, uint32 order,
						   uint32 entryCount)
	: fOrder(order),
	  fEntriesPerRow(entriesPerRow),
	  fEntryCount(entryCount),
	  fEntries(entries),
	  fValues(values)
{
}

// destructor
SparseMatrix::~SparseMatrix()
{
	delete[] fEntriesPerRow;
	delete[] fEntries;
	delete[] fValues;
}

// IsValid
bool
SparseMatrix::IsValid() const
{
	if (fOrder > 0 && fEntryCount > 0
		&& fEntriesPerRow && fEntries && fValues)
		return true;
	return false;
}

// =
void
SparseMatrix::operator=(const SparseMatrix& copy)
{
	if (this != &copy && copy.IsValid()) {
		fOrder = copy.Order();
		fEntryCount = copy.CountEntries();
		delete[] fEntriesPerRow;
		delete[] fEntries;
		delete[] fValues;
		fEntriesPerRow = new uint32[fOrder];
		fEntries = new int32[fEntryCount];
		fValues = new float[fEntryCount];
		for(uint32 i = 0; i < fOrder; i++)
			fEntriesPerRow[i] = copy.EntriesPerRow(i);
		for(uint32 i = 0; i < fEntryCount; i++) {      
			fEntries[i] = copy.EntryAt(i);
			fValues[i] = copy.ValueAt(i);
		}
	}
}

// ()
float
SparseMatrix::operator()(uint32 row, uint32 column)  
{
	if (row < fOrder && column < fOrder) {
		uint32 start = 0;
		for (uint32 i = 0; i < row; i++)
			start += fEntriesPerRow[i];
		int32* entry = fEntries + start;
		float* value = fValues + start;
		for (start = 0; start < fEntriesPerRow[row]; start++) 
			if ((uint32)*(entry + start) == column)
				return *(value + start);
	}
	return 0.0;
}

// *
float*
SparseMatrix::operator*(float* x) const
{
	float* output = NULL;
	if (x) {
		output = new float[fOrder];
		if (output) {
			memset(output, 0, sizeof(float) * fOrder);
			int32* entry = fEntries;
			float* value = fValues;
			for (uint32 i = 0; i < fOrder; i++)
				for (uint32 j = 0; j < fEntriesPerRow[i]; j++)
					output[i] += (*value++) * x[*entry++];
		}
	}
	return output;
}

// GetFloatImage
FloatImage*
SparseMatrix::GetFloatImage() const
{
	FloatImage* result = NULL;
	if (IsValid()) {
		result = new FloatImage(fOrder, fOrder);
		if (result && result->IsValid()) {
			float* p = result->Pixel();
			int32* entry = fEntries;
			float* value = fValues;
			for (uint32 j = 0; j < fOrder; j++)
				for (uint32 i = 0; i < fEntriesPerRow[j]; i++)
					p[*(entry++) + j * fOrder] = *value++;
		} else {
			delete result;
			result = NULL;
		}
	}
	return result;
}


// ---------------------------------------------
// helper functions for sharpening
// ---------------------------------------------

// s_sparse_matrix_n_entry
uint32*
s_sparse_matrix_n_entry(uint32 dx, uint32 dy)
{
	uint32* output = new uint32[dx * dy];
/*	if (output) {
		for (uint32 j = 0; j < dy; j++)
			for (uint32 i = 0; i < dx; i++)  
				if ((i == 0) || (i == dx - 1)
					|| (j == 0) || (j == dy - 1))
					output[i + j * dx] = 1;
				else
					output[i + j * dx] = 5;
	}*/
	// unrolled version:
	if (output) {
		uint32 ix = dx - 1;
		uint32 iy = dy - 1;

		for (uint32 i = 0; i < dx; i++)
			output[i] = 1;
		for (uint32 i = 0; i < dx; i++)
			output[i + iy * dx] = 1;
		for (uint32 j = 1; j < iy; j++) {
			output[j * dx] = 1;
			output[ix + j * dx] = 1;
		}

		for (uint32 j = 1; j < iy; j++)
			for (uint32 i = 1; i < ix; i++)
				output[i + j * dx] = 5;
	}
	return output;
}

// s_sparse_martix_entries
int32*
s_sparse_martix_entries(uint32 dx, uint32 dy)
{
	int32* entry = new int32[4 * (dx - 2) * (dy - 2) + dx * dy];
	int32* entryHook = entry;
	if (entry) {
		for (uint32 j = 0; j < dy; j++)    
			for (uint32 i = 0; i < dx; i++) {  
				*entryHook++ = i + j * dx;
				if ((i != 0) && (i != dx - 1) && (j != 0) && (j != dy - 1)) {
					*entryHook++ = i + 1 + j * dx;
					*entryHook++ = i - 1 + j * dx;
					*entryHook++ = i + (j + 1) * dx;
					*entryHook++ = i + (j - 1) * dx;
				}
			}
	}
	return entry;
}

// s_sparse_data
float *
s_sparse_data(FloatImage* data)
{
	float* values = NULL;
	if (data && data->IsValid()) {
		uint32 dx = data->Width();
		uint32 dy = data->Height();
		uint32 entryCount = 4 * (dx - 2) * (dy - 2) + dx * dy;
		float* g = data->Pixel();
		values = new float[entryCount];
		if (values) {
			float* valuesHook = values;
			for (uint32 j = 0; j < dy; j++)
				for (uint32 i = 0; i < dx; i++) {
					if ((i == 0) || (i == dx - 1) || (j == 0) || (j == dy - 1))
						*valuesHook++ = 1;
					else {
						*valuesHook++ = -4 * g[i + j * dx];
						*valuesHook++ = g[i + j * dx]
										+ (g[i + 1 + j * dx]
										- g[i - 1 + j * dx]) / 4;
						*valuesHook++ = g[i + j * dx]
										+ (g[i - 1 + j * dx]
										   - g[i + 1 + j * dx]) / 4;
						*valuesHook++ = g[i + j * dx]
										+ (g[i + (j + 1) * dx]
										   - g[i + (j - 1) * dx]) / 4;
						*valuesHook++ = g[i + j * dx]
										+ (g[i + (j - 1) * dx]
										   - g[i + (j + 1) * dx]) / 4;
					}
				}
		}
	}
	return values;
}

// sparse_data_sharp
//
// this option is used when the clamping component and the squeezing
// compomnent are the same.
float*
sparse_data_sharp(FloatImage* data, float mu, uint32 direction)
{
	float* values = NULL;
	if (data && data->IsValid()) {
		uint32 dx = data->Width();
		uint32 dy = data->Height();
		uint32 entryCount = 4 * (dx - 2) * (dy - 2) + dx * dy;
		float* g = data->Pixel();
		values = new float[entryCount];
		if (values) {
			float* valuesHook = values;
			if (direction == BACKWARD) {
				for (uint32 j = 0; j < dy; j++)
					for (uint32 i = 0; i < dx; i++) {
						if ((i == 0) || (i == dx - 1)
							|| (j == 0) || (j == dy - 1))
							*valuesHook++ = 1;
						else {
							*valuesHook++ = 4 * mu * g[i + j * dx]
											+ 1 / g[(i + 1) + j * dx]
											+ 1 / g[(i - 1) + j * dx]
											+ 1 / g[i + (j + 1) * dx]
											+ 1 / g[i + (j - 1) * dx];
							*valuesHook++ = -1 / g[(i + 1) + j * dx];
							*valuesHook++ = -1 / g[(i - 1) + j * dx];
							*valuesHook++ = -1 / g[i + (j + 1) * dx];
							*valuesHook++ = -1 / g[i + (j - 1) * dx];
						}
					}
			}
			if (direction == FORWARD) {
				for (uint32 j = 0; j < dy; j++)
					for (uint32 i = 0; i < dx; i++) {
						if ((i == 0) || (i == dx - 1)
							|| (j == 0) || (j == dy - 1))
							*valuesHook++ = 1;
						else {
							*valuesHook++ = 4 * mu * g[i + j * dx]
											+ g[(i + 1) + j * dx]
											+ g[(i - 1) + j * dx]
											+ g[i + (j + 1) * dx]
											+ g[i + (j - 1) * dx];
							*valuesHook++ = -g[(i + 1) + j * dx];
							*valuesHook++ = -g[(i - 1) + j * dx];
							*valuesHook++ = -g[i + (j + 1) * dx];
							*valuesHook++ = -g[i + (j - 1) * dx];
						}
					}
			}
		}
	}
	return values;
}

// sparse_data_sharp
//
// this option is used when the clamping component and the squeezing
// compomnent are different.
float*
sparse_data_sharp(FloatImage* squeeze, FloatImage* clamp, float mu)
{
	float* values = NULL;
	if (squeeze && clamp
		&& squeeze->IsValid()
		&& clamp->IsValid()) {
		uint32 dx = squeeze->Width();
		uint32 dy = squeeze->Height();
		uint32 entryCount = 4 * (dx - 2) * (dy - 2) + dx * dy;
		float* sq = squeeze->Pixel();
		float* cl = clamp->Pixel();
		values = new float[entryCount];
		if (values) {
			float* valuesHook = values;
			for (uint32 j = 0; j < dy; j++)
				for (uint32 i = 0; i < dx; i++) {
					if ((i == 0) || (i == dx - 1)
						|| (j == 0) || (j == dy - 1))
						*valuesHook++ = 1;
					else {
						*valuesHook++ = 4 * mu * cl[i + j * dx]
										+ 1 / sq[(i + 1) + j * dx]
										+ 1 / sq[(i - 1) + j * dx]
										+ 1 / sq[i + (j + 1) * dx]
										+ 1 / sq[i + (j - 1) * dx];
						*valuesHook++ = -1 / sq[(i + 1) + j * dx];
						*valuesHook++ = -1 / sq[(i - 1) + j * dx];
						*valuesHook++ = -1 / sq[i + (j + 1) * dx];
						*valuesHook++ = -1 / sq[i + (j - 1) * dx];
					}
				}
		}
	}
	return values;
}

// AssembleSharpMatrix
SparseMatrix*
SparseMatrix::AssembleSharpMatrix(FloatImage* data, float mu, uint32 direction)
{
	SparseMatrix* matrix = NULL;
	if (data && data->IsValid()) {
		uint32 dx = data->Width();
		uint32 dy = data->Height();
		uint32* a = s_sparse_matrix_n_entry(dx, dy);
		int32* b = s_sparse_martix_entries(dx, dy);
		float* c = sparse_data_sharp(data, mu, direction);
		if (a && b && c)
			matrix = new SparseMatrix(a, b, c, dx * dy,
									  4 * (dx - 2) * (dy - 2) + dx * dy);
	}
	return matrix;
}

// AssembleSharpMatrix
SparseMatrix*
SparseMatrix::AssembleSharpMatrix(FloatImage* squeeze, FloatImage* clamp, float mu)
{
	SparseMatrix* matrix = NULL;
	if (squeeze && clamp && squeeze->IsValid() && clamp->IsValid()) {
		uint32 dx = squeeze->Width();
		uint32 dy = squeeze->Height();
		uint32* a = s_sparse_matrix_n_entry(dx, dy);
		int32* b = s_sparse_martix_entries(dx, dy);
		float* c = sparse_data_sharp(squeeze, clamp, mu);
		if (a && b && c) {
			matrix = new SparseMatrix(a, b, c, dx * dy,
									  4 * (dx - 2) * (dy - 2) + dx * dy);
		}
	}
	return matrix;
}

// ---------------------------------------------
// helper functions for ppmtoccd
// ---------------------------------------------

// ccd_sparse_martix_n_entry
uint32*
ccd_sparse_martix_n_entry(RGB8Image* source, uint32 color, uint32 &total)
{
	total = 0;
	uint32* output = NULL;
	if (source && source->IsValid()) {
		uint32 dx = source->Width();
		uint32 dy = source->Height();
		uint32 i;
		uint32 j;
		uint32 size = dx * dy;
		output = new uint32[size];
		if (output) {
			for (i = 0; i < size; i++)
				output[i] = 1;
			for (j = 2; j < dy - 2; j++)
				for (i = 2; i < dx - 2; i++) 
					switch (color) {
						case GREEN:
							if (((j % 2) == 0) && ((i % 2) == 1))
								output[i + j * dx] = 13; 
							if (((j % 2) == 1) && ((i % 2) == 0))
								output[i + j * dx] = 13;
						break;
						case RED:
							if ((j % 2) == 1)
								output[i + j * dx] = 13; 
							if (((j % 2) == 0) && ((i % 2) == 0))
								output[i+j*dx] = 13;
						break;
						case BLUE:
							if ((j % 2) == 0) 
								output[i + j * dx] = 13;
							if (((j % 2) == 1) && ((i % 2) == 1))
								output[i + j * dx] = 13;
					}
			for (i = 0; i < size; i++)
				total += output[i];
		}
	}
	return output;
}

// ccd_sparse_martix_n_entry1
uint32*
ccd_sparse_martix_n_entry1(RGB8Image* source, uint32 color, uint32 &total)
{
	total = 0;
	uint32* output = NULL;
	if (source && source->IsValid()) {
		uint32 dx = source->Width();
		uint32 dy = source->Height();
		uint32 i;
		uint32 j;
		uint32 size = dx * dy;
		int32* output = new int32[size];
		if (output)
			for (i = 0; i < size; i++)
				output[i] = 1;
			for (j = 1; j < dy - 1; j++)
				for (i = 1; i < dx - 1; i++) 
					switch (color) {
						case GREEN:
							if (((j % 2) == 0) && ((i % 2) == 1))
								output[i + j * dx] = 5; 
							if (((j % 2) == 1) && ((i % 2) == 0))
								output[i + j * dx] = 5;
						break;
						case RED:
							if ((j % 2) == 1)
								output[i + j * dx] = 5; 
							if (((j % 2) == 0) && ((i % 2) == 0))
								output[i + j * dx] = 5;
						break;
						case BLUE:
							if ((j % 2) == 0)
								output[i + j * dx] = 5;
							if (((j % 2) == 1) && ((i % 2) == 1))
								output[i + j * dx] = 5;
					}
			for (i = 0; i < size; i++)
				total += output[i];   
	}
	return output;
}

// ccd_sparse_martix_entries
int32*
ccd_sparse_martix_entries(RGB8Image* source, uint32* entry, uint32 entryCount)
{
	int32* output = NULL;
	if (source && source->IsValid() && entry && entryCount > 0) {
		uint32 dx = source->Width();
		uint32 dy = source->Height();
		uint32 i;
		uint32 j;
		output = new int32[entryCount];
		if (output) {
			int32* hook = output;
			for (j = 0; j < dy; j++)
				for (i = 0; i < dx; i++) {
					if (entry[i + j * dx] == 1)
						*hook++ = i + j * dx;
					else {
						*hook++ = i + j * dx;
						*hook++ = (i - 2) + (j - 2) * dx;
						*hook++ = (i - 0) + (j - 2) * dx;
						*hook++ = (i + 2) + (j - 2) * dx;
						*hook++ = (i - 0) + (j - 1) * dx;
						*hook++ = (i - 2) + (j - 0) * dx;
						*hook++ = (i - 1) + (j - 0) * dx;
						*hook++ = (i + 1) + (j - 0) * dx;
						*hook++ = (i + 2) + (j - 0) * dx;
						*hook++ = (i - 0) + (j + 1) * dx;
						*hook++ = (i - 2) + (j + 2) * dx;
						*hook++ = (i - 0) + (j + 2) * dx;
						*hook++ = (i + 2) + (j + 2) * dx;
					}
				}
		}
	}
	return output;
}

// ccd_sparse_martix_entries1
int32*
ccd_sparse_martix_entries1(RGB8Image* source, uint32* entry, uint32 entryCount)
{
	int32* output = NULL;
	if (source && source->IsValid() && entry && entryCount > 0) {
		uint32 dx = source->Width();
		uint32 dy = source->Height();
		uint32 i;
		uint32 j;
		output = new int32[entryCount];
		if (output) {
			int32* hook = output;
			for (j = 0; j < dy; j++)
				for (i = 0; i < dx; i++) {
					if (entry[i + j * dx] == 1)
						*hook++ = i + j * dx;
					else {
						*hook++ = i + j * dx;
						*hook++ = (i - 0) + (j - 1) * dx;
						*hook++ = (i - 1) + (j + 0) * dx;
						*hook++ = (i + 1) + (j - 0) * dx;
						*hook++ = (i - 0) + (j + 1) * dx;
					}
				}
		}
	}
	return output;
}

// ccd_s_sparse_data
float*
ccd_s_sparse_data(RGB8Image* source, uint32 *entry, uint32 entryCount)
{
	float* output = NULL;
	if (source && source->IsValid() && entry && entryCount > 0) {
		uint32 dx = source->Width();
		uint32 dy = source->Height();
		uint32 i;
		uint32 j;
		output = new float[entryCount];
		if (output) {
			float* hook = output;
			for (j = 0; j < dy; j++)
				for(i = 0; i < dx; i++) { 
					if (entry[i + j * dx] == 1)
						*hook++ = 1;
					else {
						*hook++ = 25;
						*hook++ = 0.25;
						*hook++ = 1.5;
						*hook++ = 0.25;
						*hook++ = -8;
						*hook++ = 1.5;
						*hook++ = -8;
						*hook++ = -8;
						*hook++ = 1.5;
						*hook++ = -8;
						*hook++ = 0.25;
						*hook++ = 1.5;
						*hook++ = 0.25;
					}
				}
		}
	}
	return output;
}

// ccd_s_sparse_data1
float*
ccd_s_sparse_data1(RGB8Image* source, uint32* entry, uint32 entryCount)
{
	float* output = NULL;
	if (source && source->IsValid() && entry && entryCount > 0) {
		uint32 dx = source->Width();
		uint32 dy = source->Height();
		uint32 i;
		uint32 j;
		output = new float[entryCount];
		if (output) {
			float* hook = output;
			for (j = 0; j < dy; j++)
				for(i = 0; i < dx; i++) { 
					if (entry[i + j * dx] == 1)
						*hook++ = 1;
					else {
						*hook++ = -4;
						*hook++ = 1;
						*hook++ = 1;
						*hook++ = 1;
						*hook++ = 1;
					}
				}
		}
	}
	return output;
}

// AssembleCCDMatrix
SparseMatrix*
SparseMatrix::AssembleCCDMatrix(RGB8Image* source, bool first, uint32 color)
{
	SparseMatrix* matrix = NULL;
	if (source && source->IsValid()) {
		uint32 entryCount;
		uint32 dx = source->Width();
		uint32 dy = source->Height();
		uint32* a;
		int32* b;
		float* c;
		if (first) {
			a = ccd_sparse_martix_n_entry1(source, color, entryCount);
			b = ccd_sparse_martix_entries1(source, a, entryCount);
			c = ccd_s_sparse_data1(source, a, entryCount);
		} else {
			a = ccd_sparse_martix_n_entry(source, color, entryCount);
			b = ccd_sparse_martix_entries(source, a, entryCount);
			c = ccd_s_sparse_data(source, a, entryCount);
		}
		if (a && b && c)
			matrix = new SparseMatrix(a, b, c, dx * dy, entryCount);
	}
	return matrix;
} 

// ---------------------------------------------
// solving sparse systems
// ---------------------------------------------

// residual
inline float
residual(const SparseMatrix& matrix, float *x, float *b)
{
	uint32 size = matrix.Order();
	float *err = matrix * x;
	float residue = 0;
	for (uint32 i = 0; i < size; i++)
		residue += sqr(err[i] - b[i]);
	delete[] err;
	return sqrt(residue);
}

// gs_pass
inline float
gs_pass(const SparseMatrix& A, float *xn, float *b, bool residue)
{
	float* b_hook = b;
	float* xn_hook = xn;
	uint32* entriesPerRow_hook = A.EntriesPerRow();
	int32* entries_hook = A.Entry();
	register uint32 order = A.Order();
	float* values_hook = A.Value();
	while(order--) {
		float temp = *b_hook++;
		float pivot = 0.0;
		register uint32 number_of_row_entries = *entriesPerRow_hook++;
		while (number_of_row_entries--) {
			int32 k = *entries_hook++;
			float v = *values_hook++;
			if (k + order != A.Order() - 1)
				temp -= v * xn[k];
			else
				pivot = v;
		}
		if (fabs(pivot) < TINY) {
			break;
		}
		*xn_hook++ = temp / pivot;
	}
	return residue ? residual(A, xn, b) : HUGE;
}

// Solve
uint32
SparseMatrix::Solve(const SparseMatrix& A, float* x, float *b, float stop,
					uint32 nIterOrThresh)
{
	if (nIterOrThresh == TRESHOLD) {
		float error;
		uint32 iter = 0;
		while ((error = gs_pass(A, x, b, true)) >= stop)
			iter++;
		return iter;
	} else {
		for (uint32 i = 0; i < stop; i++)
			gs_pass(A, x, b, false);
		return (uint32)stop;
	}
}

