// PickObjectsState.h

#ifndef PICK_OBJECTS_STATE_H
#define PICK_OBJECTS_STATE_H

#include "CanvasViewState.h"

class PickObjectsState : public CanvasViewState {
 public:
								PickObjectsState(CanvasView* parent);
	virtual						~PickObjectsState();

								// CanvasViewState
	virtual	void				Init(Canvas* canvas,
									 Layer* layer,
									 BPoint currentMousePos);
	virtual	void				CleanUp();

	virtual	void				MouseDown(BPoint where, Point canvasWhere,
										  bool eraser = false);
	virtual	void				MouseUp(BPoint where, Point canvasWhere);
	virtual	void				MouseMoved(BPoint where, Point canvasWhere,
										   uint32 transit,
										   const BMessage* dragMessage);

	virtual	void				Draw(BView* into, BRect updateRect);

	virtual	bool				MessageReceived(BMessage* message);

	virtual	void				ModifierSelectionChanged(bool itemsSelected);

	virtual	void				ModifiersChanged();

	virtual	void				UpdateToolCursor();

 private:
			History*			fHistory;
			bool				fShiftDown;
			bool				fTracking;
			BPoint				fTrackingStart;
			BRect				fBoundingBox;
};

#endif	// PICK_OBJECTS_STATE_H
