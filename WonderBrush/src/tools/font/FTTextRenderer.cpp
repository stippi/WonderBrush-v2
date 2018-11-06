// FTTextRenderer.cpp

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Bitmap.h>
#include <ByteOrder.h>
#include <Entry.h>
#include <Path.h>
#include <UTF8.h>

#include "support.h"

#include "FontManager.h"

#include "FTTextRenderer.h"

#define	MAXPTSIZE	5000				// dtp

#define FLOOR(x)	((x) & -64)
#define CEIL(x)		(((x)+63) & -64)
#define TRUNC(x)	((x) >> 6)

// constructor
FTTextRenderer::FTTextRenderer()
	: TextRenderer(),
	  fLibrary(NULL),
	  fFace(NULL),
	  fError(0),
	  fRotation(0.0),
	  fResolution(72),
	  fTransformMatrix(),
	  fTransform(false),
	  fStringCenter(),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fGlyphCount(0)
{
	// initialize engine
	fError = FT_Init_FreeType(&fLibrary);
	if (fError)
		fprintf(stderr, "Could not initialise FreeType library\n");
}

FTTextRenderer::FTTextRenderer(BMessage* archive)
	: TextRenderer(archive),
	  fLibrary(NULL),
	  fFace(NULL),
	  fError(0),
	  fRotation(0.0),
	  fResolution(72),
	  fTransformMatrix(),
	  fTransform(false),
	  fStringCenter(),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fGlyphCount(0)
{
	// initialize engine
	fError = FT_Init_FreeType(&fLibrary);
	if (fError)
		fprintf(stderr, "Could not initialise FreeType library\n");
}

// constructor
FTTextRenderer::FTTextRenderer(const FTTextRenderer& from)
	: TextRenderer(from),
	  fLibrary(NULL),
	  fFace(NULL),
	  fError(0),
	  fRotation(0.0),
	  fResolution(72),
	  fTransformMatrix(),
	  fTransform(false),
	  fStringCenter(),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fGlyphCount(0)
{
	// initialize engine
	fError = FT_Init_FreeType(&fLibrary);
	if (fError)
		fprintf(stderr, "Could not initialise FreeType library\n");
}

// destructor
FTTextRenderer::~FTTextRenderer()
{
	Unset();
	if (fLibrary)
		FT_Done_FreeType(fLibrary);
}

// SetFont
bool
FTTextRenderer::SetFont(const char* pathToFontFile)
{
	// keep pointers to the old memory
	FT_Face oldFace = fFace;
	fError = FT_New_Face(fLibrary, pathToFontFile, 0, &fFace);
	if (!fError) {
		// prepare the text to be rendered
		_PrepareText(fText);
		_ResetScale(fPtSize);
		Update();

		if (oldFace)
			FT_Done_Face(oldFace);		

		return TextRenderer::SetFont(pathToFontFile);
	} else {
		fprintf(stderr, "%s : is not a font file or could not be opened\n",
				pathToFontFile);
		// roll back changes
		fFace = oldFace;
	}
	return false;
}

// Unset
void
FTTextRenderer::Unset()
{
	if (fFace)
		FT_Done_Face(fFace);
}

// Family
const char*
FTTextRenderer::Family() const
{
	if (fFace)
		return fFace->family_name;
	return NULL;
}

// Style
const char*
FTTextRenderer::Style() const
{
	if (fFace)
		return fFace->style_name;
	return NULL;
}

// SetRotation
void
FTTextRenderer::SetRotation(float angle)
{
	while (angle > 360.0)
		angle -= 360.0;
	fRotation = angle;
	Update();
}

// Rotation
float
FTTextRenderer::Rotation() const
{
	return fRotation;
}

// RenderString
//
// renders a given glyph vector set
void
FTTextRenderer::RenderString(BBitmap* bitmap,
							 BRect constrainRect,
							 const Transformable& transform)
{
	if (fFace && bitmap && bitmap->IsValid() && constrainRect.IsValid()) {
		if (bitmap->Bounds().Intersects(constrainRect)) {

			// clip constrainRect to bitmap area
			constrainRect = constrainRect & bitmap->Bounds();

			uint32 height = bitmap->Bounds().IntegerHeight() + 1;
			BPoint offset(0.0, 0.0);
			transform.Transform(&offset);
			
			PGlyph glyph = fGlyphs;

			// first of all, we must compute the
			// general delta for the glyph set
			FT_Vector delta;
			delta.x = (FT_Pos)(offset.x * 64.0);
			delta.y = (height << 6) - (FT_Pos)(offset.y * 64.0);

			FT_BBox clipBox;
			clipBox.xMin = (FT_Pos)constrainRect.left;
			clipBox.yMin = height - (FT_Pos)constrainRect.bottom;
			clipBox.xMax = (FT_Pos)constrainRect.right;
			clipBox.yMax = height - (FT_Pos)constrainRect.top;

			for (int n = 0; n < fGlyphCount; n++, glyph++) {
				FT_Glyph image;
				FT_Vector vec;
				
				if (!glyph->image)
					continue;

				// copy image
				fError = FT_Glyph_Copy(glyph->image, &image);
				if (fError)
					continue;

				// transform it
				vec = glyph->pos;
				FT_Vector_Transform(&vec, &fTransformMatrix);
				vec.x += delta.x;
				vec.y += delta.y;
				fError = FT_Glyph_Transform(image,
											&fTransformMatrix,
											&vec);
				if (!fError) {
					FT_BBox	bbox;
					// check bounding box, if it's not
					// within the constrainRect,
					// we don't need to render it
					FT_Glyph_Get_CBox(image, ft_glyph_bbox_pixels, &bbox);
					if (bbox.xMax >= clipBox.xMin
						&& bbox.yMax >= clipBox.yMin
						&& bbox.xMin <= clipBox.xMax
						&& bbox.yMin <= clipBox.yMax) {						 
						// convert to a bitmap - destroy native image
						fError = FT_Glyph_To_Bitmap(&image,
													fAntialias ?
													ft_render_mode_normal
													: ft_render_mode_mono,
													0, 1);
						if (!fError) {
							FT_BitmapGlyph bitmapImage = (FT_BitmapGlyph)image;
							// now render the bitmap into the display surface
							_BlitImageToBitmap(bitmap, &bitmapImage->bitmap,
											   bitmapImage->left, height - bitmapImage->top,
											   constrainRect);
						}
					}
				}
				FT_Done_Glyph(image);
			}
		}
	} else {
fprintf(stderr, "FTTextRenderer::RenderString() - invalid face\n");
	}
}

// Bounds
BRect
FTTextRenderer::Bounds(const Transformable& transform) const
{
	return transform.TransformBounds(fBounds);
}

// Update
void
FTTextRenderer::Update()
{
	// layout glyphs
	if (!fError && fFace) {
		_ResetScale(fPtSize);
		_ResetTransform();
		_LayoutGlyphs();
	}
}

// _ResetScale
void
FTTextRenderer::_ResetScale(int pointSize)
{
	FT_Set_Char_Size(fFace,
					 pointSize << 6,
					 pointSize << 6,
					 fResolution,
					 fResolution);
}

// _LayoutGlyphs
//
// layout a string of glyphs, the glyphs are untransformed
void
FTTextRenderer::_LayoutGlyphs()
{
	FT_UInt load_flags = FT_LOAD_DEFAULT;
	if( !fHinted )
		load_flags |= FT_LOAD_NO_HINTING;

	fBounds.left = 0.0;
	fBounds.top = 0.0;
	fBounds.right = -1.0;
	fBounds.bottom = -1.0;

	PGlyph glyph = fGlyphs;
	FT_Pos origin_x = 0;
	FT_UInt prev_index = 0;
	for (int n = 0; n < fGlyphCount; n++, glyph++) {
		// compute glyph origin
		if (fKerning) {
			if (prev_index) {
				FT_Vector kern;

				FT_Get_Kerning(fFace, prev_index, glyph->glyph_index,
							   fHinted ? ft_kerning_default : ft_kerning_unfitted,
							   &kern);

				origin_x += (FT_Pos)(kern.x * fAdvanceScale);
			}
			prev_index = glyph->glyph_index;
		}

		FT_Vector origin;
		origin.x = origin_x;
		origin.y = 0;

		// clear existing image if there is one
		if (glyph->image)
			FT_Done_Glyph(glyph->image);

		// load the glyph image (in its native format)
		// for now, we take a monochrome glyph bitmap
		fError = FT_Load_Glyph(fFace, glyph->glyph_index,
							   fHinted ?
							   FT_LOAD_DEFAULT : FT_LOAD_NO_HINTING);
		if (fError)
			continue;

		fError = FT_Get_Glyph(fFace->glyph, &glyph->image);
		if (fError)
			continue;

		glyph->pos = origin;

		// update bounding box
		FT_BBox bbox;
		FT_Glyph_Get_CBox(glyph->image, ft_glyph_bbox_pixels, &bbox);
		fBounds.left = min_c(bbox.xMin + origin_x / 64, fBounds.left);
		fBounds.top = min_c(bbox.yMin, fBounds.top);
		fBounds.right = max_c(bbox.xMax + origin_x / 64, fBounds.right);
		fBounds.bottom = max_c(bbox.yMax, fBounds.bottom);

		origin_x += (FT_Pos)(fFace->glyph->advance.x * fAdvanceScale);
	}
	fStringCenter.x = origin_x / 2;
	fStringCenter.y = (FT_Pos)(((fBounds.top + fBounds.bottom) * 64.0) / 2.0);

	if (fTransform) {
		FT_Vector leftTop;
		FT_Vector rightBottom;
		FT_Vector rightTop;
		FT_Vector leftBottom;

		leftTop.x = (FT_Pos)fBounds.left;
		leftTop.y = (FT_Pos)fBounds.top;
		rightBottom.x = (FT_Pos)fBounds.right;
		rightBottom.y = (FT_Pos)fBounds.bottom;
		rightTop.x = (FT_Pos)fBounds.right;
		rightTop.y = (FT_Pos)fBounds.top;
		leftBottom.x = (FT_Pos)fBounds.left;
		leftBottom.y = (FT_Pos)fBounds.bottom;

		FT_Vector_Transform( &leftTop, &fTransformMatrix );
		FT_Vector_Transform( &rightBottom, &fTransformMatrix );
		FT_Vector_Transform( &rightTop, &fTransformMatrix );
		FT_Vector_Transform( &leftBottom, &fTransformMatrix );

		fBounds.left = (float)min4(leftTop.x, rightBottom.x, rightTop.x, leftBottom.x);
		fBounds.top = (float)min4(leftTop.y, rightBottom.y, rightTop.y, leftBottom.y);
		fBounds.right = (float)max4(leftTop.x, rightBottom.x, rightTop.x, leftBottom.x);
		fBounds.bottom = (float)max4(leftTop.y, rightBottom.y, rightTop.y, leftBottom.y);
	}

	// convert fBounds to normal coordinate system (y=top->bottom)
	float temp = fBounds.bottom;
	fBounds.bottom = -fBounds.top;
	fBounds.top = -temp;
}

// _PrepareText
//
// Convert a string of text into a glyph vector
void
FTTextRenderer::_PrepareText(const char* string)
{
	fGlyphCount = 0;
	if (string && fFace) {
		int32 srcLength = strlen(string);
		int32 dstLength = srcLength * 4;

		char* buffer = new char[dstLength];

		int32 state = 0;
		if (convert_from_utf8(B_UNICODE_CONVERSION, 
							  string, &srcLength,
							  buffer, &dstLength,
							  &state, B_SUBSTITUTE) >= B_OK
			&& swap_data(B_INT16_TYPE, buffer, dstLength,
						 B_SWAP_BENDIAN_TO_HOST) >= B_OK) {
			uint16* p = (uint16*)buffer;
			PGlyph glyph = fGlyphs;
			FT_UInt glyph_index;
		
			for (int32 i = 0; i < dstLength / 2; i++) {
				glyph_index = FT_Get_Char_Index( fFace, *p );
				glyph->glyph_index = glyph_index;
				glyph->image = NULL;
				glyph++;
				fGlyphCount++;
				if (fGlyphCount >= MAX_GLYPHS)
					break;
				p++;
			}
		}
		delete[] buffer;
	}
}

// _ResetTransform
void
FTTextRenderer::_ResetTransform()
{
	double		angle	= fRotation * 3.14159 / 180.0;
	FT_Fixed	cosinus	= (FT_Fixed)(cos(angle) * 65536.0);
	FT_Fixed	sinus	= (FT_Fixed)(sin(angle) * 65536.0);

	fTransform 			= (angle != 0);
	fTransformMatrix.xx = cosinus;
	fTransformMatrix.xy = -sinus;
	fTransformMatrix.yx = sinus;
	fTransformMatrix.yy = cosinus;
}

// _BlitImageToBitmap
//
// constrainRect is expected to fit within bitmap
// bitmap is expected to be of format B_GRAY8
void
FTTextRenderer::_BlitImageToBitmap(BBitmap* bitmap,
								 FT_Bitmap* image,
								 FT_Pos left, FT_Pos top,
								 BRect constrainRect) const
{
	uint8* dst = (uint8*)bitmap->Bits();
	uint8* src = (uint8*)image->buffer;
	uint32 dstBPR = bitmap->BytesPerRow();
	uint32 srcBPR = image->pitch;
	FT_Pos right = left + image->width - 1;
	if (right > constrainRect.right)
		right = (FT_Pos)constrainRect.right;
	if (right < constrainRect.left) {
		return;
	}
	FT_Pos bottom = top + image->rows - 1;
	if (bottom > constrainRect.bottom)
		bottom = (FT_Pos)constrainRect.bottom;
	if (bottom < constrainRect.top) {
		return;
	}

	// calculate offsets into src and dst buffers
	// with additional clipping by constrainRect
	if (left < (int32)constrainRect.left) {
		int32 clippingLeft = (int32)constrainRect.left - left;
		src += clippingLeft;
		dst += left + clippingLeft;

		left += clippingLeft;
	} else {
		dst += left;
	}

	if (top < (int32)constrainRect.top) {
		int32 clippingTop = (int32)constrainRect.top - top;
		src += clippingTop * srcBPR;
		dst += (top + clippingTop) * dstBPR;

		top += clippingTop;
	} else {
		dst += top * dstBPR;
	}
	// copy the (part of the) image into the bitmap
	if (image->pixel_mode == ft_pixel_mode_grays) {
		int32 bytes = right - left + 1;
		if (bytes > 0) {
			for (; top <= bottom; top++) {
				uint8* dstHandle = dst;
				uint8* srcHandle = src;
				for (int32 x = left; x <= right; x++) {
					*dstHandle = max_c(*dstHandle, min_c(fOpacity, (*srcHandle * fOpacity) / 255));
					dstHandle++;
					srcHandle++;
				}
				dst += dstBPR;
				src += srcBPR;
			}
		}
	} else if (image->pixel_mode == ft_pixel_mode_mono) {
		// TODO: handle this
	}
}


