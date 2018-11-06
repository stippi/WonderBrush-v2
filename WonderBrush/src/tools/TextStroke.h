// TextStroke.h

#ifndef TEXT_STROKE_H
#define TEXT_STROKE_H

#include "Stroke.h"

class AGGTextRenderer;
class TextRenderer;

enum {
	FONT_FLAG_SOLID		= 1,
	FONT_FLAG_KERNING	= 1 << 1,
	FONT_FLAG_HINTING	= 1 << 2,
};

class TextStroke : public Stroke {
 public:
								TextStroke(rgb_color color);
								TextStroke(const TextStroke& other);
								TextStroke(BMessage* archive);
	virtual						~TextStroke();

	virtual	Stroke*				Clone() const;
	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

	virtual	bool				DrawLastLine(BBitmap* bitmap,
											 BRect& updateRect);
	virtual	void				Draw(BBitmap* bitmap);
	virtual	void				Draw(BBitmap* bitmap, BRect constrainRect);

	virtual	BRect				Bounds() const;
	virtual	void				Reset();

	virtual	const char*			Name() const;

	virtual	int32				ToolID() const;

	virtual	PropertyObject*		MakePropertyObject() const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

								// TextStroke
			TextRenderer*		GetTextRenderer() const;

			void				SetFamilyAndStyle(const char* family,
												  const char* style);
			void				SetText(const char* text);
			void				SetSize(float size);
			void				SetOpacity(uint8 opacity);
			void				SetAdvanceScale(float scale);
			float				AdvanceScale() const;
			void				SetLineSpacingScale(float scale);
			float				LineSpacingScale() const;

			void				SetTextWidth(float width);
			float				TextWidth() const;
			void				SetAlignment(uint32 alignment);
			uint32				Alignment() const;
			void				SetParagraphInset(float inset);
			float				ParagraphInset() const;
			void				SetParagraphSpacing(float spacing);
			float				ParagraphSpacing() const;

			void				SetFlags(uint32 flags);
			uint32				Flags() const;

 protected:
			AGGTextRenderer*		fTextRenderer;
};

#endif	// TEXT_STROKE_H
