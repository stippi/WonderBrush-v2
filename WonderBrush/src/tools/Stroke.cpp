// Stroke.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <ClassInfo.h>
#include <Message.h>
#include <TypeConstants.h>
#include <View.h>

#include "bitmap_support.h"
#include "defines.h"
#include "support.h"

#include "ColorProperty.h"
#include "ColorRenderer.h"
#include "CommonPropertyIDs.h"
#include "EraseRenderer.h"
#include "Gradient.h"
#include "GradientRenderer.h"
#include "HistoryListView.h"
#include "IntProperty.h"
#include "LanguageManager.h"
#include "OptionProperty.h"
#include "PropertyObject.h"
#include "RLEBuffer.h"

#include "Stroke.h"

// ColorObjectItemPainter
class ColorObjectItemPainter : public ObjectItemPainter {
 public:
					ColorObjectItemPainter(const char* name,
										   rgb_color color)
						: ObjectItemPainter(name),
						  fColor(color)
					{}
	virtual			~ColorObjectItemPainter()
					{}

	virtual	void	PaintObjectItem(BView* owner,
									BRect itemFrame,
									uint32 flags)
					{
						ObjectItemPainter::PaintObjectItem(owner, itemFrame, flags);

						itemFrame.InsetBy(2.0, 2.0);
						itemFrame.left = itemFrame.right - itemFrame.Height();
						owner->SetHighColor(tint_color(owner->LowColor(),
											B_DARKEN_1_TINT));
						owner->StrokeRect(itemFrame);
						itemFrame.InsetBy(1.0, 1.0);
						owner->SetHighColor(fColor);
						owner->FillRect(itemFrame);
					}
 private:
	rgb_color		fColor;
};

// GradientObjectItemPainter
class GradientObjectItemPainter : public ObjectItemPainter {
 public:
					GradientObjectItemPainter(const char* name, const Gradient* gradient)
						: ObjectItemPainter(name),
						  fBitmap(new BBitmap(BRect(0.0, 0.0, 7.0, 7.0), 0, B_RGB32))
					{
						// fill in top row by gradient
						uint8* topRow = (uint8*)fBitmap->Bits();
						uint32 width = fBitmap->Bounds().IntegerWidth() + 1;
						gradient->MakeGradient((uint32*)topRow, width);
						// copy top row to rest of bitmap
						uint32 height = fBitmap->Bounds().IntegerHeight() + 1;
						uint32 bpr = fBitmap->BytesPerRow();
						uint8* dstRow = topRow + bpr;
						for (uint32 i = 1; i < height; i++) {
							memcpy(dstRow, topRow, bpr);
							dstRow += bpr;
						}
					}
	virtual			~GradientObjectItemPainter()
					{
						delete fBitmap;
					}

	virtual	void	PaintObjectItem(BView* owner,
									BRect itemFrame,
									uint32 flags)
					{
						ObjectItemPainter::PaintObjectItem(owner, itemFrame, flags);

						itemFrame.InsetBy(2.0, 2.0);
						itemFrame.left = itemFrame.right - itemFrame.Height();
						owner->SetHighColor(tint_color(owner->LowColor(),
											B_DARKEN_1_TINT));
						owner->StrokeRect(itemFrame);
						itemFrame.InsetBy(1.0, 1.0);
						owner->DrawBitmap(fBitmap, fBitmap->Bounds(), itemFrame);
					}
 private:
	BBitmap*			fBitmap;
};


// constructor
Stroke::Stroke(rgb_color color, uint32 mode)
//	: Transformable(),
	: ArchivableTransformable(),
	  fTrackPoints(1024),
	  fColor(color),
	  fGradient(NULL),
	  fMode(mode),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fCache(NULL),
	  fRenderer(NULL)
{
	_InitCacheMatrix();
	SetRenderer(DefaultRenderer());
}

// constructor
Stroke::Stroke(const Stroke& other)
//	: Transformable(other),
	: ArchivableTransformable(other),
	  fTrackPoints(1024),
	  fColor(other.fColor),
	  fGradient(other.fGradient ? new Gradient(*other.fGradient) : NULL),
	  fMode(other.fMode),
	  fBounds(other.fBounds),
	  fCache(other.fCache ? new RLEBuffer(*other.fCache) : NULL),
	  fRenderer(other.fRenderer ? other.fRenderer->Clone() : NULL)
{
	SetTrackPoints(other.fTrackPoints);
	memcpy(fCacheMatrix, other.fCacheMatrix, 9 * sizeof(double));
}

// BArchivable constructor
Stroke::Stroke(BMessage* archive)
//	: Transformable(archive),
	: ArchivableTransformable(archive),
	  fTrackPoints(1024),
	  fColor((rgb_color){ 0, 0, 0, 255 }),
	  fGradient(NULL),
	  fMode(MODE_BRUSH),
	  fBounds(0.0, 0.0, -1.0, -1.0),
	  fCache(NULL),
	  fRenderer(NULL)
{
	if (archive) {
		// restore color
		if (restore_color_from_message(archive, fColor) < B_OK)
			fColor = (rgb_color){ 0, 0, 0, 255 };

		BMessage gradientArchive;
		if (archive->FindMessage("gradient", &gradientArchive) >= B_OK) {
			fGradient = new Gradient(&gradientArchive);
		}

		// restore renderer
		BMessage rendererArchive;
		if (archive->FindMessage("renderer", &rendererArchive) >= B_OK) {
			if (BArchivable* archivable = instantiate_object(&rendererArchive)) {
				// test against the various stroke classes
				MaskRenderer* renderer = cast_as(archivable, MaskRenderer);
				if (renderer)
					SetRenderer(renderer);
				else
					delete archivable;
			}
		}
		// restore mode
		if (archive->FindInt32("mode", (int32*)&fMode) < B_OK)
			fMode = 0;
		// restore bounds
		if (archive->FindRect("bounds", &fBounds) < B_OK)
			fBounds.Set(0.0, 0.0, -1.0, -1.0);
		// restore offset (backward compatibility)
		BPoint offset;
		if (archive->FindPoint("offset", &offset) >= B_OK)
			TranslateBy(offset);
		// restore track points
		const void* data;
		ssize_t size = sizeof(Point);
		for (int32 i = 0; archive->FindData("points", B_RAW_TYPE, i,
											&data, &size) == B_OK; i++) {
			Point* p = (Point*)data;
			// make a copy of the point from the archive
			Point* clone = new Point(*p);
			// don't leak in case of failure
			if (!fTrackPoints.AddItem((void*)clone))
				delete clone;
		}
		// restore cache
		BMessage bufferArchive;
		if (archive->FindMessage("cache", &bufferArchive) >= B_OK) {
			fCache = new RLEBuffer(&bufferArchive);
		}
		ssize_t matrixSize = 9 * sizeof(double);
		if (archive->FindData("cache matrix", B_DOUBLE_TYPE,
							  (const void**)&fCacheMatrix, &matrixSize) < B_OK
			|| matrixSize != 9 * sizeof(double)) {
			_InitCacheMatrix();
		}
	} else {
		_InitCacheMatrix();
	}
	// make sure we have a valid renderer
	if (!fRenderer)
		SetRenderer(DefaultRenderer());
}

// destructor
Stroke::~Stroke()
{
	_MakeEmpty();
	delete fGradient;
	delete fRenderer;
}

// SetTo
bool
Stroke::SetTo(const Stroke* from)
{
	if (from) {
		SetTrackPoints(from->fTrackPoints);
		fColor = from->fColor;
		fMode = from->fMode;

		delete fRenderer;
		if (from->fRenderer) {
			fRenderer = from->fRenderer->Clone();
		} else {
			fRenderer = NULL;
		}

		delete fGradient;
		if (from->fGradient) {
			fGradient = new Gradient(*from->fGradient);
		} else {
			fGradient = NULL;
		}

		fBounds = from->fBounds;
		SetTransformable(*from);

		delete fCache;
		if (from->fCache) {
			fCache = new RLEBuffer(*from->fCache);
		} else {
			fCache = NULL;
		}

		Notify();
		return true;
	}
	return false;
}

// Archive
status_t
Stroke::Archive(BMessage* into, bool deep) const
{
	// store transformation
//	status_t status = Transformable::Archive(into, deep);
	status_t status = ArchivableTransformable::Archive(into, deep);

	// add our data to the message
	if (status >= B_OK)
		// store color
		status = store_color_in_message(into, fColor);

	if (status >= B_OK && fGradient) {
		// update our internal gradient
		if (Gradient* gradient = GetGradient())
			*fGradient = *gradient;
		// store gradient
		BMessage archive;
		status = fGradient->Archive(&archive);
		if (status >= B_OK)
			status = into->AddMessage("gradient", &archive);
	}
	if (status >= B_OK && fRenderer) {
		// store renderer
		BMessage archive;
		status = fRenderer->Archive(&archive);
		if (status >= B_OK)
			status = into->AddMessage("renderer", &archive);
	}
	// store mode
	if (status >= B_OK)
		status = into->AddInt32("mode", (int32)fMode);
	// store bounds
	if (status >= B_OK)
		status = into->AddRect("bounds", fBounds);
	// store track points
	if (status >= B_OK) {
		int32 count = fTrackPoints.CountItems();
		if (count > 0) {
			ssize_t size = sizeof(Point);
			Point* p = (Point*)fTrackPoints.ItemAt(0);
			// add first point with number of points for speed
			status = into->AddData("points", B_RAW_TYPE, p, size, true, count);
			for (int32 i = 1; (p = (Point*)fTrackPoints.ItemAt(i)) && status >= B_OK; i++)
				status = into->AddData("points", B_RAW_TYPE, p, size);
		}
	}
	// store cache data
	if (IsCacheValid() && fCache && status >= B_OK) {
		BMessage bufferArchive;
		status = fCache->Archive(&bufferArchive);
		if (status >= B_OK) {
			status = into->AddMessage("cache", &bufferArchive);
		}
		status = into->AddData("cache matrix", B_DOUBLE_TYPE,
							   fCacheMatrix, 9 * sizeof(double));
	}

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "Stroke");

	return status;
}

// AddPoint
bool
Stroke::AddPoint(Point point)
{
	bool success = false;
	if (point.pressure > 0.0) {
		// transform point to "center" of pixel
		point.point.x += 0.5;
		point.point.y += 0.5;

		InverseTransform(&point.point);

		Point previous;
		if (GetLastPoint(previous)) {
			previous.point.x += 0.5;
			previous.point.y += 0.5;
			if (previous.point == point.point && previous.pressure == point.pressure) {
				return false;
			}
		}

		Point* clone = new Point(point);
		success = fTrackPoints.AddItem((void*)clone);
		if (!success)
			delete clone;
	}
	return success;
}

// GetPointAt
bool
Stroke::GetPointAt(int32 index, Point& point) const
{
	if (Point* p = (Point*)fTrackPoints.ItemAt(index)) {
		point.point = p->point;
		Transform(&point.point);
		point.point.x -= 0.5;
		point.point.y -= 0.5;

		point.pressure = p->pressure;
		point.tiltX = p->tiltX;
		point.tiltY = p->tiltY;
		return true;
	}
	return false;
}

// GetLastPoint
bool
Stroke::GetLastPoint(Point& point) const
{
	return GetPointAt(fTrackPoints.CountItems() - 1, point);
}

// SetLastPoint
bool
Stroke::SetLastPoint(BPoint point, float pressure)
{
	point.x += 0.5;
	point.y += 0.5;

	bool success = false;
	Point* first = (Point*)fTrackPoints.FirstItem();
	Point* last = (Point*)fTrackPoints.LastItem();
	if (first && last) {
		// make a copy of the first
		Point* newFirst = new Point;
		newFirst->point = first->point;
		newFirst->pressure = first->pressure;
		newFirst->tiltX = first->tiltX;
		newFirst->tiltY = first->tiltY;

		Point* newLast = new Point;
		newLast->point = point;
		InverseTransform(&newLast->point);
		newLast->pressure = pressure;
		_MakeEmpty();
		
		success = fTrackPoints.AddItem((void*)newFirst);
		if (!success)
			delete newFirst;
		else {
			success = fTrackPoints.AddItem((void*)newLast);
			if (!success)
				delete newLast;
		}
	} else if (last) {
		last->point = point;
		InverseTransform(&last->point);
		last->pressure = pressure;
		success = true;
	}
	return success;
}

// TransformationChanged
void
Stroke::TransformationChanged()
{
//	Notify();
}

// Finish
void
Stroke::Finish(BBitmap* bitmap)
{
	BRect r = Bounds();
	if ((r.Width() + 1) * (r.Height() + 1) > 1000.0)
		_SetCache(bitmap, r);
}

// IsCacheValid
bool
Stroke::IsCacheValid() const
{
	if (fCache) {
		double matrix[9];
		StoreTo(matrix);
		if (matrix[0] == fCacheMatrix[0]	// sx
			&& matrix[1] == fCacheMatrix[1]	// shy
			&& matrix[2] == fCacheMatrix[2]	// w0
			&& matrix[3] == fCacheMatrix[3]	// shx
			&& matrix[3] == fCacheMatrix[4]	// sy
			&& matrix[3] == fCacheMatrix[5]	// w1

			&& matrix[3] == fCacheMatrix[8]	// w2
			) {

			double xDiff = fabs(matrix[6] - fCacheMatrix[6]);	// tx
			double yDiff = fabs(matrix[7] - fCacheMatrix[7]);	// ty
			if (fabs(floor(xDiff + 0.5) - xDiff) < 0.001 
				&& fabs(floor(yDiff + 0.5) - yDiff) < 0.001) {
				return true;
			}
		}
	}
	return false;
}

// Touch
// 
// only supposed to be called with area being non-transformed!
void
Stroke::Touch(BRect area)
{
	if (fBounds.IsValid())
		// bounds get updated to include area as well (union)
		fBounds = fBounds | area;
	else
		// Touch() called for the first time
		fBounds = area;
}

// LayerBoundsChanged
void
Stroke::LayerBoundsChanged(const BRect& layerBounds)
{
}

// Bounds
BRect
Stroke::Bounds() const
{
	return TransformBounds(fBounds);
}

// IsPickable
bool
Stroke::IsPickable(uint32 pickMask) const
{
	return is_pickable(ToolID(), pickMask);
}

// HitTest
bool
Stroke::HitTest(const BPoint& where, BBitmap* alphaMap)
{
	if (Bounds().Contains(where)) {
		// where is assumed on integer coordinates
		BRect testArea(where, where);
		uint8* testPixel = (uint8*)alphaMap->Bits();
		testPixel += (int32)where.y * alphaMap->BytesPerRow() + (int32)where.x;
		Draw(alphaMap, testArea);
		if (*testPixel > 0) {
			// clean up after ourselves
			*testPixel = 0;
			return true;
		}
	}
	return false;
}

// HitTest
bool
Stroke::HitTest(const BRect& area, BBitmap* alphaMap)
{
	if (Bounds().Intersects(area)) {
		BRect testArea = Bounds() & area;

		int32 left, top, right, bottom;
		rect_to_int(testArea, left, top, right, bottom);

		uint8* tst = (uint8*)alphaMap->Bits();
		uint32 bpr = alphaMap->BytesPerRow();
		tst += top * bpr + left;

		Draw(alphaMap, testArea);

		// TODO: the search could be optimized by peeking into
		// the bitmap non-linearily (think binary search,
		// but searching all pixels in the end)
		for (; top <= bottom; top++) {
			uint8* testPixel = tst;
			for (int32 x = left; x <= right; x++) {
				if (*testPixel) {
					// clean up after ourselves
					clear_area(alphaMap, testArea);
					return true;
				}
				testPixel++;
			}
			tst += bpr;
		}
	}
	return false;
}

// Update
void
Stroke::Update()
{
}

// Reset
void
Stroke::Reset()
{
	fBounds.left = 0.0;
	fBounds.top = 0.0;
	fBounds.right = -1.0;
	fBounds.bottom = -1.0;
}

// MakeEmpty
void
Stroke::MakeEmpty()
{
	_MakeEmpty();
}

// DefaultRenderer
MaskRenderer*
Stroke::DefaultRenderer() const
{
	MaskRenderer* renderer = NULL;
	switch (fMode) {
		case MODE_PEN:
		case MODE_BRUSH:
			renderer = new ColorRenderer(fColor);
			break;
		case MODE_ERASER:
		case MODE_PEN_ERASER:
			renderer = new EraseRenderer();
			break;
		case MODE_GRADIENT:
			renderer = new GradientRenderer();
			break;
	}
	return renderer;
}

// SetRenderer
void
Stroke::SetRenderer(MaskRenderer* renderer)
{
	if (renderer && renderer != fRenderer) {
		// TODO: this seems hacky
		Gradient* gradient = GetGradient();
		if (gradient && fGradient)
			*fGradient = *gradient;
		// switch renderers
		delete fRenderer;
		fRenderer = renderer;
		// make sure the renderer is setup correctly
		GradientRenderer* gr = dynamic_cast<GradientRenderer*>(renderer);
		if (gr) {
			if (!fGradient) {
				fGradient = new Gradient();
				// fit new gradient into object bounds
				fGradient->FitToBounds(Bounds());
				// remove our own transformation to get a clear gradient
				Transformable transform(*this);
				if (transform.IsValid()) {
					transform.Invert();
					fGradient->Multiply(transform);
				}
			}
			gr->SetGradient(*fGradient);
		}
	}
}

// MergeWithBitmap
void
Stroke::MergeWithBitmap(BBitmap* dest, BBitmap* strokeBitmap,
						BRect area, uint32 colorSpace) const
{
	if (fRenderer) {
		fRenderer->Render(dest, strokeBitmap, area, *this);
	} else {
		switch (fMode) {
			case MODE_PEN:
			case MODE_BRUSH: {
				// convert color to correct space
				uint8 c1;
				uint8 c2;
				uint8 c3;
				get_components_for_color(fColor, colorSpace, c1, c2, c3);
				overlay_color(dest, strokeBitmap, area, c1, c2, c3);
				break;
			}
			case MODE_ERASER:
			case MODE_PEN_ERASER:
				erase(dest, strokeBitmap, area);
				break;
			case MODE_RESTORE:
//				restore(dest, strokeBitmap, area);
				break;
		}
	}
}

// MergeWithBitmap
void
Stroke::MergeWithBitmap(BBitmap* from, BBitmap* dest,
						BBitmap* strokeBitmap, BRect area, uint32 colorSpace) const
{
	if (fRenderer) {
		fRenderer->Render(dest, from, strokeBitmap, area, *this);
	} else {
		switch (fMode) {
			case MODE_PEN:
			case MODE_BRUSH: {
				// convert color to correct space
				uint8 c1;
				uint8 c2;
				uint8 c3;
				get_components_for_color(fColor, colorSpace, c1, c2, c3);
				overlay_color_copy(from, dest, strokeBitmap, area, c1, c2, c3);
				break;
			}
			case MODE_ERASER:
			case MODE_PEN_ERASER:
				erase_copy(from, dest, strokeBitmap, area);
				break;
			case MODE_RESTORE:
//				restore_copy(from, dest, strokeBitmap, area);
				break;
		}
	}
}

// SetColor
void
Stroke::SetColor(rgb_color color)
{
	if (fColor.red != color.red ||
		fColor.green != color.green ||
		fColor.blue != color.blue ||
		fColor.alpha != color.alpha) {
		fColor = color;

ColorRenderer* renderer = dynamic_cast<ColorRenderer*>(fRenderer);
if (renderer) {
	renderer->SetColor(fColor);

		Notify();
}
	}
}

// Color
rgb_color
Stroke::Color() const
{
	ColorRenderer* renderer = dynamic_cast<ColorRenderer*>(fRenderer);
	if (renderer)
		return renderer->Color();
	return fColor;
}

// IsColorObject
bool
Stroke::IsColorObject() const
{
//	return dynamic_cast<ColorRenderer*>(fRenderer) != NULL;
	return (fMode == MODE_PEN || fMode == MODE_BRUSH);
}

// SetGradient
void
Stroke::SetGradient(const Gradient* gradient)
{
	if (gradient) {
		// keep track of gradient ourselves
		if (!fGradient)
			fGradient = new Gradient(*gradient);
		else
			*fGradient = *gradient;
		// make sure the renderer is setup correctly
		GradientRenderer* gr = dynamic_cast<GradientRenderer*>(fRenderer);
		if (gr) {
			gr->SetGradient(*fGradient);

			Notify();
		}
	}
}

// GetGradient
Gradient*
Stroke::GetGradient() const
{
	GradientRenderer* renderer = dynamic_cast<GradientRenderer*>(fRenderer);
	if (renderer)
		return renderer->GetGradient();
	return NULL;
}


// Painter
ObjectItemPainter*
Stroke::Painter() const
{
	if (Gradient* gradient = GetGradient())
		return new GradientObjectItemPainter(Name(), gradient); 

	if (IsColorObject())
		return new ColorObjectItemPainter(Name(), Color());

	return new ObjectItemPainter(Name());
}

// MakePropertyObject
PropertyObject*
Stroke::MakePropertyObject() const
{
	PropertyObject* object = new PropertyObject();
	if (fMode == MODE_BRUSH ||
		fMode == MODE_PEN ||
		fMode == MODE_ERASER ||
		fMode == MODE_PEN_ERASER ||
		fMode == MODE_GRADIENT) {

		OptionProperty* property = new OptionProperty("mode",
													  PROPERTY_BLENDING_MODE);
		LanguageManager* m = LanguageManager::Default();
		property->AddOption(MODE_BRUSH, m->GetString(NORMAL, "Normal"));
		property->AddOption(MODE_ERASER, m->GetString(ERASE, "Erase"));
		property->AddOption(MODE_GRADIENT, m->GetString(GRADIENT, "Gradient"));
		switch (fMode) {
			case MODE_BRUSH:
			case MODE_PEN:
				property->SetCurrentOptionID(MODE_BRUSH);
				break;
			case MODE_ERASER:
			case MODE_PEN_ERASER:
				property->SetCurrentOptionID(MODE_ERASER);
				break;
			case MODE_GRADIENT:
				property->SetCurrentOptionID(MODE_GRADIENT);
				break;
		}

		object->AddProperty(property);

	}
	if (fRenderer) {
		fRenderer->AddProperties(object);
	} else if (IsColorObject()) {
		object->AddProperty(new ColorProperty("color", PROPERTY_COLOR,
											  Color()));
	}
	return object;
}

// SetToPropertyObject
bool
Stroke::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	bool ret = false;
	if (object) {
		if (fMode == MODE_BRUSH ||
			fMode == MODE_PEN ||
			fMode == MODE_ERASER ||
			fMode == MODE_PEN_ERASER ||
			fMode == MODE_GRADIENT) {

			OptionProperty* property = dynamic_cast<OptionProperty*>(object->FindProperty(PROPERTY_BLENDING_MODE));
			if (property) {
				int32 mode = property->CurrentOptionID();
				if (fMode != (uint32)mode) {
					fMode = mode;
					ret = true;
					SetRenderer(DefaultRenderer());
				}
			}
		}
		if (fRenderer) {
			if (fRenderer->SetToPropertyObject(object))
				ret = true;
		} else {
			ColorProperty* colorProperty = dynamic_cast<ColorProperty*>(object->FindProperty(PROPERTY_COLOR));
			if (colorProperty) {
				rgb_color color = colorProperty->Color();
				if (color.red != fColor.red ||
					color.green != fColor.green ||
					color.blue != fColor.blue)
					ret = true;
				fColor = color;
			}
		}

		if (ret) {
			fColor = Color();
			Notify();
		}
	}
	return ret;
}

// SetMode
void
Stroke::SetMode(uint32 mode)
{
	if (fMode != mode) {
		fMode = mode;
		Notify();
	}
}

// SetTrackPoints
void
Stroke::SetTrackPoints(const BList& points)
{
	_MakeEmpty();
	for (int32 i = 0; Point* point = (Point*)points.ItemAt(i); i++) {
		Point* clone = new Point;
		clone->point = point->point;
		clone->pressure = point->pressure;
		clone->tiltX = point->tiltX;
		clone->tiltY = point->tiltY;
		fTrackPoints.AddItem((void*)clone);
	}
}

// PrintToStream
void
Stroke::PrintToStream() const
{
	printf("number of track points: %ld\n", fTrackPoints.CountItems());
	printf("color: (r: %d, g: %d, b: %d, a: %d)\n", fColor.red, fColor.green, fColor.blue, fColor.alpha);
	printf("mode: %ld\n", fMode);
	printf("bounds:\n");
	fBounds.PrintToStream();
	printf("cache size: %ld\n", fCache ? fCache->Size() : 0);
	printf("valid cache: %d\n", IsCacheValid());
}

// _MakeEmpty
void
Stroke::_MakeEmpty()
{
	for (int32 i = 0; Point* point = (Point*)fTrackPoints.ItemAt(i); i++)
		delete point;
	fTrackPoints.MakeEmpty();

	delete fCache;
	fCache = NULL;
}

// _SetCache
void
Stroke::_SetCache(BBitmap* bitmap, BRect frame)
{
	if (!fCache) {
		fCache = new RLEBuffer(bitmap, frame);
		if (fCache->IsValid()) {
			StoreTo(fCacheMatrix);
		} else {
			delete fCache;
			fCache = NULL;
		}
	} else {
		if (fCache->SetTo(bitmap, frame) >= B_OK) {
			StoreTo(fCacheMatrix);
		}
	}


/*	uint32 realSize = (frame.IntegerWidth() + 1) * (frame.IntegerHeight() + 1);
	uint32 size = fCache->Size();
	printf("RLE buffer efficiency: %ld -> %ld (%fx)\n", realSize, size, (float)realSize / (float)size);*/
}

// _InitCacheMatrix
void
Stroke::_InitCacheMatrix()
{
	fCacheMatrix[0] = 1.0;	// sx
	fCacheMatrix[1] = 0.0;	// shy
	fCacheMatrix[2] = 0.0;	// w0
	fCacheMatrix[3] = 0.0;	// shx
	fCacheMatrix[4] = 1.0;	// sy
	fCacheMatrix[5] = 0.0;	// w1
	fCacheMatrix[6] = 0.0;	// tx
	fCacheMatrix[7] = 0.0;	// ty
	fCacheMatrix[8] = 1.0;	// w2
}

