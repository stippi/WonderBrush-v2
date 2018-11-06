// AIExporter.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <DataIO.h>
#include <String.h>
#include <UTF8.h>

#include "defines.h"
#include "support.h"

#include "BitmapStroke.h"
#include "Canvas.h"
#include "ExportPanel.h"
#include "History.h"
#include "Layer.h"
#include "ShapeStroke.h"
#include "TextRenderer.h"
#include "TextStroke.h"
#include "VectorPath.h"

#include "AIExporter.h"

class CMYKColor
{
	public:
						CMYKColor()
							: init_ok(false)
						{}
						CMYKColor(const CMYKColor& color)
							: c(color.c),
							  m(color.m),
							  y(color.y),
							  k(color.k),
							  init_ok(color.init_ok)
						{}
						CMYKColor(const rgb_color& rgb)
							: init_ok(true)
						{
							c = 255 - rgb.red;
								k = c;
							m = 255 - rgb.green;
								if (k > m) k = m;
							y = 255 - rgb.blue;
								if (k > y) k = y;
							c -= k;
							m -= k;
							y -= k;
							if (c < 0) c = 0;
							if (m < 0) m = 0;
							if (y < 0) y = 0;
							if (k < 0) k = 0;
							c = c / 255.0;
							m = m / 255.0;
							y = y / 255.0;
							k = k / 255.0;
						}
		CMYKColor&		operator=(const CMYKColor &color)
						{
							c = color.c;
							m = color.m;
							y = color.y;
							k = color.k;
							init_ok = color.init_ok;
							return *this;
						};
		bool			operator!=(const CMYKColor &color) const
						{
							if (!color.init_ok || !init_ok)
								return true;
							if (color.c != c || color.m != m || color.y!= y || color.k != k)
								return true;
							return false;
						}

		float			c;
		float			m;
		float			y;
		float			k;
		bool			init_ok;
};

// #pragma mark -

class GraphicsState {
 public:
						GraphicsState()
							: fStream(NULL)
						{
							Reset();
						}
	virtual				~GraphicsState() {}
	
			bool		InitCheck() const
						{
							return fStream != NULL;
						}

			void		SetTo(BPositionIO* stream)
						{
							fStream = stream;
						}
	
			void		Reset()
						{
							fStrokeColor.init_ok = false;
							fFillColor.init_ok = false;	
							fStrokeWidth = -1.0;
							fJoinMode = -1;
							fCapMode = -1;
							fStrokeDashes = -1;
						}

		status_t		SetStrokeColor(CMYKColor color, bool force = false)
						{
							if (fStrokeColor != color || force) {
								BString data;
								append_float(data, color.c);
								data << " ";
								append_float(data, color.m);
								data << " ";
								append_float(data, color.y);
								data << " ";
								append_float(data, color.k);
								data << " K\n";
								fStrokeColor = color;
								return write_string(fStream, data);
							}
							return B_OK;
						}
		status_t		SetFillColor(CMYKColor color, bool force = false)
						{
							if (fFillColor != color || force) {
								BString data;
								append_float(data, color.c);
								data << " ";
								append_float(data, color.m);
								data << " ";
								append_float(data, color.y);
								data << " ";
								append_float(data, color.k);
								data << " k\n";
								fFillColor = color;
								return write_string(fStream, data);
							}
							return B_OK;
						}
		status_t		SetStrokeWidth(float width, bool force = false)
						{
							if (fStrokeWidth != width) {
								BString data;
								append_float(data, width);
								data << " w\n";
								fStrokeWidth = width;
								return write_string(fStream, data);
							}
							return B_OK;
						}
		status_t		SetJoinMode(int32 mode, bool force = false)
						{
							if (fJoinMode != mode || force) {
								BString data;
								data << mode << " j\n";
								fJoinMode = mode;
								return write_string(fStream, data);
							}
							return B_OK;
						}
		status_t		SetCapMode(int32 mode, bool force = false)
						{
							if (fCapMode != mode || force) {
								BString data;
								data << mode << " J\n";
								fCapMode = mode;
								return write_string(fStream, data);
							}
							return B_OK;
						}
		status_t		SetStrokeDashes(int32 dashes, bool force = false)
						{
							if (fStrokeDashes != dashes || force) {
								BString data;
								// no dashes supported yet, numbers go inbetween [] describing dashes
								data << "[] 0 d\n";
								fStrokeDashes = dashes;
								return write_string(fStream, data);
							}
							return B_OK;
						}

 private:
		BPositionIO*	fStream;
	
		CMYKColor		fStrokeColor;
		CMYKColor		fFillColor;	
		float			fStrokeWidth;
		int32			fJoinMode;
		int32			fCapMode;
		int32			fStrokeDashes;
};

// #pragma mark -

// constructor
AIExporter::AIExporter()
	: Exporter(),
	  fGraphicsState(new GraphicsState())
{
}

// destructor
AIExporter::~AIExporter()
{
	delete fGraphicsState;
}

// Export
status_t
AIExporter::Export(const Canvas* canvas, const CanvasView* canvasView, BPositionIO* stream,
				   const entry_ref* refToFinalFile)
{
	status_t ret = B_NO_INIT;
	if (canvas) {
		ret = B_BAD_VALUE;
		if (stream) {

			// header
			BString name(canvas->Name());
			_QuotePSString(name);
			ret = _WriteHeader(name, canvas->Bounds(), stream);

			fGraphicsState->SetTo(stream);

			// layers
			if (ret >= B_OK) {
				int32 count = canvas->CountLayers();
				for (int32 i = count - 1; const Layer* layer = canvas->LayerAt(i); i--) {
					ret = _ExportLayer(layer, stream, canvas->Bounds().Height());
					if (ret < B_OK)
						break;
				}
			}

			fGraphicsState->SetTo(NULL);

			// footer
			if (ret >= B_OK) {
				ret = _WriteFooter(stream);
			}
		}
	}
	return ret;
}

// MIMEType
const char*
AIExporter::MIMEType() const
{
	// TODO: find out proper MIME type
	return "text/x-adobe-illustrator";
}

// Extension
const char*
AIExporter::Extension() const
{
	return "ai";
}

// CustomizeExportPanel
void
AIExporter::CustomizeExportPanel(ExportPanel* panel)
{
	panel->SetExportMode(EXPORT_ILLUSTRATOR);
}

// _ExportLayer
status_t
AIExporter::_ExportLayer(const Layer* layer, BPositionIO* stream, float canvasHeight) const
{
	status_t ret = B_ERROR;
	BString data;

	rgb_color outline = { 0, 0, 0, 255 };

	int32 visible = !(layer->Flags() & FLAG_INVISIBLE);
	int32 locked = 1; // not locked
	int32 printable = 1; // printable
	BString name(layer->Name());
	_QuotePSString(name);

	data << "%AI5_BeginLayer\n"
		<< visible << " 1 " << locked << " " << printable << " 0 0 "
		<< "-1 " << (255 * outline.red) << " " << (255 * outline.green) << " " << (255 * outline.blue) << " Lb\n"
		// the layer name
		<< "(" << name.String() << ") Ln\n"
		// no locked objects, only editable
		<< "0 A\n";

	ret = write_string(stream, data);

	// reset the graphics state to be on the safe side
	fGraphicsState->Reset();

	if (const History* history = layer->GetHistory()) {

		const Stroke* object;
		for (int32 i = 0; (object = history->ModifierAt(i)) && ret >= B_OK; i++) {
			if (const ShapeStroke* shapeObject = dynamic_cast<const ShapeStroke*>(object)) {
				ret = _ExportShape(shapeObject, stream, canvasHeight);
			} else if (const TextStroke* textObject = dynamic_cast<const TextStroke*>(object)) {
				ret = _ExportText(textObject, stream, canvasHeight);
			} else if (const BitmapStroke* bitmapObject = dynamic_cast<const BitmapStroke*>(object)) {
				ret = _ExportBitmap(bitmapObject, stream, canvasHeight);
			}
		}

		if (ret >= B_OK) {
			data = "";
			data << "LB\n"
				 << "%AI5_EndLayer--\n";
		
			ret = write_string(stream, data);
		}
	}
	return ret;
}

// #pragma mark -

const char	PS_STYLE[]	= { 'n', 'S', 'F', 'B', 'N', 's', 'f', 'b' };

// convert_join_mode_ai
int32
convert_join_mode_ai(uint32 mode)
{
	int32 aiMode = 0;
	switch (mode) {
		case JOIN_MODE_MITER:
			aiMode = 0;
			break;
		case JOIN_MODE_ROUND:
			aiMode = 1;
			break;
		case JOIN_MODE_BEVEL:
			aiMode = 2;
			break;
	}
	return aiMode;
}

// convert_cap_mode_ai
int32
convert_cap_mode_ai(uint32 mode)
{
	int32 aiMode = 0;
	switch (mode) {
		case CAP_MODE_BUTT:
			aiMode = 0;
			break;
		case CAP_MODE_SQUARE:
			aiMode = 2;
			break;
		case CAP_MODE_ROUND:
			aiMode = 1;
			break;
	}
	return aiMode;
}

// #pragma mark -

// _ExportShape
status_t
AIExporter::_ExportShape(const ShapeStroke* object,
						 BPositionIO* stream, float canvasHeight) const
{
	BString data;

	// First print properties, as they may change between objects, and AI file is just a set of commands
	// like: "change color of line", "move to...", "draw line". so color change and other properties change must go first

	int8 style = 0;

	status_t ret = B_OK;

	// WonderBrush shape may be outline OR fill, not both (at least for now)
	if (object->IsOutline()) {
		// only outline option so far is width, so set rest to default "solid line"
		style |= 0x01;
		// solid outline color
		ret = fGraphicsState->SetStrokeColor(CMYKColor(object->Color()));
		// dashes
		if (ret >= B_OK)
			ret = fGraphicsState->SetStrokeDashes(0);
		// line width
		if (ret >= B_OK)
			ret = fGraphicsState->SetStrokeWidth(object->OutlineWidth());
		// line join
		if (ret >= B_OK)
			ret = fGraphicsState->SetJoinMode(convert_join_mode_ai(object->JoinMode()));
		// line caps
		if (ret >= B_OK)
			ret = fGraphicsState->SetCapMode(convert_cap_mode_ai(object->CapMode()));
	} else {
		// only other option so far is solid fill
		style |= 0x02;
		// solid fill color
		ret = fGraphicsState->SetFillColor(CMYKColor(object->Color()));
	}

	if (ret >= B_OK)
		ret = write_string(stream, data);

	// write path data
	if (ret >= B_OK) {
		BPoint a, aIn, aOut;
		BPoint b, bIn, bOut;

		for (int32 j = 0; VectorPath* path = object->PathAt(j); j++) {

			// move to first point
			if (path->GetPointAt(0, a)) {
				append_float(data, a.x);
				data << " ";
				append_float(data, canvasHeight - a.y);
				data << " m\n";
			}
	
			for (int32 i = 0; ret >= B_OK && 
							  path->GetPointsAt(i, a, aIn, aOut); i++) {
				// write curve from current (a) to next point (b)
				// if the path is closed, write the curve from the last to the first point
				bool connected;
				if (path->GetPointsAt(i + 1, b, bIn, bOut, &connected)
					|| (path->IsClosed() && path->GetPointsAt(0, b, bIn, bOut, &connected))) {
					append_float(data, aOut.x);
					data << " ";
					append_float(data, canvasHeight - aOut.y);
					data << " ";
					append_float(data, bIn.x);
					data << " ";
					append_float(data, canvasHeight - bIn.y);
					data << " ";
					append_float(data, b.x);
					data << " ";
					append_float(data, canvasHeight - b.y);
					data << " c\n";
				}
			}
	
			if (object->IsClosed())
				style |= 0x04;
			data << PS_STYLE[style] << "\n";
		}

		ret = write_string(stream, data);
	}
	return ret;
}

// _ExportText
status_t
AIExporter::_ExportText(const TextStroke* object,
						 BPositionIO* stream, float canvasHeight) const
{
	BString data;
	status_t ret = B_OK;

	// begin text object
	// see AI7FileFormat.pdf, page 87ff
	data << "0 To\n";
	// affine matrix, "point text"
	_WriteMatrix(*object, canvasHeight, data);
	data << " 0 Tp\n";
	data << "TP\n";
	// filled text
	data << "0 Tr\n";
	// fill overprint ()
	data << "0 O\n";
	// fill setgray
	data << "0 g\n";
	// linecap, linejoin, linewidth, miterlimit, dash
	data << "0 J 0 j 1 w 4 M []0 d\n";
	data << "%AI3_Note:\n";
	// polarized fill style
	data << "0 D\n";
	// fill color
	ret = write_string(stream, data);
	if (ret >= B_OK)
		ret = fGraphicsState->SetFillColor(CMYKColor(object->Color()), true);
	// fill rule (0 = non-zero winding number, 1 = even-odd)
	data << "0 XR\n";
	// font name and size
	float ptSize = object->GetTextRenderer()->PointSize() * 16.0;
	data << "/" << object->GetTextRenderer()->PostScriptName()
//	data << "/_Helvetica"
		 << " " <<  (int32)ptSize << " " << (float)(ptSize * .8) << " " << -(float)(ptSize * .2) << " Tf\n";
	// rise (for superscript, dist of chars above baseline)
	data << "0 Ts\n";
	// text scaling
	data << "100 100 Tz\n";
	// tracking
	data << "0 Tt\n";
	// use pair (automatic) kerning
	data << "1 TA\n";
	// ????
	data << "%_ 0 XL\n";
	// tab definition (numDots, numTabs)
	data << "36 0 Xb\n";
	// end tab definition
	data << "XB\n";
	// additional char spacing (100 = 100% of one <space> char)
	data << "0 0 5 TC\n";
	// additional word spacing
	data << "100 100 200 TW\n";
	// indentation of a paragraph (first, other, stop)
	data << "0 0 0 Ti\n";
	// left alignement
	data << "0 Ta\n";
	// ?????
	data << "0 0 2 2 3 Th\n";
	// no "hanging quotations"
	data << "0 Tq\n";
	// leading (line dist within paragraph + line dist between paragraphs)
	data << "0 0 Tl\n";
	// computed char spacing
	data << "0 Tc\n";
	// computed word spacing
	data << "0 Tw\n";

	// print text
	BString text(object->GetTextRenderer()->Text());
	_QuotePSString(text);
	const char* p = text.String();
	bool finished = true;
	while (*p) {
		if (*p == '\n') {
			if (finished) {
				data << "(";
			} else {
				// Tx finishes text body, Tk means kerned text (1 -> auto kerning, 0 is ignored then)
				data << ") Tx 1 0 Tk\n(";
			}
			data << "\\r) TX 1 0 Tk\nT*\n";
			finished = true;
		} else {
			if (finished) {
				data << "(";
				finished = false;
			}
			data << *p;
		}
		p++;
	}
	if (!finished)
		data << ") Tx 1 0 Tk\n(\\r) TX\n";
	// end text object
	data << "TO\n";

	if (ret >= B_OK)
		ret = write_string(stream, data);

	return ret;
}

// _ExportBitmap
status_t
AIExporter::_ExportBitmap(const BitmapStroke* object,
						  BPositionIO* stream, float canvasHeight) const
{
	BString data;

	// fetch bitmap data
	const BBitmap* bitmap = object->Bitmap();
	uint8* bits = (uint8*)bitmap->Bits();
	BRect bounds = bitmap->Bounds();
	uint32 width = bounds.IntegerWidth() + 1;
	uint32 height = bounds.IntegerHeight() + 1;
	uint32 bpr = bitmap->BytesPerRow();

	// compensate for offset in bitmap bounds
	Transformable transform(*object);
	transform.TranslateBy(bounds.LeftTop());
	bounds.OffsetTo(BPoint(0.0, 0.0));

	// fill mode
	data << "1 O\n";
	// fill setgray
	data << "0 g\n";
	// flatness
	data << "800 Ar\n";
	// linecap, linejoin, linewidth, miterlimit, dash
	data << "0 J 0 j 1 w 4 M []0 d\n";
	data << "%AI3_Note:\n";
	// polarized fill style
	data << "0 D\n";
	// fill rule (0 = non-zero winding number, 1 = even-odd)
	data << "0 XR\n";

	data << "%AI5_File:\n";
	data << "%AI5_BeginRaster\n";
	data << "() 1 XG\n";
	// bitmap transform matrix
	data << "[ ";
	_WriteMatrix(transform, canvasHeight, data);
	data << " ] " << width << " " << height << " " << 0 << " Xh\n";

	data << "[ ";
	_WriteMatrix(transform, canvasHeight, data);
	data << " ] ";
	// lower left and upper right bounds
	BPoint ll = bounds.LeftTop();
	BPoint ur = bounds.RightBottom();
	data << (int32)ll.x << " " << (int32)ll.y << " "
		 << (int32)ur.x + 1 << " " << (int32)ur.y + 1 << " ";
	// height and width
	data << width << " "
		 << height << " ";
	// bits per pixel, image type (1 = bitmap/grayscale, 3 = RGB, 4 = CMYK),
	// alpha channel count, reserved, encoding (0 = ASCII, 1 = binary),
	// image mask (0 = opaque, 1 = transparent/colorized)
//	data << "8 3 1 0 0 0\n";
	data << "8 3 0 0 0 0\n";
	data << "%%BeginData: 204043\n";
	data <<"XI\n";

	status_t ret = write_string(stream, data);;

	// write the image data
	if (ret >= B_OK) {
		uint32 chars = 0;
		for (uint32 y = 0; y < height; y++) {
			uint8* handle = bits;
			char out[16];
			for (uint32 x = 0; x < width; x++) {
				if (chars == 0)
					data << "%";

//				sprintf(out, "%.2x%.2x%.2x%.2x", handle[2], handle[1], handle[0], handle[3]);
				sprintf(out, "%.2x%.2x%.2x", handle[2], handle[1], handle[0]);
				data << out;
				handle += 4;

//				chars += 4;
				chars += 3;
				if (chars > 32) {
					chars = 0;
					data << "\n";
				}
			}
			bits += bpr;

			ret = write_string(stream, data);
			if (ret < B_OK)
				break;
		}
		if (chars <= 32)
			data << "\n";
	}
	if (ret >= B_OK) {
		data << "%%EndData\n";
		data << "XH\n";
		data << "%AI5_EndRaster\n";
		data << "F\n";
		ret = write_string(stream, data);
	}

	return ret;
}

// _WriteHeader
status_t
AIExporter::_WriteHeader(BString& name, BRect bounds, BPositionIO* stream) const
{
	BString data;
	data << "%!PS-Adobe-3.0 EPSF-3.0\n"
		 << "%%Creator: YellowBites WonderBrush Shape Export\n"
		 << "%%Title: (" << name << ")\n"
		 << "%%DocumentProcSets: Adobe_Illustrator_1.1 0 0\n"
		 << "%%BoundingBox: "		<< (int32)floorf(bounds.left) << " "
		 							<< (int32)floorf(bounds.top) << " "
		 							<< (int32)ceilf(bounds.right) << " "
		 							<< (int32)ceilf(bounds.bottom) << "\n"
		 << "%%HiResBoundingBox: "	<< bounds.left << " "
		 							<< bounds.top << " "
		 							<< bounds.right << " "
		 							<< bounds.bottom << "\n"
		 << "%AI3_Cropmarks: "		<< bounds.left << " "
		 							<< bounds.top << " "
		 							<< bounds.right << " "
		 							<< bounds.bottom << "\n"
		 << "%AI5_FileFormat 3\n"
		 << "%%DocumentPreview: None\n"
		 << "%%EndComments\n"
		 << "%%BeginProlog\n%%EndProlog\n"
		 << "%%BeginSetup\n"
		 << "%AI3_BeginEncoding: _Courier10PitchBT-Roman Courier10PitchBT-Roman\n"
		 << "[/_Courier10PitchBT-Roman/Courier10PitchBT-Roman 0 0 0 TZ\n"
		 << "%AI3_EndEncoding TrueType\n"
		 << "%AI3_BeginEncoding: _ClassicalGaramondBT-Italic ClassicalGaramondBT-Italic\n"
		 << "[/_ClassicalGaramondBT-Italic/ClassicalGaramondBT-Italic 0 0 0 TZ\n"
		 << "%AI3_EndEncoding TrueType\n"
		 << "Adobe_Illustrator_1.1 begin\n"
		 << "n\n"
		 << "%%EndSetup\n"
		 // path resolution (in DPI), rendering flatness value
		 << "800 Ar\n"
		 // fill rule (0 = non-zero winding number, 1 = even-odd)
		 << "1 XR\n";

	return write_string(stream, data);
}

// _WriteFooter
status_t
AIExporter::_WriteFooter(BPositionIO* stream) const
{
	BString data;
	data << "%%Trailer\n"
		 << "%%EOF\n";

	return write_string(stream, data);
}

// _QuotePSString
void
AIExporter::_QuotePSString(BString& string) const
{
//	string.CharacterEscape("()\\", '\\');

	// TODO: escape also all chars above 127!!!
	// will the following do it?!?
	BString dst;

	const char* src = string.String();
	if (src) {
		int32 srcLength = strlen(src);
		if (srcLength > 0) {

			int32 dstLength = srcLength * 4;

			char* buffer = new char[dstLength];

			int32 state = 0;
			status_t ret;
			if ((ret = convert_from_utf8(B_UNICODE_CONVERSION, 
										 src, &srcLength,
										 buffer, &dstLength,
										 &state, B_SUBSTITUTE)) >= B_OK
				&& (ret = swap_data(B_INT16_TYPE, buffer, dstLength,

									B_SWAP_BENDIAN_TO_HOST)) >= B_OK) {
				uint16* p = (uint16*)buffer;
				for (int32 i = 0; i < dstLength / 2; i++) {
					if (*p == '\\' || *p == '(' || *p == ')') {
						dst << "\\" << (char)*p;
					} else if (*p > 127) {
						dst << "\\" << (int32)*p;
					} else {
						dst << (char)*p;
					}
					p++;
				}
			}
		}
	}
	string = dst;
}

// _WriteMatrix
void
AIExporter::_WriteMatrix(Transformable transform, float canvasHeight, BString& data) const
{
	// inverse the rotation (native = counter clockwise vs. clockwise in AI)
	double tx = 0.0;
	double ty = 0.0;
	transform.Transform(&tx, &ty);
	Transformable rotation;
	rotation.RotateBy(BPoint(tx, ty), -2.0
		* ((transform.rotation() * 180.0) / M_PI));
	transform.Multiply(rotation);

	append_float(data, transform.sx);
	data << " ";
	append_float(data, transform.shy);
	data << " ";
	append_float(data, transform.shx);
	data << " ";
	append_float(data, transform.sy);
	data << " ";
	append_float(data, transform.tx);
	data << " ";
	append_float(data, canvasHeight - transform.ty);
}
