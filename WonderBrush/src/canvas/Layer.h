// Layer.h

#ifndef LAYER_H
#define LAYER_H

#include "LayerBase.h"

class BBitmap;
class History;
class InfoView;

class Layer : public LayerBase {
 public:
								Layer();
								Layer(BRect frame,
									  float alpha = 1.0,
									  uint32 mode = MODE_NORMAL,
									  uint32 flags = 0);
								Layer(BBitmap* bitmap,
									  float alpha = 1.0,
									  uint32 mode = MODE_NORMAL,
									  uint32 flags = 0);
								Layer(const Layer* other,
									  BRect bounds);
	virtual						~Layer();


	virtual	status_t			InitCheck() const;

								// this function can be used to
								// minimize the memory usage of
								// the layer to the bare minimum
	virtual	void				Minimize(bool doIt);

			History*			GetHistory() const;

	virtual	BBitmap*			Bitmap() const;
	virtual	Layer*				Clone() const;

	virtual	void				MakeEmpty();

	// this one is used for cropping
	// (alias change work area, without scaling)
			status_t			ChangeSize(BRect newBounds);
	// this one is used for scaling
			status_t			ResizeTo(BRect newBounds);
	// and this one for rotation, it will calculate the smallest
	// bounds that it can fit into, and transform each object
			status_t			RotateBy(double angle);

	// active area of layer
	virtual	void				Touch(BRect area);
	virtual	BRect				ActiveBounds() const;
			BRect				RecalcActiveBounds();

	// composing
			void				Rebuild(BBitmap* strokeBitmap,
										BRect& area,
										uint32 colorSpace,
										InfoView* infoView);

	// loading / saving
	virtual	status_t			Archive(BMessage* into) const;
	virtual	status_t			Unarchive(BMessage* archive);

	// worker memory for lzo compression algorithm
	static	void				AllocateCompressionMem();
	static	void				FreeCompressionMem();

 private:
			bool				_BitmapsAreCompatible(BBitmap* bitmap1,
													  BBitmap* bitmap2) const;

			void				_MinimizeBitmap(bool minimize);

			void				_FreeCompressedBits();

			bool				_CompressBitmap(const BBitmap* bitmap,
												void** buffer,
												unsigned int* size) const;
			BBitmap*			_DecompressBitmap(const void* buffer,
												  unsigned int size,
												  BRect frame) const;

 protected:
			BBitmap*			fBitmap;

 private:
			History*			fHistory;
			// for compression in memory and decompression into a bitmap
			void*				fCompressedBits;
			unsigned int		fCompressedLength;

			BRect				fBounds;
			BRect				fOriginalBounds;
			BRect				fConstructionBounds;

			status_t			fInitStatus;

	static	void*				fCompressionWorkerMem;
};

#endif // LAYER_H
