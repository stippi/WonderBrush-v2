// TranslatorExporter.h

#ifndef TRANSLATOR_EXPORTER_H
#define TRANSLATOR_EXPORTER_H

#include "Exporter.h"

class TranslatorExporter : public Exporter {
 public:
								TranslatorExporter();
	virtual						~TranslatorExporter();

	virtual	status_t			Export(const Canvas* canvas,
									   const CanvasView* canvasView,
									   BPositionIO* stream,
									   const entry_ref* refToFinalFile);

	virtual	const char*			MIMEType() const;
	virtual	const char*			Extension() const;

	virtual	void				CustomizeExportPanel(ExportPanel* panel);

	virtual	bool				NeedsKeyfile() const
									{ return false; }

								// TranslatorExporter
			void				SetTranslatorInfo(translator_id id,
												  uint32 format);
			void				GetTranslatorInfo(translator_id& id,
												  uint32& format) const;

 private:
	translator_id				fTranslatorID;
	uint32						fTranslatorFormat;
};


#endif // TRANSLATOR_EXPORTER_H
