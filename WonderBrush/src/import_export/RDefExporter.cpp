// RDefExporter.cpp

#include "RDefExporter.h"

#include <stdio.h>

#include <Alert.h>
#include <Bitmap.h>
#include <DataIO.h>
#include <Screen.h>

#include "bitmap_support.h"

#include "Canvas.h"
#include "CanvasView.h"
#include "ExportPanel.h"
#include "LanguageManager.h"
#include "Strings.h"

// constructor
RDefExporter::RDefExporter()
	: Exporter(),
	  fAskedIfIcon(false),
	  fAsIcon(false)
{
}

// destructor
RDefExporter::~RDefExporter()
{
}

// Export
status_t
RDefExporter::Export(const Canvas* canvas, const CanvasView* canvasView,
					   BPositionIO* stream, const entry_ref* refToFinalFile)
{
	status_t status = B_NO_INIT;
	if (canvas) {
		status = B_BAD_VALUE;
		if (stream && canvas) {
			if (BBitmap* bitmap = canvas->Bitmap()) {
				canvasView->HandleDemoMode(bitmap, bitmap->Bounds());

				BRect bounds = canvas->Bounds();
				if ((bounds.Width() == 31.0 && bounds.Height() == 31.0) ||
					(bounds.Width() == 15.0 && bounds.Height() == 15.0)) {
					if (!fAskedIfIcon) {
						LanguageManager* manager = LanguageManager::Default();
						BAlert* alert = new BAlert("ask icon",
							manager->GetString(RDEF_ASK_IF_ICON,
								"Save canvas as icon resource?"),
							manager->GetString(RDEF_ICON,
								"Icon"),
							manager->GetString(RDEF_NORMAL,
								"Normal"),
							NULL, B_WIDTH_AS_USUAL, B_IDEA_ALERT);
						if (alert->Go() == 0)
							fAsIcon = true;
					}
				} else
					fAsIcon = false;

				if (fAsIcon) {
					if (bounds.Width() == 31.0 && bounds.Height() == 31.0)
						status = _ExportICON(canvas, bitmap, stream);
					else
						status = _ExportMICN(canvas, bitmap, stream);
				} else {
					status = _ExportRGBA32(canvas, bitmap, stream);
				}

				// done
				delete bitmap;
			}
			if (status > B_OK)
				status = B_OK;
		}
	}
	return status;
}

// MIMEType
const char*
RDefExporter::MIMEType() const
{
	return "text/x-vnd.Be.ResourceDef";
}

// Extension
const char*
RDefExporter::Extension() const
{
	return "rdef";
}

// CustomizeExportPanel
void
RDefExporter::CustomizeExportPanel(ExportPanel* panel)
{
	panel->SetExportMode(EXPORT_BITMAP_SOURCE);
}


// _ExportRGBA32
status_t
RDefExporter::_ExportRGBA32(const Canvas* canvas, const BBitmap* bitmap, BPositionIO* stream)
{
	BRect bounds = bitmap->Bounds();

	char buffer[2048];
	// write header
	sprintf(buffer, "\nresource(<your resource id here>, \"k%s\") archive BBitmap\n"
					"{\n"
					"	\"_frame\" = rect { %.1f, %.1f, %.1f, %.1f },\n"
					"	\"_cspace\" = %d,\n"
					"	\"_bmflags\" = 0,\n"
					"	\"_rowbytes\" = %ld,\n"
					"	\"_data\" = array\n"
					"	{\n",
					canvas->Name(), bounds.left, bounds.top, bounds.right, bounds.bottom,
					bitmap->ColorSpace(), bitmap->BytesPerRow());
	ssize_t size = strlen(buffer);
	status_t status = stream->Write(buffer, size);
	if (status == size) {

		uint8* bits = (uint8*)bitmap->Bits();
		int32 bitsLength = bitmap->BitsLength();

		// print one line (32 values)
		while (bitsLength > 32) {
			sprintf(buffer, "		$\"%.2X%.2X%.2X%.2X"
							"%.2X%.2X%.2X%.2X"
							"%.2X%.2X%.2X%.2X"
							"%.2X%.2X%.2X%.2X"
							"%.2X%.2X%.2X%.2X"
							"%.2X%.2X%.2X%.2X"
							"%.2X%.2X%.2X%.2X"
							"%.2X%.2X%.2X%.2X\"\n",
							bits[0], bits[1], bits[2], bits[3],
							bits[4], bits[5], bits[6], bits[7],
							bits[8], bits[9], bits[10], bits[11],
							bits[12], bits[13], bits[14], bits[15],
							bits[16], bits[17], bits[18], bits[19],
							bits[20], bits[21], bits[22], bits[23],
							bits[24], bits[25], bits[26], bits[27],
							bits[28], bits[29], bits[30], bits[31]);

			size = strlen(buffer);
			status = stream->Write(buffer, size);
			if (status != size) {
				if (status >= B_OK)
					status = B_ERROR;
				break;
			}

			bitsLength -= 32;
			bits += 32;
		}
		if (status >= B_OK && bitsLength > 0) {
			sprintf(buffer, "		$\"");
			size = strlen(buffer);
			status = stream->Write(buffer, size);
			if (status != size) {
				if (status >= B_OK)
					status = B_ERROR;
			}
		}
		// last line (up to 32 values)
		if (status >= B_OK) {
			for (int32 i = 0; i < bitsLength; i++) {
				sprintf(buffer, "%.2X", bits[i]);
				size = strlen(buffer);
				status = stream->Write(buffer, size);
				if (status != size) {
					if (status >= B_OK)
						status = B_ERROR;
					break;
				}
			}
		}
		// finish -> sourceSize - 1
		if (status >= B_OK) {
			sprintf(buffer, "\"\n	}\n};\n");
			size = strlen(buffer);
			status = stream->Write(buffer, size);
			if (status != size) {
				if (status >= B_OK)
					status = B_ERROR;
			}
		}
	}
	return status;
}

// _ExportICON
status_t
RDefExporter::_ExportICON(const Canvas* canvas, const BBitmap* bitmap, BPositionIO* stream)
{
	char buffer[2048];
	// write header
	sprintf(buffer, "\nresource(%s) #'ICON' array {\n", canvas->Name());
	ssize_t size = strlen(buffer);
	status_t status = stream->Write(buffer, size);
	if (status == size) {

		uint8* bits = (uint8*)bitmap->Bits();
		int32 bpr = bitmap->BytesPerRow();

		BScreen screen;

		// print one line (32 values)
		for (int32 y = 0; y < 32; y++) {
			uint8 indexBuffer[32];
			uint8* b = bits;
			for (int32 x = 0; x < 32; x++) {
				if (b[3] < 100)
					indexBuffer[x] = B_TRANSPARENT_MAGIC_CMAP8;
				else
					indexBuffer[x] = screen.IndexForColor(b[2], b[1], b[0]);
				b += 4;
			}
			bits += bpr;

			sprintf(buffer, "	$\"%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X"
							"%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X"
							"%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X"
							"%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X\"\n",
							indexBuffer[ 0],
							indexBuffer[ 1],
							indexBuffer[ 2],
							indexBuffer[ 3],
							indexBuffer[ 4],
							indexBuffer[ 5],
							indexBuffer[ 6],
							indexBuffer[ 7],
							indexBuffer[ 8],
							indexBuffer[ 9],
							indexBuffer[10],
							indexBuffer[11],
							indexBuffer[12],
							indexBuffer[13],
							indexBuffer[14],
							indexBuffer[15],
							indexBuffer[16],
							indexBuffer[17],
							indexBuffer[18],
							indexBuffer[19],
							indexBuffer[20],
							indexBuffer[21],
							indexBuffer[22],
							indexBuffer[23],
							indexBuffer[24],
							indexBuffer[25],
							indexBuffer[26],
							indexBuffer[27],
							indexBuffer[28],
							indexBuffer[29],
							indexBuffer[30],
							indexBuffer[31]);

			size = strlen(buffer);
			status = stream->Write(buffer, size);
			if (status != size) {
				if (status >= B_OK)
					status = B_ERROR;
				break;
			}
		}
		// finish
		sprintf(buffer, "};\n");
		size = strlen(buffer);
		status = stream->Write(buffer, size);
		if (status != size) {
			if (status >= B_OK)
				status = B_ERROR;
		}
	}
	return status;
}

// _ExportMICN
status_t
RDefExporter::_ExportMICN(const Canvas* canvas, const BBitmap* bitmap, BPositionIO* stream)
{
	char buffer[2048];
	// write header
	sprintf(buffer, "\nresource(%s) #'MICN' array {\n", canvas->Name());
	ssize_t size = strlen(buffer);
	status_t status = stream->Write(buffer, size);
	if (status == size) {

		uint8* bits = (uint8*)bitmap->Bits();
		int32 bpr = bitmap->BytesPerRow();

		BScreen screen;

		// print one line (16 values)
		for (int32 y = 0; y < 16; y++) {
			uint8 indexBuffer[16];
			uint8* b = bits;
			for (int32 x = 0; x < 16; x++) {
				if (b[3] < 100)
					indexBuffer[x] = B_TRANSPARENT_MAGIC_CMAP8;
				else
					indexBuffer[x] = screen.IndexForColor(b[2], b[1], b[0]);
				b += 4;
			}
			bits += bpr;

			sprintf(buffer, "	$\"%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X"
							"%.2X%.2X%.2X%.2X%.2X%.2X%.2X%.2X\"\n",
							indexBuffer[ 0],
							indexBuffer[ 1],
							indexBuffer[ 2],
							indexBuffer[ 3],
							indexBuffer[ 4],
							indexBuffer[ 5],
							indexBuffer[ 6],
							indexBuffer[ 7],
							indexBuffer[ 8],
							indexBuffer[ 9],
							indexBuffer[10],
							indexBuffer[11],
							indexBuffer[12],
							indexBuffer[13],
							indexBuffer[14],
							indexBuffer[15]);

			size = strlen(buffer);
			status = stream->Write(buffer, size);
			if (status != size) {
				if (status >= B_OK)
					status = B_ERROR;
				break;
			}
		}
		// finish
		sprintf(buffer, "};\n");
		size = strlen(buffer);
		status = stream->Write(buffer, size);
		if (status != size) {
			if (status >= B_OK)
				status = B_ERROR;
		}
	}
	return status;
}

