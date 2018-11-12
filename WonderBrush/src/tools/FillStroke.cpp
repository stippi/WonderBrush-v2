// FillStroke.cpp

#include <math.h>
#include <hash_set>
#include <stdio.h>
#include <string.h>
#include <stack>
#include <vector>

#include <Bitmap.h>
#include <Message.h>

#include "bitmap_compression.h"
#include "bitmap_support.h"
#include "blending.h"
#include "defines.h"
#include "support.h"

#include "CommonPropertyIDs.h"
#include "IntProperty.h"
#include "LanguageManager.h"
#include "PropertyObject.h"
#include "RLEBuffer.h"
#include "Stack.h"
#include "Vector.h"

#include "FillStroke.h"

#include <agg_path_storage.h>
#include <agg_trans_affine.h>
#include <agg_conv_transform.h>
#include <agg_span_interpolator_trans.h>
#include <agg_scanline_u.h>

#include <agg_basics.h>
#include <agg_pixfmt_gray.h>
#include <agg_span_image_filter.h>
#include <agg_renderer_mclip.h>
#include <agg_renderer_scanline.h>
#include <agg_rasterizer_scanline_aa.h>


namespace agg
{
	//=======================================span_image_filter_gray8_bilinear
	template<class Interpolator,
			 class Allocator = span_allocator<gray8> >
	class span_image_filter_gray8_bilinear :
	public span_image_filter<gray8, Interpolator, Allocator>
	{
		uint8	m_opacity;

	public:
		typedef Interpolator interpolator_type;
		typedef Allocator alloc_type;
		typedef span_image_filter<gray8, Interpolator, alloc_type> base_type;
		typedef gray8 color_type;

		//--------------------------------------------------------------------
		span_image_filter_gray8_bilinear(alloc_type& alloc)
			: base_type(alloc),
			  m_opacity(255)
		{}

		//--------------------------------------------------------------------
		span_image_filter_gray8_bilinear(alloc_type& alloc,
										  const rendering_buffer& src,
										  const color_type& back_color,
										  interpolator_type& inter)
			: base_type(alloc, src, back_color, inter, 0),
			  m_opacity(255)
		{}

		void set_opacity(uint8 opacity)
		{
			m_opacity = opacity;
		}

		//--------------------------------------------------------------------
		color_type* generate(int x, int y, unsigned len)
		{
			base_type::interpolator().begin(x + 0.5, y + 0.5, len);

			int fg;
			int back = base_type::background_color().v;

			const unsigned char *fg_ptr;

			int stride = base_type::source_image().stride() - 2;
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
					fg = image_subpixel_size * image_subpixel_size / 2;

					x_hr &= image_subpixel_mask;
					y_hr &= image_subpixel_mask;
					fg_ptr = base_type::source_image().row(y_lr) + (x_lr);

					weight = (image_subpixel_size - x_hr) *
							 (image_subpixel_size - y_hr);
					fg += weight * *fg_ptr++;

					weight = x_hr * (image_subpixel_size - y_hr);
					fg += weight * *fg_ptr++;

					fg_ptr += stride;

					weight = (image_subpixel_size - x_hr) * y_hr;
					fg += weight * *fg_ptr++;

					weight = x_hr * y_hr;
					fg += weight * *fg_ptr++;

					fg >>= image_subpixel_shift * 2;
				}
				else
				{
					if(x_lr < -1   || y_lr < -1 ||
					   x_lr > maxx || y_lr > maxy)
					{
						fg = back;
					}
					else
					{
						fg = image_subpixel_size * image_subpixel_size / 2;

						x_hr &= image_subpixel_mask;
						y_hr &= image_subpixel_mask;

						weight = (image_subpixel_size - x_hr) *
								 (image_subpixel_size - y_hr);
						if(x_lr >= 0	&& y_lr >= 0 &&
						   x_lr <= maxx && y_lr <= maxy)
						{
							fg_ptr = base_type::source_image().row(y_lr) + (x_lr);
							fg += weight * *fg_ptr++;
						}
						else
						{
							fg += back * weight;
						}

						x_lr++;

						weight = x_hr * (image_subpixel_size - y_hr);
						if(x_lr >= 0	&& y_lr >= 0 &&
						   x_lr <= maxx && y_lr <= maxy)
						{
							fg_ptr = base_type::source_image().row(y_lr) + (x_lr);
							fg += weight * *fg_ptr++;
						}
						else
						{
							fg += back * weight;
						}

						x_lr--;
						y_lr++;

						weight = (image_subpixel_size - x_hr) * y_hr;
						if(x_lr >= 0	&& y_lr >= 0 &&
						   x_lr <= maxx && y_lr <= maxy)
						{
							fg_ptr = base_type::source_image().row(y_lr) + (x_lr);
							fg += weight * *fg_ptr++;
						}
						else
						{
							fg += back * weight;
						}

						x_lr++;

						weight = x_hr * y_hr;
						if(x_lr >= 0	&& y_lr >= 0 &&
						   x_lr <= maxx && y_lr <= maxy)
						{
							fg_ptr = base_type::source_image().row(y_lr) + (x_lr);
							fg += weight * *fg_ptr++;
						}
						else
						{
							fg += back * weight;
						}

						fg >>= image_subpixel_shift * 2;
					}
				}

				*span = (gray8)INT_MULT(fg, m_opacity, t);
				++span;
				++base_type::interpolator();

			} while(--len);

			return base_type::allocator().span();
		}
	};

}


class FillPoint {
 public:
	int32 x;
	int32 y;
	FillPoint(int32 _x, int32 _y)
		: x(_x),
		  y(_y)
	{
	}
	~FillPoint() {}
	FillPoint& operator+=(const FillPoint& other)
	{
		x += other.x;
		y += other.y;
		return *this;
	}
	FillPoint operator+(const FillPoint& other)
	{
		FillPoint result(*this);
		return result += other;
	}
};

struct PointHash {
	int operator()(const FillPoint& point) const
	{
		return (point.x << 16) ^ point.y;
	}
};

struct PointEqual {
	bool operator()(const FillPoint& a, const FillPoint& b) const
	{
		return (a.x == b.x) && (a.y == b.y);
	}
};


class Filler {
 public:
 				Filler(BBitmap* bitmap, BBitmap* strokeBitmap,
 					   BPoint startPoint, uint8 tolerance, uint8 softness)
 				: fSrcBits((uint8*)bitmap->Bits()),
 				  fSrcBPR(bitmap->BytesPerRow()),
 				  fDestBits((uint8*)strokeBitmap->Bits()),
 				  fDestBPR(strokeBitmap->BytesPerRow()),
 				  fTolerance(tolerance),
 				  fSoftness(softness)
 				{
 					if (bitmap->Bounds().Contains(startPoint)) {
						uint8* src = fSrcBits + (int32)startPoint.x * 4
									 + (int32)startPoint.y * fSrcBPR;
	 					fFillColor.alpha = src[3];
	 					fFillColor.red = src[2];
	 					fFillColor.green = src[1];
	 					fFillColor.blue = src[0];
 					}
 				}
				~Filler() {}

	bool		IsPixelFilled(const FillPoint& point) const
				{
					uint8* dst = fDestBits + point.x + point.y * fDestBPR;
					return (*dst > 0);
				}

	uint8		FillLevel(const FillPoint& point) const
				{
					uint8* src = fSrcBits + point.x * 4 + point.y * fSrcBPR;

					if (fFillColor.alpha == 0) {
						// the point the user clicked on is totally transparent,
						// we cannot compare to the color then
						if (fTolerance == 0) {
							if (fFillColor.alpha == src[3]) {
								return (uint8)255;
							}
							return (uint8)0;
						}
						int32 value = (int32)fFillColor.alpha - src[3];
						if (value < 0)
							value = -value;
						if (value <= fTolerance) {
							return (uint8)(255 - value * fSoftness / fTolerance);
						}
						return (uint8)0;
					} else {
						if (fTolerance == 0) {
							if (fFillColor.blue == src[0]
								&& fFillColor.green == src[1]
								&& fFillColor.red == src[2]
								&& fFillColor.alpha == src[3]) {
								return (uint8)255;
							}
							return (uint8)0;
						}
						int32 blueDiff = (int32)fFillColor.blue - src[0];
						int32 greenDiff = (int32)fFillColor.green - src[1];
						int32 redDiff = (int32)fFillColor.red - src[2];
						int32 alphaDiff = (int32)fFillColor.alpha - src[3];
						int32 alphaProduct = (int32)fFillColor.alpha * src[3];

						int32 colorDist = blueDiff * blueDiff + greenDiff * greenDiff + redDiff * redDiff;
						colorDist = (int32)sqrt(colorDist / 3);
						int32 alphaDist = (int32)sqrt(alphaDiff * alphaDiff);

						int32 value = colorDist * alphaProduct + alphaDist * (65025 - alphaProduct);

						value /= 65025;

						if (value <= fTolerance) {
							return (uint8)(255 - value * fSoftness / fTolerance);
						}
					}
					return (uint8)0;
				}

	bool		FillPixel(const FillPoint& point) const
				{
					uint8 fillLevel = FillLevel(point);
					if (fillLevel > 0) {
						uint8* dst = fDestBits + point.x + point.y * fDestBPR;
						*dst = fillLevel;
						return true;
					}
					return false;
				}

 private:
 	uint8*		fSrcBits;
 	uint32		fSrcBPR;
 	uint8*		fDestBits;
 	uint32		fDestBPR;
 	rgb_color	fFillColor;
 	uint8		fTolerance;
 	uint8		fSoftness;
};


const FillPoint kNeighbors4[] = { FillPoint(-1, 0),
								  FillPoint(0, -1),
								  FillPoint(1, 0),
								  FillPoint(0, 1), };

const FillPoint kNeighbors8[] = { FillPoint(-1, 0),
								  FillPoint(-1, -1),
								  FillPoint(0, -1),
								  FillPoint(1, -1),
								  FillPoint(1, 0),
								  FillPoint(1, 1),
								  FillPoint(0, 1),
								  FillPoint(-1, 1), };
/*
// fill
bool
fill(const Filler* filler, FillPoint startPoint,
	 FillPoint leftTop, FillPoint rightBottom,
	 const FillPoint* neighbors, int32 neighborCount,
	 BRect& filledArea)
{
	int32 left = leftTop.x;
	int32 top = leftTop.y;
	int32 right = rightBottom.x;
	int32 bottom = rightBottom.y;

	if (startPoint.x >= left && startPoint.x <= right
		&& startPoint.y >= top && startPoint.y <= bottom) {

		vector<FillPoint> border;

		border.push_back(startPoint);
		filler->FillPixel(startPoint);
		int32 filledLeft = startPoint.x;
		int32 filledRight = startPoint.x;
		int32 filledTop = startPoint.y;
		int32 filledBottom = startPoint.y;

		while (!border.empty()) {
			FillPoint p = border[border.size() - 1];
			border.pop_back();
			for (int32 i = 0; i < neighborCount; i++) {
				FillPoint neighbor = p + neighbors[i];

				if (neighbor.x >= left && neighbor.x <= right
					&& neighbor.y >= top && neighbor.y <= bottom) {
					if (!filler->IsPixelFilled(neighbor)) {
						if (filler->FillPixel(neighbor)) {
							border.push_back(neighbor);
							// keep track of filled area
							if (neighbor.x < filledLeft)
								filledLeft = neighbor.x;
							if (neighbor.x > filledRight)
								filledRight = neighbor.x;
							if (neighbor.y < filledTop)
								filledTop = neighbor.y;
							if (neighbor.y > filledBottom)
								filledBottom = neighbor.y;
						}
					}
				}
			}
		}
		filledArea.Set(filledLeft, filledTop, filledRight, filledBottom);

		return true;
	}
	return false;
}

// fill2
bool
fill2(const Filler* filler, FillPoint startPoint,
	 FillPoint leftTop, FillPoint rightBottom,
	 const FillPoint* neighbors, int32 neighborCount,
	 BRect& filledArea)
{
	int32 left = leftTop.x;
	int32 top = leftTop.y;
	int32 right = rightBottom.x;
	int32 bottom = rightBottom.y;

	if (startPoint.x >= left && startPoint.x <= right
		&& startPoint.y >= top && startPoint.y <= bottom) {

		stack<FillPoint> points;

		points.push(startPoint);
		int32 filledLeft = startPoint.x;
		int32 filledRight = startPoint.x;
		int32 filledTop = startPoint.y;
		int32 filledBottom = startPoint.y;

		while (!points.empty()) {

			FillPoint p = points.top();
			points.pop();
			if (filler->FillPixel(p)) {

				if (filledTop > p.y)
					filledTop = p.y;
				if (filledBottom < p.y)
					filledBottom = p.y;

				// search neighbors right
				FillPoint neighbor(p.x + 1, p.y);
				while (neighbor.x <= right && filler->FillPixel(neighbor)) {
					if (filledLeft > neighbor.x)
						filledLeft = neighbor.x;
					if (filledRight < neighbor.x)
						filledRight = neighbor.x;

					neighbor.x++;
				}
				int32 maxX = neighbor.x - 1;

				// search neighbors left
				neighbor.x = p.x - 1;
				while (neighbor.x >= left && filler->FillPixel(neighbor)) {
					if (filledLeft > neighbor.x)
						filledLeft = neighbor.x;
					if (filledRight < neighbor.x)
						filledRight = neighbor.x;

					neighbor.x--;
				}
				int32 minX = neighbor.x + 1;

				// search top and bottom adjacent lines in the range minX -> maxX
				bool addedLastAbove = false;
				bool addedLastBelow = false;
				for (int32 x = minX; x <= maxX; x++) {
					// above
					if (p.y - 1 >= top) {
						FillPoint above(x, p.y - 1);

						if (!filler->IsPixelFilled(above) && filler->FillLevel(above) > 0) {
							if (!addedLastAbove) {
								points.push(above);
								addedLastAbove = true;
							}
						} else {
							addedLastAbove = false;
						}
					}
					// below
					if (p.y + 1 <= bottom) {
						FillPoint below(x, p.y + 1);

						if (!filler->IsPixelFilled(below) && filler->FillLevel(below) > 0) {
							if (!addedLastBelow) {
								points.push(below);
								addedLastBelow = true;
							}
						} else {
							addedLastBelow = false;
						}
					}
				}
			}
		}
		filledArea.Set(filledLeft, filledTop, filledRight, filledBottom);

		return true;
	}
	return false;
}
*/
// fill_contiguous
bool
fill_contiguous(const Filler* filler, FillPoint startPoint,
				 FillPoint leftTop, FillPoint rightBottom, BRect& filledArea)
{
	int32 left = leftTop.x;
	int32 top = leftTop.y;
	int32 right = rightBottom.x;
	int32 bottom = rightBottom.y;

	if (startPoint.x >= left && startPoint.x <= right
		&& startPoint.y >= top && startPoint.y <= bottom) {

//		stack<FillPoint*> points;
		Stack<FillPoint*, false> points;

//		points.push(new FillPoint(startPoint.x, startPoint.y));
		points.Push(new FillPoint(startPoint.x, startPoint.y));
		int32 filledLeft = startPoint.x;
		int32 filledRight = startPoint.x;
		int32 filledTop = startPoint.y;
		int32 filledBottom = startPoint.y;

		while (!points.IsEmpty()) {

			FillPoint* p = points.Top();
			points.Pop();
			if (filler->FillPixel(*p)) {

				if (filledTop > p->y)
					filledTop = p->y;
				if (filledBottom < p->y)
					filledBottom = p->y;

				// search neighbors right
				FillPoint neighbor(p->x + 1, p->y);
				while (neighbor.x <= right && filler->FillPixel(neighbor)) {
					if (filledLeft > neighbor.x)
						filledLeft = neighbor.x;
					if (filledRight < neighbor.x)
						filledRight = neighbor.x;

					neighbor.x++;
				}
				int32 maxX = neighbor.x - 1;

				// search neighbors left
				neighbor.x = p->x - 1;
				while (neighbor.x >= left && filler->FillPixel(neighbor)) {
					if (filledLeft > neighbor.x)
						filledLeft = neighbor.x;
					if (filledRight < neighbor.x)
						filledRight = neighbor.x;

					neighbor.x--;
				}
				int32 minX = neighbor.x + 1;

				// search top and bottom adjacent lines in the range minX -> maxX
				bool addedLastAbove = false;
				bool addedLastBelow = false;
				for (int32 x = minX; x <= maxX; x++) {
					// above
					if (p->y - 1 >= top) {
						FillPoint above(x, p->y - 1);

						if (!filler->IsPixelFilled(above) && filler->FillLevel(above) > 0) {
							if (!addedLastAbove) {
								points.Push(new FillPoint(above.x, above.y));
								addedLastAbove = true;
							}
						} else {
							addedLastAbove = false;
						}
					}
					// below
					if (p->y + 1 <= bottom) {
						FillPoint below(x, p->y + 1);

						if (!filler->IsPixelFilled(below) && filler->FillLevel(below) > 0) {
							if (!addedLastBelow) {
								points.Push(new FillPoint(below.x, below.y));
								addedLastBelow = true;
							}
						} else {
							addedLastBelow = false;
						}
					}
				}
			}
			delete p;
		}
		filledArea.Set(filledLeft, filledTop, filledRight, filledBottom);

		return true;
	}
	return false;
}

// fill
bool
fill(const Filler* filler, FillPoint startPoint,
	 FillPoint leftTop, FillPoint rightBottom, BRect& filledArea)
{
	int32 left = leftTop.x;
	int32 top = leftTop.y;
	int32 right = rightBottom.x;
	int32 bottom = rightBottom.y;

	if (startPoint.x >= left && startPoint.x <= right
		&& startPoint.y >= top && startPoint.y <= bottom) {

		int32 filledLeft = right;
		int32 filledRight = left;
		int32 filledTop = bottom;
		int32 filledBottom = top;

		FillPoint p(top, left);
		for (; p.y <= bottom; p.y++) {
			for (p.x = left; p.x <= right; p.x++) {
				if (filler->FillPixel(p)) {
					if (filledLeft > p.x)
						filledLeft = p.x;
					if (filledRight < p.x)
						filledRight = p.x;
					if (filledTop > p.y)
						filledTop = p.y;
					if (filledBottom < p.y)
						filledBottom = p.y;
				}
			}
		}

		filledArea.Set(filledLeft, filledTop, filledRight, filledBottom);

		return true;
	}
	return false;
}

// constructor
FillStroke::FillStroke(BPoint fillStart,
					   rgb_color fillColor,
					   uint8 opacity,
					   uint8 tolerance,
					   uint8 softness)
	: Stroke(fillColor, MODE_BRUSH),
	  fFillStart(fillStart),
	  fOpacity(opacity),
	  fTolerance(tolerance),
	  fSoftness(softness),
	  fBitmap(NULL)
{
}

// BArchivable constructor
FillStroke::FillStroke(BMessage* archive)
	: Stroke(archive),
	  fFillStart(-1.0, -1.0),
	  fOpacity(255),
	  fTolerance(0),
	  fSoftness(0),
	  fBitmap(NULL)
{
	if (archive) {

		int32 value;
		if (archive->FindInt32("opacity", &value) >= B_OK)
			fOpacity = (uint8)value;
		if (archive->FindInt32("tolerance", &value) >= B_OK)
			fTolerance = (uint8)value;
		if (archive->FindInt32("softness", &value) >= B_OK)
			fSoftness = (uint8)value;

		if (archive->FindPoint("fill start", &fFillStart) < B_OK)
			fFillStart = BPoint(-1.0, -1.0);

		// backward compatibility (find cache)
		BMessage bufferArchive;
		if (archive->FindMessage("cache", &bufferArchive) >= B_OK) {
			RLEBuffer buffer(&bufferArchive);
			fBitmap = new BBitmap(buffer.Bounds(), 0, B_GRAY8);
			buffer.Fill(fBitmap, 0, 0, fBitmap->Bounds());
		} else {
			status_t ret = extract_bitmap(&fBitmap, archive, "fillmap");
			if (ret < B_OK)
				fprintf(stderr, "FillStroke(BMessage*) - no fillmap found: %s\n", strerror(ret));
		}
	}
}

// copy constructor
FillStroke::FillStroke(const FillStroke& other)
	: Stroke(other),
	  fFillStart(other.fFillStart),
	  fOpacity(other.fOpacity),
	  fTolerance(other.fTolerance),
	  fSoftness(other.fSoftness),
	  fBitmap(other.fBitmap ? new BBitmap(other.fBitmap) : NULL)
{
}

// destructor
FillStroke::~FillStroke()
{
}

// Clone
Stroke*
FillStroke::Clone() const
{
	return new FillStroke(*this);
}

// SetTo
bool
FillStroke::SetTo(const Stroke* from)
{
	const FillStroke* fillStroke = dynamic_cast<const FillStroke*>(from);

	AutoNotificationSuspender _(this);

	if (fillStroke && Stroke::SetTo(from)) {
		fOpacity = fillStroke->fOpacity;
		fFillStart = fillStroke->fFillStart;
		fTolerance = fillStroke->fTolerance;
		fSoftness = fillStroke->fSoftness;
		delete fBitmap;
		fBitmap = fillStroke->fBitmap ? new BBitmap(fillStroke->fBitmap) : NULL;
		Notify();
		return true;
	}
	return false;
}

// Instantiate
BArchivable*
FillStroke::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "FillStroke"))
		return new FillStroke(archive);
	return NULL;
}

// Archive
status_t
FillStroke::Archive(BMessage* into, bool deep) const
{
	status_t status = Stroke::Archive(into, deep);

	if (status >= B_OK)
		status = into->AddInt32("opacity", (int32)fOpacity);
	if (status >= B_OK)
		status = into->AddInt32("tolerance", (int32)fTolerance);
	if (status >= B_OK)
		status = into->AddInt32("softness", (int32)fSoftness);
	if (status >= B_OK)
		status = into->AddPoint("fill start", fFillStart);
	if (status >= B_OK)
		status = archive_bitmap(fBitmap, into, "fillmap");

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "FillStroke");

	return status;
}

// Finish
void
FillStroke::Finish(BBitmap* bitmap)
{
	// never reset the contents of the cache
	if (!fBitmap) {
		fBitmap = cropped_bitmap(bitmap, fBounds);
		fBounds.right ++;
		fBounds.bottom ++;
	}
}

// DrawLastLine
bool
FillStroke::DrawLastLine(BBitmap* bitmap, BRect& updateRect)
{
	return false;
}

// Draw
void
FillStroke::Draw(BBitmap* bitmap)
{
	Draw(bitmap, bitmap->Bounds());
}

typedef agg::pixfmt_gray8											pixfmt;
typedef agg::renderer_mclip<pixfmt>									renderer_base;
typedef agg::span_allocator<agg::gray8>								span_alloc_type;
typedef agg::span_interpolator_trans<Transformable>					interpolator_type;
typedef agg::span_image_filter_gray8_bilinear<interpolator_type>	span_gen_type;
typedef agg::renderer_scanline_aa<renderer_base, span_gen_type>		renderer_type;
//typedef agg::renderer_scanline_u<renderer_base, span_gen_type>		renderer_type;

// Draw
void
FillStroke::Draw(BBitmap* bitmap, BRect constrainRect)
{
	if (bitmap && bitmap->IsValid()
		&& constrainRect.IsValid() && constrainRect.Intersects(bitmap->Bounds())
		&& fBitmap && fBitmap->IsValid()) {
		// constrain rect to passed bitmap bounds
		constrainRect = constrainRect & bitmap->Bounds();
		// further constrain to our bitmap bounds
		BRect bitmapRect = fBitmap->Bounds();

		// destination rendering buffer
		agg::rendering_buffer dstBuffer;
		uint8* bits = (uint8*)bitmap->Bits();
		uint32 bpr = bitmap->BytesPerRow();
		int width = bitmap->Bounds().IntegerWidth() + 1;
		int height = bitmap->Bounds().IntegerHeight() + 1;
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
		rect_to_int(constrainRect, left, top, right, bottom);
		// init clipping
		rb.reset_clipping(false);
		rb.add_clip_box(left, top, right, bottom);

		Transformable srcMatrix(*this);
		Transformable imgMatrix;
		imgMatrix *= agg::trans_affine_translation(bitmapRect.left, bitmapRect.top);
		imgMatrix *= *this;

		if (!imgMatrix.IsValid()) {
			fprintf(stderr, "fill matrix cannot be inverted\n");
			return;
		}

		imgMatrix.invert();

		span_alloc_type sa;
		interpolator_type interpolator(imgMatrix);

		span_gen_type sg(sa, srcBuffer, agg::gray8(0), interpolator);
		sg.set_opacity(fOpacity);

		renderer_type ri(rb, sg);

		agg::rasterizer_scanline_aa<> pf;
		pf.clip_box(left, top, right + 1, bottom + 1);
		agg::scanline_u8 sl;

		// path encloses image
		agg::path_storage path;
		path.move_to(bitmapRect.left - 1, bitmapRect.top - 1);
		path.line_to(bitmapRect.right + 2, bitmapRect.top - 1);
		path.line_to(bitmapRect.right + 2, bitmapRect.bottom + 2);
		path.line_to(bitmapRect.left - 1, bitmapRect.bottom + 2);
		path.line_to(bitmapRect.left - 1, bitmapRect.top - 1);

		agg::conv_transform<agg::path_storage, Transformable> tr(path, srcMatrix);

		pf.add_path(tr);
		agg::render_scanlines(pf, sl, ri);
	}
}

// Reset
void
FillStroke::Reset()
{
	// don't do anything (thereby override default behaviour)
}

// Name
const char*
FillStroke::Name() const
{
	LanguageManager* manager = LanguageManager::Default();
	return manager->GetString(BUCKET_FILL, "Bucket Fill");
}

// ToolID
int32
FillStroke::ToolID() const
{
	return TOOL_BUCKET_FILL;
}


// MakePropertyObject
PropertyObject*
FillStroke::MakePropertyObject() const
{
	PropertyObject* object = Stroke::MakePropertyObject();
	if (object) {
		// opacity
		object->AddProperty(new IntProperty("opacity",
											PROPERTY_OPACITY,
											fOpacity));
	}
	return object;
}

// SetToPropertyObject
bool
FillStroke::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	bool ret = Stroke::SetToPropertyObject(object);
	if (object) {
		// opacity
		int32 i = object->FindIntProperty(PROPERTY_OPACITY, fOpacity);
		if (i != fOpacity)
			ret = true;
		fOpacity = i;

		if (ret) {
			Notify();
		}
	}
	return ret;
}

// Fill
bool
FillStroke::Fill(BBitmap* bitmap, BBitmap* strokeBitmap, bool contiguous)
{
	if (strokeBitmap && strokeBitmap->IsValid() && bitmap && bitmap->IsValid()) {
		clear_area(strokeBitmap, strokeBitmap->Bounds());

		Filler filler(bitmap, strokeBitmap, fFillStart, fTolerance, fSoftness);

		BRect r = strokeBitmap->Bounds();
		FillPoint startPoint((int32)floorf(fFillStart.x),
							 (int32)floorf(fFillStart.y));
		FillPoint leftTop((int32)floorf(r.left),
						  (int32)floorf(r.top));
		FillPoint rightBottom((int32)floorf(r.right),
							  (int32)floorf(r.bottom));
//		if (fill(&filler, startPoint, leftTop, rightBottom, kNeighbors4, 4, fBounds)) {
//		if (fill2(&filler, startPoint, leftTop, rightBottom, kNeighbors4, 4, fBounds)) {
		if (contiguous) {
			if (fill_contiguous(&filler, startPoint, leftTop, rightBottom, fBounds)) {
				Finish(strokeBitmap);
				return true;
			}
		} else {
			if (fill(&filler, startPoint, leftTop, rightBottom, fBounds)) {
				Finish(strokeBitmap);
				return true;
			}
		}
	}
	return false;
}


