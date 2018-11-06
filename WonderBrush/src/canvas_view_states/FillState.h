// FillState.h

#ifndef FILL_STATE_H
#define FILL_STATE_H

#include "defines.h"

#include "CanvasViewState.h"

class BMessage;
class CanvasView;
class Layer;
class Scrollable;

class FillState : public CanvasViewState {
 public:
								FillState(CanvasView* parent);
	virtual						~FillState();

								// CanvasViewState
	virtual	void				Init(Canvas* canvas,
									 Layer* layer,
									 BPoint currentMousePos);
	virtual	void				CleanUp();

	virtual	void				MouseDown(BPoint where, Point canvasWhere,
										  bool eraser = false);
//	virtual	void				MouseMoved(BPoint where, Point canvasWhere,
//										   uint32 transit,
//										   const BMessage* dragMessage);

	virtual	bool				MessageReceived(BMessage* message);

	virtual	void				UpdateToolCursor();

			void				SetOpacity(uint8 opacity);
			uint8				Opacity() const
									{ return fOpacity; }
			void				SetTolerance(uint8 tolerance);
			uint8				Tolerance() const
									{ return fTolerance; }
			void				SetSoftness(uint8 softness);
			uint8				Softness() const
									{ return fSoftness; }
			void				SetContiguous(bool contiguous);
			bool				Contiguous() const
									{ return fContiguous; }

 private:
	Layer*						fLayer;
	uint8						fOpacity;
	uint8						fTolerance;
	uint8						fSoftness;
	bool						fContiguous;
};

#endif	// FILL_STATE_H
