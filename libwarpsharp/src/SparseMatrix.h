// SparseMatrix.h

#ifndef SPARSE_MATRIX_H
#define SPARSE_MATRIX_H

#include <SupportDefs.h>

#include "warpsharp.h"

class FloatImage;
class RGB8Image;

class IMPEXPLIBWARPSHARP SparseMatrix {
 public:
								SparseMatrix(); 
								SparseMatrix(const SparseMatrix& copy);
								// copies all data
								SparseMatrix(uint32 order,
											 uint32 entryCount,
											 uint32* entriesPerRow,
											 int32* entries,
											 float* values);
								// copies pointers only
								SparseMatrix(uint32* entriesPerRow,
											 int32* entries,
											 float* values,
											 uint32 order,
											 uint32 entryCount);
	virtual						~SparseMatrix();

			bool				IsValid() const;

	inline	uint32				Order() const
									{ return fOrder; }
	inline	uint32				CountEntries() const
									{ return fEntryCount; }

	inline	int32*				Entry() const
									{ return fEntries; }
	inline	int32				EntryAt(uint32 index) const
									{ return fEntries[index]; }

	inline	float*				Value() const
									{ return fValues; }
	inline	float				ValueAt(uint32 index) const
									{ return fValues[index]; }

	inline	uint32*				EntriesPerRow() const
									{ return fEntriesPerRow; }
	inline	uint32				EntriesPerRow(uint32 index) const
									{ return fEntriesPerRow[index]; }

			void				operator=(const SparseMatrix& copy);
			float				operator()(uint32 row, uint32 column);
			float*				operator*(float *vector) const;

			FloatImage*			GetFloatImage() const;

	// is used when clamping and squeezing are the same ID-component.
	static	SparseMatrix*		AssembleSharpMatrix(FloatImage* source, float mu,
													uint32 direction = BACKWARD);
	// is used when clamping and squeezing are different ID-components.
	static	SparseMatrix*		AssembleSharpMatrix(FloatImage* squeeze,
													FloatImage* clamp, float mu);

	static	SparseMatrix*		AssembleCCDMatrix(RGB8Image* source, bool first,
												  uint32 color);

	// returns number of Gauss-Seidel iterations carried out.
	// if NIterOrThresh==Thresh then stop is the threshold
	// else stop is the number of iterations to be carried out.
	static	uint32				Solve(const SparseMatrix& A, float* x, float *b,
									  float stop, uint32 nIterOrThresh);
private:
			uint32				fOrder;			// such as NxN
			uint32*				fEntriesPerRow;	// for each row this stores the 
												// number of elements in that row that !=0
			uint32				fEntryCount;	// total number of non-void entries
			int32*				fEntries;		// this stores the coordinates of the non-void
												// entries in row major order, it is of size "fEntryCount"
			float*				fValues;		// the values that are not zero arranged by rows
												// it is of size "fEntryCount"
};

#endif // SPARSE_MATRIX_H
