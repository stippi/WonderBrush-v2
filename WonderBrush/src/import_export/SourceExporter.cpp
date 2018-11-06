// SourceExporter.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <DataIO.h>

#include "bitmap_support.h"

#include "Canvas.h"
#include "CanvasView.h"
#include "ExportPanel.h"

#include "SourceExporter.h"

// constructor
SourceExporter::SourceExporter()
	: Exporter()
{
}

// destructor
SourceExporter::~SourceExporter()
{
}

// Export
status_t
SourceExporter::Export(const Canvas* canvas, const CanvasView* canvasView,
					   BPositionIO* stream, const entry_ref* refToFinalFile)
{
	status_t status = B_NO_INIT;
	if (canvas) {
		status = B_BAD_VALUE;
		if (stream && canvas) {
			uint32 width = canvas->Bounds().IntegerWidth() + 1;
			uint32 height = canvas->Bounds().IntegerHeight() + 1;
			if (BBitmap* bitmap = canvas->Bitmap()) {
				canvasView->HandleDemoMode(bitmap, bitmap->Bounds());
				char buffer[1024];
				// write header
				sprintf(buffer, "const uint32 kBitmapWidth = %ld;\n"
								"const uint32 kBitmapHeight = %ld;\n"
								"const color_space kBitmapFormat = %s;\n\n"
								"const unsigned char k%sBits[] = {\n\t",
								width, height, string_for_color_space(bitmap->ColorSpace()), canvas->Name());
				ssize_t size = strlen(buffer);
				if ((status = stream->Write(buffer, size)) == size) {
	
					uint8* bits = (uint8*)bitmap->Bits();
					int32 bitsLength = bitmap->BitsLength();
	
					// print one line (16 values)
					while (bitsLength > 16) {
						sprintf(buffer, "0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, "
										"0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, "
										"0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x, "
										"0x%.2x, 0x%.2x, 0x%.2x, 0x%.2x,\n\t",
										bits[0], bits[1], bits[2], bits[3],
										bits[4], bits[5], bits[6], bits[7],
										bits[8], bits[9], bits[10], bits[11],
										bits[12], bits[13], bits[14], bits[15]);
	
						stream->Write(buffer, strlen(buffer));
	
						bitsLength -= 16;
						bits += 16;
					}
					// last line (up to 16 values)
					for (int32 i = 0; i < bitsLength - 1; i++) {
						sprintf(buffer, "0x%.2x, ", bits[i]);
						stream->Write(buffer, strlen(buffer));
					}
					// finish
					sprintf(buffer, "0x%.2x\n};\n\n", bits[bitsLength - 1]);
					stream->Write(buffer, strlen(buffer));
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
SourceExporter::MIMEType() const
{
	return "text/x-source-code";
}

// Extension
const char*
SourceExporter::Extension() const
{
	return "h";
}

// CustomizeExportPanel
void
SourceExporter::CustomizeExportPanel(ExportPanel* panel)
{
	panel->SetExportMode(EXPORT_BITMAP_SOURCE);
}


