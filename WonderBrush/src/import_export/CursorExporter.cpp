// CursorExporter.cpp

#include <stdio.h>

#include <Alert.h>
#include <Bitmap.h>
#include <DataIO.h>

#include "Canvas.h"
#include "ExportPanel.h"
#include "LanguageManager.h"

#include "CursorExporter.h"

// constructor
CursorExporter::CursorExporter()
	: Exporter()
{
}

// destructor
CursorExporter::~CursorExporter()
{
}

// Export
status_t
CursorExporter::Export(const Canvas* canvas, const CanvasView* canvasView,
					   BPositionIO* stream, const entry_ref* refToFinalFile)
{
	status_t status = B_NO_INIT;
	if (canvas) {
		status = B_BAD_VALUE;
		if (stream && canvas) {
			uint32 width = canvas->Bounds().IntegerWidth() + 1;
			uint32 height = canvas->Bounds().IntegerHeight() + 1;
			if (width == 16 && height == width) {
				if (BBitmap* bitmap = canvas->Bitmap()) {
					char buffer[512];
					// write header
					sprintf(buffer, "const unsigned char kCursorData[] = { %ld, 1, 8, 8,\n	", width);
					ssize_t size = strlen(buffer);
					if ((status = stream->Write(buffer, size)) == size) {
	
						uint8* bits = (uint8*)bitmap->Bits();
						uint32 bpr = bitmap->BytesPerRow();
	
						uint8 byte;	// one row of pixel data (16 bits)
	
						// write pixel data (1 = black, 0 = white)
						int32 linesWritten = 1;
						for (uint32 y = 0; y < 16; y++) {
							uint8* bitsHandle = bits;
	
							// first 8 pixels
							byte = 0;
							for (uint32 x = 0; x < 8; x++) {
								if (bitsHandle[0] < 127
									&& bitsHandle[1] < 127
									&& bitsHandle[2] < 127
									&& bitsHandle[3]) {
									byte |= 128 >> x;
								}
								bitsHandle += 4;
							}
							sprintf(buffer, "0x%.2x, ", byte);
							stream->Write(buffer, strlen(buffer));
	
							// last 8 pixels
							byte = 0;
							for (uint32 x = 8; x < 16; x++) {
								if (bitsHandle[0] < 127
									&& bitsHandle[1] < 127
									&& bitsHandle[2] < 127
									&& bitsHandle[3]) {
									byte |= 128 >> (x - 8);
								}
								bitsHandle += 4;
							}
							linesWritten++;
							if (y < 15) {
								if ((linesWritten - 1) % 4 == 0)
									sprintf(buffer, "0x%.2x,\n	", byte);
								else
									sprintf(buffer, "0x%.2x, ", byte);
							} else
								sprintf(buffer, "0x%.2x,\n\n	", byte);
							stream->Write(buffer, strlen(buffer));
	
							bits += bpr;
						}
		
						// write transparency mask (1 = opaque, 0 = transparent)
						bits = (uint8*)bitmap->Bits();
						for (uint32 y = 0; y < 16; y++) {
							uint8* bitsHandle = bits;
	
							// first 8 pixels
							byte = 0;
							for (uint32 x = 0; x < 8; x++) {
								// check alpha or if pixel is black
								if (bitsHandle[3] > 127) {
									byte |= 128 >> x;
								}
								bitsHandle += 4;
							}
							sprintf(buffer, "0x%.2x, ", byte);
							stream->Write(buffer, strlen(buffer));
	
							// last 8 pixels
							byte = 0;
							for (uint32 x = 8; x < 16; x++) {
								if (bitsHandle[3] > 127) {
									byte |= 128 >> (x - 8);
								}
								bitsHandle += 4;
							}
	
							linesWritten++;
							if (y < 15) {
								if ((linesWritten - 1) % 4 == 0)
									sprintf(buffer, "0x%.2x,\n	", byte);
								else
									sprintf(buffer, "0x%.2x, ", byte);
							} else
								sprintf(buffer, "0x%.2x };\n\n", byte);
							stream->Write(buffer, strlen(buffer));
	
							bits += bpr;
						}
					}
					// done
					delete bitmap;
				}
			} else {
				LanguageManager* manager = LanguageManager::Default();
				BAlert* alert = new BAlert("wrong size", manager->GetString(UNSUPPORTED_CURSOR_SIZE,
															 "The Canvas needs to be 16 x 16 "
															 "pixels in size. Other cursor "
															 "sizes are not supported."),
														 manager->GetString(OK, "Ok"),
														 NULL, NULL, B_WIDTH_AS_USUAL,
														 B_WARNING_ALERT);
				alert->Go(NULL);
			}
			if (status > B_OK)
				status = B_OK;
		}
	}
	return status;
}

// MIMEType
const char*
CursorExporter::MIMEType() const
{
	return "text/x-source-code";
}

// Extension
const char*
CursorExporter::Extension() const
{
	return "h";
}

// CustomizeExportPanel
void
CursorExporter::CustomizeExportPanel(ExportPanel* panel)
{
	panel->SetExportMode(EXPORT_CURSOR_SOURCE);
}

