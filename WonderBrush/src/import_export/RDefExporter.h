// RDefExporter.h

#ifndef RDEF_EXPORTER_H
#define RDEF_EXPORTER_H

#include "Exporter.h"

class BBitmap;

class RDefExporter : public Exporter {
 public:
								RDefExporter();
	virtual						~RDefExporter();

	virtual	status_t			Export(const Canvas* canvas,
									   const CanvasView* canvasView,
									   BPositionIO* stream,
									   const entry_ref* refToFinalFile);

	virtual	const char*			MIMEType() const;
	virtual	const char*			Extension() const;

	virtual	void				CustomizeExportPanel(ExportPanel* panel);

	virtual	bool				NeedsKeyfile() const
									{ return false; }

 private:
			status_t			_ExportRGBA32(const Canvas* canvas,
											  const BBitmap* bitmap,
											  BPositionIO* stream);
			status_t			_ExportICON(const Canvas* canvas,
											const BBitmap* bitmap,
											BPositionIO* stream);
			status_t			_ExportMICN(const Canvas* canvas,
											const BBitmap* bitmap,
											BPositionIO* stream);

			bool				fAskedIfIcon;
			bool				fAsIcon;
};


#endif // SOURCE_EXPORTER_H
