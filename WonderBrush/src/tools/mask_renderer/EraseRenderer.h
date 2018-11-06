// EraseRenderer.h

#ifndef ERASE_RENDERER_H
#define ERASE_RENDERER_H

#include "MaskRenderer.h"

class EraseRenderer : public MaskRenderer {
 public:
								EraseRenderer();
								EraseRenderer(BMessage* archive);
								EraseRenderer(const EraseRenderer& other);
	virtual						~EraseRenderer();

								// BArchivable
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

								// MaskRenderer
	virtual	bool				SetTo(const MaskRenderer* other);
	virtual	MaskRenderer*		Clone() const;

	virtual	bool				Render(BBitmap* into, BBitmap* from, BBitmap* alphaMap,
									   BRect clipping,
									   const Transformable& parentTransform) const;

	virtual	bool				Render(BBitmap* into, BBitmap* alphaMap,
									   BRect clipping,
									   const Transformable& parentTransform) const;
};

#endif // ERASE_RENDERER_H
