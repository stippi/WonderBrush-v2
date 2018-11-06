// GradientRenderer.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <Message.h>

#include <agg_alpha_mask_u8.h>
#include <agg_basics.h>
#include <agg_color_gray.h>
#include <agg_path_storage.h>
#include <agg_pixfmt_amask_adaptor.h>
#include <agg_pixfmt_gray.h>
#include <agg_pixfmt_rgba.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_span_gradient.h>
#include <agg_span_interpolator_trans.h>

#include "alpha_mask_u8.h"
#include "bitmap_support.h"

#include "CommonPropertyIDs.h"
#include "Gradient.h"
#include "Icons.h"
#include "IconProperty.h"
#include "PropertyObject.h"

#include "GradientRenderer.h"


static const int kGradientColorCount = 1024;


// constructor
GradientRenderer::GradientRenderer()
	: MaskRenderer(),
	  fGradient(new Gradient()),
	  fColors(new color_type[kGradientColorCount]),
	  fRasterizer(),
	  fScanline()
{
	_Init();
}

// constructor
GradientRenderer::GradientRenderer(const Gradient& gradient)
	: MaskRenderer(),
	  fGradient(new Gradient(gradient)),
	  fColors(new color_type[kGradientColorCount]),
	  fRasterizer(),
	  fScanline()
{
	_Init();
}

// constructor
GradientRenderer::GradientRenderer(BMessage* archive)
	: MaskRenderer(archive),
	  fGradient(new Gradient(archive)),
	  fColors(new color_type[kGradientColorCount]),
	  fRasterizer(),
	  fScanline()
{
	_Init();
}

// constructor
GradientRenderer::GradientRenderer(const GradientRenderer& other)
	: MaskRenderer(),
	  fGradient(new Gradient(*other.fGradient)),
	  fColors(new color_type[kGradientColorCount]),
	  fRasterizer(),
	  fScanline()
{
	_Init();
}

// destructor
GradientRenderer::~GradientRenderer()
{
	delete[] fColors;
	delete fGradient;
}

// Archive
status_t
GradientRenderer::Archive(BMessage* into, bool deep) const
{
	status_t ret = MaskRenderer::Archive(into, deep);
	if (ret >= B_OK)
		// store gradient
		ret = fGradient->Archive(into);

	if (ret >= B_OK)
		// finish off
		ret = into->AddString("class", "GradientRenderer");

	return ret;
}

// Instantiate
BArchivable*
GradientRenderer::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "GradientRenderer"))
		return new GradientRenderer(archive);
	return NULL;
}

// SetTo
bool
GradientRenderer::SetTo(const MaskRenderer* other)
{
	const GradientRenderer* renderer = dynamic_cast<const GradientRenderer*>(other);
	if (renderer) {
		*fGradient = *renderer->fGradient;

		return MaskRenderer::SetTo(other);
	}
	return false;
}

// Clone
MaskRenderer*
GradientRenderer::Clone() const
{
	return new GradientRenderer(*this);
}

// Render
bool
GradientRenderer::Render(BBitmap* into, BBitmap* from, BBitmap* alphaMap,
						 BRect clipping, const Transformable& parentTransform) const
{
	// the AGG backend does not support
	// rendering *and* copying at the same time
	copy_area(from, into, clipping);
	return Render(into, alphaMap, clipping, parentTransform);
}

// Render
bool
GradientRenderer::Render(BBitmap* into, BBitmap* alphaMap, BRect clipping,
						 const Transformable& parentTransform) const
{
	if (into && into->IsValid() && alphaMap && alphaMap->IsValid() &&
		clipping.IsValid() && into->Bounds().Intersects(clipping)) {
		clipping = clipping & into->Bounds();

		switch (fGradient->Type()) {
			case GRADIENT_LINEAR: {
			    agg::gradient_x function;
				return _RenderGradient(into, alphaMap, clipping, parentTransform,
									   function, -200, 200);
			}
			case GRADIENT_CIRCULAR: {
			    agg::gradient_radial function;
				return _RenderGradient(into, alphaMap, clipping, parentTransform,
									   function, 0, 200);
			}
			case GRADIENT_DIAMONT: {
			    agg::gradient_diamond function;
				return _RenderGradient(into, alphaMap, clipping, parentTransform,
									   function, 0, 200);
			}
			case GRADIENT_CONIC: {
			    agg::gradient_conic function;
				return _RenderGradient(into, alphaMap, clipping, parentTransform,
									   function, 0, 200);
			}
			case GRADIENT_XY: {
			    agg::gradient_xy function;
				return _RenderGradient(into, alphaMap, clipping, parentTransform,
									   function, 0, 200);
			}
			case GRADIENT_SQRT_XY: {
			    agg::gradient_sqrt_xy function;
				return _RenderGradient(into, alphaMap, clipping, parentTransform,
									   function, 0, 200);
			}
		}
	}
	return false;
}

// AddProperties
void
GradientRenderer::AddProperties(PropertyObject* object) const
{
	if (object) {
		// gradient
		BMessage* archivedGradient = new BMessage();
		if (fGradient->Archive(archivedGradient) >= B_OK) {
			object->AddProperty(new IconProperty("gradient",
												 PROPERTY_GRADIENT,
												 kGradientPropertyIconBits,
												 kGradientPropertyIconWidth,
												 kGradientPropertyIconHeight,
												 kGradientPropertyIconFormat,
												 archivedGradient));
		} else
			delete archivedGradient;
	}
}

// SetToPropertyObject
bool
GradientRenderer::SetToPropertyObject(PropertyObject* object)
{
	bool ret = false;
	if (object) {
		// gradient
		IconProperty* gradientProperty = dynamic_cast<IconProperty*>(object->FindProperty(PROPERTY_GRADIENT));
		if (gradientProperty && gradientProperty->Message()) {
			Gradient gradient(gradientProperty->Message());
			if (*fGradient != gradient) {
				SetGradient(gradient);
				ret = true;
			}
		}
	}
	return ret;
}

// ObjectChanged
void
GradientRenderer::ObjectChanged(const Observable* object)
{
	if (dynamic_cast<const Gradient*>(object) == fGradient) {
		fGradient->MakeGradient((uint32*)fColors, kGradientColorCount);
	}
}

// SetGradient
void
GradientRenderer::SetGradient(const Gradient& gradient)
{
	*fGradient = gradient;
}

// _Init
void
GradientRenderer::_Init()
{
	fGradient->AddObserver(this);
	fGradient->MakeGradient((uint32*)fColors, kGradientColorCount);
}

typedef agg::pixfmt_rgba32_plain							pixfmt;
//typedef agg::amask_no_clip_gray8							alpha_mask_type;
typedef agg::amask_no_clip_gray8_precise					alpha_mask_type;
typedef agg::pixfmt_amask_adaptor<pixfmt, alpha_mask_type>	pixfmt_amask_type;
typedef agg::renderer_base<pixfmt_amask_type>				amask_ren_type;
typedef agg::renderer_base<pixfmt>							base_ren_type;

// _RenderGradient
template<class GradientFunction>
bool
GradientRenderer::_RenderGradient(const BBitmap* into, const BBitmap* alphaMap,
								  const BRect& clipping, const Transformable& parentTransform,
								  GradientFunction function, int32 start, int32 stop) const
{
	// the path encloses the clipping rect
	agg::path_storage path;
	path.move_to(clipping.left, clipping.top);
	path.line_to(clipping.left, clipping.bottom + 1);
	path.line_to(clipping.right + 1, clipping.bottom + 1);
	path.line_to(clipping.right + 1, clipping.top);
	path.close_polygon();

	typedef agg::span_interpolator_trans<Transformable>		interpolator_type;
	typedef agg::pod_auto_array<agg::rgba8, kGradientColorCount> color_array_type;
	typedef agg::span_gradient<color_type, 
							   interpolator_type, 
							   GradientFunction,
							   color_array_type>			span_gradient_type;

	typedef	agg::renderer_scanline_aa<amask_ren_type,
									  span_gradient_type>	gradient_renderer_type;

	// attach an AGG rendering buffer to the alpha map
	agg::rendering_buffer alpha_mask_rbuf;
	alpha_mask_rbuf.attach((uint8*)alphaMap->Bits(),
						   alphaMap->Bounds().IntegerWidth() + 1,
						   alphaMap->Bounds().IntegerHeight() + 1,
						   alphaMap->BytesPerRow());

	alpha_mask_type alpha_mask(alpha_mask_rbuf);

	// attach an AGG rendering buffer to the dest image
	agg::rendering_buffer bitmap_rbuf;
	bitmap_rbuf.attach((uint8*)into->Bits(),
					   into->Bounds().IntegerWidth() + 1,
					   into->Bounds().IntegerHeight() + 1,
					   into->BytesPerRow());
	

	pixfmt pf(bitmap_rbuf);
	pixfmt_amask_type pfa(pf, alpha_mask);
	amask_ren_type renderer(pfa);


	Transformable gradientTransform;
	agg::span_allocator<color_type> spanAllocator;

	interpolator_type interpolator(gradientTransform);

	color_array_type array(fColors);
	span_gradient_type gradientGenerator(spanAllocator,
										 interpolator,
										 function,
										 array,
										 start, stop);

	gradient_renderer_type gradientRenderer(renderer, gradientGenerator);

	// gradient transformation
	gradientTransform = *fGradient;
	// inherit from parent if applicable
	if (fGradient->InheritTransformation())
		gradientTransform *= parentTransform;

	if (gradientTransform.IsValid()) {
	
		gradientTransform.invert();
	
		// render the path (clipping rect) with the gradient
		fRasterizer.reset();
		fRasterizer.add_path(path);
		agg::render_scanlines(fRasterizer, fScanline, gradientRenderer);
	
		return true;
	} else
		fprintf(stderr, "gradient transformation can not be inverted\n");
	return false;
}
