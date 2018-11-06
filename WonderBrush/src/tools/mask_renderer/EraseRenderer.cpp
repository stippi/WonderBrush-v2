// EraseRenderer.cpp

#include <stdio.h>

#include <Message.h>

#include "bitmap_support.h"

#include "EraseRenderer.h"

// constructor
EraseRenderer::EraseRenderer()
	: MaskRenderer()
{
}

// constructor
EraseRenderer::EraseRenderer(BMessage* archive)
	: MaskRenderer(archive)
{
}

// constructor
EraseRenderer::EraseRenderer(const EraseRenderer& other)
	: MaskRenderer()
{
}

// destructor
EraseRenderer::~EraseRenderer()
{
}

// Archive
status_t
EraseRenderer::Archive(BMessage* into, bool deep) const
{
	status_t ret = MaskRenderer::Archive(into, deep);
	if (ret >= B_OK)
		// finish off
		ret = into->AddString("class", "EraseRenderer");

	return ret;
}

// Instantiate
BArchivable*
EraseRenderer::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "EraseRenderer"))
		return new EraseRenderer(archive);
	return NULL;
}

// SetTo
bool
EraseRenderer::SetTo(const MaskRenderer* other)
{
	const EraseRenderer* renderer = dynamic_cast<const EraseRenderer*>(other);
	if (renderer) {
		return MaskRenderer::SetTo(other);
	}
	return false;
}

// Clone
MaskRenderer*
EraseRenderer::Clone() const
{
	return new EraseRenderer(*this);
}

// Render
bool
EraseRenderer::Render(BBitmap* into, BBitmap* from, BBitmap* alphaMap,
					  BRect clipping, const Transformable& parentTransform) const
{
	erase_copy(from, into, alphaMap, clipping);
	return true;
}

// Render
bool
EraseRenderer::Render(BBitmap* into, BBitmap* alphaMap, BRect clipping,
					  const Transformable& parentTransform) const
{
	erase(into, alphaMap, clipping);
	return true;
}

