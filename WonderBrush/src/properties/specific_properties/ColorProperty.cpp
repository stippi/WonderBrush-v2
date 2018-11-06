// ColorProperty.cpp

#include <Message.h>

#include "support.h"

#include "ColorValueView.h"

#include "ColorProperty.h"

// constructor
ColorProperty::ColorProperty(const char* name, int32 id,
							 rgb_color color)
	: Property(name, id),
	  fColor(color)
{
}

// archive constructor
ColorProperty::ColorProperty(BMessage* archive)
	: Property(archive)
{
	if (!archive || restore_color_from_message(archive, fColor) < B_OK) {
		fColor.red = 0;
		fColor.green = 0;
		fColor.blue = 0;
		fColor.alpha = 255;
	}
}

// destrucor
ColorProperty::~ColorProperty()
{
}

// Archive
status_t
ColorProperty::Archive(BMessage* into, bool deep) const
{
	status_t status = Property::Archive(into, deep);

	if (status >= B_OK)
		status = store_color_in_message(into, fColor);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "ColorProperty");

	return status;
}

// Instantiate
BArchivable*
ColorProperty::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "ColorProperty"))
		return new ColorProperty(archive);
	return NULL;
}

// Editor
PropertyItemValueView*		
ColorProperty::Editor()
{
	return new ColorValueView(this);
}

// SetColor
void
ColorProperty::SetColor(rgb_color color)
{
	fColor = color;
}

// Color
rgb_color
ColorProperty::Color() const
{
	return fColor;
}



