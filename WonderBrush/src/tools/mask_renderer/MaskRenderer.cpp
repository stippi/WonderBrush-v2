// MaskRenderer.cpp

#include <Message.h>

#include "MaskRenderer.h"

// constructor
MaskRenderer::MaskRenderer()
	: BArchivable()
{
}

// constructor
MaskRenderer::MaskRenderer(BMessage* archive)
	: BArchivable(archive)
{
}

// destructor
MaskRenderer::~MaskRenderer()
{
}

// Archive
status_t
MaskRenderer::Archive(BMessage* into, bool deep) const
{
	status_t ret = BArchivable::Archive(into, deep);
	if (ret >= B_OK) {
		ret = into->AddString("class", "MaskRenderer");
	}
	return ret;
}

// SetTo
bool
MaskRenderer::SetTo(const MaskRenderer* other)
{
	if (other) {
		return true;
	}
	return false;
}

// Render
bool
MaskRenderer::Render(BBitmap* into, BBitmap* from, BBitmap* alphaMap,
					 BRect clipping, const Transformable& parentTransform) const
{
	return false;
}

// Render
bool
MaskRenderer::Render(BBitmap* into, BBitmap* alphaMap,
					 BRect clipping, const Transformable& parentTransform) const
{
	return false;
}

// AddProperties
void
MaskRenderer::AddProperties(PropertyObject* object) const
{
}

// SetToPropertyObject
bool
MaskRenderer::SetToPropertyObject(PropertyObject* object)
{
	return false;
}
