// Layer.cpp

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <Archivable.h>
#include <Bitmap.h>
#include <ClassInfo.h>
#include <DataIO.h>
#include <Message.h>

#include "bitmap_compression.h"
#include "bitmap_support.h"
#if USE_COMPRESSION
	#include "minilzo.h"
#endif
#include "support.h"

#include "AdvancedTransformable.h"
#include "BitmapStroke.h"
#include "History.h"
#include "InfoView.h"
#include "Stroke.h"

#include "Layer.h"

void*
Layer::fCompressionWorkerMem = NULL;

// constructor
Layer::Layer()
	: LayerBase(),
	  fBitmap(NULL),
	  fHistory(new History()),
	  fCompressedBits(NULL),
	  fCompressedLength(0),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fOriginalBounds(0.0, 0.0, -1.0, -1.0),
	  fConstructionBounds(0.0, 0.0, -1.0, -1.0),
	  fInitStatus(B_NO_INIT)
{
}

// constructor
Layer::Layer(BRect frame, float alpha, uint32 mode, uint32 flags)
	: LayerBase(alpha, mode, flags),
	  fBitmap(new BBitmap(frame, 0, B_RGBA32)),
	  fHistory(new History()),
	  fCompressedBits(NULL),
	  fCompressedLength(0),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fOriginalBounds(0.0, 0.0, -1.0, -1.0),
	  fConstructionBounds(frame),
	  fInitStatus(fBitmap->InitCheck())
{
	if (fInitStatus >= B_OK)
		memset(fBitmap->Bits(), 0, fBitmap->BitsLength());
	else {
		delete fBitmap;
		fBitmap = NULL;
	}
}

// bitmaps are always expected to be B_RGBA32 (or B_RGB32)
//
// constructor
Layer::Layer(BBitmap* bitmap, float alpha, uint32 mode, uint32 flags)
	: LayerBase(alpha, mode, flags),
	  fBitmap(bitmap),
	  fHistory(new History()),
	  fCompressedBits(NULL),
	  fCompressedLength(0),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fOriginalBounds(0.0, 0.0, -1.0, -1.0),
	  fConstructionBounds(0.0, 0.0, -1.0, -1.0),
	  fInitStatus(B_NO_INIT)
{
	if (fBitmap)
		fInitStatus = fBitmap->InitCheck();
	if (fInitStatus >= B_OK) {
		// insert bitmap modifier to back up visual contents
		fBounds = fBitmap->Bounds();
		BBitmap* croppedBitmap = cropped_bitmap(fBitmap, fBounds, false, true);
		if (croppedBitmap && croppedBitmap->IsValid()) {
			BitmapStroke* modifier = new BitmapStroke(croppedBitmap);
			if (fHistory->AddModifier(modifier)) {
				// success
				fBounds = croppedBitmap->Bounds();
				fOriginalBounds = fBounds;
				fConstructionBounds = fBitmap->Bounds();
			} else {
				delete modifier;

				delete fBitmap;
				fBitmap = NULL;
				fInitStatus = B_NO_MEMORY;
			}
		} else {
			delete fBitmap;
			fBitmap = NULL;
			fInitStatus = B_NO_MEMORY;
		}
	} else {
		delete fBitmap;
		fBitmap = NULL;
	}
}

// constructor (kind of a copy constructor)
Layer::Layer(const Layer* other, BRect bounds)
	: LayerBase(other->fAlpha, other->fMode, other->fFlags),
	  fBitmap(new BBitmap(bounds, 0, B_RGBA32)),
	  fHistory(new History(*other->GetHistory())),
	  fCompressedBits(NULL),
	  fCompressedLength(other->fCompressedLength),
	  fBounds(other->fBounds),
	  fOriginalBounds(bounds),
	  fConstructionBounds(bounds),
	  fInitStatus(B_NO_INIT)
{
	fBounds = fBounds & bounds;
	fMinimized = other->fMinimized;
	fName = other->fName;
	memset(fBitmap->Bits(), 0, fBitmap->BitsLength());
	if (other->fBitmap) {
		copy_area(other->fBitmap, fBitmap, other->fBitmap->Bounds());
	} else {
		// make a copy of the clones compressed data
		if (other->fCompressedBits) {
			fCompressedBits = malloc(other->fCompressedLength);
			memcpy(fCompressedBits, other->fCompressedBits, fCompressedLength);
		}
	}
	if (fBitmap || fCompressedBits)
		fInitStatus = fBitmap ? fBitmap->InitCheck() : B_OK;
}

// destructor
Layer::~Layer()
{
	delete fHistory;
	delete fBitmap;
	_FreeCompressedBits();
}

// InitCheck
status_t
Layer::InitCheck() const
{
	return fInitStatus;
}

// Minimize
void
Layer::Minimize(bool doIt)
{
	if (fMinimized != doIt) {
		_MinimizeBitmap(doIt);
		fMinimized = doIt;
	}
}

// GetHistory
History*
Layer::GetHistory() const
{
	return fHistory;
}

// Bitmap
BBitmap*
Layer::Bitmap() const
{
	return fBitmap;
}

// Clone
Layer*
Layer::Clone() const
{
	return new Layer(this, fConstructionBounds);
}

// MakeEmpty
void
Layer::MakeEmpty()
{
	// clear buffer to black and fully transparent
	memset(fBitmap->Bits(), 0, fBitmap->BitsLength());
	fBounds = fOriginalBounds;
	// empty history
	for (int32 i = 0; Stroke* stroke = fHistory->ModifierAt(i); i++)
		delete stroke;
	fHistory->MakeEmpty();
}

// Note: When this method returns,
// the content of the layer bitmap
// needs yet to be actually rendered!
//
// ChangeSize
status_t
Layer::ChangeSize(BRect newBounds)
{
	status_t status = newBounds.IsValid() ? B_OK : B_BAD_VALUE;
	if (status >= B_OK && newBounds != fConstructionBounds) {
		// delete now invalid data
		delete fBitmap;
		fBitmap = NULL;
		_FreeCompressedBits();

		// make a bitmap with the new size
		fConstructionBounds.Set(0.0, 0.0,
								ceilf(newBounds.Width()),
								ceilf(newBounds.Height()));

		fBitmap = new BBitmap(fConstructionBounds, 0, B_RGBA32);

		if (fBitmap && (status = fBitmap->InitCheck()) >= B_OK) {
			// clean bitmap contents
			memset(fBitmap->Bits(), 0, fBitmap->BitsLength());

			BPoint offset(-newBounds.left, -newBounds.top);
	
			// offset bounds and original bounds members
			fBounds.Set(0.0, 0.0, -1.0, -1.0);
			fOriginalBounds.OffsetBy(offset);
	
			// offset each modifier in the layers contruction
			// history
			if (fHistory) {
				for (int32 i = 0; Stroke* stroke = fHistory->ModifierAt(i); i++) {
					stroke->TranslateBy(offset);
					stroke->LayerBoundsChanged(fConstructionBounds);
					if (i == 0)
						fBounds = stroke->Bounds();
					else
						fBounds = fBounds | stroke->Bounds();
				}
			}
	
			// restore miminized state
			if (fMinimized) {
				fMinimized = false;
				Minimize(true);
			}
		} else {
			// clean up
			fInitStatus = status;
			delete fBitmap;
			fBitmap = NULL;
		}
	}
	return status;
}

// Note: When this method returns,
// the content of the layer bitmap
// needs yet to be actually rendered!
//
// ChangeSize
status_t
Layer::ResizeTo(BRect newBounds)
{
	status_t status = newBounds.IsValid() ? B_OK : B_BAD_VALUE;
	if (status >= B_OK) {
		newBounds.OffsetTo(BPoint(0.0, 0.0));
		if (newBounds != fConstructionBounds) {
			// delete now invalid data
			delete fBitmap;
			fBitmap = NULL;
			_FreeCompressedBits();

			// calculate scale
			double xScale = (newBounds.Width() + 1.0) / (fConstructionBounds.Width() + 1.0);
			double yScale = (newBounds.Height() + 1.0) / (fConstructionBounds.Height() + 1.0);
	
			// make a bitmap with the new size
			fConstructionBounds.Set(0.0, 0.0,
									ceilf(newBounds.Width()),
									ceilf(newBounds.Height()));
	
			fBitmap = new BBitmap(fConstructionBounds, 0, B_RGBA32);
	
			if (fBitmap && (status = fBitmap->InitCheck()) >= B_OK) {
				// clean bitmap contents
				memset(fBitmap->Bits(), 0, fBitmap->BitsLength());
	

				// scale bounds and original bounds members
				fBounds.Set(0.0, 0.0, -1.0, -1.0);
				fOriginalBounds.Set(fOriginalBounds.left * xScale,
									fOriginalBounds.top * yScale,
									fOriginalBounds.right * xScale,
									fOriginalBounds.bottom * yScale);
		
				// offset each modifier in the layers contruction
				// history
				if (fHistory) {
					for (int32 i = 0; Stroke* stroke = fHistory->ModifierAt(i); i++) {
						stroke->ScaleBy(BPoint(0.0, 0.0), xScale, yScale);
						stroke->LayerBoundsChanged(fConstructionBounds);
						if (i == 0)
							fBounds = stroke->Bounds();
						else
							fBounds = fBounds | stroke->Bounds();
					}
				}
		
				// restore miminized state
				if (fMinimized) {
					fMinimized = false;
					Minimize(true);
				}
			} else {
				// clean up
				fInitStatus = status;
				delete fBitmap;
				fBitmap = NULL;
			}
		}
	}
	return status;
}

// Note: When this method returns,
// the content of the layer bitmap
// needs yet to be actually rendered!
//
// RotateBy
status_t
Layer::RotateBy(double angle)
{
	status_t status = angle != 0.0 ? B_OK : B_BAD_VALUE;
	if (status >= B_OK) {

		// calculate a transform and the new bounds
		Transformable transform;
//		AdvancedTransformable transform;
		BPoint center((fConstructionBounds.left + fConstructionBounds.right + 1.0) / 2.0,
					  (fConstructionBounds.top + fConstructionBounds.bottom + 1.0) / 2.0);
		transform.RotateBy(center, angle);
		BRect newBounds = transform.TransformBounds(fConstructionBounds);

		newBounds.OffsetTo(BPoint(0.0, 0.0));
		BPoint newCenter((newBounds.left + newBounds.right + 1.0) / 2.0,
						 (newBounds.top + newBounds.bottom + 1.0) / 2.0);

		transform.TranslateBy(BPoint(newCenter - center));

		if (newBounds.IsValid()) {
			// delete now invalid data
			delete fBitmap;
			fBitmap = NULL;
			_FreeCompressedBits();

			// make a bitmap with the new size
			fConstructionBounds.Set(0.0, 0.0,
									ceilf(newBounds.Width()),
									ceilf(newBounds.Height()));
	
			fBitmap = new BBitmap(fConstructionBounds, 0, B_RGBA32);
			if (fBitmap && (status = fBitmap->InitCheck()) >= B_OK) {
				// clean bitmap contents
				memset(fBitmap->Bits(), 0, fBitmap->BitsLength());
	

				// rotate bounds and original bounds members
				fBounds.Set(0.0, 0.0, -1.0, -1.0);
				fOriginalBounds = transform.TransformBounds(fOriginalBounds);
				// transform each modifier in the layers contruction
				// history
				if (fHistory) {
					for (int32 i = 0; Stroke* stroke = fHistory->ModifierAt(i); i++) {
						stroke->Multiply(transform);
						stroke->LayerBoundsChanged(fConstructionBounds);
						if (i == 0)
							fBounds = stroke->Bounds();
						else
							fBounds = fBounds | stroke->Bounds();
					}
				}
				// restore miminized state
				if (fMinimized) {
					fMinimized = false;
					Minimize(true);
				}
			} else {
				// clean up
				fInitStatus = status;
				delete fBitmap;
				fBitmap = NULL;
			}
		}
	}
	return status;
}

// Touch
void
Layer::Touch(BRect area)
{
	if (fBitmap) {
		area = area & fBitmap->Bounds();
		if (fBounds.IsValid())
			fBounds = fBounds | area;
		else
			fBounds = area;
	}
}

// ActiveBounds
BRect
Layer::ActiveBounds() const
{
	if (fMode == MODE_MULTIPLY_ALPHA || fMode == MODE_MULTIPLY_INVERSE_ALPHA)
		return fConstructionBounds;
	return fBounds;
}

// RecalcActiveBounds
BRect
Layer::RecalcActiveBounds()
{
	fBounds.Set(0.0, 0.0, -1.0, -1.0);
	if (fHistory) {
		if (Stroke* object = fHistory->ModifierAt(0))
			fBounds = object->Bounds();
		for (int32 i = 1; Stroke* object = fHistory->ModifierAt(i); i++) {
			fBounds = fBounds | object->Bounds();
		}
		if (fBitmap)
			fBounds = fBounds & fBitmap->Bounds();
	}
	return ActiveBounds();
}

// Rebuild
void
Layer::Rebuild(BBitmap* strokeBitmap, BRect& area,
			   uint32 colorSpace, InfoView* infoView)
{
	if (fBitmap && area.IsValid() && area.Intersects(strokeBitmap->Bounds())) {
		area = area & strokeBitmap->Bounds();
		// give every stroke a chance to modify the rebuild area
		// and count strokes that intersect area for progress indication
		int32 count = 0;
		for (int32 i = 0; Stroke* stroke = fHistory->ModifierAt(i); i++) {
			stroke->ExtendRebuildArea(area, fBounds);
			if (stroke->Bounds().Intersects(area))
				count++;
		}
		// make sure we don't have a wrong constrain rect
		area.left = floorf(area.left);
		area.top = floorf(area.top);
		area.right = ceilf(area.right);
		area.bottom = ceilf(area.bottom);
	
		area = fBitmap->Bounds() & area;
	
		clear_area(fBitmap, area, 0, 0, 0, 0);
		clear_area(strokeBitmap, area);
	
		float progress = 0.0;
		float progressStep = 100.0 / count;
		if (count > 0)
			infoView->JobStarted();
	
		// traverse strokes
		BRect mergeRect(0.0, 0.0, -1.0, -1.0);
		for (int32 i = 0; Stroke* stroke = fHistory->ModifierAt(i); i++) {
			if (stroke->Bounds().Intersects(area)) {
				clear_area(strokeBitmap, mergeRect);
				mergeRect = area & stroke->Bounds();

				mergeRect.left = floorf(mergeRect.left);
				mergeRect.top = floorf(mergeRect.top);
				mergeRect.right = ceilf(mergeRect.right);
				mergeRect.bottom = ceilf(mergeRect.bottom);

				stroke->Draw(strokeBitmap, mergeRect);
				stroke->MergeWithBitmap(fBitmap, strokeBitmap, mergeRect, colorSpace);
				// progress
				progress += progressStep;
				infoView->JobProgress(progress);
			}
		}
	
		infoView->JobDone();
	}
}


static const uint32 	CURRENT_BITMAP				= 'crnt';
static const char*		CURRENT_BITMAP_KEY			= "current bitmap";
static const char*		STROKE_KEY					= "stroke";
static const char*		BOUNDS_KEY					= "bounds";
static const char*		ORIGINAL_BOUNDS_KEY			= "original bounds";

// Archive
status_t
Layer::Archive(BMessage* into) const
{
	status_t status = LayerBase::Archive(into);
	if (status >= B_OK) {
		status = fInitStatus;
		if (status >= B_OK) {
			// add current (resulting) bitmap (or compressed data)
			if (fBitmap) {
//printf("storing bitmap\n");
/*				BMessage bitmapArchive(CURRENT_BITMAP);
				if ((status = fBitmap->Archive(&bitmapArchive)) == B_OK)
					status = into->AddMessage(CURRENT_BITMAP_KEY, &bitmapArchive);
			} else if (fCompressedBits) {
//printf("storing compressed bits\n");
				status = into->AddData(CURRENT_COMPRESSED_KEY, B_RAW_TYPE,
									   fCompressedBits,
									   fCompressedLength);
				if (status >= B_OK && !into->HasRect(CONSTRUCTION_BOUNDS_KEY))
					status = into->AddRect(CONSTRUCTION_BOUNDS_KEY,
										   fConstructionBounds);*/
				status = archive_bitmap(fBitmap, into, CURRENT_BITMAP_KEY);
			}
			if (status >= B_OK) {
				// add all strokes to archive
				for (int32 i = 0; Stroke* stroke = fHistory->ModifierAt(i); i++) {
					BMessage strokeArchive;
					if ((status = stroke->Archive(&strokeArchive)) == B_OK)
						status = into->AddMessage(STROKE_KEY, &strokeArchive);
					if (status < B_OK)
						break;
				}
			}
			if (status >= B_OK) {
				// the rest of our parameters (they are not essential,
				// and that's why we don't check failure to store them)
				into->AddRect(BOUNDS_KEY, fBounds);
				into->AddRect(ORIGINAL_BOUNDS_KEY, fOriginalBounds);
			}
		}
	}
	return status;
}

// Unarchive
status_t
Layer::Unarchive(BMessage* archive)
{
	status_t status = LayerBase::Unarchive(archive);
	if (status >= B_OK) {
		// delete current contents
		delete fBitmap;
		fBitmap = NULL;

/*		// extract bitmap from archive
		BMessage bitmapArchive(CURRENT_BITMAP);
		const void* compressedData = NULL;
		ssize_t compressedSize = 0;
		compressedData = NULL;
		compressedSize = 0;
		if (archive->FindMessage(CURRENT_BITMAP_KEY, &bitmapArchive) == B_OK) {
//printf("restored bitmap\n");
			fBitmap = new BBitmap(&bitmapArchive);
		} else if (archive->FindData(CURRENT_COMPRESSED_KEY,
									 B_RAW_TYPE, &compressedData,
									 &compressedSize) >= B_OK
				   && archive->FindRect(CONSTRUCTION_BOUNDS_KEY,
				   						&fConstructionBounds) >= B_OK) {
//printf("restoring bitmap from compressed data\n");
			// create bitmap from compressed data
			fBitmap = _DecompressBitmap(compressedData,
										compressedSize,
										fConstructionBounds);
		}*/
		status = extract_bitmap(&fBitmap, archive, CURRENT_BITMAP_KEY);
		// empty history
		for (int32 i = 0; Stroke* stroke = fHistory->ModifierAt(i); i++)
			delete stroke;
		fHistory->MakeEmpty();

		// restore strokes from archive
		BMessage strokeMessage;
		for (int32 i = 0; archive->FindMessage(STROKE_KEY, i, &strokeMessage) == B_OK; i++) {
			if (BArchivable* archivable = instantiate_object(&strokeMessage)) {
				// test against the various stroke classes
				Stroke* stroke = cast_as(archivable, Stroke);
				if (!stroke || !fHistory->AddModifier(stroke))
					delete archivable;
			}
			strokeMessage.MakeEmpty();
		}
		BRect bounds;
		if (archive->FindRect(BOUNDS_KEY, &bounds) == B_OK)
			fBounds = bounds;
		else
			fBounds.Set(0.0, 0.0, -1.0, -1.0);
		if (archive->FindRect(ORIGINAL_BOUNDS_KEY, &bounds) == B_OK)
			fOriginalBounds = bounds;
		else {
			if (fBitmap)
				fOriginalBounds = fBitmap->Bounds();
			else
				fOriginalBounds.Set(0.0, 0.0, -1.0, -1.0);
		}
		// validate status fBitmap
		if (fBitmap) {
			status = fInitStatus = fBitmap->InitCheck();
			if (status < B_OK) {
				delete fBitmap;
				fBitmap = NULL;
			} else {
				fConstructionBounds = fBitmap->Bounds();
			}
		} else
			fInitStatus = B_NO_INIT;
	}
	return status;
}

// AllocateCompressionMem
void
Layer::AllocateCompressionMem()
{
#if USE_COMPRESSION
	if (!fCompressionWorkerMem)
		fCompressionWorkerMem = malloc(LZO1X_MEM_COMPRESS);
#endif
}

// FreeCompressionMem
void
Layer::FreeCompressionMem()
{
	if (fCompressionWorkerMem)
		free(fCompressionWorkerMem);
}

// _MinimizeBitmap
void
Layer::_MinimizeBitmap(bool minimize)
{
	if (minimize) {
		// limit memory usage to the bare minimum
		if (!fCompressedBits) {
			if (_CompressBitmap(fBitmap, &fCompressedBits, &fCompressedLength)) {
				delete fBitmap;
				fBitmap = NULL;
			}
		}
	} else {
		// use as much memory as needed
		if (!fBitmap) {
			fBitmap = _DecompressBitmap(fCompressedBits, fCompressedLength,
										fConstructionBounds);
			if (fBitmap)
				_FreeCompressedBits();
		}
	}
}

// _CompressBitmap
bool
Layer::_CompressBitmap(const BBitmap* bitmap, void** buffer, unsigned int* size) const
{
	bool result = false;
#if USE_COMPRESSION
	if (bitmap) {
		lzo_byte* src = (lzo_byte*)bitmap->Bits();
		lzo_uint srcLength = bitmap->BitsLength();
		*size = srcLength  + (srcLength / 64) + 16 + 3;
		*buffer = malloc(*size);
		if (*buffer && fCompressionWorkerMem) {
			if (!lzo1x_1_compress(src, srcLength,
								  (lzo_byte*)*buffer,
								  (lzo_uint*)size,
								  (lzo_byte*)fCompressionWorkerMem)) {
//printf("compressed %d bytes bitmap into %d bytes\n", srcLength, *size);
				if (srcLength  + (srcLength / 64) + 16 + 3 != *size)
					*buffer = realloc(*buffer, *size);
				result = true;
			} else {
				// error compressing
				free(*buffer);
				*buffer = NULL;
				*size = 0;
			}
		} else
			*size = 0;
	}
#endif
	return result;
}

// _DecompressBitmap
BBitmap*
Layer::_DecompressBitmap(const void* buffer, unsigned int size, BRect frame) const
{
	BBitmap* bitmap = NULL;
#if USE_COMPRESSION
	if (buffer) {
		bitmap = new BBitmap(frame, 0, B_RGBA32);
		if (bitmap->IsValid()) {
			lzo_byte* dst = (lzo_byte*)bitmap->Bits();
			lzo_uint dstLength = bitmap->BitsLength();
			if (!lzo1x_decompress((lzo_byte*)buffer,
								  (lzo_uint)size,
								  dst, &dstLength,
								  (lzo_byte*)fCompressionWorkerMem)) {
//printf("decompressed %d bytes into %d bytes bitmap\n", size, dstLength);
				if (dstLength != (uint32)bitmap->BitsLength()) {
					// error decompressing
					delete bitmap;
					bitmap = NULL;
				}
			}
		} else {
			delete bitmap;
			bitmap = NULL;
		}
	}
#else
	bitmap = new BBitmap(frame, B_RGBA32, 0);
	memset(bitmap->Bits(), 0, bitmap->BitsLength());
#endif
	return bitmap;
}

// _BitmapsAreCompatible
bool
Layer::_BitmapsAreCompatible(BBitmap* bitmap1, BBitmap* bitmap2) const
{
	return (bitmap1->Bounds().Width() == bitmap2->Bounds().Width()
			&& bitmap1->Bounds().Height() == bitmap2->Bounds().Height()
			&& bitmap1->BitsLength() == bitmap2->BitsLength()
			&& bitmap1->ColorSpace() == bitmap2->ColorSpace());
}

// _FreeCompressedBits
void
Layer::_FreeCompressedBits()
{
	if (fCompressedBits) {
		free(fCompressedBits);
		fCompressedBits = NULL;
	}
}
