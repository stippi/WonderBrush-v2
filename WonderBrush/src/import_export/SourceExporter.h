// SourceExporter.h

#ifndef SOURCE_EXPORTER_H
#define SOURCE_EXPORTER_H

#include "Exporter.h"

class SourceExporter : public Exporter {
 public:
								SourceExporter();
	virtual						~SourceExporter();

	virtual	status_t			Export(const Canvas* canvas,
									   const CanvasView* canvasView,
									   BPositionIO* stream,
									   const entry_ref* refToFinalFile);

	virtual	const char*			MIMEType() const;
	virtual	const char*			Extension() const;

	virtual	void				CustomizeExportPanel(ExportPanel* panel);

	virtual	bool				NeedsKeyfile() const
									{ return false; }

};


#endif // SOURCE_EXPORTER_H
