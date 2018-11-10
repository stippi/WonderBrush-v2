// BitmapStroke.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Message.h>

#include <agg_basics.h>
#include <agg_conv_clip_polygon.h>
#include <agg_conv_segmentator.h>
#include <agg_conv_transform.h>
#include <agg_path_storage.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_mclip.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_u.h>
#include <agg_span_image_filter.h>
#include <agg_span_image_filter_rgba.h>
#include <agg_span_image_resample_rgba_conv.h>
#include <agg_span_interpolator_trans.h>
#include <agg_span_interpolator_persp.h>
#include <agg_span_converter.h>
#include <agg_span_subdiv_adaptor.h>

#include "bitmap_compression.h"
#include "bitmap_support.h"
#include "blending.h"
#include "defines.h"
#include "support.h"

#include "LanguageManager.h"
#include "CommonPropertyIDs.h"
#include "IntProperty.h"
#include "OptionProperty.h"
#include "PropertyObject.h"

#include "BitmapStroke.h"

namespace agg
{
	//=============================================span_image_filter_rgba_nn_conv
	template<class ColorT,
			 class Order,
			 class Interpolator,
			 class Allocator = span_allocator<ColorT> >
	class span_image_filter_rgba_nn_conv :
	public span_image_filter<ColorT, Interpolator, Allocator>
	{
	public:
		typedef ColorT color_type;
		typedef Order order_type;
		typedef Interpolator interpolator_type;
		typedef Allocator alloc_type;
		typedef span_image_filter<color_type, interpolator_type, alloc_type> base_type;
		typedef typename color_type::value_type value_type;
		typedef typename color_type::calc_type calc_type;
		enum
		{
			base_shift = color_type::base_shift,
			base_mask  = color_type::base_mask
		};

		//--------------------------------------------------------------------
		span_image_filter_rgba_nn_conv(alloc_type& alloc)
			: base_type(alloc),
			  m_opacity(255)
		{}

		//--------------------------------------------------------------------
		span_image_filter_rgba_nn_conv(alloc_type& alloc,
								  const rendering_buffer& src,
								  const color_type& back_color,
								  interpolator_type& inter)
			: base_type(alloc, src, back_color, inter, 0),
			  m_opacity(255)
		{}

		//--------------------------------------------------------------------
		void set_opacity(int8u opacity)
		{
			m_opacity = opacity;
		}

		//--------------------------------------------------------------------
		color_type* generate(int x, int y, unsigned len)
		{
			base_type::interpolator().begin(x + base_type::filter_dx_dbl(),
											y + base_type::filter_dy_dbl(), len);

			calc_type fg[4];

			const value_type *fg_ptr;
			color_type* span = base_type::allocator().span();

			int maxx = base_type::source_image().width() - 1;
			int maxy = base_type::source_image().height() - 1;
			int t;

			do
			{
				base_type::interpolator().coordinates(&x, &y);

				x >>= image_subpixel_shift;
				y >>= image_subpixel_shift;

				if(x >= 0	&& y >= 0 &&
				   x <= maxx && y <= maxy)
				{
					fg_ptr = (const value_type*)base_type::source_image().row(y) + (x << 2);
					fg[0] = *fg_ptr++;
					fg[1] = *fg_ptr++;
					fg[2] = *fg_ptr++;
					fg[3] = *fg_ptr++;
				}
				else
				{
				   fg[order_type::R] = base_type::background_color().r;
				   fg[order_type::G] = base_type::background_color().g;
				   fg[order_type::B] = base_type::background_color().b;
				   fg[order_type::A] = base_type::background_color().a;
				}

				*span = rgba8(fg[Order::R],
							  fg[Order::G],
							  fg[Order::B],
							  fg[Order::A]).demultiply();

				if (m_opacity < 255)
					span->a = INT_MULT(span->a, m_opacity, t);

				++span;
				++base_type::interpolator();

			} while(--len);

			return base_type::allocator().span();
		}
		private:
			int8u	m_opacity;

	};


	//=======================================span_image_filter_rgba32_bilinear_conv
	template<class Order,
			 class Interpolator,
			 class Allocator = span_allocator<rgba8> >
	class span_image_filter_rgba32_bilinear_conv :
	public span_image_filter<rgba8, Interpolator, Allocator>
	{
	public:
		typedef Interpolator interpolator_type;
		typedef Allocator alloc_type;
		typedef span_image_filter<rgba8, Interpolator, alloc_type> base_type;
		typedef rgba8 color_type;

		//--------------------------------------------------------------------
		span_image_filter_rgba32_bilinear_conv(alloc_type& alloc)
			: base_type(alloc),
			  m_opacity(255)
		{}

		//--------------------------------------------------------------------
		span_image_filter_rgba32_bilinear_conv(alloc_type& alloc,
										  const rendering_buffer& src,
										  const color_type& back_color,
										  interpolator_type& inter)
			: base_type(alloc, src, back_color, inter, 0),
			  m_opacity(255)
		{}

		//--------------------------------------------------------------------
		void set_opacity(int8u opacity)
		{
			m_opacity = opacity;
		}

		//--------------------------------------------------------------------
		color_type* generate(int x, int y, unsigned len)
		{
			base_type::interpolator().begin(x + 0.5, y + 0.5, len);

			int fg[4];
			int back_r = base_type::background_color().r;
			int back_g = base_type::background_color().g;
			int back_b = base_type::background_color().b;
			int back_a = base_type::background_color().a;

			const unsigned char *fg_ptr;

			int stride = base_type::source_image().stride() - 2 * 4;
			color_type* span = base_type::allocator().span();

			int maxx = base_type::source_image().width() - 1;
			int maxy = base_type::source_image().height() - 1;
			int t;

			do
			{
				int x_hr;
				int y_hr;

				base_type::interpolator().coordinates(&x_hr, &y_hr);

				x_hr -= image_subpixel_size / 2;
				y_hr -= image_subpixel_size / 2;

				int x_lr = x_hr >> image_subpixel_shift;
				int y_lr = y_hr >> image_subpixel_shift;

				int weight;

				if(x_lr >= 0	&& y_lr >= 0 &&
				   x_lr <  maxx && y_lr <  maxy)
				{
					fg[0] =
					fg[1] =
					fg[2] =
					fg[3] = image_subpixel_size * image_subpixel_size / 2;

					x_hr &= image_subpixel_mask;
					y_hr &= image_subpixel_mask;
					fg_ptr = base_type::source_image().row(y_lr) + (x_lr << 2);

					weight = (image_subpixel_size - x_hr) *
							 (image_subpixel_size - y_hr);
					fg[0] += weight * *fg_ptr++;
					fg[1] += weight * *fg_ptr++;
					fg[2] += weight * *fg_ptr++;
					fg[3] += weight * *fg_ptr++;

					weight = x_hr * (image_subpixel_size - y_hr);
					fg[0] += weight * *fg_ptr++;
					fg[1] += weight * *fg_ptr++;
					fg[2] += weight * *fg_ptr++;
					fg[3] += weight * *fg_ptr++;

					fg_ptr += stride;

					weight = (image_subpixel_size - x_hr) * y_hr;
					fg[0] += weight * *fg_ptr++;
					fg[1] += weight * *fg_ptr++;
					fg[2] += weight * *fg_ptr++;
					fg[3] += weight * *fg_ptr++;

					weight = x_hr * y_hr;
					fg[0] += weight * *fg_ptr++;
					fg[1] += weight * *fg_ptr++;
					fg[2] += weight * *fg_ptr++;
					fg[3] += weight * *fg_ptr++;

					fg[0] >>= image_subpixel_shift * 2;
					fg[1] >>= image_subpixel_shift * 2;
					fg[2] >>= image_subpixel_shift * 2;
					fg[3] >>= image_subpixel_shift * 2;
				}
				else
				{
					if(x_lr < -1   || y_lr < -1 ||
					   x_lr > maxx || y_lr > maxy)
					{
						fg[Order::R] = back_r;
						fg[Order::G] = back_g;
						fg[Order::B] = back_b;
						fg[Order::A] = back_a;
					}
					else
					{
						fg[0] =
						fg[1] =
						fg[2] =
						fg[3] = image_subpixel_size * image_subpixel_size / 2;

						x_hr &= image_subpixel_mask;
						y_hr &= image_subpixel_mask;

						weight = (image_subpixel_size - x_hr) *
								 (image_subpixel_size - y_hr);
						if(x_lr >= 0	&& y_lr >= 0 &&
						   x_lr <= maxx && y_lr <= maxy)
						{
							fg_ptr = base_type::source_image().row(y_lr) + (x_lr << 2);
							fg[0] += weight * *fg_ptr++;
							fg[1] += weight * *fg_ptr++;
							fg[2] += weight * *fg_ptr++;
							fg[3] += weight * *fg_ptr++;
						}
						else
						{
							fg[Order::R] += back_r * weight;
							fg[Order::G] += back_g * weight;
							fg[Order::B] += back_b * weight;
							fg[Order::A] += back_a * weight;
						}

						x_lr++;

						weight = x_hr * (image_subpixel_size - y_hr);
						if(x_lr >= 0	&& y_lr >= 0 &&
						   x_lr <= maxx && y_lr <= maxy)
						{
							fg_ptr = base_type::source_image().row(y_lr) + (x_lr << 2);
							fg[0] += weight * *fg_ptr++;
							fg[1] += weight * *fg_ptr++;
							fg[2] += weight * *fg_ptr++;
							fg[3] += weight * *fg_ptr++;
						}
						else
						{
							fg[Order::R] += back_r * weight;
							fg[Order::G] += back_g * weight;
							fg[Order::B] += back_b * weight;
							fg[Order::A] += back_a * weight;
						}

						x_lr--;
						y_lr++;

						weight = (image_subpixel_size - x_hr) * y_hr;
						if(x_lr >= 0	&& y_lr >= 0 &&
						   x_lr <= maxx && y_lr <= maxy)
						{
							fg_ptr = base_type::source_image().row(y_lr) + (x_lr << 2);
							fg[0] += weight * *fg_ptr++;
							fg[1] += weight * *fg_ptr++;
							fg[2] += weight * *fg_ptr++;
							fg[3] += weight * *fg_ptr++;
						}
						else
						{
							fg[Order::R] += back_r * weight;
							fg[Order::G] += back_g * weight;
							fg[Order::B] += back_b * weight;
							fg[Order::A] += back_a * weight;
						}

						x_lr++;

						weight = x_hr * y_hr;
						if(x_lr >= 0	&& y_lr >= 0 &&
						   x_lr <= maxx && y_lr <= maxy)
						{
							fg_ptr = base_type::source_image().row(y_lr) + (x_lr << 2);
							fg[0] += weight * *fg_ptr++;
							fg[1] += weight * *fg_ptr++;
							fg[2] += weight * *fg_ptr++;
							fg[3] += weight * *fg_ptr++;
						}
						else
						{
							fg[Order::R] += back_r * weight;
							fg[Order::G] += back_g * weight;
							fg[Order::B] += back_b * weight;
							fg[Order::A] += back_a * weight;
						}

						fg[0] >>= image_subpixel_shift * 2;
						fg[1] >>= image_subpixel_shift * 2;
						fg[2] >>= image_subpixel_shift * 2;
						fg[3] >>= image_subpixel_shift * 2;
					}
				}

				*span = rgba8(fg[Order::R],
							  fg[Order::G],
							  fg[Order::B],
						 	  fg[Order::A]).demultiply();

				if (m_opacity < 255)
					span->a = INT_MULT(span->a, m_opacity, t);

				++span;
				++base_type::interpolator();

			} while(--len);

			return base_type::allocator().span();
		}
		private:
			int8u	m_opacity;
	};

}

// constructor
BitmapStroke::BitmapStroke(BBitmap* bitmap)
	: Stroke((rgb_color){ 0, 0, 0, 0 }, MODE_UNDEFINED),
	  fBitmap(bitmap),
	  fInterpolation(INTERPOLATION_BILINEAR),
	  fOpacity(255)
{
	if (fBitmap && fBitmap->IsValid()) {
		BRect b(fBitmap->Bounds());
		b.left -= 0.5;
		b.top -= 0.5;
		b.right += 1.5;
		b.bottom += 1.5;
		Touch(b);
		_Premultiply(fBitmap);
	}
}

// copy constructor
BitmapStroke::BitmapStroke(const BitmapStroke& other)
	: Stroke(other),
	  fBitmap(other.fBitmap ? new BBitmap(other.fBitmap) : NULL),
	  fInterpolation(other.fInterpolation),
	  fOpacity(other.fOpacity)
{
}

// BArchivable constructor
BitmapStroke::BitmapStroke(BMessage* archive)
	: Stroke(archive),
	  fBitmap(NULL),
	  fInterpolation(INTERPOLATION_BILINEAR),
	  fOpacity(255)
{
	if (extract_bitmap(&fBitmap, archive, "bitmap") >= B_OK) {
		if (!archive->HasBool("premultiplied"))
			_Premultiply(fBitmap);
		Reset();
		BRect b(fBitmap->Bounds());
		b.right ++;
		b.bottom ++;
		Touch(b);
	}
	if (archive) {
		if (archive->FindInt32("interpolation", (int32*)&fInterpolation) < B_OK)
			fInterpolation = INTERPOLATION_BILINEAR;
		if (archive->FindInt8("opacity", (int8*)&fOpacity) < B_OK)
			fOpacity = 255;
	}
}

// destructor
BitmapStroke::~BitmapStroke()
{
	delete fBitmap;
}

// Clone
Stroke*
BitmapStroke::Clone() const
{
	return new BitmapStroke(*this);
}

// SetTo
bool
BitmapStroke::SetTo(const Stroke* from)
{
	const BitmapStroke* bitmapStroke = dynamic_cast<const BitmapStroke*>(from);

	AutoNotificationSuspender _(this);

	if (bitmapStroke && Stroke::SetTo(from)) {
		delete fBitmap;
		fBitmap = bitmapStroke->fBitmap ? new BBitmap(bitmapStroke->fBitmap) : NULL;
		fInterpolation = bitmapStroke->fInterpolation;
		fOpacity = bitmapStroke->fOpacity;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
BitmapStroke::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "BitmapStroke"))
		return new BitmapStroke(archive);
	return NULL;
}

// Archive
status_t
BitmapStroke::Archive(BMessage* into, bool deep) const
{
	status_t status = Stroke::Archive(into, deep);

	if (fBitmap)
		status = fBitmap->InitCheck();
	else
		status = B_NO_INIT;

	// add our data to the message
	if (status >= B_OK)
		status = archive_bitmap(fBitmap, into, "bitmap");

	if (status >= B_OK)
		status = into->AddBool("premultiplied", true);

	if (status >= B_OK)
		status = into->AddInt32("interpolation", fInterpolation);

	if (status >= B_OK)
		status = into->AddInt8("opacity", fOpacity);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "BitmapStroke");

	return status;
}


// DrawLastLine
bool
BitmapStroke::DrawLastLine(BBitmap* bitmap, BRect& updateRect)
{
	return false;
}

// Draw
void
BitmapStroke::Draw(BBitmap* bitmap)
{
}

// Draw
void
BitmapStroke::Draw(BBitmap* bitmap, BRect constrainRect)
{
}

// Reset
void
BitmapStroke::Reset()
{
	// override default behaviour and
	// don't do anything
}

// IsPickable
bool
BitmapStroke::IsPickable(uint32 pickMask) const
{
	return pickMask & PICK_MASK_BITMAP;
}

// HitTest
bool
BitmapStroke::HitTest(const BPoint& where, BBitmap* alphaMap)
{
	double ltx = fBounds.left;
	double lty = fBounds.top;

	double rtx = fBounds.right;
	double rty = fBounds.top;

	double rbx = fBounds.right;
	double rby = fBounds.bottom;

	double lbx = fBounds.left;
	double lby = fBounds.bottom;

	Transform(&ltx, &lty);
	Transform(&rtx, &rty);
	Transform(&rbx, &rby);
	Transform(&lbx, &lby);

	return agg::point_in_triangle(ltx, lty, rtx, rty, lbx, lby, where.x, where.y) ||
		   agg::point_in_triangle(lbx, lby, rtx, rty, rbx, rby, where.x, where.y);
}

// HitTest
bool
BitmapStroke::HitTest(const BRect& area, BBitmap* alphaMap)
{
/*	typedef agg::conv_transform<agg::path_storage, agg::trans_affine> transformed_path;
	typedef agg::conv_segmentator<transformed_path> segmented_path;
	typedef agg::conv_clip_polygon<segmented_path> clipped_path;

	agg::path_storage boundingBox;
	boundingBox.move_to(fBounds.left, fBounds.top);
	boundingBox.line_to(fBounds.right, fBounds.top);
	boundingBox.line_to(fBounds.right, fBounds.bottom);
	boundingBox.line_to(fBounds.left, fBounds.bottom);
	boundingBox.line_to(fBounds.left, fBounds.top);

	transformed_path transformedBoundingBox(boundingBox, *this);
	segmented_path polygon(transformedBoundingBox);
	clipped_path clipped(polygon);
	clipped.clip_box(area.left, area.top, area.right, area.bottom);

	double x, y;
	return !agg::is_stop(clipped.vertex(&x, &y));*/
	return Bounds().Intersects(area);
}

typedef agg::pixfmt_bgra32_plain pixfmt;
typedef agg::renderer_mclip<pixfmt> renderer_base;
typedef agg::span_allocator<agg::rgba8> span_alloc_type;
typedef agg::span_interpolator_trans<Transformable> interpolator_type;
//typedef agg::renderer_scanline_u<renderer_base, span_gen_type> renderer_type;

// MergeWithBitmap
void
BitmapStroke::MergeWithBitmap(BBitmap* dest, BBitmap* strokeBitmap,
							  BRect area, uint32 colorSpace) const
{
	if (dest && dest->IsValid()
		&& area.IsValid() && area.Intersects(dest->Bounds())
		&& fBitmap && fBitmap->IsValid()) {
		// constrain rect to passed bitmap bounds
		area = area & dest->Bounds();
		// further constrain to our bitmap bounds
		BRect bitmapRect = fBitmap->Bounds();

		// destination rendering buffer
		agg::rendering_buffer dstBuffer;
		uint8* bits = (uint8*)dest->Bits();
		uint32 bpr = dest->BytesPerRow();
		int width = dest->Bounds().IntegerWidth() + 1;
		int height = dest->Bounds().IntegerHeight() + 1;
		dstBuffer.attach(bits, width, height, bpr);

		// source rendering buffer
		agg::rendering_buffer srcBuffer;
		bits = (uint8*)fBitmap->Bits();
		bpr = fBitmap->BytesPerRow();
		width = fBitmap->Bounds().IntegerWidth() + 1;
		height = fBitmap->Bounds().IntegerHeight() + 1;
		srcBuffer.attach(bits, width, height, bpr);

		pixfmt pixf(dstBuffer);
		renderer_base rb(pixf);

		// integer version of constrain rect
		int32 left, top, right, bottom;
		rect_to_int(area, left, top, right, bottom);
		// init clipping
		rb.reset_clipping(false);
		rb.add_clip_box(left, top, right, bottom);

		Transformable srcMatrix(*this);
		Transformable imgMatrix;
		imgMatrix *= agg::trans_affine_translation(bitmapRect.left, bitmapRect.top);
		imgMatrix *= *this;

		if (!imgMatrix.IsValid()) {
			fprintf(stderr, "image matrix cannot be inverted\n");
			return;
		}

		imgMatrix.invert();

		// path encloses image
		agg::path_storage path;
		path.move_to(bitmapRect.left - 2, bitmapRect.top - 2);
		path.line_to(bitmapRect.right + 2, bitmapRect.top - 2);
		path.line_to(bitmapRect.right + 2, bitmapRect.bottom + 2);
		path.line_to(bitmapRect.left - 2, bitmapRect.bottom + 2);
		path.line_to(bitmapRect.left - 2, bitmapRect.top - 2);

		agg::conv_transform<agg::path_storage, Transformable> tr(path, srcMatrix);

		agg::rasterizer_scanline_aa<> pf;
		pf.clip_box(left, top, right + 1, bottom + 1);
		pf.add_path(tr);

		agg::scanline_u8 sl;
		span_alloc_type sa;

		interpolator_type interpolator(imgMatrix);

		switch (fInterpolation) {
			case INTERPOLATION_RESAMPLE: {
				typedef agg::span_interpolator_persp_exact<> resample_interpolator_type;
				typedef agg::span_subdiv_adaptor<resample_interpolator_type> subdiv_adaptor_type;

				resample_interpolator_type resampleInterpolator(imgMatrix);
                subdiv_adaptor_type subdivAdaptor(resampleInterpolator);

				typedef agg::span_image_resample_rgba_conv<agg::rgba8,
														   agg::order_bgra,
														   subdiv_adaptor_type,
														   span_alloc_type> span_gen_type;
				typedef agg::renderer_scanline_aa<renderer_base, span_gen_type> renderer_type;

				agg::image_filter_hanning filterKernel;
				agg::image_filter_lut filter(filterKernel, true);

				span_gen_type sg(sa, srcBuffer, agg::rgba(0,0,0,0), subdivAdaptor, filter);
				sg.set_opacity(fOpacity);
//				sg.blur(m_blur.value());

				renderer_type ri(rb, sg);

				agg::render_scanlines(pf, sl, ri);
				break;
			}
			case INTERPOLATION_NN: {
				typedef agg::span_image_filter_rgba_nn_conv<agg::rgba8,
															agg::order_bgra,
															interpolator_type> span_gen_type;
				typedef agg::renderer_scanline_aa<renderer_base, span_gen_type> renderer_type;

				span_gen_type sg(sa, srcBuffer, agg::rgba(0, 0, 0, 0), interpolator);
				sg.set_opacity(fOpacity);

				renderer_type ri(rb, sg);

				agg::render_scanlines(pf, sl, ri);
				break;
			}
			case INTERPOLATION_BILINEAR:
			default: {
				typedef agg::span_image_filter_rgba32_bilinear_conv<agg::order_bgra,
																	interpolator_type> span_gen_type;
				typedef agg::renderer_scanline_aa<renderer_base, span_gen_type> renderer_type;

				span_gen_type sg(sa, srcBuffer, agg::rgba(0, 0, 0, 0), interpolator);
				sg.set_opacity(fOpacity);

				renderer_type ri(rb, sg);

				agg::render_scanlines(pf, sl, ri);
			}
		}
	}
}

// MergeWithBitmap
void
BitmapStroke::MergeWithBitmap(BBitmap* from, BBitmap* dest,
							  BBitmap* strokeBitmap, BRect area, uint32 colorSpace) const
{
	MergeWithBitmap(dest, strokeBitmap, area, colorSpace);
}

// MakePropertyObject
PropertyObject*
BitmapStroke::MakePropertyObject() const
{
	PropertyObject* object = new PropertyObject();

	// opacity
	object->AddProperty(new IntProperty("opacity",
										PROPERTY_OPACITY,
										fOpacity));

	// interpolation
	OptionProperty* property = new OptionProperty("interpolation",
												  PROPERTY_BITMAP_INTERPOLATION);
	LanguageManager* m = LanguageManager::Default();
	property->AddOption(INTERPOLATION_NN, m->GetString(NEAREST_NEIGHBOR, "Repeat"));
	property->AddOption(INTERPOLATION_BILINEAR, m->GetString(BILINEAR, "Bilinear"));
	property->AddOption(INTERPOLATION_RESAMPLE, m->GetString(RESAMPLE, "Resample"));
	property->SetCurrentOptionID(fInterpolation);

	object->AddProperty(property);

	return object;
}

// SetToPropertyObject
bool
BitmapStroke::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	bool ret = false;
	if (object) {
		// opacity
		int32 i = object->FindIntProperty(PROPERTY_OPACITY, fOpacity);
		if (i != fOpacity)
			ret = true;
		fOpacity = i;

		// interpolation
		OptionProperty* property = dynamic_cast<OptionProperty*>(object->FindProperty(PROPERTY_BITMAP_INTERPOLATION));
		if (property) {
			int32 interpolation = property->CurrentOptionID();
			if (fInterpolation != (uint32)interpolation) {
				fInterpolation = (uint32)interpolation;
				ret = true;
			}
		}
		if (ret) {
			Notify();
		}
	}
	return ret;
}

// Name
const char*
BitmapStroke::Name() const
{
	LanguageManager* manager = LanguageManager::Default();
	return manager->GetString(BITMAP, "Bitmap");
}

// ToolID
int32
BitmapStroke::ToolID() const
{
	return TOOL_TRANSLATE;
}

// _Premultiply
void
BitmapStroke::_Premultiply(BBitmap* bitmap)
{
	multiply_alpha(bitmap);
}

