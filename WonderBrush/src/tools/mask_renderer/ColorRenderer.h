// ColorRenderer.h

#ifndef COLOR_RENDERER_H
#define COLOR_RENDERER_H

#include "MaskRenderer.h"

class ColorRenderer : public MaskRenderer {
 public:
								ColorRenderer();
								ColorRenderer(BMessage* archive);
								ColorRenderer(const ColorRenderer& other);
								ColorRenderer(const rgb_color& color);
	virtual						~ColorRenderer();

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

	virtual	void				AddProperties(PropertyObject* object) const;
	virtual	bool				SetToPropertyObject(PropertyObject* object);

								// ColorRenderer
			bool				SetColor(rgb_color color);
			rgb_color			Color() const
									{ return fColor; }

 private:
			rgb_color			fColor;
};

#endif // COLOR_RENDERER_H
