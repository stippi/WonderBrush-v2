// Exporter.h

#ifndef EXPORTER_H
#define EXPORTER_H

#include <Entry.h>
#include <TranslationDefs.h>

class BPositionIO;
class Canvas;
class CanvasView;
class ExportPanel;

enum {
	EXPORT_NATIVE = 0,
	EXPORT_TRANSLATOR,
	EXPORT_CURSOR_SOURCE,
	EXPORT_BITMAP_SOURCE,
	EXPORT_SVG,
	EXPORT_ILLUSTRATOR,
	EXPORT_RDEF_SOURCE,
};

class Exporter {
 public:
								Exporter();
	virtual						~Exporter();

	virtual	status_t			Export(const Canvas* canvas,
									   const CanvasView* canvasView,
									   BPositionIO* stream,
									   const entry_ref* refToFinalFile) = 0;

	virtual	const char*			MIMEType() const = 0;
	virtual	const char*			Extension() const = 0;

	virtual	void				CustomizeExportPanel(ExportPanel* panel) = 0;

	virtual	bool				NeedsKeyfile() const
									{ return true; }

	static	Exporter*			ExporterFor(uint32 mode,
											translator_id id = 0,
											uint32 format = 0);

};


#endif // EXPORTER_H
