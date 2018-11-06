// CursorExporter.h

#ifndef CURSOR_EXPORTER_H
#define CURSOR_EXPORTER_H

#include "Exporter.h"

class CursorExporter : public Exporter {
 public:
								CursorExporter();
	virtual						~CursorExporter();

	virtual	status_t			Export(const Canvas* canvas,
									   const CanvasView* canvasView,
									   BPositionIO* stream,
									   const entry_ref* refToFinalFile);

	virtual	const char*			MIMEType() const;
	virtual	const char*			Extension() const;

	virtual	void				CustomizeExportPanel(ExportPanel* panel);
};


#endif // CURSOR_EXPORTER_H
