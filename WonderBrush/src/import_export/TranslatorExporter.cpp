// TranslatorExporter.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <BitmapStream.h>
#include <DataIO.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>

#include "Canvas.h"
#include "CanvasView.h"
#include "ExportPanel.h"

#include "TranslatorExporter.h"

// constructor
TranslatorExporter::TranslatorExporter()
	: Exporter()
{
}

// destructor
TranslatorExporter::~TranslatorExporter()
{
}

// Export
status_t
TranslatorExporter::Export(const Canvas* canvas,
						   const CanvasView* canvasView,
						   BPositionIO* stream,
						   const entry_ref* refToFinalFile)
{
	status_t status = B_NO_INIT;
	if (canvas) {
		status = B_BAD_VALUE;
		if (stream) {
			BTranslatorRoster* roster = BTranslatorRoster::Default();
			if (BBitmap* bitmap = canvas->Bitmap()) {
				canvasView->HandleDemoMode(bitmap, bitmap->Bounds());
				BBitmapStream outStream(bitmap);
				status = roster->Translate(fTranslatorID, &outStream, NULL, stream, fTranslatorFormat);
				if (status < B_OK)
					fprintf(stderr, "failed to export bitmap: %s\n", strerror(status));
				outStream.DetachBitmap(&bitmap);
				delete bitmap;
			} else
				fprintf(stderr, "no bitmap from canvas!\n");
		}
	}
	return status;
}

// MIMEType
const char*
TranslatorExporter::MIMEType() const
{
	const translation_format* formats;
	int32 count;

	BTranslatorRoster* roster = BTranslatorRoster::Default();
	status_t status = roster->GetOutputFormats(fTranslatorID, &formats, &count);

	if (status >= B_OK) {
		for (int32 ix = 0; ix < count; ix++) {
			if (formats[ix].type == fTranslatorFormat) {
				return formats[ix].MIME;
			}
		}
	}
	return "image/x-unkown";
}

// Extension
const char*
TranslatorExporter::Extension() const
{
	const char* extension = "";
	switch (fTranslatorFormat) {
		case B_GIF_FORMAT:
			extension = "gif";
			break;
		case B_JPEG_FORMAT:
			extension = "jpg";
			break;
		case B_PNG_FORMAT:
			extension = "png";
			break;
		case B_PPM_FORMAT:
			extension = "ppm";
			break;
		case B_TGA_FORMAT:
			extension = "tga";
			break;
		case B_BMP_FORMAT:
			extension = "bmp";
			break;
		case B_TIFF_FORMAT:
			extension = "tif";
			break;
		// some custom translators, which I happen to know
		case 'SGI ':
			extension = "sgi";
			break;
		case 'PSD ':
			extension = "psd";
			break;
		case 'ILBM':
			extension = "iff";
			break;
		case 'JP2 ':
			extension = "jp2";
			break;
	}
	return extension;
}

// CustomizeExportPanel
void
TranslatorExporter::CustomizeExportPanel(ExportPanel* panel)
{
	panel->SetExportMode(EXPORT_TRANSLATOR);
	panel->SetTranslator(fTranslatorID, fTranslatorFormat);
}

// SetTranslatorInfo
void
TranslatorExporter::SetTranslatorInfo(translator_id id, uint32 format)
{
	fTranslatorID = id;
	fTranslatorFormat = format;
}

// GetTranslatorInfo
void
TranslatorExporter::GetTranslatorInfo(translator_id& id, uint32& format) const
{
	id = fTranslatorID;
	format = fTranslatorFormat;
}


