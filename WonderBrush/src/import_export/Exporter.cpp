// Exporter.cpp

#include "AIExporter.h"
#include "CursorExporter.h"
#include "RDefExporter.h"
#include "SourceExporter.h"
#include "SVGExporter.h"
#include "TranslatorExporter.h"

#include "Exporter.h"

// constructor
Exporter::Exporter()
{
}

// destructor
Exporter::~Exporter()
{
}

// ExporterFor
Exporter*
Exporter::ExporterFor(uint32 mode, translator_id id, uint32 format)
{
	Exporter* exporter = NULL;
	switch (mode) {
		case EXPORT_TRANSLATOR: {
			TranslatorExporter* e = new TranslatorExporter();
			e->SetTranslatorInfo(id, format);
			exporter = e;
			break;
		}
		case EXPORT_CURSOR_SOURCE:
			exporter = new CursorExporter();
			break;
		case EXPORT_BITMAP_SOURCE:
			exporter = new SourceExporter();
			break;
		case EXPORT_SVG:
			exporter = new SVGExporter();
			break;
		case EXPORT_ILLUSTRATOR:
			exporter = new AIExporter();
			break;
		case EXPORT_RDEF_SOURCE:
			exporter = new RDefExporter();
			break;
	
	}
	return exporter;
}
