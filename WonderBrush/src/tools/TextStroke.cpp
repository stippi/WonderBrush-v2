// TextStroke.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Entry.h>
#include <Font.h>
#include <Message.h>

#include "blending.h"
#include "defines.h"
#include "support.h"

#include "AGGTextRenderer.h"
#include "BoolProperty.h"
#include "CommonPropertyIDs.h"
#include "FloatProperty.h"
#include "Icons.h"
#include "IconProperty.h"
#include "IntProperty.h"
#include "LanguageManager.h"
#include "OptionProperty.h"
#include "PropertyObject.h"

#include "TextStroke.h"

// constructor
TextStroke::TextStroke(rgb_color color)
	: Stroke(color, MODE_BRUSH),
	  fTextRenderer(new AGGTextRenderer())
{
	font_family family;
	font_style style;
	be_plain_font->GetFamilyAndStyle(&family, &style);
	SetFamilyAndStyle(family, style);
}

// copy constructor
TextStroke::TextStroke(const TextStroke& other)
	: Stroke(other),
	  fTextRenderer(new AGGTextRenderer())
{
	fTextRenderer->SetTo(other.fTextRenderer);
}

// BArchivable constructor
TextStroke::TextStroke(BMessage* archive)
	: Stroke(archive),
	  fTextRenderer(NULL)
{
	BMessage renderMessage;
	if (archive->FindMessage("text renderer", &renderMessage) >= B_OK) {
		fTextRenderer = new AGGTextRenderer(&renderMessage);
		// backward compatibility
		float rotation;
		if (renderMessage.FindFloat("rotation", &rotation) >= B_OK) {
			BPoint p(0.0, 0.0);
			Transform(&p);
			RotateBy(p, rotation);
		}
	} else
		fTextRenderer = new AGGTextRenderer();
}

// destructor
TextStroke::~TextStroke()
{
	delete fTextRenderer;
}

// Clone
Stroke*
TextStroke::Clone() const
{
	return new TextStroke(*this);
}

// SetTo
bool
TextStroke::SetTo(const Stroke* from)
{
	const TextStroke* textStroke = dynamic_cast<const TextStroke*>(from);

	AutoNotificationSuspender _(this);

	if (textStroke && Stroke::SetTo(from)) {
		fTextRenderer->SetTo(textStroke->fTextRenderer);
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
TextStroke::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "TextStroke"))
		return new TextStroke(archive);
	return NULL;
}

// Archive
status_t
TextStroke::Archive(BMessage* into, bool deep) const
{
	status_t status = Stroke::Archive(into, deep);

	// archive the text renderer
	BMessage renderMessage;
	if ((status = fTextRenderer->Archive(&renderMessage)) >= B_OK)
		status = into->AddMessage("text renderer", &renderMessage);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "TextStroke");

	return status;
}


// DrawLastLine
bool
TextStroke::DrawLastLine(BBitmap* bitmap, BRect& updateRect)
{
	return false;
}

// Draw
void
TextStroke::Draw(BBitmap* bitmap)
{
	Draw(bitmap, bitmap->Bounds());
}

// Draw
void
TextStroke::Draw(BBitmap* bitmap, BRect constrainRect)
{
	fTextRenderer->RenderString(bitmap, constrainRect, *this);
}

// Bounds
BRect
TextStroke::Bounds() const
{
	return fTextRenderer->Bounds(*this);
}

// Reset
void
TextStroke::Reset()
{
	// override default behavioud and
	// don't do anything
}

// Name
const char*
TextStroke::Name() const
{
	LanguageManager* manager = LanguageManager::Default();
	return manager->GetString(TEXT, "Text");
}

// MakePropertyObject
PropertyObject*
TextStroke::MakePropertyObject() const
{
	PropertyObject* object = Stroke::MakePropertyObject();
	if (object && fTextRenderer) {
		// opacity
		object->AddProperty(new IntProperty("opacity",
											PROPERTY_OPACITY,
											fTextRenderer->Opacity(),
											0, 255));
		// font
		BMessage* archivedFont = new BMessage();
		if (fTextRenderer->Family())
			archivedFont->AddString("family", fTextRenderer->Family());
		if (fTextRenderer->Style())
			archivedFont->AddString("style", fTextRenderer->Style());
		object->AddProperty(new IconProperty("font",
											 PROPERTY_FONT,
											 kFontPropertyIconBits,
											 kFontPropertyIconWidth,
											 kFontPropertyIconHeight,
											 kFontPropertyIconFormat,
											 archivedFont));
		// size
		object->AddProperty(new FloatProperty("size",
											  PROPERTY_FONT_SIZE,
											  fTextRenderer->PointSize() * 16.0,
											  0.0, 5000.0));
		// advance scale
		object->AddProperty(new FloatProperty("x scale",
											  PROPERTY_FONT_X_SCALE,
											  fTextRenderer->AdvanceScale(),
											  0.0, 2.0));

		// text
		BMessage* archivedText = new BMessage();
		if (fTextRenderer->Text())
			archivedText->AddString("text", fTextRenderer->Text());
		object->AddProperty(new IconProperty("text",
											 PROPERTY_TEXT,
											 kTextPropertyIconBits,
											 kTextPropertyIconWidth,
											 kTextPropertyIconHeight,
											 kTextPropertyIconFormat,
											 archivedText));
		// text width
		object->AddProperty(new FloatProperty("text width",
											  PROPERTY_WIDTH,
											  fTextRenderer->TextWidth(),
											  0.0, 100000.0));
		// alignment
		LanguageManager* m = LanguageManager::Default();
		OptionProperty* property = new OptionProperty("alignment",
													  PROPERTY_ALIGNMENT);
		property->AddOption(ALIGN_LEFT, m->GetString(ALIGNMENT_LEFT, "Left"));
		property->AddOption(ALIGN_CENTER, m->GetString(ALIGNMENT_CENTER, "Center"));
		property->AddOption(ALIGN_RIGHT, m->GetString(ALIGNMENT_RIGHT, "Right"));
		property->AddOption(ALIGN_JUSTIFY, m->GetString(ALIGNMENT_JUSTIFY, "Justify"));
		property->SetCurrentOptionID(fTextRenderer->Alignment());

		object->AddProperty(property);
		// line spacing scale
		object->AddProperty(new FloatProperty("line spacing scale",
											  PROPERTY_FONT_LINE_SCALE,
											  fTextRenderer->LineSpacingScale(),
											  0.0, 3.0));

		// paragraph spacing
		object->AddProperty(new FloatProperty("paragraph spacing",
											  PROPERTY_PARAGRAPH_SPACING,
											  fTextRenderer->ParagraphSpacing(),
											  1.0, 3.0));
		// paragraph inset
		object->AddProperty(new FloatProperty("paragraph inset",
											  PROPERTY_PARAGRAPH_INSET,
											  fTextRenderer->ParagraphInset(),
											  -100000.0, 100000.0));
		// hinting
		object->AddProperty(new BoolProperty("hinting",
											  PROPERTY_FONT_HINTING,
											  fTextRenderer->Hinting()));
		// kerning
		object->AddProperty(new BoolProperty("kerning",
											  PROPERTY_FONT_KERNING,
											  fTextRenderer->Kerning()));
		// antialiasing
		object->AddProperty(new BoolProperty("solid",
											  PROPERTY_NO_ANTIALIASING,
											  !fTextRenderer->Antialiasing()));

	}
	return object;
}

// SetToPropertyObject
bool
TextStroke::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	bool ret = Stroke::SetToPropertyObject(object);
	if (object && fTextRenderer) {
		// opacity
		int32 i = object->FindIntProperty(PROPERTY_OPACITY, fTextRenderer->Opacity());
		if (i != fTextRenderer->Opacity())
			ret = true;
		fTextRenderer->SetOpacity(i);
		// font
		IconProperty* fontProperty = dynamic_cast<IconProperty*>(object->FindProperty(PROPERTY_FONT));
		if (fontProperty && fontProperty->Message()) {
			const char* family;
			const char* style;
			if (fontProperty->Message()->FindString("family", &family) >= B_OK &&
				fontProperty->Message()->FindString("style", &style) >= B_OK) {
				if (!fTextRenderer->Family() || !fTextRenderer->Style() ||
					strcmp(family, fTextRenderer->Family()) != 0 ||
					strcmp(style, fTextRenderer->Style())) {
					if (fTextRenderer->SetFamilyAndStyle(family, style)) {
						ret = true;
					}
				}
			}
		}
		// size
		float f = object->FindFloatProperty(PROPERTY_FONT_SIZE, fTextRenderer->PointSize() * 16.0);
		if (f != fTextRenderer->PointSize() * 16.0)
			ret = true;
		fTextRenderer->SetPointSize(f / 16.0);

		// advance scale
		f = object->FindFloatProperty(PROPERTY_FONT_X_SCALE, fTextRenderer->AdvanceScale());
		if (f != fTextRenderer->AdvanceScale())
			ret = true;
		fTextRenderer->SetAdvanceScale(f);

		// line spacing scale
		f = object->FindFloatProperty(PROPERTY_FONT_LINE_SCALE, fTextRenderer->LineSpacingScale());
		if (f != fTextRenderer->LineSpacingScale())
			ret = true;
		fTextRenderer->SetLineSpacingScale(f);

		// font
		IconProperty* textProperty = dynamic_cast<IconProperty*>(object->FindProperty(PROPERTY_TEXT));
		if (textProperty && textProperty->Message()) {
			const char* text;
			if (textProperty->Message()->FindString("text", &text) >= B_OK) {
				if (!fTextRenderer->Text() || strcmp(text, fTextRenderer->Text()) != 0) {
					fTextRenderer->SetText(text);
					ret = true;
				}
			}
		}
		// alignment
		OptionProperty* property = dynamic_cast<OptionProperty*>(object->FindProperty(PROPERTY_ALIGNMENT));
		if (property && (int32)fTextRenderer->Alignment() != property->CurrentOptionID()) {
			fTextRenderer->SetAlignment(property->CurrentOptionID());
			ret = true;
		}
		// text width
		f = object->FindFloatProperty(PROPERTY_WIDTH, fTextRenderer->TextWidth());
		if (f != fTextRenderer->TextWidth())
			ret = true;
		fTextRenderer->SetTextWidth(f);
		// paragraph inset
		f = object->FindFloatProperty(PROPERTY_PARAGRAPH_INSET, fTextRenderer->ParagraphInset());
		if (f != fTextRenderer->ParagraphInset())
			ret = true;
		fTextRenderer->SetParagraphInset(f);
		// paragraph spacing
		f = object->FindFloatProperty(PROPERTY_PARAGRAPH_SPACING, fTextRenderer->ParagraphSpacing());
		if (f != fTextRenderer->ParagraphSpacing())
			ret = true;
		fTextRenderer->SetParagraphSpacing(f);
		// hinting
		bool b = object->FindBoolProperty(PROPERTY_FONT_HINTING, fTextRenderer->Hinting());
		if (b != fTextRenderer->Hinting())
			ret = true;
		fTextRenderer->SetHinting(b);

		// kerning
		b = object->FindBoolProperty(PROPERTY_FONT_KERNING, fTextRenderer->Kerning());
		if (b != fTextRenderer->Kerning())
			ret = true;
		fTextRenderer->SetKerning(b);

		// antialiasing
		b = object->FindBoolProperty(PROPERTY_NO_ANTIALIASING, !fTextRenderer->Antialiasing());
		if (b != !fTextRenderer->Antialiasing())
			ret = true;
		fTextRenderer->SetAntialiasing(!b);

		if (ret)
			Notify();
	}
	return ret;
}

// ToolID
int32
TextStroke::ToolID() const
{
	return TOOL_TEXT;
}

TextRenderer*
TextStroke::GetTextRenderer() const
{
	return fTextRenderer;
}

// SetFamilyAndStyle
void
TextStroke::SetFamilyAndStyle(const char* family,
							  const char* style)
{
	if (family && style) {
		if (fTextRenderer->Family() && fTextRenderer->Style()) {
			if (strcmp(fTextRenderer->Family(), family) == 0 &&
				strcmp(fTextRenderer->Style(), style) == 0)
			return;
		}
		if (fTextRenderer->SetFamilyAndStyle(family, style)) {
			Notify();
//			fBounds = fTextRenderer->Bounds(*this);
		} else {
			fprintf(stderr, "error setting family and style: %s/%s\n", family, style);
		}
	}
}

// SetText
void
TextStroke::SetText(const char* text)
{
	if (text) {
		if (fTextRenderer->Text() && strcmp(fTextRenderer->Text(), text) == 0)
			return;
		fTextRenderer->SetText(text);
		Notify();
	//	fBounds = fTextRenderer->Bounds(*this);
	}
}

// SetSize
void
TextStroke::SetSize(float size)
{
	if (fTextRenderer->PointSize() != size) {
		fTextRenderer->SetPointSize(size);
		Notify();
	//	fBounds = fTextRenderer->Bounds(*this);
	}
}

// SetOpacity
void
TextStroke::SetOpacity(uint8 opacity)
{
	if (fTextRenderer->Opacity() != opacity) {
		fTextRenderer->SetOpacity(opacity);
		Notify();
	}
}

// SetAdvanceScale
void
TextStroke::SetAdvanceScale(float scale)
{
	if (fTextRenderer->AdvanceScale() != scale) {
		fTextRenderer->SetAdvanceScale(scale);
		Notify();
	//	fBounds = fTextRenderer->Bounds(*this);
	}
}

// AdvanceScale
float
TextStroke::AdvanceScale() const
{
	return fTextRenderer->AdvanceScale();
}

// SetLineSpacingScale
void
TextStroke::SetLineSpacingScale(float scale)
{
	if (fTextRenderer->LineSpacingScale() != scale) {
		fTextRenderer->SetLineSpacingScale(scale);
		Notify();
	//	fBounds = fTextRenderer->Bounds(*this);
	}
}

// LineSpacingScale
float
TextStroke::LineSpacingScale() const
{
	return fTextRenderer->LineSpacingScale();
}

// SetTextWidth
void
TextStroke::SetTextWidth(float width)
{
	if (fTextRenderer->TextWidth() != width) {
		fTextRenderer->SetTextWidth(width);
		Notify();
	}
}

// TextWidth
float
TextStroke::TextWidth() const
{
	return fTextRenderer->TextWidth();
}

// SetAlignment
void
TextStroke::SetAlignment(uint32 alignment)
{
	if (fTextRenderer->Alignment() != alignment) {
		fTextRenderer->SetAlignment(alignment);
		Notify();
	}
}

// Alignment
uint32
TextStroke::Alignment() const
{
	return fTextRenderer->Alignment();
}

// SetParagraphInset
void
TextStroke::SetParagraphInset(float inset)
{
	if (fTextRenderer->ParagraphInset() != inset) {
		fTextRenderer->SetParagraphInset(inset);
		Notify();
	}
}

// ParagraphInset
float
TextStroke::ParagraphInset() const
{
	return fTextRenderer->ParagraphInset();
}

// SetParagraphSpacing
void
TextStroke::SetParagraphSpacing(float spacing)
{
	if (fTextRenderer->ParagraphSpacing() != spacing) {
		fTextRenderer->SetParagraphSpacing(spacing);
		Notify();
	}
}

// ParagraphSpacing
float
TextStroke::ParagraphSpacing() const
{
	return fTextRenderer->ParagraphSpacing();
}

// SetFlags
void
TextStroke::SetFlags(uint32 flags)
{
	if (Flags() != flags) {
		fTextRenderer->SetAntialiasing(!(flags & FONT_FLAG_SOLID));
		fTextRenderer->SetKerning(flags & FONT_FLAG_KERNING);
		fTextRenderer->SetHinting(flags & FONT_FLAG_HINTING);
		Notify();
	}
}

// Flags
uint32
TextStroke::Flags() const
{
	uint32 flags = 0;

	if (!fTextRenderer->Antialiasing())
		flags |= FONT_FLAG_SOLID;
	if (fTextRenderer->Kerning())
		flags |= FONT_FLAG_KERNING;
	if (fTextRenderer->Hinting())
		flags |= FONT_FLAG_HINTING;

	return flags;
}

