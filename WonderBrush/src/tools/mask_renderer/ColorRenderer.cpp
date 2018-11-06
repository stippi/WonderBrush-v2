// ColorRenderer.cpp

#include <stdio.h>

#include <Message.h>

#include "bitmap_support.h"
#include "support.h"

#include "ColorProperty.h"
#include "CommonPropertyIDs.h"
#include "PropertyObject.h"

#include "ColorRenderer.h"

// constructor
ColorRenderer::ColorRenderer()
	: MaskRenderer(),
	  fColor((rgb_color){ 0, 0, 0, 255 })
{
}

// constructor
ColorRenderer::ColorRenderer(BMessage* archive)
	: MaskRenderer(archive),
	  fColor((rgb_color){ 0, 0, 0, 255 })
{
	if (archive) {
		// restore color
		if (restore_color_from_message(archive, fColor) < B_OK)
			fColor = (rgb_color){ 0, 0, 0, 255 };
	}
}

// constructor
ColorRenderer::ColorRenderer(const ColorRenderer& other)
	: MaskRenderer(),
	  fColor(other.fColor)
{
}

// constructor
ColorRenderer::ColorRenderer(const rgb_color& color)
	: MaskRenderer(),
	  fColor((rgb_color){ color.red, color.green, color.blue, color.alpha })
{
}

// destructor
ColorRenderer::~ColorRenderer()
{
}

// Archive
status_t
ColorRenderer::Archive(BMessage* into, bool deep) const
{
	status_t ret = MaskRenderer::Archive(into, deep);
	if (ret >= B_OK)
		// store color
		ret = store_color_in_message(into, fColor);

	if (ret >= B_OK)
		// finish off
		ret = into->AddString("class", "ColorRenderer");

	return ret;
}

// Instantiate
BArchivable*
ColorRenderer::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "ColorRenderer"))
		return new ColorRenderer(archive);
	return NULL;
}

// SetTo
bool
ColorRenderer::SetTo(const MaskRenderer* other)
{
	const ColorRenderer* renderer = dynamic_cast<const ColorRenderer*>(other);
	if (renderer) {
		fColor = renderer->fColor;

		return MaskRenderer::SetTo(other);
	}
	return false;
}

// Clone
MaskRenderer*
ColorRenderer::Clone() const
{
	return new ColorRenderer(*this);
}

// Render
bool
ColorRenderer::Render(BBitmap* into, BBitmap* from, BBitmap* alphaMap,
					  BRect clipping, const Transformable& parentTransform) const
{
	overlay_color_copy(from, into, alphaMap, clipping,
					   fColor.blue, fColor.green, fColor.red);
	return true;
}

// Render
bool
ColorRenderer::Render(BBitmap* into, BBitmap* alphaMap, BRect clipping,
					  const Transformable& parentTransform) const
{
	overlay_color(into, alphaMap, clipping,
				  fColor.blue, fColor.green, fColor.red);
	return true;
}

// AddProperties
void
ColorRenderer::AddProperties(PropertyObject* object) const
{
	if (object) {
		object->AddProperty(new ColorProperty("color", PROPERTY_COLOR,
											  fColor));
	}
}

// SetToPropertyObject
bool
ColorRenderer::SetToPropertyObject(PropertyObject* object)
{
	bool ret = false;
	if (object) {
		ColorProperty* colorProperty = dynamic_cast<ColorProperty*>(object->FindProperty(PROPERTY_COLOR));
		if (colorProperty) {
			ret = SetColor(colorProperty->Color());
		}
	}
	return ret;
}

// SetColor
bool
ColorRenderer::SetColor(rgb_color color)
{
	if (color.red != fColor.red ||
		color.green != fColor.green ||
		color.blue != fColor.blue ||
		color.alpha != fColor.alpha) {

		fColor = color;
		return true;
	}
	return false;
}

