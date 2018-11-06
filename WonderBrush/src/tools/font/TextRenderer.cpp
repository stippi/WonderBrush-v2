// TextRenderer.cpp

#include <stdio.h>
#include <string.h>

#include <Entry.h>
#include <Path.h>

#include "defines.h"

#include "FontManager.h"

#include "TextRenderer.h"

#define MAXPTSIZE 5000

// constructor
TextRenderer::TextRenderer()
	: fText(NULL),
//	  fFontFilePath(),
	  fPtSize(12.0),
	  fHinted(false),
	  fAntialias(true),
	  fKerning(true),
	  fOpacity(255),
	  fAdvanceScale(1.0),
	  fLineSpacingScale(1.2),
	  fTextWidth(0.0),
	  fAlignment(ALIGN_LEFT),
	  fParagraphInset(0.0),
	  fParagraphSpacing(1.0)
{
	fFontFilePath[0] = 0;
}

TextRenderer::TextRenderer(BMessage* archive)
	: fText(NULL),
//	  fFontFilePath(NULL),
	  fPtSize(12.0),
	  fHinted(false),
	  fAntialias(true),
	  fKerning(true),
	  fOpacity(255),
	  fAdvanceScale(1.0),
	  fLineSpacingScale(1.2),
	  fTextWidth(0.0),
	  fAlignment(ALIGN_LEFT),
	  fParagraphInset(0.0),
	  fParagraphSpacing(1.0)
{
//printf("TextRenderer::TextRenderer(BMessage*)\n");
	const char* text;
	if (archive->FindString("text", &text) >= B_OK)
		SetText(text);

	// backwards compatibility crap
	int32 size;
	if (archive->FindInt32("size", &size) < B_OK) {
		if (archive->FindFloat("size", &fPtSize) < B_OK)
			fPtSize = 12.0;
	} else {
		fPtSize = size / 16.0;
	}

	if (archive->FindBool("hinted", &fHinted) < B_OK)
		fHinted = false;
	if (archive->FindBool("antialias", &fAntialias) < B_OK)
		fAntialias = true;
	if (archive->FindBool("kerning", &fKerning) < B_OK)
		fKerning = true;

	int32 opacity;
	if (archive->FindInt32("gray levels", &opacity) < B_OK)
		fOpacity = 255;
	else
		fOpacity = max_c(0, min_c(255, opacity));

	if (archive->FindFloat("advance scale", &fAdvanceScale) < B_OK)
		fAdvanceScale = 1.0;

	if (archive->FindFloat("line spacing scale", &fLineSpacingScale) < B_OK)
		fLineSpacingScale = 1.2;

	if (archive->FindFloat("text width", &fTextWidth) < B_OK)
		fTextWidth = 0.0;

	if (archive->FindInt32("text alignment", (int32*)&fAlignment) < B_OK)
		fAlignment = ALIGN_LEFT;

	if (archive->FindFloat("paragraph inset", &fParagraphInset) < B_OK)
		fParagraphInset = 0.0;

	if (archive->FindFloat("paragraph spacing", &fParagraphSpacing) < B_OK)
		fParagraphSpacing = 1.0;

	fFontFilePath[0] = 0;
	const char* family;
	const char* style;
	if (archive->FindString("family", &family) >= B_OK
		&& archive->FindString("style", &style) >= B_OK) {
		FontManager* fm = FontManager::Default();
//printf("locking font manager,  %s, %s\n", family, style);
		if (fm->Lock()) {
//printf("setting font: %s, %s\n", family, style);
			SetFontRef(fm->FontFileFor(family, style));
			fm->Unlock();
		}
	} else {
//printf("no family or style!\n");
	}
}

// constructor
TextRenderer::TextRenderer(const TextRenderer& from)
	: fText(NULL),
//	  fFontFilePath(NULL),
	  fPtSize(12.0),
	  fHinted(false),
	  fAntialias(true),
	  fKerning(true),
	  fOpacity(255),
	  fAdvanceScale(1.0),
	  fLineSpacingScale(1.2),
	  fTextWidth(0.0),
	  fAlignment(ALIGN_LEFT),
	  fParagraphInset(0.0),
	  fParagraphSpacing(1.0)
{
	fFontFilePath[0] = 0;
	SetTo(&from);
}

// destructor
TextRenderer::~TextRenderer()
{
	Unset();
}

// SetTo
void
TextRenderer::SetTo(const TextRenderer* other)
{
	Unset();

	fFontFilePath[0] = 0;
	fText = strdup(other->fText);

	fPtSize = other->fPtSize;
	fHinted = other->fHinted;
	fAntialias = other->fAntialias;
	fKerning = other->fKerning;
	fOpacity = other->fOpacity;
	fAdvanceScale = other->fAdvanceScale;
	fLineSpacingScale = other->fLineSpacingScale;
	fTextWidth = other->fTextWidth;
	fAlignment = other->fAlignment;
	fParagraphInset = other->fParagraphInset;
	fParagraphSpacing = other->fParagraphSpacing;

	SetFont(other->fFontFilePath);

	Update();
}

// Archive
status_t
TextRenderer::Archive(BMessage* into, bool deep) const
{
	status_t status = BArchivable::Archive(into, deep);

	if (status >= B_OK) {
		status = into->AddString("text", fText);
		if (status >= B_OK && Family())
			status = into->AddString("family", Family());
		else
			fprintf(stderr, "no font family to put into message!\n");
		if (status >= B_OK && Style())
			status = into->AddString("style", Style());
		else
			fprintf(stderr, "no font style to put into message!\n");
		if (status >= B_OK)
			status = into->AddFloat("size", fPtSize);

		if (status >= B_OK)
			status = into->AddBool("hinted", fHinted);
		if (status >= B_OK)
			status = into->AddBool("antialias", fAntialias);
		if (status >= B_OK)
			status = into->AddBool("kerning", fKerning);

		if (status >= B_OK)
			status = into->AddInt32("gray levels", fOpacity);
		if (status >= B_OK)
			status = into->AddFloat("advance scale", fAdvanceScale);
		if (status >= B_OK)
			status = into->AddFloat("line spacing scale", fLineSpacingScale);
		if (status >= B_OK)
			status = into->AddFloat("text width", fTextWidth);
		if (status >= B_OK)
			status = into->AddInt32("text alignment", fAlignment);
		if (status >= B_OK)
			status = into->AddFloat("paragraph inset", fParagraphInset);
		if (status >= B_OK)
			status = into->AddFloat("paragraph spacing", fParagraphSpacing);

		// finish
		if (status >= B_OK)
			status = into->AddString("class", "TextRenderer");
	}
	return status;
}

// SetText
void
TextRenderer::SetText(const char* text)
{
	if (text) {
		if (fText)
			free(fText);
		fText = strdup(text);
		Update();
	}
}

// Text
const char*
TextRenderer::Text() const
{
	return fText;
}

// SetFontRef
bool
TextRenderer::SetFontRef(const entry_ref* ref)
{
//printf("TextRenderer::SetFontRef(%s)\n", ref ? ref->name : "no ref!");
	if (ref) {
		BPath path(ref);
		if (path.InitCheck() >= B_OK)
			return SetFont(path.Path());
	}
	return false;
}

// SetFont
bool
TextRenderer::SetFont(const char* pathToFontFile)
{
//printf("TextRenderer::SetFont(%s)\n", pathToFontFile ? pathToFontFile : "no path!");
	if (pathToFontFile) {
		sprintf(fFontFilePath, "%s", pathToFontFile);
//printf("font: %s\n", fFontFilePath);
		Update();
		return true;
	}
	return false;
}

// Unset
void
TextRenderer::Unset()
{
	if (fText)
		free(fText);
	fFontFilePath[0] = 0;
}

// SetFamilyAndStyle
bool
TextRenderer::SetFamilyAndStyle(const char* family, const char* style)
{
	FontManager* fm = FontManager::Default();
	if (fm->Lock()) {
		const entry_ref* fontRef = fm->FontFileFor(family, style);
		fm->Unlock();

		return SetFontRef(fontRef);
	}
	return false;
}

// SetPointSize
void
TextRenderer::SetPointSize(float size)
{
	if (size < 0.0)
		size = 0.0;
	if (size * 16.0 > MAXPTSIZE)
		size = MAXPTSIZE / 16.0;
	if (size != fPtSize) {
		fPtSize = size;
		Update();
	}
}

// PointSize
float
TextRenderer::PointSize() const
{
	return fPtSize;
}

// SetHinting
void
TextRenderer::SetHinting(bool hinting)
{
	if (fHinted != hinting) {
		fHinted = hinting;
		Update();
	}
}

// SetAntialiasing
void
TextRenderer::SetAntialiasing(bool antialiasing)
{
	if (fAntialias != antialiasing) {
		fAntialias = antialiasing;
		Update();
	}
}

// SetKerning
void
TextRenderer::SetKerning(bool kerning)
{
	if (fKerning != kerning) {
		fKerning = kerning;
		Update();
	}
}

// SetOpacity
void
TextRenderer::SetOpacity(uint8 opacity)
{
	fOpacity = opacity;
}

// Opacity
uint8
TextRenderer::Opacity() const
{
	return fOpacity;
}

// SetAdvanceScale
void
TextRenderer::SetAdvanceScale(float scale)
{
	if (scale < 0.0)
		scale = 0.0;
	if (fAdvanceScale != scale) {
		fAdvanceScale = scale;
		Update();
	}
}

// AdvanceScale
float
TextRenderer::AdvanceScale() const
{
	return fAdvanceScale;
}

// SetLineSpacingScale
void
TextRenderer::SetLineSpacingScale(float scale)
{
	if (scale < 0.0)
		scale = 0.0;
	if (fLineSpacingScale != scale) {
		fLineSpacingScale = scale;
		Update();
	}
}

// LineSpacingScale
float
TextRenderer::LineSpacingScale() const
{
	return fLineSpacingScale;
}

// LineOffset
float
TextRenderer::LineOffset() const
{
	return fLineSpacingScale * fPtSize * 16.0;
}

// SetTextWidth
void
TextRenderer::SetTextWidth(float width)
{
	if (width < 0.0)
		width = 0.0;
	if (fTextWidth != width) {
		fTextWidth = width;
		Update();
	}
}

// SetAlignment
void
TextRenderer::SetAlignment(uint32 alignment)
{
	if (fAlignment != alignment) {
		fAlignment = alignment;
		Update();
	}
}

// SetParagraphInset
void
TextRenderer::SetParagraphInset(float inset)
{
	if (fParagraphInset != inset) {
		fParagraphInset = inset;
		Update();
	}
}

// SetParagraphSpacing
void
TextRenderer::SetParagraphSpacing(float spacing)
{
	if (spacing < 1.0)
		spacing = 1.0;
	if (spacing > 3.0)
		spacing = 3.0;
	if (fParagraphSpacing != spacing) {
		fParagraphSpacing = spacing;
		Update();
	}
}

