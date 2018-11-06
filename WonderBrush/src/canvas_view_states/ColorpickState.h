// ColorpickState.h

#ifndef COLOR_PICK_STATE_H
#define COLOR_PICK_STATE_H

#include "defines.h"

#include "CanvasViewState.h"

class ColorpickState : public CanvasViewState {
 public:
								ColorpickState(CanvasView* parent);
	virtual						~ColorpickState();

	virtual	void				MouseDown(BPoint where, Point canvasWhere,
										  bool eraser = false);
	virtual	void				MouseMoved(BPoint where, Point canvasWhere,
										   uint32 transit,
										   const BMessage* dragMessage);

	virtual	bool				MessageReceived(BMessage* message);

	virtual	void				UpdateToolCursor();

								//  ColorpickState
			void				SetTipSize(uint32 size);
			uint32				TipSize() const
									{ return fTipSize; }

			void				SetUsesAllLayers(bool all);
			bool				UsesAllLayers() const
									{ return fUseAllLayers; }

 private:
			uint32				fTipSize;
			bool				fUseAllLayers;
};

#endif	// COLOR_PICK_STATE_H
