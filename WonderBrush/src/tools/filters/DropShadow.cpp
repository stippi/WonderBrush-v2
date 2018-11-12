// DropShadow.cpp

#include <new>
#include <stdio.h>

#include <Bitmap.h>
#include <Message.h>

#include <BitmapStream.h>
#include <File.h>
#include <InterfaceDefs.h>
#include <TranslatorRoster.h>

#include "blending.h"
#include "support.h"

#include "CommonPropertyIDs.h"
#include "FilterFactory.h"
#include "FloatProperty.h"
#include "IntProperty.h"
#include "LanguageManager.h"
#include "PropertyObject.h"
#include "StackBlurFilter.h"

#include "DropShadow.h"


// convolve_1d_fixed
inline void
convolve_1d_fixed_blur(uint8* s, uint8* t, int32 length, int32* kernel, int32 kernelRadius)
{
	for (register int32 x = 0; x < length; ++x) {

		int32 alpha = 0;

		uint8 *tmp = s - kernelRadius;
		int32 *kernel_tmp = kernel - kernelRadius;

		for (int32 index = -kernelRadius; index <= kernelRadius; ++index) {
			alpha += (*tmp * *kernel_tmp++);
			tmp++;
		}

		*t++ = alpha >> 15;
		s++;
	}
}

// convolve_1d_fixed
inline void
convolve_1d_fixed(uint8* s, uint8* t, int32 length, int32* kernel, int32 kernelRadius)
{
/*	uint8 *tmp = s - kernelRadius;
	uint8 *t_tmp = t;

	register int32 x = 0;
	for (; x < length; ++x) {
		if (*tmp)
			break;
		tmp++;
		*t_tmp = 0;
		t_tmp++;
	}
	x -= kernelRadius;
	s += x;
	t += x;
	length -= x;*/
	convolve_1d_fixed_blur(s, t, length, kernel, kernelRadius);
}


// gaussian_blur
status_t
gaussian_blur(BBitmap *bitmap, float radius)
{
	int32 kernelRadius = (int32)ceil(radius);
	float* kernelArray = new float[2 * kernelRadius + 1];
	float* kernel = &kernelArray[kernelRadius];
	float sum = 0;
	int32* fixedKernelArray = new int32[2 * kernelRadius + 1];
	int32* fixedKernel = &fixedKernelArray[kernelRadius];

	// was before -log(0.002)
	float p = -log(0.004) / (pow(radius, 2) * log(2));
	for (int32 i=-kernelRadius;i<=kernelRadius;i++) {
		kernel[i] = pow(2,-p * i * i);
		sum += kernel[i];
	}
	for (int32 i=-kernelRadius;i<=kernelRadius;i++) {
		kernel[i] /= sum;
		fixedKernel[i] = (int32)(kernel[i] * 32768);
	}

	BBitmap* intermediate;
	BRect bitmapBounds = bitmap->Bounds();
	BRect intermediateBounds;
	intermediateBounds.left = bitmapBounds.top;
	intermediateBounds.top = bitmapBounds.left;
	intermediateBounds.right = bitmapBounds.bottom;
	intermediateBounds.bottom = bitmapBounds.right;

	intermediate = new BBitmap(intermediateBounds, 0, bitmap->ColorSpace());

	uint8* bBits = (uint8*)bitmap->Bits();
	uint8* iBits = (uint8*)intermediate->Bits();
	int32 bBPR = bitmap->BytesPerRow();
	int32 iBPR = intermediate->BytesPerRow();

	// Blur from bitmap to intermediate and rotate
	int32 width = bitmapBounds.IntegerWidth() + 1;
	int32 height = bitmapBounds.IntegerHeight() + 1;
	uint8* sourceArray = new uint8[width + kernelRadius * 2];
	uint8* targetArray = new uint8[width];


	for (int32 y = 0; y < height; y++) {
		// copy first pixel radius times into source array
		uint8* sourceArrayHandle = sourceArray;
		for (int32 dx = 0; dx < kernelRadius; dx++) {
			*sourceArrayHandle++ = *bBits;
		}
		// copy width pixels into source array
		for (int32 dx = 0;dx < width; dx++) {
			*sourceArrayHandle++ = *bBits++;
		}
		// copy last pixel radius times into source array
		bBits--;
		for (int32 dx = 0; dx < kernelRadius; dx++) {
			*sourceArrayHandle++ = *bBits;
		}
		bBits++;

		convolve_1d_fixed(sourceArray + kernelRadius,
						  targetArray,
						  width,
						  fixedKernel,
						  kernelRadius);

		uint8* targetArrayHandle = targetArray;
		for (int32 x = 0; x < width; x++) {
			*(iBits + (height - y - 1) + x * iBPR) = *targetArrayHandle++;
		}

	}


	bBits = (uint8*)bitmap->Bits();
	iBits = (uint8*)intermediate->Bits();


	delete[] sourceArray;
	delete[] targetArray;


	// Blur from intermediate to bitmap and rotate
	width = intermediateBounds.IntegerWidth() + 1;
	height = intermediateBounds.IntegerHeight() + 1;
	sourceArray = new uint8[width + kernelRadius * 2];
	targetArray = new uint8[width];

	for (int32 y = 0; y < height; y++) {
		uint8* sourceArrayHandle = sourceArray;
		for (int32 dx = 0; dx < kernelRadius; dx++) {
			*sourceArrayHandle++ = *iBits;
		}
		for (int32 dx = 0; dx < width; dx++) {
			*sourceArrayHandle++ = *iBits++;
		}
		iBits--;
		for (int32 dx = 0; dx < kernelRadius; dx++) {
			*sourceArrayHandle++ = *iBits;
		}
		iBits++;

		convolve_1d_fixed(sourceArray + kernelRadius,
						  targetArray,
						  width,
						  fixedKernel,
						  kernelRadius);

		uint8* targetArrayHandle = targetArray;
		for (int32 dx = 0; dx < width; dx++) {
			*(bBits + y + (width - dx - 1) * bBPR) = *targetArrayHandle++;
		}

	}

	delete[] sourceArray;
	delete[] targetArray;
	delete[] kernelArray;
	delete[] fixedKernelArray;
	delete intermediate;

	return B_OK;
}


















// constructor
DropShadow::DropShadow(rgb_color color,
					   BPoint offset,
					   float blurRadius)
	: FilterObject(FILTER_DROP_SHADOW),
	  fOffset(offset),
	  fBlurRadius(blurRadius),
	  fOpacity(255)
{
	SetColor(color);
}

// copy constructor
DropShadow::DropShadow(const DropShadow& other)
	: FilterObject(other),
	  fOffset(other.fOffset),
	  fBlurRadius(other.fBlurRadius),
	  fOpacity(other.fOpacity)
{
}

// BArchivable constructor
DropShadow::DropShadow(BMessage* archive)
	: FilterObject(archive),
	  fOffset(5.0, 5.0),
	  fBlurRadius(5.0),
	  fOpacity(255)
{
	if (archive) {
		if (archive->FindPoint("offset", &fOffset) < B_OK)
			fOffset = BPoint(5.0, 5.0);
		if (archive->FindFloat("radius", &fBlurRadius) < B_OK)
			fBlurRadius = 5.0;
		int32 opacity;
		if (archive->FindInt32("opacity", &opacity) >= B_OK)
			fOpacity = opacity;

		TransformationChanged();
	} else {
		SetFilterID(FILTER_DROP_SHADOW);
	}
}

// destructor
DropShadow::~DropShadow()
{
}

// Clone
Stroke*
DropShadow::Clone() const
{
	return new DropShadow(*this);
}

// SetTo
bool
DropShadow::SetTo(const Stroke* from)
{
	const DropShadow* dropShadow = dynamic_cast<const DropShadow*>(from);

	AutoNotificationSuspender _(this);

	if (dropShadow && FilterObject::SetTo(from)) {
		fOffset = dropShadow->fOffset;
		fBlurRadius = dropShadow->fBlurRadius;
		fOpacity = dropShadow->fOpacity;

		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
DropShadow::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "DropShadow"))
		return new DropShadow(archive);
	return NULL;
}

// Archive
status_t
DropShadow::Archive(BMessage* into, bool deep) const
{
	status_t status = FilterObject::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddPoint("offset", fOffset);

	if (status >= B_OK)
		status = into->AddFloat("radius", fBlurRadius);

	if (status >= B_OK)
		status = into->AddInt32("opacity", fOpacity);


	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "DropShadow");

	return status;
}

// ExtendRebuildArea
void
DropShadow::ExtendRebuildArea(BRect& area, BRect layerBounds) const
{
	area = layerBounds;
}

// cropped_bitmap_no_clip
BBitmap*
cropped_bitmap_no_clip(BBitmap* source, BRect bounds)
{
	BRect s = source->Bounds();

	bounds.left = floorf(bounds.left);
	bounds.top = floorf(bounds.top);
	bounds.right = ceilf(bounds.right);
	bounds.bottom = ceilf(bounds.bottom);

	int32 w = bounds.IntegerWidth() + 1;
	int32 h = bounds.IntegerHeight() + 1;
	bounds.right = bounds.left + (w + (4 - w % 4)) - 1;
	bounds.bottom = bounds.top + (h + (4 - h % 4)) - 1;

	BBitmap* ret = new(std::nothrow) BBitmap(bounds.OffsetToCopy(B_ORIGIN), 0, source->ColorSpace());
	if (ret && ret->InitCheck() >= B_OK) {

		uint32 emptyStartLines	= bounds.top	< 0.0		? (uint32)-bounds.top					: 0;
		uint32 emptyEndLines	= bounds.bottom	> s.bottom	? (uint32)(bounds.bottom - s.bottom)	: 0;
		uint32 emptyStartPixels	= bounds.left	< 0.0		? (uint32)-bounds.left					: 0;
		uint32 emptyEndPixels	= bounds.right	> s.right	? (uint32)(bounds.right - s.right)		: 0;

		uint32 lines = bounds.IntegerHeight() + 1 - (emptyStartLines + emptyEndLines);
		uint32 pixels = bounds.IntegerWidth() + 1 - (emptyStartPixels + emptyEndPixels);
	// NOTE: we know the color space is B_GRAY8
	/*	switch (source->ColorSpace()) {
			case B_RGBA32:
			case B_RGB32:
				pixels *= 4;
				break;
			case B_RGB24:
				pixels *= 3;
				break;
			case B_RGB16:
			case B_RGB15:
				pixels *= 2;
				break;
			case B_CMAP8:
			case B_GRAY8:
			default:
				break;
		}*/

		uint8* src = (uint8*)source->Bits();
		uint8* dst = (uint8*)ret->Bits();
		uint32 srcBPR = source->BytesPerRow();
		uint32 dstBPR = ret->BytesPerRow();

		BRect t = s & bounds;
		src += (int)(t.left - s.left + (t.top - s.top) * srcBPR);

		// make starting lines empty
		for (uint32 y = 0; y < emptyStartLines; y++) {
			memset(dst, 0, dstBPR);
			dst += dstBPR;
		}
		// copy bitmap contents
		for (uint32 y = 0; y < lines; y++) {
			uint8* handle = dst;
			if (emptyStartPixels > 0) {
				memset(handle, 0, emptyStartPixels);
				handle += emptyStartPixels;
			}
			memcpy(handle, src, pixels);
			handle += pixels;
			if (emptyEndPixels) {
				memset(handle, 0, emptyEndPixels);
			}
			dst += dstBPR;
			src += srcBPR;
		}
		// make ending lines empty
		for (uint32 y = 0; y < emptyEndLines; y++) {
			memset(dst, 0, dstBPR);
			dst += dstBPR;
		}
	}

	return ret;
}

// copy_bitmap
void
copy_bitmap(BBitmap* dest, BBitmap* source, int32 x, int32 y)
{
	BRect b = source->Bounds();
	b.OffsetBy(x, y);
	b = b & dest->Bounds();

	uint8* src = (uint8*)source->Bits();
	uint8* dst = (uint8*)dest->Bits();
	uint32 srcBPR = source->BytesPerRow();
	uint32 dstBPR = dest->BytesPerRow();

	src += (int)((b.left - x) + (b.top - y) * srcBPR);
	dst += (int)(b.left + b.top * dstBPR);

	uint32 bytes = (uint32)(b.right - b.left + 1);
	uint32 lines = (uint32)(b.bottom - b.top + 1);

	for (uint32 y = 0; y < lines; y++) {
		memcpy(dst, src, bytes);
		src += srcBPR;
		dst += dstBPR;
	}
}


#define TIMING 0

// ProcessBitmap
//
// on the fly drop shadow generation
void
DropShadow::ProcessBitmap(BBitmap* dest, BBitmap* strokeBitmap, BRect area) const
{
	if (dest && dest->IsValid()
		&& area.IsValid() && area.Intersects(dest->Bounds())) {
		// constrain rect to passed bitmap bounds
		area = area & dest->Bounds();

		float blurRadius = fBlurRadius * scale();
		BPoint offset(fOffset);
		Transform(&offset);
//offset.PrintToStream();

		// this is the area we need for the gaussian blur
		BRect blurArea = area;
		blurArea.InsetBy(-blurRadius, -blurRadius);
		blurArea.OffsetBy(-offset.x, -offset.y);
		blurArea = blurArea & dest->Bounds();
		// integer version of constrain rect
		int32 left, top, right, bottom;
		rect_to_int(blurArea, left, top, right, bottom);

		// handles and byte lengths
		uint8* dst = (uint8*)dest->Bits();
		uint32 dstBPR = dest->BytesPerRow();
		uint8* src = (uint8*)strokeBitmap->Bits();
		uint32 srcBPR = strokeBitmap->BytesPerRow();

		// offsets into bitmaps
		dst += left * 4 + top * dstBPR;
		src += left + top * srcBPR;
#if TIMING
bigtime_t extract = system_time();
#endif
		int32 t;
		// first pass:
		// copy alpha channel of dest bitmap and blur it
		for (int32 y = top; y <= bottom; y++) {

			uint8* dstHandle = dst;
			uint8* srcHandle = src;

			for (int32 x = left; x <= right; x++) {

				srcHandle[0] = INT_MULT(dstHandle[3], fOpacity, t);

				dstHandle += 4;
				srcHandle += 1;
			}
			dst += dstBPR;
			src += srcBPR;
		}

/*if (modifiers() & B_SHIFT_KEY) {
BFile fileStream("/boot/home/Desktop/alphapass.png", B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
BTranslatorRoster* roster = BTranslatorRoster::Default();
BBitmapStream bitmapStream(strokeBitmap);
roster->Translate(&bitmapStream, NULL, NULL, &fileStream, B_PNG_FORMAT, 0);
bitmapStream.DetachBitmap(&strokeBitmap);
}*/

		// do the gaussian blur
#if TIMING
bigtime_t crop = system_time();
#endif
		BBitmap* temp = cropped_bitmap_no_clip(strokeBitmap, blurArea);
		if (!temp || temp->InitCheck() < B_OK) {
fprintf(stderr, "DropShadow::ProcessBitmap(): not enough memory to create temporary bitmap!\n");
			delete temp;
			return;
		}
//blurArea.PrintToStream();
//strokeBitmap->Bounds().PrintToStream();

/*if (modifiers() & B_SHIFT_KEY) {
BFile fileStream("/boot/home/Desktop/croppass.png", B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
BTranslatorRoster* roster = BTranslatorRoster::Default();
BBitmapStream bitmapStream(temp);
roster->Translate(&bitmapStream, NULL, NULL, &fileStream, B_PNG_FORMAT, 0);
bitmapStream.DetachBitmap(&temp);
}*/

#if TIMING
bigtime_t blur = system_time();
#endif
		//gaussian_blur(temp, blurRadius);
		StackBlurFilter filter;
		filter.Filter(temp, blurRadius);

/*if (modifiers() & B_SHIFT_KEY) {
BFile fileStream("/boot/home/Desktop/blurpass.png", B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
BTranslatorRoster* roster = BTranslatorRoster::Default();
BBitmapStream bitmapStream(temp);
roster->Translate(&bitmapStream, NULL, NULL, &fileStream, B_PNG_FORMAT, 0);
bitmapStream.DetachBitmap(&temp);
}*/

#if TIMING
bigtime_t copy = system_time();
#endif
		copy_bitmap(strokeBitmap, temp, (int32)(blurArea.left + offset.x), (int32)(blurArea.top + offset.y));
		delete temp;
#if TIMING
bigtime_t merge = system_time();
#endif
		// second pass:
		// compose shadow (strokeBitmap) with color and dest on top together

		//  this is the area we're actually supposed to render
		rect_to_int(area, left, top, right, bottom);

		// reset offsets into bitmaps
		dst = (uint8*)dest->Bits();
		src = (uint8*)strokeBitmap->Bits();
		dst += left * 4 + top * dstBPR;
		src += left + top * srcBPR;

		uint8 bottomColor[4];
		bottomColor[0] = fColor.blue;
		bottomColor[1] = fColor.green;
		bottomColor[2] = fColor.red;
		for (int32 y = top; y <= bottom; y++) {

			uint8* dstHandle = dst;
			uint8* srcHandle = src;

			for (int32 x = left; x <= right; x++) {

				bottomColor[3] = srcHandle[0];

				blend_colors_copy(bottomColor, dstHandle[3], dstHandle,
								  dstHandle[0], dstHandle[1], dstHandle[2]);

				dstHandle += 4;
				srcHandle += 1;
			}
			dst += dstBPR;
			src += srcBPR;
		}
#if TIMING
bigtime_t finish = system_time();
bigtime_t total = finish - extract;
printf("drop shadow timing\n");
printf("     extract alpha: %lld (%f%%)\n", crop - extract, ((float)(crop - extract) / (float)(total)) * 100.0);
printf("              crop: %lld (%f%%)\n", blur - crop, ((float)(blur - crop) / (float)(total)) * 100.0);
printf("              blur: %lld (%f%%)\n", copy - blur, ((float)(copy - blur) / (float)(total)) * 100.0);
printf("              copy: %lld (%f%%)\n", merge - copy, ((float)(merge - copy) / (float)(total)) * 100.0);
printf("             merge: %lld (%f%%)\n", finish - merge, ((float)(finish - merge) / (float)(total)) * 100.0);
printf("             total: %lld\n", total);*/
#endif // TIMING
	}
}

// MakePropertyObject
PropertyObject*
DropShadow::MakePropertyObject() const
{
	PropertyObject* object = Stroke::MakePropertyObject();
	if (object) {
		object->AddProperty(new IntProperty("opacity",
											PROPERTY_OPACITY,
											fOpacity,
											0, 255));
		object->AddProperty(new FloatProperty("radius",
											  PROPERTY_BLUR_RADIUS,
											  fBlurRadius,
											  0.0, 100.0));
		object->AddProperty(new FloatProperty("x offset",
											  PROPERTY_X_OFFSET,
											  fOffset.x,
											  -10000.0, 10000.0));
		object->AddProperty(new FloatProperty("y offset",
											  PROPERTY_Y_OFFSET,
											  fOffset.y,
											  -10000.0, 10000.0));
	}
	return object;
}

// SetToPropertyObject
bool
DropShadow::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	bool ret = Stroke::SetToPropertyObject(object);
	bool ret2 = false;
	if (object) {
		int32 i = object->FindIntProperty(PROPERTY_OPACITY, fOpacity);
		if (i != fOpacity)
			ret2 = true;
		fOpacity = i;

		float f = object->FindFloatProperty(PROPERTY_BLUR_RADIUS, fBlurRadius);
		if (f != fBlurRadius)
			ret2 = true;
		fBlurRadius = f;

		f = object->FindFloatProperty(PROPERTY_X_OFFSET, fOffset.x);
		if (f != fOffset.x)
			ret2 = true;
		fOffset.x = f;

		f = object->FindFloatProperty(PROPERTY_Y_OFFSET, fOffset.y);
		if (f != fOffset.y)
			ret2 = true;
		fOffset.y = f;

		if (ret2) {
			Notify();
			SaveSettings();
		}
	}
	return ret || ret2;
}

// TransformationChanged
void
DropShadow::TransformationChanged()
{
	// undo any translation, we're only interested in rotation and scale
	tx = 0.0;
	ty = 0.0;

	// undo any perspective influence
	w0 = 0.0;
	w1 = 0.0;
	w2 = 1.0;

	Notify();
}


