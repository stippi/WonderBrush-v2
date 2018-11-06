// GradientRenderer.h

#ifndef GRADIENT_RENDERER_H
#define GRADIENT_RENDERER_H

#include <agg_color_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_scanline_u.h>

#include "MaskRenderer.h"
#include "ObjectObserver.h"

class Gradient;

class GradientRenderer : public MaskRenderer,
						 public ObjectObserver {
 public:
								GradientRenderer();
								GradientRenderer(const Gradient& gradient);
								GradientRenderer(BMessage* archive);
								GradientRenderer(const GradientRenderer& other);
	virtual						~GradientRenderer();

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

								// ObjectObserver
	virtual	void				ObjectChanged(const Observable* object);

								// GradientRenderer
			void				SetGradient(const Gradient& gradient);
			Gradient*			GetGradient() const
									{ return fGradient; }

 private:
			void				_Init();

			template<class GradientFunction>
			bool				_RenderGradient(const BBitmap* into,
												const BBitmap* alphaMap,
												const BRect& clipping,
												const Transformable& parentTransform,
												GradientFunction function,
												int32 start, int32 stop) const;

	Gradient*					fGradient;

    typedef agg::rgba8						color_type;
	typedef agg::rasterizer_scanline_aa<>	rasterizer_type;
	typedef agg::scanline_u8				scanline_type;

	color_type*					fColors;
	mutable	rasterizer_type		fRasterizer;
	mutable	scanline_type		fScanline;
};

#endif // GRADIENT_RENDERER_H
