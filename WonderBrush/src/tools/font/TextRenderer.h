// TextRenderer.h

#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include "Transformable.h"

struct entry_ref;

class BBitmap;
class ShapeStroke;

class TextRenderer : public BArchivable {
 public:
								TextRenderer();
								TextRenderer(BMessage* archive);
								TextRenderer(const TextRenderer& from);
	virtual						~TextRenderer();

	virtual	void				SetTo(const TextRenderer* other);

	virtual	status_t			Archive(BMessage* into, bool deep = true) const;

			void				SetText(const char* text);
			const char*			Text() const;

			bool				SetFontRef(const entry_ref* ref);
	virtual	bool				SetFont(const char* pathToFontFile);
	virtual	void				Unset();

			bool				SetFamilyAndStyle(const char* family,
												  const char* style);
	virtual	const char*			Family() const = 0;
	virtual	const char*			Style() const = 0;
	virtual	const char*			PostScriptName() const = 0;

	virtual	void				SetPointSize(float size);
			float				PointSize() const;

	virtual	void				SetHinting(bool hinting);
			bool				Hinting() const
									{ return fHinted; }

	virtual	void				SetKerning(bool kerning);
			bool				Kerning() const
									{ return fKerning; }

	virtual	void				SetAntialiasing(bool antialiasing);
			bool				Antialiasing() const
									{ return fAntialias; }

	virtual	void				SetOpacity(uint8 opacity);
			uint8				Opacity() const;

	virtual	void				SetAdvanceScale(float scale);
			float				AdvanceScale() const;

	virtual	void				SetLineSpacingScale(float scale);
			float				LineSpacingScale() const;

	virtual	void				RenderString(BBitmap* bitmap,
											 const BRect& constrainRect,
											 const Transformable& transform) = 0;

	virtual	BRect				Bounds(const Transformable& transform) const = 0;

	virtual	void				Update() {};

	virtual	float				LineOffset() const;

	virtual	void				GetPaths(ShapeStroke* object) = 0;

			void				SetTextWidth(float width);
			float				TextWidth() const
									{ return fTextWidth; }

			void				SetAlignment(uint32 alignment);
			uint32				Alignment() const
									{ return fAlignment; }

			void				SetParagraphInset(float inset);
			float				ParagraphInset() const
									{ return fParagraphInset; }

			void				SetParagraphSpacing(float spacing);
			float				ParagraphSpacing() const
									{ return fParagraphSpacing; }

 protected:
// TODO: replace fText with BString
			char*				fText;
			char				fFontFilePath[B_PATH_NAME_LENGTH];
		
			float				fPtSize;
		
			bool				fHinted;			// is glyph hinting active?
			bool				fAntialias;			// is anti-aliasing active?
			bool				fKerning;
			uint8				fOpacity;
			float				fAdvanceScale;
			float				fLineSpacingScale;

			float				fTextWidth;
			uint32				fAlignment;
			float				fParagraphInset;
			float				fParagraphSpacing;
};

#endif // FT_TEXT_RENDER_H
