// ArchivableTransformable.cpp

#include <Message.h>

#include "ArchivableTransformable.h"

// constructor
ArchivableTransformable::ArchivableTransformable()
	: Transformable()
{
}

// constructor
ArchivableTransformable::ArchivableTransformable(const BMessage* archive)
	: Transformable()
{
	if (!archive)
		return;

	double storage[9];
	status_t ret = B_OK;
	for (int32 i = 0; i < 6; i++) {
		ret = archive->FindDouble("affine matrix", i, &storage[i]);
		if (ret < B_OK)
			break;
	}
	if (ret < B_OK)
		return;

	// load the additional matrix fields for perspective
	// transformations since WonderBurhs 2.1
	if (archive->FindDouble("affine matrix", 6, &storage[6]) < B_OK)
		storage[6] = 0.0; // w0
	if (archive->FindDouble("affine matrix", 7, &storage[7]) < B_OK)
		storage[7] = 0.0; // w1
	if (archive->FindDouble("affine matrix", 8, &storage[8]) < B_OK)
		storage[8] = 1.0; // w2

	// NOTE: can not use load_from() since the field don't
	// match with agg::trans_affine

	sx = storage[0];
	shy = storage[1];
	shx = storage[2];
	sy = storage[3];
	tx = storage[4];
	ty = storage[5];

	w0 = storage[6];
	w1 = storage[7];
	w2 = storage[8];
}

// destructor
ArchivableTransformable::~ArchivableTransformable()
{
}

// Archive
status_t
ArchivableTransformable::Archive(BMessage* into, bool deep) const
{
	status_t ret = BArchivable::Archive(into, deep);
	// store the matrix
	// NOTE: can't use store_to since agg::trans_perspective
	// is not compatible with the agg::trans_affine with
	// respect to the matrix field index meaning
	if (ret >= B_OK)
		ret = into->AddDouble("affine matrix", sx);
	if (ret >= B_OK)
		ret = into->AddDouble("affine matrix", shy);
	if (ret >= B_OK)
		ret = into->AddDouble("affine matrix", shx);
	if (ret >= B_OK)
		ret = into->AddDouble("affine matrix", sy);
	if (ret >= B_OK)
		ret = into->AddDouble("affine matrix", tx);
	if (ret >= B_OK)
		ret = into->AddDouble("affine matrix", ty);

	if (ret >= B_OK)
		ret = into->AddDouble("affine matrix", w0);
	if (ret >= B_OK)
		ret = into->AddDouble("affine matrix", w1);
	if (ret >= B_OK)
		ret = into->AddDouble("affine matrix", w2);

	// finish off
	if (ret >= B_OK) {
		// added for backwards compatibility, previously
		// Transformable itself derived from BArchivable
		ret = into->AddString("class", "Transformable");
	}
	if (ret >= B_OK)
		ret = into->AddString("class", "ArchivableTransformable");

	return ret;
}
