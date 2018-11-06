// LayerBase.cpp

#include <stdio.h>
//#include <string.h>
//#include <malloc.h>

#include <Archivable.h>
#include <Bitmap.h>
#include <ClassInfo.h>
//#include <DataIO.h>
#include <Message.h>

#include "bitmap_support.h"
#include "blending.h"
#include "lab_convert.h"
#include "support.h"

#include "LanguageManager.h"

#include "LayerBase.h"

// constructor
LayerBase::LayerBase(float alpha, uint32 mode, uint32 flags)
	: fName(""),
	  fAlpha(alpha),
	  fMode(mode),
	  fFlags(flags),
	  fMinimized(false),
	  fCurrent(false)
{
}

// destructor
LayerBase::~LayerBase()
{
}

// InitCheck
status_t
LayerBase::InitCheck() const
{
	return B_OK;
}

// AttachedToCanvas
void
LayerBase::AttachedToContainer(LayerContainer* container)
{
	Minimize(false);
}

// DetachedFromCanvas
void
LayerBase::DetachedFromContainer()
{
	fCurrent = false;
	Minimize(true);
}

// IsMinimized
bool
LayerBase::IsMinimized() const
{
	return fMinimized;
}

// Name
const char*
LayerBase::Name() const
{
	if (fName.CountChars() == 0) {
		LanguageManager* manager = LanguageManager::Default();
		return manager->GetString(UNNAMED, "<unnamed>");
	} else
		return fName.String();
}

// SetName
void
LayerBase::SetName(const char* name)
{
	fName.SetTo(name);
}

// SetAlpha
void
LayerBase::SetAlpha(float alpha)
{
	constrain(alpha, 0.0, 1.0);
	if (alpha != fAlpha) {
		fAlpha = alpha;
		// TODO: send update messages
	}
}

// Alpha
float
LayerBase::Alpha() const
{
	return fAlpha;
}

// SetMode
void
LayerBase::SetMode(uint32 mode)
{
	if (mode != fMode) {
		fMode = mode;
		// TODO: send update messages
	}
}

// Mode
uint32
LayerBase::Mode() const
{
	return fMode;
}

// SetFlags
void
LayerBase::SetFlags(uint32 flags)
{
	if (flags != fFlags) {
		fFlags = flags;
		// TODO: send update messages
	}
}

// Flags
uint32
LayerBase::Flags() const
{
	return fFlags;
}

// SetCurrent
void
LayerBase::SetCurrent(bool current)
{
	fCurrent = current;
}

// IsCurrent
bool
LayerBase::IsCurrent() const
{
	return fCurrent;
}

// Compose
status_t
LayerBase::Compose(const BBitmap* into, BRect area)
{
	BBitmap* bitmap = Bitmap();
	if (!bitmap || !bitmap->IsValid()
		|| (bitmap->ColorSpace() != B_RGBA32 && bitmap->ColorSpace() != B_RGB32))
		return B_NO_INIT;
	status_t status = B_BAD_VALUE;
	if (into
		&& area.IsValid()
		&& (status = into->InitCheck()) >= B_OK) {

		// make sure we don't access memory outside of our bitmap
		area = area & bitmap->Bounds();

		BRect r = ActiveBounds();
		if (r.IsValid() && !(fFlags & FLAG_INVISIBLE) && r.Intersects(area)) {
			r = r & area;
			int32 left, top, right, bottom;
			rect_to_int(r, left, top, right, bottom);

			uint8* src = (uint8*)bitmap->Bits();
			uint8* dst = (uint8*)into->Bits();
			uint32 bpr = into->BytesPerRow();
			src += 4 * left + bpr * top;
			dst += 4 * left + bpr * top;
			uint8 alphaOverride = (uint8)(fAlpha * 255);
			switch (fMode) {
				case MODE_SOFT_LIGHT:
					for (; top <= bottom; top++) {
						uint8* srcHandle = src;
						uint8* dstHandle = dst;
						for (int32 x = left; x <= right; x++) {
							if (srcHandle[3] > 0) {
								// compose
								//		c := a * b >> 8; 
								//		result := c + a * (255 - ((255-a)*(255-b) >> 8)-c) >> 8;
		
								uint8 c1 = dstHandle[0] * srcHandle[0] >> 8;
								c1 = c1 + dstHandle[0] * (255 - ((255 - dstHandle[0]) * (255 - srcHandle[0]) >> 8) - c1) >> 8;
								c1 = (c1 * dstHandle[3] + srcHandle[0] * (255 - dstHandle[3])) >> 8;

								uint8 c2 = dstHandle[1] * srcHandle[1] >> 8;
								c2 = c2 + dstHandle[1] * (255 - ((255 - dstHandle[1]) * (255 - srcHandle[1]) >> 8) - c2) >> 8;
								c2 = (c2 * dstHandle[3] + srcHandle[1] * (255 - dstHandle[3])) >> 8;

								uint8 c3 = dstHandle[2] * srcHandle[2] >> 8;
								c3 = c3 + dstHandle[2] * (255 - ((255 - dstHandle[2]) * (255 - srcHandle[2]) >> 8) - c3) >> 8;
								c3 = (c3 * dstHandle[3] + srcHandle[2] * (255 - dstHandle[3])) >> 8;

								blend_colors(dstHandle, (srcHandle[3] * alphaOverride) >> 8,
											 c1, c2, c3);
							}
							srcHandle += 4;
							dstHandle += 4;
						}
						src += bpr;
						dst += bpr;
					}
					break;
				case MODE_LIGHTEN:
					for (; top <= bottom; top++) {
						uint8* srcHandle = src;
						uint8* dstHandle = dst;
						for (int32 x = left; x <= right; x++) {
							if (srcHandle[3] > 0) {
								// compose
								uint8 c1 = (max_c(srcHandle[0], dstHandle[0]) * dstHandle[3]
											+ srcHandle[0] * (255 - dstHandle[3])) / 255;
								uint8 c2 = (max_c(srcHandle[1], dstHandle[1]) * dstHandle[3]
											+ srcHandle[1] * (255 - dstHandle[3])) / 255;
								uint8 c3 = (max_c(srcHandle[2], dstHandle[2]) * dstHandle[3]
											+ srcHandle[2] * (255 - dstHandle[3])) / 255;
								blend_colors(dstHandle, (srcHandle[3] * alphaOverride) / 255,
											 c1, c2, c3);
							}
							srcHandle += 4;
							dstHandle += 4;
						}
						src += bpr;
						dst += bpr;
					}
					break;
				case MODE_DARKEN:
					for (; top <= bottom; top++) {
						uint8* srcHandle = src;
						uint8* dstHandle = dst;
						for (int32 x = left; x <= right; x++) {
							if (srcHandle[3] > 0) {
								// compose
								uint8 c1 = (min_c(srcHandle[0], dstHandle[0]) * dstHandle[3]
											+ srcHandle[0] * (255 - dstHandle[3])) / 255;
								uint8 c2 = (min_c(srcHandle[1], dstHandle[1]) * dstHandle[3]
											+ srcHandle[1] * (255 - dstHandle[3])) / 255;
								uint8 c3 = (min_c(srcHandle[2], dstHandle[2]) * dstHandle[3]
											+ srcHandle[2] * (255 - dstHandle[3])) / 255;
								blend_colors(dstHandle, (srcHandle[3] * alphaOverride) / 255,
											 c1, c2, c3);
							}
							srcHandle += 4;
							dstHandle += 4;
						}
						src += bpr;
						dst += bpr;
					}
					break;
				case MODE_REPLACE_RED:
					for (; top <= bottom; top++) {
						uint8* srcHandle = src;
						uint8* dstHandle = dst;
						for (int32 x = left; x <= right; x++) {
							if (srcHandle[3] > 0) {
								// compose
								uint32 alpha = srcHandle[3] * alphaOverride;
								dstHandle[2] = (srcHandle[2] * alpha
												+ dstHandle[2] * (65025 - alpha)) / 65025;
							}
							srcHandle += 4;
							dstHandle += 4;
						}
						src += bpr;
						dst += bpr;
					}
					break;
				case MODE_REPLACE_GREEN:
					for (; top <= bottom; top++) {
						uint8* srcHandle = src;
						uint8* dstHandle = dst;
						for (int32 x = left; x <= right; x++) {
							if (srcHandle[3] > 0) {
								// compose
								uint32 alpha = srcHandle[3] * alphaOverride;
								dstHandle[1] = (srcHandle[1] * alpha
												+ dstHandle[1] * (65025 - alpha)) / 65025;
							}
							srcHandle += 4;
							dstHandle += 4;
						}
						src += bpr;
						dst += bpr;
					}
					break;
				case MODE_REPLACE_BLUE:
					for (; top <= bottom; top++) {
						uint8* srcHandle = src;
						uint8* dstHandle = dst;
						for (int32 x = left; x <= right; x++) {
							if (srcHandle[3] > 0) {
								// compose
								uint32 alpha = srcHandle[3] * alphaOverride;
								dstHandle[0] = (srcHandle[0] * alpha
												+ dstHandle[0] * (65025 - alpha)) / 65025;
							}
							srcHandle += 4;
							dstHandle += 4;
						}
						src += bpr;
						dst += bpr;
					}
					break;


				case MODE_MULTIPLY_INVERSE_ALPHA:
					for (; top <= bottom; top++) {
						uint8* srcHandle = src;
						uint8* dstHandle = dst;
						for (int32 x = left; x <= right; x++) {
							// compose
//							uint8 temp = (dstHandle[3] * (255 - srcHandle[3])) / 255;
							uint8 temp = min_c(dstHandle[3], 255 - srcHandle[3]);
							dstHandle[3] = (dstHandle[3] * (255 - alphaOverride) + temp * alphaOverride) / 255;
							srcHandle += 4;
							dstHandle += 4;
						}
						src += bpr;
						dst += bpr;
					}
					break;
				case MODE_MULTIPLY_ALPHA:
					for (; top <= bottom; top++) {
						uint8* srcHandle = src;
						uint8* dstHandle = dst;
						for (int32 x = left; x <= right; x++) {
							// compose
//							uint8 temp = (dstHandle[3] * srcHandle[3]) / 255;
							uint8 temp = min_c(dstHandle[3], srcHandle[3]);
							dstHandle[3] = (dstHandle[3] * (255 - alphaOverride) + temp * alphaOverride) / 255;
							srcHandle += 4;
							dstHandle += 4;
						}
						src += bpr;
						dst += bpr;
					}
					break;
				case MODE_LUMINANCE:
					for (; top <= bottom; top++) {
						uint8* srcHandle = src;
						uint8* dstHandle = dst;
						for (int32 x = left; x <= right; x++) {
							if (srcHandle[3] > 0) {
								// compose
								uint8 r = dstHandle[2];
								uint8 g = dstHandle[1];
								uint8 b = dstHandle[0];
								uint8 alpha = dstHandle[3];
								replace_luminance(r, g, b, srcHandle[2], srcHandle[1], srcHandle[0]);
								blend_colors(dstHandle, (srcHandle[3] * alphaOverride) / 255,
											 b, g, r);
								dstHandle[3] = alpha;
							}
							srcHandle += 4;
							dstHandle += 4;
						}
						src += bpr;
						dst += bpr;
					}
					break;
				case MODE_INVERSE_MULTIPLY:
					for (; top <= bottom; top++) {
						uint8* srcHandle = src;
						uint8* dstHandle = dst;
						for (int32 x = left; x <= right; x++) {
							if (srcHandle[3] > 0) {
								// compose
/*								uint8 c1 = ((((255 - srcHandle[0]) * dstHandle[0]) / 255) * dstHandle[3]
											+ srcHandle[0] * (255 - dstHandle[3])) / 255;
								uint8 c2 = ((((255 - srcHandle[1]) * dstHandle[1]) / 255) * dstHandle[3]
											+ srcHandle[1] * (255 - dstHandle[3])) / 255;
								uint8 c3 = ((((255 - srcHandle[2]) * dstHandle[2]) / 255) * dstHandle[3]
											+ srcHandle[2] * (255 - dstHandle[3])) / 255;*/
								uint8 c1 = 255 - ((((255 - srcHandle[0]) * (255 - dstHandle[0])) / 255) * dstHandle[3]
											+ (255 - srcHandle[0]) * (255 - dstHandle[3])) / 255;
								uint8 c2 = 255 - ((((255 - srcHandle[1]) * (255 - dstHandle[1])) / 255) * dstHandle[3]
											+ (255 - srcHandle[1]) * (255 - dstHandle[3])) / 255;
								uint8 c3 = 255 - ((((255 - srcHandle[2]) * (255 - dstHandle[2])) / 255) * dstHandle[3]
											+ (255 - srcHandle[2]) * (255 - dstHandle[3])) / 255;
								blend_colors(dstHandle, (srcHandle[3] * alphaOverride) / 255,
											 c1, c2, c3);
							}
							srcHandle += 4;
							dstHandle += 4;
						}
						src += bpr;
						dst += bpr;
					}
					break;
				case MODE_MULTIPLY:
					for (; top <= bottom; top++) {
						uint8* srcHandle = src;
						uint8* dstHandle = dst;
						for (int32 x = left; x <= right; x++) {
							if (srcHandle[3] > 0) {
								// compose
								uint8 c1 = (((srcHandle[0] * dstHandle[0]) / 255) * dstHandle[3]
											+ srcHandle[0] * (255 - dstHandle[3])) / 255;
								uint8 c2 = (((srcHandle[1] * dstHandle[1]) / 255) * dstHandle[3]
											+ srcHandle[1] * (255 - dstHandle[3])) / 255;
								uint8 c3 = (((srcHandle[2] * dstHandle[2]) / 255) * dstHandle[3]
											+ srcHandle[2] * (255 - dstHandle[3])) / 255;
								blend_colors(dstHandle, (srcHandle[3] * alphaOverride) / 255,
											 c1, c2, c3);
							}
							srcHandle += 4;
							dstHandle += 4;
						}
						src += bpr;
						dst += bpr;
					}
					break;
				case MODE_NORMAL:
				default:
					if (alphaOverride == 255) {
						// use an optimized version that composes the bitmaps directly
						for (; top <= bottom; top++) {
							uint8* srcHandle = src;
							uint8* dstHandle = dst;
							for (int32 x = left; x <= right; x++) {
								blend_colors(dstHandle, srcHandle);
								srcHandle += 4;
								dstHandle += 4;
							}
							src += bpr;
							dst += bpr;
						}
					} else {
						for (; top <= bottom; top++) {
							uint8* srcHandle = src;
							uint8* dstHandle = dst;
							for (int32 x = left; x <= right; x++) {
								blend_colors(dstHandle, srcHandle, alphaOverride);
								srcHandle += 4;
								dstHandle += 4;
							}
							src += bpr;
							dst += bpr;
						}
					}
					break;
			}
		}
	}	
	return status;
}

static const char*		NAME_KEY					= "name";
static const char*		ALPHA_KEY					= "alpha";
static const char*		MODE_KEY					= "mode";
static const char*		FLAGS_KEY					= "flags";

// Archive
status_t
LayerBase::Archive(BMessage* into) const
{
	status_t status = B_BAD_VALUE;
	if (into) {
		// these parameters are not essential,
		// and that's why we don't check failure to store them
		into->AddString(NAME_KEY, fName.String());
		into->AddFloat(ALPHA_KEY, fAlpha);
		into->AddInt32(MODE_KEY, fMode);
		into->AddInt32(FLAGS_KEY, fFlags);
		status = B_OK;
	}
	return status;
}

// Unarchive
status_t
LayerBase::Unarchive(BMessage* archive)
{
	status_t status = B_BAD_VALUE;
	if (archive) {
		// restore attributes
		const char* name;
		if (archive->FindString(NAME_KEY, &name) == B_OK)
			fName.SetTo(name);
		float alpha;
		if (archive->FindFloat(ALPHA_KEY, &alpha) == B_OK) {
			constrain(alpha, 0.0, 1.0);
			fAlpha = alpha;
		} else
			fAlpha = 1.0;
		uint32 mode;
		if (archive->FindInt32(MODE_KEY, (int32*)&mode) == B_OK)
			fMode = mode;
		else
			fMode = MODE_NORMAL;
		uint32 flags;
		if (archive->FindInt32(FLAGS_KEY, (int32*)&flags) == B_OK)
			fFlags = flags;
		else
			fFlags = 0;
		status = B_OK;
	}
	return status;
}
