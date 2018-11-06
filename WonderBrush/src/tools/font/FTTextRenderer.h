// FTTextRenderer.h

#ifndef FT_TEXT_RENDERER_H
#define FT_TEXT_RENDERER_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "TextRenderer.h"

#define MAX_GLYPHS 512

struct entry_ref;

class FTTextRenderer : public TextRenderer {
 public:
								FTTextRenderer();
								FTTextRenderer(BMessage* archive);
								FTTextRenderer(const FTTextRenderer& from);
	virtual						~FTTextRenderer();

	virtual	bool				SetFont(const char* pathToFontFile);
	virtual	void				Unset();

	virtual	const char*			Family() const;
	virtual	const char*			Style() const;

			void				SetRotation(float rotation);
			float				Rotation() const;

	virtual	void				RenderString(BBitmap* bitmap,
											 BRect constrainRect,
											 const Transformable& transform);

	virtual	BRect				Bounds(const Transformable& transform) const;

	virtual	void				Update();

 private:
			void				_LayoutGlyphs();
			void				_PrepareText(const char* string);
			void				_ResetTransform();
			void				_ResetScale(int pointSize);
			void				_BlitImageToBitmap(BBitmap* dest,
												   FT_Bitmap* source,
												   FT_Pos left, FT_Pos top,
												   BRect constrainRect) const;


			FT_Library			fLibrary;			// the FreeType fLibrary
			FT_Face				fFace;				// the font face
			FT_Error			fError;				// error returned by FreeType?
		
			float				fRotation;
			int32				fResolution;		// default resolution in dpi

			FT_Matrix			fTransformMatrix;
			bool				fTransform;
		
			FT_Vector			fStringCenter;
			BRect				fBounds;

	typedef struct TGlyph_ {
		FT_UInt		glyph_index;	// glyph index in face
		FT_Vector	pos;			// position of glyph origin
		FT_Glyph	image;			// glyph image

	} TGlyph, *PGlyph;

			// The following array is used to store the glyph set
			// that makes up a string of text
			TGlyph				fGlyphs[MAX_GLYPHS];
			int					fGlyphCount;
};

#endif // FT_TEXT_RENDERER_H
