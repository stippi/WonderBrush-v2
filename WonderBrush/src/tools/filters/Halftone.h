// Halftone.h

#ifndef HALFTONE_H
#define HALFTONE_H

#include "FilterObject.h"

#define	ROUND_DOT_SIZE		8
#define	DIAGONAL_LINE_SIZE	5
#define	ORDERED_MATRIX_SIZE	4

union color {
	uint8 bytes[4];
	uint32 word;
};

class Halftone : public FilterObject {
 public:
								Halftone();
								Halftone(const Halftone& other);
								Halftone(BMessage* archive);
	virtual						~Halftone();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	void				ExtendRebuildArea(BRect& area,
												  BRect layerBounds) const;

	virtual	void				ProcessBitmap(BBitmap* bitmap,
											  BBitmap* alphaBitmap,
											  BRect constrainRect) const;

	virtual	bool				IsColorObject() const
									{ return true; }

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

 private:
			void				_InitPatterns();

			void				round_dot_halftone(uint32* bits, uint32 ppl,
												   int32 left, int32 top,
												   int32 right, int32 bottom,
												   color c1, color c2) const;
			void				diagonal_line_halftone(uint32* bits, uint32 ppl,
													   int32 left, int32 top,
													   int32 right, int32 bottom,
													   color c1, color c2) const;
			void				ordered_dither_halftone(uint32* bits, uint32 ppl,
														int32 left, int32 top,
														int32 right, int32 bottom,
														color c1, color c2) const;
			void				fs_dither_halftone(uint32* bits, uint32 ppl,
												   int32 left, int32 top,
												   int32 right, int32 bottom,
												   color c1, color c2) const;
			void				ncandidate_dither_halftone(uint32* bits, uint32 ppl,
														   int32 left, int32 top,
														   int32 right, int32 bottom,
														   color c1, color c2) const;

			int32				fMode;

			uint32				round_dot_pattern[ROUND_DOT_SIZE][ROUND_DOT_SIZE];
			uint32				diagonal_line_pattern[DIAGONAL_LINE_SIZE][DIAGONAL_LINE_SIZE];
			uint32				ordered_matrix[ORDERED_MATRIX_SIZE][ORDERED_MATRIX_SIZE];
};

#endif	// HALFTONE_H
