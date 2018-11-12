//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.2
// Copyright (C) 2002-2004 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//		  mcseemagg@yahoo.com
//		  http://www.antigrain.com
//----------------------------------------------------------------------------
//
// SVG path renderer.
//
//----------------------------------------------------------------------------

#include <stdio.h>

#include <Bitmap.h>

#include <agg_bounding_rect.h>

#include "bitmap_support.h"

#include "Canvas.h"
#include "History.h"
#include "Layer.h"
#include "ShapeStroke.h"
#include "SVGGradients.h"
#include "VectorPath.h"

#include "DocumentBuilder.h"

namespace agg
{
namespace svg
{

// constructor
DocumentBuilder::DocumentBuilder()
	: fGradients(20),
	  fCurrentGradient(NULL),
	  fWidth(0),
	  fHeight(0),
	  fViewBox(0.0, 0.0, -1.0, -1.0),
	  fTitle("")
{
}


// remove_all
void
DocumentBuilder::remove_all()
{
	fPathStorage.remove_all();
	fAttributesStorage.remove_all();
	fAttributesStack.remove_all();
	fTransform.reset();
}

// begin_path
void
DocumentBuilder::begin_path()
{
	push_attr();
	unsigned idx = fPathStorage.start_new_path();
	fAttributesStorage.add(path_attributes(cur_attr(), idx));
}

// end_path
void
DocumentBuilder::end_path()
{
	if (fAttributesStorage.size() == 0) {
		throw exception("end_path: The path was not begun");
	}
	path_attributes attr = cur_attr();
	unsigned idx = fAttributesStorage[fAttributesStorage.size() - 1].index;
	attr.index = idx;
	fAttributesStorage[fAttributesStorage.size() - 1] = attr;
	pop_attr();
}

// move_to
void
DocumentBuilder::move_to(double x, double y, bool rel)		  // M, m
{
	if (rel)
		fPathStorage.move_rel(x, y);
	else
		fPathStorage.move_to(x, y);
}

// line_to
void
DocumentBuilder::line_to(double x,  double y, bool rel)		 // L, l
{
	if (rel)
		fPathStorage.line_rel(x, y);
	else
		fPathStorage.line_to(x, y);
}

// hline_to
void
DocumentBuilder::hline_to(double x, bool rel)				   // H, h
{
	if (rel)
		fPathStorage.hline_rel(x);
	else
		fPathStorage.hline_to(x);
}

// vline_to
void
DocumentBuilder::vline_to(double y, bool rel)				   // V, v
{
	if (rel)
		fPathStorage.vline_rel(y);
	else
		fPathStorage.vline_to(y);
}

// curve3
void
DocumentBuilder::curve3(double x1, double y1,				   // Q, q
						double x,  double y, bool rel)
{
	if (rel)
		fPathStorage.curve3_rel(x1, y1, x, y);
	else
		fPathStorage.curve3(x1, y1, x, y);
}

// curve3
void
DocumentBuilder::curve3(double x, double y, bool rel)		   // T, t
{
	if (rel)
		fPathStorage.curve3_rel(x, y);
	else
		fPathStorage.curve3(x, y);
}

// curve4
void
DocumentBuilder::curve4(double x1, double y1,				   // C, c
						double x2, double y2,
						double x,  double y, bool rel)
{
	if (rel) {
		fPathStorage.curve4_rel(x1, y1, x2, y2, x, y);
	} else {
		fPathStorage.curve4(x1, y1, x2, y2, x, y);
	}
}

// curve4
void
DocumentBuilder::curve4(double x2, double y2,				   // S, s
						double x,  double y, bool rel)
{
	if (rel) {
		fPathStorage.curve4_rel(x2, y2, x, y);
	} else {
		fPathStorage.curve4(x2, y2, x, y);
	}
}

// elliptical_arc
void
DocumentBuilder::elliptical_arc(double rx, double ry, double angle,
							    bool large_arc_flag, bool sweep_flag,
							    double x, double y, bool rel)
{
	angle = angle / 180.0 * pi;
	if (rel) {
		fPathStorage.arc_rel(rx, ry, angle, large_arc_flag, sweep_flag, x, y);
	} else {
		fPathStorage.arc_to(rx, ry, angle, large_arc_flag, sweep_flag, x, y);
	}
}

// close_subpath
void
DocumentBuilder::close_subpath()
{
	fPathStorage.end_poly(path_flags_close);
}

// SetTitle
void
DocumentBuilder::SetTitle(const char* title)
{
	fTitle = title;
}

// SetDimensions
void
DocumentBuilder::SetDimensions(uint32 width, uint32 height, BRect viewBox)
{
	fWidth = width;
	fHeight = height;
	fViewBox = viewBox;
}

// cur_attr
path_attributes&
DocumentBuilder::cur_attr()
{
	if (fAttributesStack.size() == 0) {
		throw exception("cur_attr: Attribute stack is empty");
	}
	return fAttributesStack[fAttributesStack.size() - 1];
}

// push_attr
void
DocumentBuilder::push_attr()
{
//printf("DocumentBuilder::push_attr() (size: %d)\n", fAttributesStack.size());
	fAttributesStack.add(fAttributesStack.size() ? fAttributesStack[fAttributesStack.size() - 1]
												 : path_attributes());
}

// pop_attr
void
DocumentBuilder::pop_attr()
{
//printf("DocumentBuilder::pop_attr() (size: %d)\n", fAttributesStack.size());
	if (fAttributesStack.size() == 0) {
		throw exception("pop_attr: Attribute stack is empty");
	}
	fAttributesStack.remove_last();
}

// fill
void
DocumentBuilder::fill(const rgba8& f)
{
	path_attributes& attr = cur_attr();
	attr.fill_color = f;
	attr.fill_flag = true;
}

// stroke
void
DocumentBuilder::stroke(const rgba8& s)
{
	path_attributes& attr = cur_attr();
	attr.stroke_color = s;
	attr.stroke_flag = true;
}

// even_odd
void
DocumentBuilder::even_odd(bool flag)
{
	cur_attr().even_odd_flag = flag;
}

// stroke_width
void
DocumentBuilder::stroke_width(double w)
{
	path_attributes& attr = cur_attr();
	attr.stroke_width = w;
	attr.stroke_flag = true;
}

// fill_none
void
DocumentBuilder::fill_none()
{
	cur_attr().fill_flag = false;
}

// fill_url
void
DocumentBuilder::fill_url(const char* url)
{
	sprintf(cur_attr().fill_url, "%s", url);
}

// stroke_none
void
DocumentBuilder::stroke_none()
{
	cur_attr().stroke_flag = false;
}

// stroke_url
void
DocumentBuilder::stroke_url(const char* url)
{
	sprintf(cur_attr().stroke_url, "%s", url);
}

// opacity
void
DocumentBuilder::opacity(double op)
{
	cur_attr().opacity *= op;
//printf("opacity: %.1f\n", cur_attr().opacity);
}

// fill_opacity
void
DocumentBuilder::fill_opacity(double op)
{
	cur_attr().fill_color.opacity(op);
//	cur_attr().opacity *= op;
}

// stroke_opacity
void
DocumentBuilder::stroke_opacity(double op)
{
	cur_attr().stroke_color.opacity(op);
//	cur_attr().opacity *= op;
}

// line_join
void
DocumentBuilder::line_join(line_join_e join)
{
	cur_attr().line_join = join;
}

// line_cap
void
DocumentBuilder::line_cap(line_cap_e cap)
{
	cur_attr().line_cap = cap;
}

// miter_limit
void
DocumentBuilder::miter_limit(double ml)
{
	cur_attr().miter_limit = ml;
}

// transform
trans_affine&
DocumentBuilder::transform()
{
	return cur_attr().transform;
}

// parse_path
void
DocumentBuilder::parse_path(PathTokenizer& tok)
{
	char lastCmd = 0;
	while(tok.next()) {
		double arg[10];
		char cmd = tok.last_command();
		unsigned i;
		switch(cmd) {
			case 'M': case 'm':
				arg[0] = tok.last_number();
				arg[1] = tok.next(cmd);
				if (lastCmd != cmd)
					move_to(arg[0], arg[1], cmd == 'm');
				else
					line_to(arg[0], arg[1], lastCmd == 'm');
				break;

			case 'L': case 'l':
				arg[0] = tok.last_number();
				arg[1] = tok.next(cmd);
				line_to(arg[0], arg[1], cmd == 'l');
				break;

			case 'V': case 'v':
				vline_to(tok.last_number(), cmd == 'v');
				break;

			case 'H': case 'h':
				hline_to(tok.last_number(), cmd == 'h');
				break;

			case 'Q': case 'q':
				arg[0] = tok.last_number();
				for(i = 1; i < 4; i++) {
					arg[i] = tok.next(cmd);
				}
				curve3(arg[0], arg[1], arg[2], arg[3], cmd == 'q');
				break;

			case 'T': case 't':
				arg[0] = tok.last_number();
				arg[1] = tok.next(cmd);
				curve3(arg[0], arg[1], cmd == 't');
				break;

			case 'C': case 'c':
				arg[0] = tok.last_number();
				for(i = 1; i < 6; i++) {
					arg[i] = tok.next(cmd);
				}
				curve4(arg[0], arg[1], arg[2], arg[3], arg[4], arg[5], cmd == 'c');
				break;

			case 'S': case 's':
				arg[0] = tok.last_number();
				for(i = 1; i < 4; i++) {
					arg[i] = tok.next(cmd);
				}
				curve4(arg[0], arg[1], arg[2], arg[3], cmd == 's');
				break;

			case 'A': case 'a': {
				arg[0] = tok.last_number();
				for(i = 1; i < 3; i++) {
					arg[i] = tok.next(cmd);
				}
				bool large_arc_flag = (bool)tok.next(cmd);
				bool sweep_flag = (bool)tok.next(cmd);
				for(i = 3; i < 5; i++) {
					arg[i] = tok.next(cmd);
				}
				elliptical_arc(arg[0], arg[1], arg[2],
							   large_arc_flag, sweep_flag,
							   arg[3], arg[4], cmd == 'a');
				break;
			}

			case 'Z': case 'z':
				close_subpath();
				break;

			default:
			{
				char buf[100];
				sprintf(buf, "parse_path: Invalid path command '%c'", cmd);
				throw exception(buf);
			}
		}
		lastCmd = cmd;
	}
}

// convert_cap_mode
static uint32
convert_cap_mode(line_cap_e aggMode)
{
	uint32 mode = CAP_MODE_BUTT;
	switch (aggMode) {
		case butt_cap:
			mode = CAP_MODE_BUTT;
			break;
		case square_cap:
			mode = CAP_MODE_SQUARE;
			break;
		case round_cap:
			mode = CAP_MODE_ROUND;
			break;
	}
	return mode;
}

// convert_cap_mode
static uint32
convert_join_mode(line_join_e aggMode)
{
	uint32 mode = JOIN_MODE_MITER;
	switch (aggMode) {
		case miter_join:
			mode = JOIN_MODE_MITER;
			break;
		case round_join:
			mode = JOIN_MODE_ROUND;
			break;
		case bevel_join:
			mode = JOIN_MODE_BEVEL;
			break;
		default:
			break;
	}
	return mode;
}

// GetCanvas
Canvas*
DocumentBuilder::GetCanvas(const char* fallbackName)
{
	double xMin;
	double yMin;
	double xMax;
	double yMax;

	int32 pathCount = fAttributesStorage.size();

	agg::conv_transform<agg::path_storage, agg::trans_affine> transformedPaths(fPathStorage, fTransform);
	agg::bounding_rect(transformedPaths, *this, 0, pathCount, &xMin, &yMin, &xMax, &yMax);

	xMin = floor(xMin);
	yMin = floor(yMin);
	xMax = ceil(xMax);
	yMax = ceil(yMax);

	BRect bounds;
	if (fViewBox.IsValid())
		bounds = fViewBox;
	else
		bounds.Set(0.0, 0.0, (int32)fWidth - 1, (int32)fHeight - 1);

	BRect boundingBox(xMin, yMin, xMax, yMax);

	if (!bounds.IsValid() || !boundingBox.Intersects(bounds)) {
		bounds = boundingBox;
	}

	BPoint offset(-bounds.left, -bounds.top);
	bounds.OffsetTo(B_ORIGIN);

	Canvas* canvas = new Canvas(bounds);
	if (fTitle.CountChars() > 0) {
		canvas->SetName(fTitle.String());
	} else {
		canvas->SetName(fallbackName);
	}

	Layer* layer = new Layer(bounds);
	canvas->AddLayer(layer);
	History* history = layer->GetHistory();

	for (int32 i = 0; i < pathCount; i++) {

		path_attributes& attributes = fAttributesStorage[i];

		if (attributes.fill_flag) {
			_AddShape(attributes, false, offset, layer, history);
		}
		if (attributes.stroke_flag) {
			_AddShape(attributes, true, offset, layer, history);
		}
	}
	if (layer->InitCheck() >= B_OK) {
		// now we need to render the layer
		BRect mergeRect = bounds;
		BBitmap alphaMap(bounds, 0, B_GRAY8);

		if (alphaMap.InitCheck() >= B_OK) {
			for (int32 i = 0; Stroke* stroke = history->ModifierAt(i); i++) {
				if (stroke->Bounds().Intersects(bounds)) {
					clear_area(&alphaMap, mergeRect);
					mergeRect = bounds & stroke->Bounds();

					mergeRect.left = floorf(mergeRect.left);
					mergeRect.top = floorf(mergeRect.top);
					mergeRect.right = ceilf(mergeRect.right);
					mergeRect.bottom = ceilf(mergeRect.bottom);

					stroke->Draw(&alphaMap, mergeRect);
					stroke->MergeWithBitmap(layer->Bitmap(), &alphaMap, mergeRect, 0);
				}
			}
		}
	}

	return canvas;
}

// StartGradient
void
DocumentBuilder::StartGradient(bool radial)
{
	if (fCurrentGradient) {
		fprintf(stderr, "DocumentBuilder::StartGradient() - ERROR: "
						"previous gradient (%s) not finished!\n", fCurrentGradient->ID());
	}

	fCurrentGradient = radial ? (SVGGradient*)(new SVGRadialGradient())
		: (SVGGradient*)(new SVGLinearGradient());

	_AddGradient(fCurrentGradient);
}

// EndGradient
void
DocumentBuilder::EndGradient()
{
	if (fCurrentGradient) {
//		fCurrentGradient->PrintToStream();
	} else {
		fprintf(stderr, "DocumentBuilder::EndGradient() - ERROR: no gradient started!\n");
	}
	fCurrentGradient = NULL;
}

// _AddGradient
void
DocumentBuilder::_AddGradient(SVGGradient* gradient)
{
	if (gradient) {
		fGradients.AddItem((void*)gradient);
	}
}

// _GradientAt
SVGGradient*
DocumentBuilder::_GradientAt(int32 index) const
{
	return (SVGGradient*)fGradients.ItemAt(index);
}

// _FindGradient
SVGGradient*
DocumentBuilder::_FindGradient(const char* name) const
{
	for (int32 i = 0; SVGGradient* g = _GradientAt(i); i++) {
		if (strcmp(g->ID(), name) == 0)
			return g;
	}
	return NULL;
}

// _AddShape
void
DocumentBuilder::_AddShape(path_attributes& attributes, bool outline,
						   const BPoint& offset, Layer* layer, History* history)
{
	rgb_color color;
	ShapeStroke* object = new ShapeStroke(color);

	object->AddVertexSource(fPathStorage, attributes.index);

	object->multiply(attributes.transform);
	object->TranslateBy(offset);

	if (outline) {
		object->SetAlpha((uint8)(attributes.stroke_color.a * attributes.opacity));
		object->SetOutline(true);
		object->SetOutlineWidth(attributes.stroke_width);
		object->SetCapMode(convert_cap_mode(attributes.line_cap));
		object->SetJoinMode(convert_join_mode(attributes.line_join));
	} else {
		object->SetAlpha((uint8)(attributes.fill_color.a * attributes.opacity));
		if (attributes.even_odd_flag)
			object->SetFillingRule(FILL_MODE_EVEN_ODD);
		else
			object->SetFillingRule(FILL_MODE_NON_ZERO);
	}

	object->UpdateBounds();

	Gradient* gradient = NULL;
	const char* url = outline ? attributes.stroke_url : attributes.fill_url;
	if (url[0] != 0) {
		if (SVGGradient* g = _FindGradient(url)) {
			gradient = g->GetGradient(object->Bounds());
		}
	}

	color_step* step;
	if (gradient && (step = gradient->ColorAt(0))) {
		color		= step->color;
	} else {
		if (outline) {
			color.red	= attributes.stroke_color.r;
			color.green	= attributes.stroke_color.g;
			color.blue	= attributes.stroke_color.b;
			color.alpha	= 255;
		} else {
			color.red	= attributes.fill_color.r;
			color.green	= attributes.fill_color.g;
			color.blue	= attributes.fill_color.b;
			color.alpha	= 255;
		}
	}

	if (gradient) {
		object->SetGradient(gradient);
		object->SetMode(MODE_GRADIENT);
		object->SetRenderer(object->DefaultRenderer());
		delete gradient;
	} else {
		object->SetColor(color);
	}

	layer->Touch(object->Bounds());

	history->AddModifier(object);
}

} // namespace svg
} // namespace agg

