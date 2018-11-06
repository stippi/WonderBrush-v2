// ShapeStroke.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Entry.h>
#include <Message.h>
#include <Path.h>

#include <agg_alpha_mask_u8.h>
#include <agg_conv_curve.h>
#include <agg_conv_stroke.h>
#include <agg_conv_transform.h>
#include <agg_curves.h>
#include <agg_path_storage.h>
#include <agg_pixfmt_gray.h>
#include <agg_rasterizer_outline.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_mclip.h>
#include <agg_renderer_primitives.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_bin.h>
#include <agg_scanline_p.h>
#include <agg_scanline_u.h>

#include "bitmap_support.h"
#include "blending.h"
#include "defines.h"
#include "support.h"

#include "BoolProperty.h"
#include "CommonPropertyIDs.h"
#include "Icons.h"
#include "IconProperty.h"
#include "IntProperty.h"
#include "FloatProperty.h"
#include "Gradient.h"
#include "LanguageManager.h"
#include "OptionProperty.h"
#include "PropertyObject.h"
#include "VectorPath.h"

#include "ShapeStroke.h"

// agg_filling_rule_for
inline agg::filling_rule_e
agg_filling_rule_for(uint32 fillMode)
{
	return (agg::filling_rule_e)fillMode;
}


// constructor
ShapeStroke::ShapeStroke(rgb_color color)
	: Stroke(color, MODE_BRUSH/*MODE_UNDEFINED*/),
	  fPath(new VectorPath()),
	  fSubPaths(4),
	  fAlpha(255),
	  fOutline(false),
	  fOutlineWidth(1.0),
	  fCapMode(CAP_MODE_BUTT),
	  fJoinMode(JOIN_MODE_MITER),
	  fSolid(false),
	  fFillMode(FILL_MODE_EVEN_ODD)
{
	AddPath(fPath);
}

// copy constructor
ShapeStroke::ShapeStroke(const ShapeStroke& other)
	: Stroke(other),
	  fPath(NULL),
	  fSubPaths(4),
	  fAlpha(other.fAlpha),
	  fOutline(other.fOutline),
	  fOutlineWidth(other.fOutlineWidth),
	  fCapMode(other.fCapMode),
	  fJoinMode(other.fJoinMode),
	  fSolid(other.fSolid),
	  fFillMode(other.fFillMode)
{
	for (int32 i = 0; VectorPath* path = other.PathAt(i); i++)
		AddPath(new VectorPath(*path));

	fPath = PathAt(other.fSubPaths.IndexOf((void*)other.fPath));
}

// BArchivable constructor
ShapeStroke::ShapeStroke(BMessage* archive)
	: Stroke(archive),
	  fPath(NULL),
	  fAlpha(255),
	  fOutline(false),
	  fOutlineWidth(1.0),
	  fCapMode(CAP_MODE_BUTT),
	  fJoinMode(JOIN_MODE_MITER),
  	  fSolid(false),
	  fFillMode(FILL_MODE_EVEN_ODD)
{
	// restore all sub paths
	BMessage pathArchive;
	for (int32 i = 0; archive->FindMessage("path", i, &pathArchive) >= B_OK; i++) {
		VectorPath* path = new VectorPath(&pathArchive);
		if (!fPath)
			fPath = path;
		AddPath(path);
		pathArchive.MakeEmpty();
	}
		
	// backward compatibility
	if (!fPath) {
		fPath = new VectorPath(archive);
		AddPath(fPath);
	}

	int32 alpha;
	if (archive->FindInt32("alpha", &alpha) >= B_OK)
		fAlpha = alpha;
	if (archive->FindBool("outline", &fOutline) < B_OK)
		fOutline = false;
	if (archive->FindFloat("outline width", &fOutlineWidth) < B_OK)
		fOutlineWidth = 1.0;
	if (archive->FindInt32("cap mode", (int32*)&fCapMode) < B_OK)
		fCapMode = CAP_MODE_BUTT;
	if (archive->FindInt32("join mode", (int32*)&fJoinMode) < B_OK)
		fJoinMode = JOIN_MODE_MITER;
	if (archive->FindBool("solid", &fSolid) < B_OK)
		fSolid = false;
	if (archive->FindInt32("filling rule", (int32*)&fFillMode) < B_OK)
		fFillMode = FILL_MODE_NON_ZERO; // default is for backwards compatibility

	UpdateBounds();
}

// destructor
ShapeStroke::~ShapeStroke()
{
	_MakeEmpty();
}

// Clone
Stroke*
ShapeStroke::Clone() const
{
	return new ShapeStroke(*this);
}

// SetTo
bool
ShapeStroke::SetTo(const Stroke* from)
{
	const ShapeStroke* shapeStroke = dynamic_cast<const ShapeStroke*>(from);

	AutoNotificationSuspender _(this);

	if (shapeStroke) {

		int32 thisCount = CountPaths();
		int32 fromCount = shapeStroke->CountPaths();

		// copy "fromCount" paths
		for (int32 i = 0; i < fromCount; i++) {
			VectorPath* thisPath = PathAt(i);
			VectorPath* fromPath = shapeStroke->PathAt(i);
			if (thisPath) {
				if (fromPath)
					*thisPath = *fromPath;
				else
					// should not happen
					fprintf(stderr, "ShapeStroke::SetTo() - path missing in \"from\"");
			} else {
				if (fromPath)
					AddPath(new VectorPath(*fromPath));
				else
					// should not happen
					fprintf(stderr, "ShapeStroke::SetTo() - path missing in \"from\"");
			}
		}

		// delete remaining subpaths that are not in "from"
		if (thisCount > fromCount) {
			for (int32 i = thisCount - 1; i >= fromCount; i--) {
				VectorPath* path = (VectorPath*)fSubPaths.RemoveItem(i);
				delete path;
			}
		}

		// reset current path
		fPath = PathAt(0);

		fAlpha			= shapeStroke->fAlpha;
		fOutline		= shapeStroke->fOutline;
		fOutlineWidth	= shapeStroke->fOutlineWidth;
		fCapMode		= shapeStroke->fCapMode;
		fJoinMode		= shapeStroke->fJoinMode;
		fSolid			= shapeStroke->fSolid;
		fFillMode		= shapeStroke->fFillMode;

		Notify();
	}
	return Stroke::SetTo(from);
}

// Instantiate
BArchivable*
ShapeStroke::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "ShapeStroke"))
		return new ShapeStroke(archive);
	return NULL;
}

// Archive
status_t
ShapeStroke::Archive(BMessage* into, bool deep) const
{
	status_t status = Stroke::Archive(into, deep);

	// archive the paths
	BMessage pathArchive;
	for (int32 i = 0; VectorPath* path = PathAt(i); i++) {
		pathArchive.MakeEmpty();
		status = path->Archive(&pathArchive);
		if (status >= B_OK)
			status = into->AddMessage("path", &pathArchive);
		if (status < B_OK)
			break;
	}

	// add alpha
	if (status >= B_OK)
		status = into->AddInt32("alpha", fAlpha);

	if (status >= B_OK)
		status = into->AddBool("outline", fOutline);

	if (status >= B_OK)
		status = into->AddFloat("outline width", fOutlineWidth);

	if (status >= B_OK)
		status = into->AddInt32("cap mode", fCapMode);

	if (status >= B_OK)
		status = into->AddInt32("join mode", fJoinMode);

	if (status >= B_OK)
		status = into->AddBool("solid", fSolid);

	if (status >= B_OK)
		status = into->AddInt32("filling rule", fFillMode);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "ShapeStroke");

	return status;
}


// DrawLastLine
bool
ShapeStroke::DrawLastLine(BBitmap* bitmap, BRect& updateRect)
{
	return false;
}

// Draw
void
ShapeStroke::Draw(BBitmap* bitmap)
{
	Draw(bitmap, bitmap->Bounds());
}

typedef agg::renderer_mclip<agg::pixfmt_gray8>				BaseRenderer;
typedef agg::renderer_scanline_aa_solid<BaseRenderer>		Renderer;

typedef agg::renderer_primitives<BaseRenderer>				OutlineRenderer;
typedef agg::rasterizer_outline<OutlineRenderer>			OutlineRasterizer;

typedef agg::rasterizer_scanline_aa<>						Rasterizer;

// _DrawPaths
template<class Rasterizer, class Renderer, class Scanline>
void
ShapeStroke::_DrawPaths(Rasterizer& rasterizer,
						Scanline& scanline,
						Renderer& renderer,
						bool outline) const
{
	// add each sub path to the rasterizer
	for (int32 i = 0; VectorPath* path = PathAt(i); i++) {

		path->SetTransformable(*this);

		agg::path_storage pathStorage;
		if (path->GetAGGPathStorage(pathStorage, fOutline)) {
			// NOTE: the conversion pipeline is different
			// * for outlines the transformation is done at the
			// last stage in order to get "distorted" outlines
			// * for normal shapes, this is not necessary and results
			// in considerable overhead, so the transformation is
			// the first stage, when there are only a few points yet
			if (outline) {
				// convert curves to line segments
				agg::conv_curve<agg::path_storage> curve(pathStorage);
				curve.approximation_scale(scale() * 4.0);
	
				// convert to outline width specific width
				agg::conv_stroke<agg::conv_curve<agg::path_storage> > outline(curve);

				outline.line_cap(convert_cap_mode(fCapMode));
				outline.line_join(convert_join_mode(fJoinMode));

				outline.approximation_scale(scale() * 3.0);
				outline.width(fOutlineWidth);

				// transform the outline
				agg::conv_transform<agg::conv_stroke<agg::conv_curve<agg::path_storage> >,
									Transformable> trans(outline, *this);

				rasterizer.add_path(trans);
			} else {
				// transform the path
				agg::conv_transform<agg::path_storage,
									Transformable> trans(pathStorage, *this);

				// convert curves to line segments
				agg::conv_curve<agg::conv_transform<agg::path_storage,
													Transformable> > curve(trans);
				curve.approximation_scale(2.0);
	
				rasterizer.add_path(curve);
			}
		}
	}
}

// Draw
void
ShapeStroke::Draw(BBitmap* alphaMap, BRect area)
{
	if (alphaMap && alphaMap->IsValid()
		&& area.IsValid() && alphaMap->Bounds().Intersects(area)) {

		// don't draw outside alphaMap
		area = area & alphaMap->Bounds();

		// attach AGG rendering buffer to bitmap
		agg::rendering_buffer buffer;
		buffer.attach((uint8*)alphaMap->Bits(),
					  alphaMap->Bounds().IntegerWidth() + 1,
					  alphaMap->Bounds().IntegerHeight() + 1,
					  alphaMap->BytesPerRow());

		// pixel format and base renderer
		agg::pixfmt_gray8 pixelFormat(buffer);
		BaseRenderer baseRenderer(pixelFormat);

		// integer version of constrain rect
		int32 left, top, right, bottom;
		rect_to_int(area, left, top, right, bottom);
		// init clipping
		baseRenderer.reset_clipping(false);
		baseRenderer.add_clip_box(left, top, right, bottom);

		if (fSolid && fOutline && fOutlineWidth <= 1.2) {

			// create and configure renderer
			OutlineRenderer renderer(baseRenderer);
			renderer.line_color(agg::gray8(fAlpha));

			// create and configure rasterizer
			OutlineRasterizer rasterizer(renderer);

			// scanline
			agg::scanline_bin scanline;

			_DrawPaths(rasterizer, scanline, renderer, false);

		} else {

			// create and configure rasterizer
			Rasterizer rasterizer;
			rasterizer.clip_box(left, top, right + 1, bottom + 1);
			if (!fOutline)
				rasterizer.filling_rule(agg_filling_rule_for(fFillMode));
			if (fSolid)
				rasterizer.gamma(agg::gamma_threshold(0.6));

			// create and configure renderer
			Renderer renderer(baseRenderer);
			renderer.color(agg::gray8(fAlpha));

			// scanline
			agg::scanline_p8 scanline;

			_DrawPaths(rasterizer, scanline, renderer, fOutline);
			agg::render_scanlines(rasterizer, scanline, renderer);
		}
	}
}

// Bounds
BRect
ShapeStroke::Bounds() const
{
	return fBounds;
}

// Name
const char*
ShapeStroke::Name() const
{
	LanguageManager* manager = LanguageManager::Default();
	return manager->GetString(SHAPE, "Shape");
}

// ToolID
int32
ShapeStroke::ToolID() const
{
	return TOOL_SHAPE;
}

// MakePropertyObject
PropertyObject*
ShapeStroke::MakePropertyObject() const
{
	PropertyObject* object = Stroke::MakePropertyObject();
	if (object) {
		// opacity
		object->AddProperty(new IntProperty("opacity",
											PROPERTY_OPACITY,
											fAlpha));
		// archive the paths
		BMessage* archivedPaths = new BMessage();
		BMessage pathArchive;
		status_t status = B_ERROR;
		for (int32 i = 0; VectorPath* path = PathAt(i); i++) {
			pathArchive.MakeEmpty();
			status = path->Archive(&pathArchive);
			if (status >= B_OK)
				status = archivedPaths->AddMessage("path", &pathArchive);
			if (status < B_OK)
				break;
		}

		if (status >= B_OK) {
			object->AddProperty(new IconProperty("path",
												 PROPERTY_PATH,
												 kPathPropertyIconBits,
												 kPathPropertyIconWidth,
												 kPathPropertyIconHeight,
												 kPathPropertyIconFormat,
												 archivedPaths));
		} else
			delete archivedPaths;
		// outline
		object->AddProperty(new BoolProperty("outline",
											  PROPERTY_OUTLINE,
											  fOutline));

		LanguageManager* m = LanguageManager::Default();
	
		if (fOutline) {
			// outline width
			object->AddProperty(new FloatProperty("width",
												  PROPERTY_OUTLINE_WIDTH,
												  fOutlineWidth,
												  0.0, 100.0));
	
			OptionProperty* property = new OptionProperty("caps",
														  PROPERTY_CAP_MODE);
			property->AddOption(CAP_MODE_BUTT, m->GetString(BUTT_CAP, "Butt"));
			property->AddOption(CAP_MODE_SQUARE, m->GetString(SQUARE_CAP, "Square"));
			property->AddOption(CAP_MODE_ROUND, m->GetString(ROUND_CAP, "Round"));
			property->SetCurrentOptionID(fCapMode);
	
			object->AddProperty(property);
	
			property = new OptionProperty("joints", PROPERTY_JOIN_MODE);
			property->AddOption(JOIN_MODE_MITER, m->GetString(MITER_JOIN, "Miter"));
			property->AddOption(JOIN_MODE_ROUND, m->GetString(ROUND_JOIN, "Round"));
			property->AddOption(JOIN_MODE_BEVEL, m->GetString(BEVEL_JOIN, "Bevel"));
			property->SetCurrentOptionID(fJoinMode);
	
			object->AddProperty(property);
		} else {
			OptionProperty* property = new OptionProperty("filling rule",
														  PROPERTY_FILLING_RULE);
			property->AddOption(CAP_MODE_BUTT, m->GetString(NON_ZERO, "Non Zero"));
			property->AddOption(CAP_MODE_SQUARE, m->GetString(EVEN_ODD, "Even-Odd"));
			property->SetCurrentOptionID(fFillMode);
			object->AddProperty(property);
		}

		// antialiasing
		object->AddProperty(new BoolProperty("solid",
											  PROPERTY_NO_ANTIALIASING,
											  fSolid));
	}
	return object;
}

// SetToPropertyObject
bool
ShapeStroke::SetToPropertyObject(PropertyObject* object)
{
	AutoNotificationSuspender _(this);

	bool ret = Stroke::SetToPropertyObject(object);
	if (object) {
		// opacity
		int32 i = object->FindIntProperty(PROPERTY_OPACITY, fAlpha);
		if (i != fAlpha)
			ret = true;
		fAlpha = i;
		// path
		IconProperty* pathProperty = dynamic_cast<IconProperty*>(object->FindProperty(PROPERTY_PATH));
		if (pathProperty && pathProperty->Message()) {
			const BMessage* archive = pathProperty->Message();
			// restore all sub paths
			BMessage pathArchive;
			int32 i = 0;
			for (; archive->FindMessage("path", i, &pathArchive) >= B_OK; i++) {
				VectorPath* path = new VectorPath(&pathArchive);
				if (SetPath(path, i))
					delete path;
				else
					AddPath(path);
				pathArchive.MakeEmpty();
			}
			// delete any additional subpaths
			int32 count = CountPaths();
			if (i < count) {
				for (int32 j = count - 1; j >= i; j--) {
					delete (VectorPath*)fSubPaths.RemoveItem(j);
				}
			}
			// make sure fPath points to a path that is still ours
			if (!fSubPaths.HasItem((void*)fPath))
				fPath = PathAt(0);
			// in any case
			ret = true;
		}
		// outline
		bool b = object->FindBoolProperty(PROPERTY_OUTLINE, fOutline);
		if (b != fOutline)
			ret = true;
		fOutline = b;
		// outline width
		float f = object->FindFloatProperty(PROPERTY_OUTLINE_WIDTH, fOutlineWidth);
		if (f != fOutlineWidth)
			ret = true;
		fOutlineWidth = f;

		// cap mode
		OptionProperty* property = dynamic_cast<OptionProperty*>(object->FindProperty(PROPERTY_CAP_MODE));
		if (property && (int32)fCapMode != property->CurrentOptionID()) {
			fCapMode = property->CurrentOptionID();
			ret = true;
		}
		// join mode
		property = dynamic_cast<OptionProperty*>(object->FindProperty(PROPERTY_JOIN_MODE));
		if (property && (int32)fJoinMode != property->CurrentOptionID()) {
			fJoinMode = property->CurrentOptionID();
			ret = true;
		}
		// antialiasing
		b = object->FindBoolProperty(PROPERTY_NO_ANTIALIASING, fSolid);
		if (b != fSolid)
			ret = true;
		fSolid = b;

		// filling rule
		property = dynamic_cast<OptionProperty*>(object->FindProperty(PROPERTY_FILLING_RULE));
		if (property && (int32)fFillMode != property->CurrentOptionID()) {
			fFillMode = property->CurrentOptionID();
			ret = true;
		}

		if (ret) {
			UpdateBounds();
			Notify();
		}
	}
	return ret;
}

// SetPath
void
ShapeStroke::SetPath(const VectorPath* from)
{
	if (from) {
		*fPath = *from;
		UpdateBounds();
		Notify();
	}
}

// SetPath
bool
ShapeStroke::SetPath(const VectorPath* from, int32 index)
{
	VectorPath* path = PathAt(index);
	if (from && path) {
		*path = *from;
		UpdateBounds();
		Notify();
		return true;
	}
	return false;
}

// AddPath
bool
ShapeStroke::AddPath(VectorPath* path)
{
	if (path && fSubPaths.AddItem((void*)path)) {
		path->SetTransformable(*this);
		Notify();
		return true;
	}
	return false;
}

// AddPath
bool
ShapeStroke::AddPath(VectorPath* path, int32 index)
{
	if (path && fSubPaths.AddItem((void*)path, index)) {
		path->SetTransformable(*this);
		Notify();
		return true;
	}
	return false;
}

// RemovePath
bool
ShapeStroke::RemovePath(VectorPath* path)
{
	if (fSubPaths.RemoveItem((void*)path)) {
		if (fPath == path)
			fPath = PathAt(0);
		Notify();
		return true;
	}
	return false;
}

// PathAt
VectorPath*
ShapeStroke::PathAt(int32 index) const
{
	return (VectorPath*)fSubPaths.ItemAt(index);
}

// IndexOf
int32
ShapeStroke::IndexOf(VectorPath* path) const
{
	return fSubPaths.IndexOf((void*)path);
}

// CountPaths
int32
ShapeStroke::CountPaths() const
{
	return fSubPaths.CountItems();
}

// HasPath
bool
ShapeStroke::HasPath(VectorPath* path) const
{
	return fSubPaths.HasItem((void*)path);
}

// SetCurrentPath
void
ShapeStroke::SetCurrentPath(VectorPath* path)
{
	if (fSubPaths.HasItem((void*)path))
		fPath = path;
}

// FreezeTransformations
void
ShapeStroke::FreezeTransformations()
{
	if (!IsIdentity()) {
		for (int32 i = 0; VectorPath* path = PathAt(i); i++)
			path->FreezeTransformations();

		if (Gradient* gradient = GetGradient()) {
			if (gradient->InheritTransformation())
				gradient->Multiply(*this);
		}
		fOutlineWidth *= scale();

		Transformable::Reset();
		Notify();
	}
}

// SetAlpha
void
ShapeStroke::SetAlpha(uint8 alpha)
{
	if (fAlpha != alpha) {
		fAlpha = alpha;
		Notify();
	}
}

// SetOutline
void
ShapeStroke::SetOutline(bool outline)
{
	if (fOutline != outline) {
		fOutline = outline;
//		UpdateBounds();
		Notify();
	}
}

// SetOutlineWidth
void
ShapeStroke::SetOutlineWidth(float width)
{
	if (fOutlineWidth != width) {
		fOutlineWidth = width;
//		UpdateBounds();
		Notify();
	}
}

// SetClosed
void
ShapeStroke::SetClosed(bool closed)
{
	if (fPath && closed != fPath->IsClosed()) {
		fPath->SetClosed(closed);
		Notify();
	}
}

// IsClosed
bool
ShapeStroke::IsClosed() const
{
	if (fPath)
		return fPath->IsClosed();
	return false;
}

// SetCapMode
void
ShapeStroke::SetCapMode(uint32 mode)
{
	if (mode != fCapMode) {
		fCapMode = mode;
		Notify();
	}
}

// SetJoinMode
void
ShapeStroke::SetJoinMode(uint32 mode)
{
	if (mode != fJoinMode) {
		fJoinMode = mode;
		Notify();
	}
}

// SetFillingRule
void
ShapeStroke::SetFillingRule(uint32 mode)
{
	if (mode != fFillMode) {
		fFillMode = mode;
		Notify();
	}
}

// SetSolid
void
ShapeStroke::SetSolid(bool solid)
{
	if (fSolid != solid) {
		fSolid = solid;
		Notify();
	}
}

// UpdateBounds
void
ShapeStroke::UpdateBounds()
{
	Reset();
	for (int32 i = 0; VectorPath* path = PathAt(i); i++) {
		path->SetTransformable(*this);
		BRect pathBounds = path->Bounds(fOutline, fOutlineWidth, fCapMode, fJoinMode);
		if (i == 0)
			fBounds = pathBounds;
		else
			fBounds = fBounds | pathBounds;
	}
// UpdateBounds() means the path has changed, and we
// *should* actually notify, *but* the ShapeState is
// calling Notify() on us during MouseUp(), so we don't
// have to keep notifying uselessly all the time
//	Notify();
}

// TransformationChanged
void
ShapeStroke::TransformationChanged()
{
	UpdateBounds();
}

// _MakeEmpty
void
ShapeStroke::_MakeEmpty()
{
	// delete all but the first path, make that one empty
	int32 count = fSubPaths.CountItems();
	for (int32 i = count - 1; i >= 0; i--) {
		if (VectorPath* path = (VectorPath*)fSubPaths.RemoveItem(i))
			delete path;
	}
	fPath = NULL;
}

