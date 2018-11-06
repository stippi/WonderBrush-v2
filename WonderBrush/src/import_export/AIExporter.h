// AIExporter.h

#ifndef AI_EXPORTER_H
#define AI_EXPORTER_H

#include <Rect.h>
#include <String.h>

#include "Exporter.h"

class BitmapStroke;
class GraphicsState;
class Layer;
class ShapeStroke;
class TextStroke;
class Transformable;

class AIExporter : public Exporter {
 public:
								AIExporter();
	virtual						~AIExporter();

	virtual	status_t			Export(const Canvas* canvas,
									   const CanvasView* canvasView,
									   BPositionIO* stream,
									   const entry_ref* refToFinalFile);

	virtual	const char*			MIMEType() const;
	virtual	const char*			Extension() const;

	virtual	void				CustomizeExportPanel(ExportPanel* panel);

 private:
			status_t			_ExportLayer(const Layer* layer,
											 BPositionIO* stream,
											 float canvasHeight) const;

			status_t			_ExportShape(const ShapeStroke* object,
											 BPositionIO* stream,
											 float canvasHeight) const;
			status_t			_ExportText(const TextStroke* object,
											BPositionIO* stream,
											float canvasHeight) const;
			status_t			_ExportBitmap(const BitmapStroke* object,
											  BPositionIO* stream,
											  float canvasHeight) const;


			status_t			_WriteHeader(BString& name,
											 BRect canvasBounds,
											 BPositionIO* stream) const;
			status_t			_WriteFooter(BPositionIO* stream) const;
			void				_QuotePSString(BString& string) const;

			void				_WriteMatrix(Transformable transform,
											 float canvasHeight,
											 BString& string) const;

			GraphicsState*		fGraphicsState;
};


#endif // AI_EXPORTER_H
