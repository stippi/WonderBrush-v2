// SVGExporter.cpp

#include <stdio.h>

#include <Alert.h>
#include <Bitmap.h>
#include <BitmapStream.h>
#include <DataIO.h>
#include <File.h>
#include <String.h>
#include <TranslatorRoster.h>

#include "bitmap_support.h"
#include "defines.h"
#include "support.h"

#include "BitmapStroke.h"
#include "Canvas.h"
#include "ExportPanel.h"
#include "FillStroke.h"
#include "Gradient.h"
#include "History.h"
#include "LanguageManager.h"
#include "Layer.h"
#include "PenStroke.h"
#include "ShapeStroke.h"
#include "TextRenderer.h"
#include "TextStroke.h"
#include "VectorPath.h"

#include "SVGExporter.h"

class Base64Encoder {
 public:
								Base64Encoder() {}
	virtual						~Base64Encoder() {}

			status_t			Encode(BPositionIO* input,
									   BPositionIO* output) const;
 private:
			void				_Convert(const uint8 src[3], uint8 dst[4],
										 int32 validBytes = 3) const;

};

// Encode
status_t
Base64Encoder::Encode(BPositionIO* input, BPositionIO* output) const
{
	status_t ret = B_BAD_VALUE;
	if (input && output) {
		uint8 src[3];
		uint8 dst[4];

		ssize_t read = input->Read(src, 3);
		ssize_t written = 0;

		char lineBreak[2];
		lineBreak[0] = '\n';
		lineBreak[1] = 0;

		while (read > B_OK) {
			// convert and write data
			_Convert(src, dst, read);
			ret = output->Write(dst, 4);

			// handle write error
			if (ret < 4) {
				if (ret >= B_OK)
					ret = B_ERROR;
				break;
			}
			// insert linebreaks
			written += ret;
			if (written % 76 == 0) {
				ret = output->Write(lineBreak, 1);
				// handle write error
				if (ret < 1)
					break;
			}
			// read next input
			read = input->Read(src, 3);
		}
	}
	return ret;
}

static const uint8 kAlphabet[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
								   'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
								   'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
								   'Y', 'Z' ,'a', 'b', 'c', 'd', 'e', 'f',
								   'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
								   'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
								   'w', 'x', 'y', 'z', '0', '1', '2', '3',
								   '4', '5', '6', '7', '8', '9', '+', '/' };

// _Convert
void
Base64Encoder::_Convert(const uint8 src[3], uint8 dst[4], int32 validBytes) const
{
	if (validBytes > 0 && validBytes <= 3) {
		// x x x x   x x x x | x x x x   x x x x | x x x x   x x x x
		// a a a a   a a|a a   b b b b | b b b b   c c|c c   c c c c
		switch (validBytes) {
			case 3:
				dst[0] = kAlphabet[ (src[0] & 252) >> 2 ];
				dst[1] = kAlphabet[ ((src[0] & 3) << 4) | ((src[1] & 240) >> 4) ];
				dst[2] = kAlphabet[ ((src[1] & 15) << 2) | ((src[2] & 192) >> 6) ];
				dst[3] = kAlphabet[ src[2] & 63 ];
				break;
			case 2:
				dst[0] = kAlphabet[ (src[0] & 252) >> 2 ];
				dst[1] = kAlphabet[ ((src[0] & 3) << 4) | ((src[1] & 240) >> 4) ];
				dst[2] = kAlphabet[ ((src[1] & 15) << 2) ];
				dst[3] = '=';
				break;
			case 1:
				dst[0] = kAlphabet[ (src[0] & 252) >> 2 ];
				dst[1] = kAlphabet[ ((src[0] & 3) << 4) ];
				dst[2] = '=';
				dst[3] = '=';
				break;
		} 
	} else {
		fprintf(stderr, "Base64Encoder::_Convert() - invalid byte count: %ld\n", validBytes);
	}
}

// write_line
status_t
write_line(BPositionIO* stream, BString& string)
{
	ssize_t written = stream->Write(string.String(), string.Length());
	if (written > B_OK && written < string.Length())
		written = B_ERROR;
	string.SetTo("");
	return written;
}

// constructor
SVGExporter::SVGExporter()
	: Exporter(),
	  fMaskCount(0),
	  fGradientCount(0),
	  fOriginalEntry(NULL)
{
}

// destructor
SVGExporter::~SVGExporter()
{
	delete fOriginalEntry;
}

// Export
status_t
SVGExporter::Export(const Canvas* canvas, const CanvasView* canvasView,
					BPositionIO* stream, const entry_ref* refToFinalFile)
{
	if (canvas == NULL || stream == NULL)
		return B_BAD_VALUE;

	if (fOriginalEntry && *fOriginalEntry == *refToFinalFile) {
		if (!_DisplayWarning()) {
			return B_CANCELED;
		} else {
			delete fOriginalEntry;
			fOriginalEntry = NULL;
		}
	}

	BString helper;

	fMaskCount = 0;
	fGradientCount = 0;

	// header
	helper << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n";
	status_t ret = write_line(stream, helper);

	// image size
	if (ret >= B_OK) {
		helper << "<svg width=\""	<< canvas->Bounds().IntegerWidth() + 1 << "\""
			   << " height=\""	<< canvas->Bounds().IntegerHeight() + 1 << "\""
			   << " color-interpolation=\"linearRGB\""
			   << " xmlns:svg=\"http://www.w3.org/2000/svg\""
			   << " xmlns:xlink=\"http://www.w3.org/1999/xlink\""
			   << " xmlns=\"http://www.w3.org/2000/svg\">\n";
		ret = write_line(stream, helper);
	}

	if (ret < B_OK)
		return ret;

	bool relativeCoords = !(modifiers() & B_SHIFT_KEY);

	// layers
	int32 count = canvas->CountLayers();
	for (int32 i = count - 1; const Layer* layer = canvas->LayerAt(i); i--) {
		const History* history = layer->GetHistory();
		if (history == NULL)
			continue;

		helper << " <g";
		// isn't there a better way?!?
		if (layer->Flags() & FLAG_INVISIBLE) {
			helper << " visibility=\"hidden\"";
		}

		// group opacity
		float alpha = layer->Alpha();
		if (alpha < 1.0) {
			helper << " opacity=\"";
			append_float(helper, alpha);
			helper << "\"";
		}
		helper << ">\n";
		ret = write_line(stream, helper);
		if (ret < B_OK)
			break;

		// export all known objects
		for (int32 j = 0; const Stroke* stroke = history->ModifierAt(j); j++) {
			if (const ShapeStroke* object = dynamic_cast<const ShapeStroke*>(stroke)) {
				// avoid writing a transformation matrix for non-outline objects
				ShapeStroke* clone = NULL;
				if (!object->IsIdentity()
					&& (!object->IsOutline() || object->IsNotDistorted())
					&& (clone = dynamic_cast<ShapeStroke*>(object->Clone()))) {
					clone->FreezeTransformations();
					ret = _ExportShape(clone, relativeCoords, stream);
					delete clone;
				} else
					ret = _ExportShape(object, relativeCoords, stream);
			} else if (const BitmapStroke* object = dynamic_cast<const BitmapStroke*>(stroke)) {
				ret = _ExportBitmap(object, stream);
			} else if (const TextStroke* object = dynamic_cast<const TextStroke*>(stroke)) {
				ret = _ExportText(object, stream);
			} else if (const PenStroke* object = dynamic_cast<const PenStroke*>(stroke)) {
				ret = _ExportPen(object, stream);
			} else if (const FillStroke* object = dynamic_cast<const FillStroke*>(stroke)) {
				ret = _ExportFill(object, stream);
			}
			if (ret < B_OK)
				break;
		}

		// finish layer
		if (ret >= B_OK) {
			helper << " </g>\n";
			ret = write_line(stream, helper);
		} else
			break;
	}

	// footer
	if (ret >= B_OK) {
		helper << "</svg>\n";
		ret = write_line(stream, helper);
	}

	return ret;
}

// MIMEType
const char*
SVGExporter::MIMEType() const
{
	return "image/svg+xml";
}

// Extension
const char*
SVGExporter::Extension() const
{
	return "svg";
}

// CustomizeExportPanel
void
SVGExporter::CustomizeExportPanel(ExportPanel* panel)
{
	panel->SetExportMode(EXPORT_SVG);
}

// SetWarnOverwrite
void
SVGExporter::SetOriginalEntry(const entry_ref* ref)
{
	if (ref) {
		delete fOriginalEntry;
		fOriginalEntry = new entry_ref(*ref);
	}
}

// DisplayWarning
bool
SVGExporter::_DisplayWarning() const
{
	LanguageManager* m = LanguageManager::Default();
	BAlert* alert = new BAlert("warning",
							   m->GetString(WARN_SVG_DATA_LOSS,
							   				"WonderBrush might not have "
							   				"interpreted all data from the SVG "
							   				"when it was loaded. "
							   				"By overwriting the original "
							   				"file, this information would now "
							   				"be lost."),
							   m->GetString(CANCEL, "Cancel"),
							   m->GetString(OVERWRITE, "Overwrite"));
	return alert->Go() == 1;
}

// #pragma mark -

// convert_join_mode_svg
const char*
convert_join_mode_svg(uint32 mode)
{
	const char* svgMode = "miter";
	switch (mode) {
		case JOIN_MODE_MITER:
			svgMode = "miter";
			break;
		case JOIN_MODE_ROUND:
			svgMode = "round";
			break;
		case JOIN_MODE_BEVEL:
			svgMode = "bevel";
			break;
	}
	return svgMode;
}

// convert_cap_mode_svg
const char*
convert_cap_mode_svg(uint32 mode)
{
	const char* svgMode = "butt";
	switch (mode) {
		case CAP_MODE_BUTT:
			svgMode = "butt";
			break;
		case CAP_MODE_SQUARE:
			svgMode = "square";
			break;
		case CAP_MODE_ROUND:
			svgMode = "round";
			break;
	}
	return svgMode;
}

// #pragma mark -

// _ExportShape
status_t
SVGExporter::_ExportShape(const ShapeStroke* object, bool relativeCoords,
						  BPositionIO* stream)
{
	BString helper;

	status_t ret = B_OK;

	char color[64];
	ret = _GetFill(object, color, stream);

	if (ret < B_OK)
		return ret;

	// The transformation matrix is extracted again in order to
	// maintain the effect of a distorted outline. There is of
	// course a difference when applying the transformation to
	// the points of the path, then stroking the transformed
	// path with an outline of a certain width, opposed to applying
	// the transformation to the points of the generated stroke
	// as well. Adobe SVG Viewer is supporting this fairly well,
	// though I have come across SVG software that doesn't (InkScape).

	// start new object and write transform matrix
	helper << "  <path ";

	if (object->IsOutline()) {
		helper << "style=\"fill:none; stroke:" << color;
		if (object->Alpha() < 255) {
			helper << "; stroke-opacity:";
			append_float(helper, object->Alpha() / 255.0);
		}
		helper << "; stroke-width:";
		append_float(helper, object->OutlineWidth());

		if (object->CapMode() != CAP_MODE_BUTT) {
			helper << "; stroke-linecap:";
			helper << convert_cap_mode_svg(object->CapMode());
		}

		if (object->JoinMode() != JOIN_MODE_MITER) {
			helper << "; stroke-linejoin:";
			helper << convert_join_mode_svg(object->JoinMode());
		}

		helper << "\"\n";
	} else {
		helper << "style=\"fill:" << color;

		if (object->Alpha() < 255) {
			helper << "; fill-opacity:";
			append_float(helper, object->Alpha() / 255.0);
		}

		if (object->FillingRule() == FILL_MODE_EVEN_ODD &&
			object->CountPaths() > 1)
			helper << "; fill-rule:evenodd";

		helper << "\"\n";
	}

	helper << "        d=\"";
	ret = write_line(stream, helper);

	if (ret >= B_OK) {
		BPoint a, aIn, aOut;
		BPoint b, bIn, bOut;

		for (int32 i = 0; VectorPath* path = object->PathAt(i); i++) {

			if (i > 0) {
				helper << "\n           ";
			}


			if (path->GetPointAt(0, a)) {
				// inverse transform point to get native location
				object->InverseTransform(&a);
	
				helper << "M";
				append_float(helper, a.x, 2);
				helper << " ";
				append_float(helper, a.y, 2);
			}
	
			for (int32 i = 0; ret >= B_OK && 
							  path->GetPointsAt(i, a, aIn, aOut); i++) {
			
				if (path->GetPointsAt(i + 1, b, bIn, bOut)
					|| (path->IsClosed() && path->GetPointsAt(0, b, bIn, bOut))) {
	
					// inverse transform points to get native locations
					object->InverseTransform(&a);
					object->InverseTransform(&aOut);
					object->InverseTransform(&bIn);
					object->InverseTransform(&b);

					if (relativeCoords) {
						if (aOut == a && bIn == b) {
							if (a.x == b.x) {
								helper << "v";
								append_float(helper, b.y - a.y, 2);
							} else if (a.y == b.y) {
								helper << "h";
								append_float(helper, b.x - a.x, 2);
							} else {
								helper << "l";
								append_float(helper, b.x - a.x, 2);
								helper << " ";
								append_float(helper, b.y - a.y, 2);
							}
						} else {
							helper << "c";
							append_float(helper, aOut.x - a.x, 2);
							helper << " ";
							append_float(helper, aOut.y - a.y, 2);
							helper << " ";
							append_float(helper, bIn.x - a.x, 2);
							helper << " ";
							append_float(helper, bIn.y - a.y, 2);
							helper << " ";
							append_float(helper, b.x - a.x, 2);
							helper << " ";
							append_float(helper, b.y - a.y, 2);
						}
					} else {
						if (aOut == a && bIn == b) {
							if (a.x == b.x) {
								helper << "V";
								append_float(helper, b.y, 2);
							} else if (a.y == b.y) {
								helper << "H";
								append_float(helper, b.x, 2);
							} else {
								helper << "L";
								append_float(helper, b.x, 2);
								helper << " ";
								append_float(helper, b.y, 2);
							}
						} else {
							helper << "C";
							append_float(helper, aOut.x, 2);
							helper << " ";
							append_float(helper, aOut.y, 2);
							helper << " ";
							append_float(helper, bIn.x, 2);
							helper << " ";
							append_float(helper, bIn.y, 2);
							helper << " ";
							append_float(helper, b.x, 2);
							helper << " ";
							append_float(helper, b.y, 2);
						}
					}
				}
			}
			if (path->IsClosed())
				helper << "z";
			ret = write_line(stream, helper);
		}
		helper << "\"\n";
	}

	if (!object->IsIdentity()) {
		helper << "        transform=\"";
		_AppendMatrix(object, helper);
		helper << "\n";
	}

	if (ret >= B_OK) {
		helper << "  />\n";

		ret = write_line(stream, helper);
	}

	return ret;
}

// _ExportBitmap
status_t
SVGExporter::_ExportBitmap(const BitmapStroke* object, BPositionIO* stream)
{
	status_t ret = B_OK;
	BBitmap* bitmap = NULL;
	if (object->Bitmap()) {
		bitmap = new BBitmap(object->Bitmap());
		ret = bitmap->InitCheck();
		if (ret >= B_OK) {
			demultiply_alpha(bitmap);
		} else {
			delete bitmap;
			bitmap = NULL;
		}
	}
	if (bitmap) {

		int32 width = bitmap->Bounds().IntegerWidth() + 1;
		int32 height = bitmap->Bounds().IntegerHeight() + 1;

		BString data;
		data << "  <image x=\"" << (int32)bitmap->Bounds().left << "\"";
		data << " y=\"" << (int32)bitmap->Bounds().top << "\"";
		data << " width=\"" << width << "px\" height=\"" << height << "px\"\n";
		if (object->Interpolation() == BitmapStroke::INTERPOLATION_NN) {
			data << " image-rendering=\"optimizeSpeed\"\n";
		} else if (object->Interpolation() == BitmapStroke::INTERPOLATION_RESAMPLE) {
			data << " image-rendering=\"optimizeQuality\"\n";
		}
		data << "         xlink:href=\"data:image/png;base64,\n";

		ret = write_line(stream, data);

		if (ret >= B_OK) {
			BMallocIO pngStream;
			BTranslatorRoster* roster = BTranslatorRoster::Default();
			BBitmapStream bitmapStream(bitmap);
			ret = roster->Translate(&bitmapStream, NULL, NULL, &pngStream,
									B_PNG_FORMAT, 0);
	
			if (ret >= B_OK) {
				Base64Encoder encoder;
				pngStream.Seek(0, SEEK_SET);
				ret = encoder.Encode(&pngStream, stream);
				if (ret >= B_OK) {
					data << "\"\n";
					if (!object->IsIdentity()) {
						data << "         transform=\"";
						_AppendMatrix(object, data);
						data << "\n";
					}
					data << "  />\n";
					ret = write_line(stream, data);
				} else
					fprintf(stderr, "encoding to base64 failed: %s\n", strerror(ret));
			} else {
				fprintf(stderr, "translating to png failed: %s\n", strerror(ret));
			}

			bitmapStream.DetachBitmap(&bitmap);
		}
	}
	delete bitmap;
	return ret;
}

// _ExportText
status_t
SVGExporter::_ExportText(const TextStroke* object, BPositionIO* stream)
{
	status_t ret = B_OK;
	if (TextRenderer* textRenderer = object->GetTextRenderer()) {
		const char* text = textRenderer->Text();
		if (text && strlen(text) > 0) {
			// export color
			char color[64];
			ret = _GetFill(object, color, stream);
		
			if (ret < B_OK)
				return ret;

			BString data;
			data << "  <text x=\"0\" y=\"0\"\n";

			data << "        style=\"";
			data << "fill:#" << color;
	
			// opacity
			if (textRenderer->Opacity() < 255) {
				data << "; fill-opacity:";
				append_float(data, textRenderer->Opacity() / 255.0);
			}
	
			// font family and style
			if (textRenderer->Family())
				data << "; font-family:" << textRenderer->Family();
			if (textRenderer->Style()) {
				BString style(textRenderer->Style());
				if (style.IFindFirst("italic") > B_ERROR)
					data << "; font-style:italic";
				else if(style.IFindFirst("oblique") > B_ERROR)
					data << "; font-style:oblique";
				else
					data << "; font-style:normal";
				if (style.IFindFirst("bold") > B_ERROR)
					data << "; font-weight:bold";
			}
			// text size
			data << "; font-size:";
			float ptSize = textRenderer->PointSize() * 16.0;
			append_float(data, ptSize, 1);

			// letter spacing
			// TODO: validate this works
//			if (textRenderer->AdvanceScale() != 1.0) {
//				data << "; letter-spacing:";
//				append_float(data, (textRenderer->AdvanceScale() - 1.0) * ptSize);
//			}
	
			// finish "style" attribute
			data << "\"\n";

			// transformation
			if (!object->IsIdentity()) {
				data << "        transform=\"";
				_AppendMatrix(object, data);
				data << "\n";
			}

/*			if (textRenderer->TextWidth() > 0.0) {
				data << "        text-length=\"";
				append_float(data, textRenderer->TextWidth(), 2);
			}*/
				
	
			// finish text tag
			data << "  >\n";
	
			// put each line into a separate tspan tag
			bool finished = true;
			float y = 0.0;
			while (*text) {
				if (*text == '\n') {
					data << "</tspan>\n";
					finished = true;
					y += textRenderer->LineOffset();
				} else {
					if (finished) {
						data << "    <tspan x=\"0\" y=\"";
						append_float(data, y);
						data << "\">";
					}
					finished = false;
					data << *text;
				}
				text++;
			}
			// just in case the text contained no linebreaks
			if (!finished) {
				data << "</tspan>\n";
			}

			// finish text object
			data << "  </text>\n";
			ret = write_line(stream, data);
		}
	}
	return ret;
}

// _ExportPen
status_t
SVGExporter::_ExportPen(const PenStroke* object,
						BPositionIO* stream)
{
	status_t ret;
	// eraser pens are not supported yet
	if (!object->IsColorObject())
		return B_OK;

	BString helper;

	char color[64];
	ret = _GetFill(object, color, stream);

	if (ret < B_OK)
		return ret;

	// start new object
	helper << "  <path\n";

	helper << "        style=\"fill:none; stroke:#" << color;
	if (object->Alpha().max < 1.0) {
		helper << "; stroke-opacity:";
		append_float(helper, object->Alpha().max);
	}
	helper << "; stroke-width:";
	append_float(helper, 1.0);

	helper << "\"\n";

	helper << "        d=\"";
	ret = write_line(stream, helper);

	if (ret >= B_OK) {
		Point p;

		// the pen stroke rendering in WB is not based on AGG and treats
		// the top-left corner of a pixel as the center, until this is
		// fixed, we need this conversion.
		BPoint offset(0.5, 0.5);

		if (object->GetPointAt(0, p)) {

			p.point = p.point + offset;

			helper << "M";
			append_float(helper, p.point.x);
			helper << " ";
			append_float(helper, p.point.y);
		}

		for (int32 i = 1; object->GetPointAt(i, p); i++) {

			p.point = p.point + offset;
		
			helper << " L";
			append_float(helper, p.point.x);
			helper << " ";
			append_float(helper, p.point.y);
		}

		helper << "\"\n";
		ret = write_line(stream, helper);
	}

	if (ret >= B_OK) {
		helper << "  />\n";

		ret = write_line(stream, helper);
	}

	return ret;
}

// _ExportFill
status_t
SVGExporter::_ExportFill(const FillStroke* object, BPositionIO* stream)
{
	FillStroke clone(*object);
	// create a bitmap for painting the object in
	BRect bounds = object->Bounds();
	clone.TranslateBy(BPoint(-bounds.left, -bounds.top));
	BBitmap* bitmap = new BBitmap(bounds.OffsetToCopy(0.0, 0.0), 0, B_GRAY8);
	status_t ret = bitmap->InitCheck();
	if (ret >= B_OK) {
		// fill bitmap with object
		memset(bitmap->Bits(), 0, bitmap->BitsLength());
		clone.Draw(bitmap);

		int32 width = bitmap->Bounds().IntegerWidth() + 1;
		int32 height = bitmap->Bounds().IntegerHeight() + 1;

		BString data;
		data << "  <g>\n";

		BString maskName("mask");
		maskName << fMaskCount++;

		data << "   <mask id=\"" << maskName << "\"";
//		data << " x=\"" << (int32)bounds.left << "\"";
//		data << " y=\"" << (int32)bounds.top << "\"";
//		data << " width=\"" << width << "\" height=\"" << height << "\"";
//		data << " maskUnits=\"userSpaceOnUse\"";
		data << ">\n";
		// the image is used as a mask
		data << "    <image x=\"" << (int32)bounds.left << "\"";
		data << " y=\"" << (int32)bounds.top << "\"";
		data << " width=\"" << width << "px\" height=\"" << height << "px\"\n";
		data << "           xlink:href=\"data:image/png;base64,\n";

		ret = write_line(stream, data);

		if (ret >= B_OK) {
			BMallocIO pngStream;
//BFile pngStream("/boot/home/Desktop/pngStream.png", B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
			BTranslatorRoster* roster = BTranslatorRoster::Default();
			BBitmapStream bitmapStream(bitmap);
			ret = roster->Translate(&bitmapStream, NULL, NULL, &pngStream,
									B_PNG_FORMAT, 0);
	
			if (ret >= B_OK) {
				Base64Encoder encoder;
				pngStream.Seek(0, SEEK_SET);
				ret = encoder.Encode(&pngStream, stream);
				if (ret >= B_OK) {
					// end of encoded PNG image
					data << "\"\n";
					// close image tag
					data << "    />\n";
					// close mask tag
					data << "   </mask>\n";
					// create rect object which is then masked away by the bitmap
					data << "   <rect";
					data << " x=\"" << (int32)bounds.left << "\"";
					data << " y=\"" << (int32)bounds.top << "\"";
					data << " width=\"" << width << "\" height=\"" << height << "\"";
					// color
					char color[16];
					sprintf(color, "%.2x%.2x%.2x", object->Color().red,
												   object->Color().green,
												   object->Color().blue);
					color[6] = 0;
					data << " fill=\"#" << color << "\"";
					data << " mask=\"url(#" << maskName << ")\"";
					// finish rect
					data << "/>\n";

					// finish the entire object
					data << "  </g>\n";

					ret = write_line(stream, data);



				} else
					fprintf(stderr, "encoding to base64 failed: %s\n", strerror(ret));
			} else {
				fprintf(stderr, "translating to png failed: %s\n", strerror(ret));
			}

			bitmapStream.DetachBitmap(&bitmap);
		}
	}
	delete bitmap;
	return ret;
}

// _ExportGradient
status_t
SVGExporter::_ExportGradient(const Gradient* gradient, BPositionIO* stream)
{
	status_t ret = B_OK;
	BString helper;

	// start new gradient tag
	if (gradient->Type() == GRADIENT_CIRCULAR) {
		helper << "  <radialGradient ";
	} else {
		helper << "  <linearGradient ";
	}

	// id
	BString gradientName("gradient");
	gradientName << fGradientCount;

	helper << "id=\"" << gradientName << "\" gradientUnits=\"userSpaceOnUse\"";

	// write gradient transformation
	if (gradient->Type() == GRADIENT_CIRCULAR) {
		helper << " cx=\"0\" cy=\"0\" r=\"200\"";
		if (!gradient->IsIdentity()) {
			helper << " gradientTransform=\"";
			_AppendMatrix(gradient, helper);
		}
	} else {
		double x1 = -200.0;
		double y1 = -200.0;
		double x2 = 200.0;
		double y2 = -200.0;
		gradient->Transform(&x1, &y1);
		gradient->Transform(&x2, &y2);

		helper << " x1=\"";
		append_float(helper, x1, 2);
		helper << "\"";
		helper << " y1=\"";
		append_float(helper, y1, 2);
		helper << "\"";
		helper << " x2=\"";
		append_float(helper, x2, 2);
		helper << "\"";
		helper << " y2=\"";
		append_float(helper, y2, 2);
		helper << "\"";
	}

	helper << ">\n";

	// write stop tags
	char color[16];
	for (int32 i = 0; color_step* stop = gradient->ColorAt(i); i++) {

		sprintf(color, "%.2x%.2x%.2x", stop->color.red,
									   stop->color.green,
									   stop->color.blue);
		color[6] = 0;

		helper << "   <stop offset=\"";
		append_float(helper, stop->offset);
		helper << "\" stop-color=\"#" << color << "\"";

		if (stop->color.alpha < 255) {
			helper << " stop-opacity=\"";
			append_float(helper, (float)stop->color.alpha / 255.0);
			helper << "\"";
		}
		helper << "/>\n";
	}

	// finish gradient tag
	if (gradient->Type() == GRADIENT_CIRCULAR) {
		helper << "  </radialGradient>\n";
	} else {
		helper << "  </linearGradient>\n";
	}

	if (ret >= B_OK) {
		ret = write_line(stream, helper);
	}

	return ret;
}

// _AppendMatrix
void
SVGExporter::_AppendMatrix(const Transformable* object, BString& string) const
{
	string << "matrix(";
	append_float(string, object->sx);
	string << ",";
	append_float(string, object->shy);
	string << ",";
	append_float(string, object->shx);
	string << ",";
	append_float(string, object->sy);
	string << ",";
	append_float(string, object->tx);
	string << ",";
	append_float(string, object->ty);
	string << ")\"";
}

// _GetFill
status_t
SVGExporter::_GetFill(const Stroke* object, char* string,
					  BPositionIO* stream)
{
	status_t ret = B_OK;
	if (Gradient* gradient = object->GetGradient()) {
		ret = _ExportGradient(gradient, stream);
		sprintf(string, "url(#gradient%ld)", fGradientCount++);
	} else {
		sprintf(string, "#%.2x%.2x%.2x", object->Color().red,
										 object->Color().green,
										 object->Color().blue);
	}
	return ret;
}
