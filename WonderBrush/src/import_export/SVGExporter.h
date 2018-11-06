// SVGExporter.h

#ifndef SVG_EXPORTER_H
#define SVG_EXPORTER_H

#include <String.h>

#include "Exporter.h"

class BitmapStroke;
class FillStroke;
class Gradient;
class PenStroke;
class ShapeStroke;
class Stroke;
class TextStroke;
class Transformable;

class SVGExporter : public Exporter {
 public:
								SVGExporter();
	virtual						~SVGExporter();

	virtual	status_t			Export(const Canvas* canvas,
									   const CanvasView* canvasView,
									   BPositionIO* stream,
									   const entry_ref* refToFinalFile);

	virtual	const char*			MIMEType() const;
	virtual	const char*			Extension() const;

	virtual	void				CustomizeExportPanel(ExportPanel* panel);

								// SVGExporter
			void				SetOriginalEntry(const entry_ref* ref);

 private:
			bool				_DisplayWarning() const;

			status_t			_ExportShape(const ShapeStroke* object,
											 bool relativeCoords,
											 BPositionIO* stream);
			status_t			_ExportBitmap(const BitmapStroke* object,
											  BPositionIO* stream);
			status_t			_ExportText(const TextStroke* object,
											BPositionIO* stream);
			status_t			_ExportPen(const PenStroke* object,
										   BPositionIO* stream);
			status_t			_ExportFill(const FillStroke* object,
										    BPositionIO* stream);
			status_t			_ExportGradient(const Gradient* gradient,
												BPositionIO* stream);
			void				_AppendMatrix(const Transformable* object,
											  BString& string) const;

			status_t			_GetFill(const Stroke* object,
										 char* string,
										 BPositionIO* stream);

 			int32				fMaskCount;
 			int32				fGradientCount;
			entry_ref*			fOriginalEntry;
};


#endif // SVG_EXPORTER_H
