// MaskRenderer.h

#ifndef MASK_RENDERER_H
#define MASK_RENDERER_H

#include <Archivable.h>
#include <Rect.h>

class BBitmap;
class PropertyObject;
class Transformable;

class MaskRenderer : public BArchivable {
 public:
								MaskRenderer();
								MaskRenderer(BMessage* archive);
	virtual						~MaskRenderer();

								// BArchivable
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;

								// MaskRenderer
	virtual	bool				SetTo(const MaskRenderer* other);
	virtual	MaskRenderer*		Clone() const = 0;

	virtual	bool				Render(BBitmap* into, BBitmap* from, BBitmap* alphaMap,
									   BRect clipping,
									   const Transformable& parentTransform) const;

	virtual	bool				Render(BBitmap* into, BBitmap* alphaMap,
									   BRect clipping,
									   const Transformable& parentTransform) const;

	virtual	void				AddProperties(PropertyObject* object) const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);
};

#endif // MASK_RENDERER_H
