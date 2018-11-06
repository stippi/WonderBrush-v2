// AGGTextRenderer.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <ByteOrder.h>
#include <Entry.h>
#include <Message.h>
#include <UTF8.h>

#include <agg_basics.h>
#include <agg_bounding_rect.h>
#include <agg_conv_segmentator.h>
#include <agg_conv_transform.h>
#include <agg_path_storage.h>
#include <agg_scanline_u.h>
#include <agg_scanline_bin.h>
#include <agg_renderer_mclip.h>
#include <agg_renderer_scanline.h>
#include <agg_renderer_primitives.h>
#include <agg_rendering_buffer.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_pixfmt_gray.h>

#include "support.h"

#include "FontManager.h"
#include "ShapeStroke.h"
#include "VectorPath.h"

#include "AGGTextRenderer.h"

// word
struct AGGTextRenderer::word {
	word(double x)
		: x_offset(x),
		  length(0.0),
		  offsets(NULL),
		  indices(NULL),
		  logical_count(0),
		  physical_count(0)
	{
	}
	~word()
	{
		free((void*)offsets);
		free((void*)indices);
	}
	void add(uint16 index, double width)
	{
		set_count(logical_count + 1);
		offsets[logical_count - 1] = width;
		indices[logical_count - 1] = index;
		length += width;
	}
	double end()
	{
		return x_offset + length;
	}
	void set_count(int32 count)
	{
		if (count > physical_count) {
			physical_count = ((count + 15) / 16) * 16;
			if (offsets)
				offsets = (double*)realloc((void*)offsets, physical_count * sizeof(double));
			else
				offsets = (double*)malloc(physical_count * sizeof(double));
			if (indices)
				indices = (uint16*)realloc((void*)indices, physical_count * sizeof(uint16));
			else
				indices = (uint16*)malloc(physical_count * sizeof(uint16));
		}
		logical_count = count;
	}
	void print_to_stream() const
	{
		printf("    offset: %.2f\n", x_offset);
		for (int32 i = 0; i < logical_count; i++) {
			printf("     width: %.2f\n", offsets[i]);
		}
	}

	double		x_offset;
	double		length;
	double*		offsets;
	uint16*		indices;
	int32		logical_count;
	int32		physical_count;
};

// line
struct AGGTextRenderer::line {
	line(double y)
		: y_offset(y),
		  words(NULL),
		  logical_count(0),
		  physical_count(0),
		  terminated(false)
	{
	}
	~line()
	{
		for (int32 i = 0; i < logical_count; i++)
			delete words[i];
		free((void*)words);
	}
	void add(word* w)
	{
		set_count(logical_count + 1);
		words[logical_count - 1] = w;
	}
	word* remove_last()
	{
		word* w = NULL;
		if (logical_count > 0) {
			w = words[logical_count - 1];
			set_count(logical_count - 1);
		}
		return w;
	}
	double length()
	{
		double l = 0.0;
		if (logical_count > 0)
			l = words[logical_count - 1]->x_offset + words[logical_count - 1]->length;
		return l;
	}
	void set_count(int32 count)
	{
		if (count > physical_count) {
			physical_count = ((count + 15) / 16) * 16;
			if (words)
				words = (word**)realloc((void*)words, physical_count * sizeof(word*));
			else
				words = (word**)malloc(physical_count * sizeof(word*));
		}
		logical_count = count;
	}
	void justify(uint32 alignment, double maxWidth, bool hinted)
	{
		if (logical_count >= 1) {
			switch (alignment) {
				case ALIGN_CENTER:
				case ALIGN_RIGHT: {
					double start = words[0]->x_offset;
					double end = maxWidth - words[logical_count - 1]->end();
					double shift = end - start;
					if (alignment == ALIGN_CENTER)
						shift /= 2.0;
					if (hinted)
						shift = floorf(shift + 0.5);
					for (int32 i = 0; i < logical_count; i++) {
						words[i]->x_offset += shift;
					}
					break;
				}
				case ALIGN_JUSTIFY:
					if (!terminated) {
						double room = maxWidth - words[logical_count - 1]->end();
						if (logical_count > 1) {
							// more than one word
							if (room > maxWidth * 0.125) {
								// we have so much room, that it will look better to
								// add spacing between glyphs as well, the glyph<->space
								// ration is not perfect, but pretty good for now
								double glyphSpaceRatio = 1.0 / (double)(logical_count - 1);
								double glyphRoom = (room - maxWidth * 0.125) * glyphSpaceRatio;
								double spacingRoom = room - glyphRoom;
								spacingRoom /= (double)(logical_count - 1);
								glyphRoom /= (double)(count_glyphs() - logical_count);
								double glyphSpaceAdded = 0.0;
								for (int32 i = 0; i < logical_count; i++) {
									if (i > 0) {
										// add spacing before word
										words[i]->x_offset += glyphSpaceAdded + i * spacingRoom;
										if (hinted)
											words[i]->x_offset = floorf(words[i]->x_offset + (1.0 / (logical_count - 1) * i));
									}
									// add spacing to glyphs
									if (words[i]->logical_count > 1) {
										for (int32 c = 0; c < words[i]->logical_count - 1; c++) {
											words[i]->offsets[c] += glyphRoom;
											glyphSpaceAdded += glyphRoom;
											if (hinted)
												words[i]->offsets[c] = floorf(words[i]->offsets[c] + (1.0 / (words[i]->logical_count - 1) * c));
										}
									}
								}
							} else {
								// add room between words only
								room /= (double)(logical_count - 1);
								for (int32 i = 1; i < logical_count; i++) {
									words[i]->x_offset += i * room;
									if (hinted)
										words[i]->x_offset = floorf(words[i]->x_offset + (1.0 / (logical_count - 1) * i));
								}
							}
						} else {
							// only one word, add room between glyphs
							if (room > 0.0) {
								word* w = words[0];
								if (w->logical_count > 1) {
									room /= (double)(w->logical_count - 1);
									for (int32 i = 0; i < w->logical_count - 1; i++) {
										w->offsets[i] += room;
										if (hinted)
											w->offsets[i] = floorf(w->offsets[i] + (1.0 / (w->logical_count - 1) * i));
									}
								}
							}
						}
					}
					break;
			}
		}
	}
	uint32 count_glyphs() const
	{
		uint32 count = 0;
		for (int32 i = 0; i < logical_count; i++) {
			count += words[i]->logical_count;
		}
		return count;
	}
	void print_to_stream() const
	{
		for (int32 i = 0; i < logical_count; i++) {
			printf("  word %ld\n", i);
			words[i]->print_to_stream();
		}
		if (terminated)
			printf("->return\n");
	}

	double		y_offset;
	word**		words;
	int32		logical_count;
	int32		physical_count;
	bool		terminated;
};

// constructor
AGGTextRenderer::AGGTextRenderer()
	: TextRenderer(),
	  fFontEngine(),
	  fFontManager(fFontEngine),
	  fCurves(fFontManager.path_adaptor()),
	  fContour(fCurves),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fNeedsLayout(true),
	  fLines(4)
{
	fCurves.approximation_scale(2.0);
	fContour.auto_detect_orientation(false);
	fFontEngine.flip_y(false);
}

AGGTextRenderer::AGGTextRenderer(BMessage* archive)
	: TextRenderer(archive),
	  fFontEngine(),
	  fFontManager(fFontEngine),
	  fCurves(fFontManager.path_adaptor()),
	  fContour(fCurves),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fNeedsLayout(true),
	  fLines(4)
{
//printf("AGGTextRenderer::AGGTextRenderer(BMessage*)\n");
	fCurves.approximation_scale(2.0);
	fContour.auto_detect_orientation(false);
	fFontEngine.flip_y(false);

	if (fFontFilePath)
		SetFont(fFontFilePath);
}

// constructor
AGGTextRenderer::AGGTextRenderer(const AGGTextRenderer& from)
	: TextRenderer(from),
	  fFontEngine(),
	  fFontManager(fFontEngine),
	  fCurves(fFontManager.path_adaptor()),
	  fContour(fCurves),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fNeedsLayout(true),
	  fLines(4)
{
	fCurves.approximation_scale(2.0);
	fContour.auto_detect_orientation(false);
	fFontEngine.flip_y(false);

	if (fFontFilePath)
		SetFont(fFontFilePath);
}

// destructor
AGGTextRenderer::~AGGTextRenderer()
{
	Unset();
}

// SetTo
void
AGGTextRenderer::SetTo(const TextRenderer* other)
{
	const AGGTextRenderer* casted = dynamic_cast<const AGGTextRenderer*>(other);
	if (casted) {
		fBounds = casted->fBounds;
		TextRenderer::SetTo(other);
	}
}

// Archive
status_t
AGGTextRenderer::Archive(BMessage* into, bool deep) const
{
	status_t status = TextRenderer::Archive(into, deep);
	if (status >= B_OK) {
		status = into->AddString("class", "AGGTextRenderer");
	}
	return status;
}

// SetFont
bool
AGGTextRenderer::SetFont(const char* pathToFontFile)
{
	if (pathToFontFile) {
		if (fFontEngine.load_font(pathToFontFile, 0, agg::glyph_ren_outline)) {
			
			return TextRenderer::SetFont(pathToFontFile);
		} else {
			fprintf(stderr, "%s : is not a font file or could not be opened\n",
					pathToFontFile);
		}
	}
	return false;
}

// Unset
void
AGGTextRenderer::Unset()
{
}

// Family
const char*
AGGTextRenderer::Family() const
{
	const char* family = NULL;
	if (fFontFilePath) {
		entry_ref ref;
		if (get_ref_for_path(fFontFilePath, &ref) >= B_OK) {
			FontManager* fm = FontManager::Default();
			if (fm->Lock()) {
				family = fm->FamilyFor(&ref);
				fm->Unlock();
			}
		}
	}
	return family;
}

// Style
const char*
AGGTextRenderer::Style() const
{
	const char* style = NULL;
	if (fFontFilePath) {
		entry_ref ref;
		if (get_ref_for_path(fFontFilePath, &ref) >= B_OK) {
			FontManager* fm = FontManager::Default();
			if (fm->Lock()) {
				style = fm->StyleFor(&ref);
				fm->Unlock();
			}
		}
	}
	return style;
}

// PostScriptName
const char*
AGGTextRenderer::PostScriptName() const
{
	const char* name = NULL;
	if (fFontFilePath) {
		entry_ref ref;
		if (get_ref_for_path(fFontFilePath, &ref) >= B_OK) {
			FontManager* fm = FontManager::Default();
			if (fm->Lock()) {
				name = fm->PostScriptNameFor(&ref);
				fm->Unlock();
			}
		}
	}
	return name;
}

typedef agg::pixfmt_gray8									pixfmt_type;
typedef agg::renderer_mclip<pixfmt_type>					base_ren_type;
typedef agg::renderer_scanline_aa_solid<base_ren_type>		renderer_solid;
typedef agg::scanline_u8									scanline_type;
typedef agg::rasterizer_scanline_aa<>						rasterizer_type;


class Renderer {
	public:
		Renderer(const BRect& constrainRect,
				 rasterizer_type& rasterizer,
				 scanline_type& scanline,
				 renderer_solid& renderer,
				 uint8 opacity,
				 bool antialias,
				 float scale)
			: fConstrainRect(constrainRect),
			  fRasterizer(rasterizer),
			  fScanline(scanline),
			  fRenderer(renderer),
			  fOpacity(opacity),
			  fAntialias(antialias),
			  fScale(scale)
		{
			// convert constrain rect from pixel index into vector format
			fConstrainRect.right++;
			fConstrainRect.bottom++;
			fRasterizer.clip_box(fConstrainRect.left, fConstrainRect.top,
								 fConstrainRect.right, fConstrainRect.bottom);
		}

		template<class VertexSource>
		void AddGlyph(VertexSource& glyph)
		{
			double left = 0.0;
			double top = 0.0;
			double right = -1.0;
			double bottom = -1.0;
			uint32 pathID[1];
			pathID[0] = 0;
			agg::bounding_rect(glyph, pathID, 0, 1, &left, &top, &right, &bottom);
			BRect glyphsBounds(left, top, right, bottom);
			if (glyphsBounds.IsValid() && fConstrainRect.Intersects(glyphsBounds)) {
				agg::conv_curve<VertexSource> segmentedGlyph(glyph);
				segmentedGlyph.approximation_scale(fScale);
	
				fRasterizer.reset();
				fRasterizer.add_path(segmentedGlyph);
				if (!fAntialias) {
					fRasterizer.gamma(agg::gamma_threshold(0.6));
				}
				fRenderer.color(agg::gray8(fOpacity));
				agg::render_scanlines(fRasterizer, fScanline, fRenderer);
			}
		}

		void Finish()
		{
		}

	private:
		BRect				fConstrainRect;
		rasterizer_type&	fRasterizer;
		scanline_type&		fScanline;
		renderer_solid&		fRenderer;
		uint8				fOpacity;
		bool				fAntialias;
		float				fScale;
};

class Updater {
	public:
		Updater()
			: fBounds(0.0, 0.0, -1.0, -1.0)
		{
		}

		template<class VertexSource>
		void AddGlyph(VertexSource& glyph)
		{
			double left = 0.0;
			double top = 0.0;
			double right = -1.0;
			double bottom = -1.0;
			uint32 pathID[1];
			pathID[0] = 0;
			agg::bounding_rect(glyph, pathID, 0, 1, &left, &top, &right, &bottom);
			BRect glyphsBounds(left, top, right, bottom);
			if (glyphsBounds.IsValid())
				fBounds = fBounds.IsValid() ? fBounds | glyphsBounds : glyphsBounds;
		}

		void Finish()
		{
		}

		BRect Bounds() const
		{
			return fBounds;
		}

	private:
		BRect	fBounds;
};

class ShapeConverter {
	public:
		ShapeConverter(ShapeStroke* object)
			: fShapeObject(object)
		{
		}

		template<class VertexSource>
		void AddGlyph(VertexSource& glyph)
		{
			fAllGlyphsPaths.add_path(glyph, 0, false);
		}

		void Finish()
		{
			fShapeObject->AddVertexSource(fAllGlyphsPaths, 0);
		}

	private:
		ShapeStroke*		fShapeObject;
		agg::path_storage	fAllGlyphsPaths;
};


// RenderString
void
AGGTextRenderer::RenderString(BBitmap* bitmap,
							  const BRect& constrainRect,
							  const Transformable& transform)
{
	// set up the renderer
	agg::rendering_buffer ren_buffer;
	ren_buffer.attach((uint8*)bitmap->Bits(),
					  bitmap->Bounds().IntegerWidth() + 1,
					  bitmap->Bounds().IntegerHeight() + 1,
					  bitmap->BytesPerRow());

	pixfmt_type pf(ren_buffer);
	base_ren_type ren_base(pf);
	renderer_solid ren_solid(ren_base);
	
	// integer version of constrain rect
	int32 left, top, right, bottom;
	rect_to_int(constrainRect, left, top, right, bottom);
	// init clipping
	ren_base.reset_clipping(false);
	ren_base.add_clip_box(left, top, right, bottom);

	scanline_type scanline;
	rasterizer_type rasterizer;
	rasterizer.clip_box(left, top, right + 1, bottom + 1);

	Renderer renderer(constrainRect, rasterizer, scanline, ren_solid,
					  fOpacity, fAntialias, transform.scale());
	_LayoutGlyphs(renderer, transform);
}

// Bounds
BRect
AGGTextRenderer::Bounds(const Transformable& transform) const
{
	
	return transform.TransformBounds(fBounds);
}

// Update
void
AGGTextRenderer::Update()
{
	fFontEngine.hinting(fHinted);
	fFontEngine.height((int32)(fPtSize * 16.0));

	Transformable identity;

	// init fBounds and layout glyphs
	fNeedsLayout = true;

	Updater updater;
	_LayoutGlyphs(updater, identity);

	fBounds = updater.Bounds();
}

// GetPaths
void
AGGTextRenderer::GetPaths(ShapeStroke* object)
{
	Transformable identity;

	ShapeConverter converter(object);
	_LayoutGlyphs(converter, identity);
}

#define TIMING 0

// _LayoutGlyphs
template<class GlyphConsumer>
void
AGGTextRenderer::_LayoutGlyphs(GlyphConsumer& consumer, Transformable transform)
{
	// do a UTF8 -> Unicode conversion
	const char* string = Text();
	if (!string)
		return;

#if TIMING
bigtime_t now = system_time();
#endif

	if (fNeedsLayout) {
		// the cached layout is invalid

		// free previous lines
		for (int32 i = 0; line* l = fLines.ItemAt(i); i++)
			delete l;
		fLines.MakeEmpty();

		// convert UTF text to "Unicode" for use as FreeType glyph indices
		int32 srcLength = strlen(string);
		int32 dstLength = srcLength * 4;
	
		char* buffer = new char[dstLength];
	
		int32 state = 0;
		status_t ret;
		if ((ret = convert_from_utf8(B_UNICODE_CONVERSION, 
									 string, &srcLength,
									 buffer, &dstLength,
									 &state, B_SUBSTITUTE)) >= B_OK
			&& (ret = swap_data(B_INT16_TYPE, buffer, dstLength,
								B_SWAP_BENDIAN_TO_HOST)) >= B_OK) {
	
			uint16* p = (uint16*)buffer;

			uint16 lastIndex = 0;
			double x0 = fHinted ? floorf(fParagraphInset + 0.5) : fParagraphInset;
			double x  = x0;
			double y0 = 0.0;
			double y  = y0;
	
			double advanceX = 0.0;
			double advanceY = 0.0;
			word* currentWord = NULL;
			line* currentLine = new line(y0);

			fLines.AddItem(currentLine);
	
			for (int32 i = 0; i < dstLength / 2; i++) {

				// separate words
				if (*p == '\n') {
					// line break
					if (currentWord) {
						// this means the last glyphs was not a ' '
						// -> so we add the width of that glyph
						currentWord->add(lastIndex, advanceX);
						currentLine->add(currentWord);
						currentWord = NULL;
					}
					y0 += fHinted ? ceilf(LineOffset() * fParagraphSpacing)
								  : LineOffset() * fParagraphSpacing;
					x = x0;
					y = y0;
					advanceX = 0.0;
					advanceY = 0.0;
					++p;
					currentLine->terminated = true;
					// start a new line
					currentLine = new line(y0);
					fLines.AddItem(currentLine);
					continue;
				}

				const agg::glyph_cache* glyph = fFontManager.glyph(*p);
				if (glyph) {
					// calculate the advance offset
					if (fKerning)
						fFontManager.add_kerning(&advanceX, &advanceY);
	
					double totalAdvanceX = fAdvanceScale * advanceX;
					if (advanceX > 0.0 && fAdvanceScale > 1.0)
						totalAdvanceX += (fAdvanceScale - 1.0) * fFontEngine.height();

					if (fHinted)
						totalAdvanceX = floorf(totalAdvanceX + 0.5);

					x += totalAdvanceX;
					y += advanceY;
	
					if (*p == ' ') {
						if (currentWord) {
							// means the last glyph was not a ' '
							// -> so we add the width of that glyph
							currentWord->add(lastIndex, totalAdvanceX);
							// test for soft wrapping
							if (fTextWidth > 0.0 && currentWord->end() > fTextWidth &&
								currentLine->logical_count > 0) {
								y0 += fHinted ? ceilf(LineOffset()) : LineOffset();
								x = currentWord->length;
								y = y0;
								currentWord->x_offset = 0.0;
								currentLine = new line(y0);
								fLines.AddItem(currentLine);
							}
							currentLine->add(currentWord);
							currentWord = NULL;
						}
					} else {
						if (!currentWord) {
							// means the last glyph was a ' '
							// or this is the first glyph of a new line
							currentWord = new word(x);
						} else {
							// means the last glyph was not a ' '
							// -> so we add the width of that glyph
							currentWord->add(lastIndex, totalAdvanceX);
							// test for soft wrapping
							if (fTextWidth > 0.0 && currentWord->end() > fTextWidth &&
								currentLine->logical_count > 0) {
								y0 += fHinted ? ceilf(LineOffset()) : LineOffset();
								x = currentWord->length;
								y = y0;
								currentWord->x_offset = 0.0;
								currentLine = new line(y0);
								fLines.AddItem(currentLine);
							}
						}
					}
					// increment pen position
					advanceX = glyph->advance_x;
					advanceY = glyph->advance_y;
				}
				lastIndex = *p;
				++p;
			}
			if (currentWord) {
				// means the last glyph was not a ' '
				// -> so we add the width of that glyph
				currentWord->add(lastIndex, advanceX);
				// test for soft wrapping
				if (fTextWidth > 0.0 && currentWord->end() > fTextWidth &&
					currentLine->logical_count > 0) {
					y0 += fHinted ? ceilf(LineOffset()) : LineOffset();
					x = currentWord->length;
					y = y0;
					currentWord->x_offset = 0.0;
					currentLine = new line(y0);
					fLines.AddItem(currentLine);
				}
				currentLine->add(currentWord);
			}
			// the last line is always terminated
			currentLine->terminated = true;

			// post process lines for "center", "right" and "justify" alignment
			if (fAlignment != ALIGN_LEFT) {
				for (int32 i = 0; line* l = fLines.ItemAt(i); i++) {
					l->justify(fAlignment, fTextWidth, fHinted);
				}
			}
//for (int32 i = 0; line* l = fLines.ItemAt(i); i++) {
//	printf("line %ld - (y = %.2f):\n", i, l->y_offset);
//	l->print_to_stream();
//}
		} else {
			fprintf(stderr, "UTF8 -> Unicode conversion failed: %s\n", strerror(ret));
		}
		fNeedsLayout = false;

		delete[] buffer;
	}

#if TIMING
bigtime_t layout = system_time() - now;
#endif

	// process glyphs with cached layout

	// prepare an AGG pipeline for the transformation of the glyph outline
	Transformable currentTransform;
	typedef agg::conv_transform<font_manager_type::path_adaptor_type,
								Transformable>					conv_font_trans_type;
	conv_font_trans_type ftrans(fFontManager.path_adaptor(), currentTransform);

	if (fHinted && transform.IsTranslationOnly()) {
		BPoint p(0.0, 0.0);
		transform.Transform(&p);
		p.x = roundf(p.x) - p.x;
		p.y = roundf(p.y) - p.y;
		transform.TranslateBy(p);
	}

	// lines
	for (int32 i = 0; line* l = fLines.ItemAt(i); i++) {
		double y = l->y_offset;
		// words
		for (int32 j = 0; j < l->logical_count; j++) {
			word* currentWord = l->words[j];
			// offset of first glyph in word
			double x = currentWord->x_offset;
			// glyphs
			for (int32 c = 0; c < currentWord->logical_count; c++) {
				// retrieve glyph with cached index
				const agg::glyph_cache* glyph = fFontManager.glyph(currentWord->indices[c]);
				if (glyph) {
					// offset glyph in untransformed (font) space
					fFontManager.init_embedded_adaptors(glyph, x, y);
	
					// flip current transformation on current baseline
					currentTransform.Reset();
					currentTransform.ScaleBy(BPoint(0.0, y), 1.0, -1.0);

					// apply canvas transformation
					currentTransform.Multiply(transform);
	
					consumer.AddGlyph(ftrans);

					// advance to next glyph
					x += currentWord->offsets[c];
				}
			}
		}
	}

#if TIMING
printf("layout: %lldµs, rendering: %lldµs\n", layout, (system_time() - now) - layout);
#endif

	consumer.Finish();
}
