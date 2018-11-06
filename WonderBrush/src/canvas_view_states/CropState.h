// CropState.h

#ifndef CROP_STATE_H
#define CROP_STATE_H

#include <Rect.h>

#include "defines.h"

#include "CanvasViewState.h"

class ChangeCropRectAction;
class HistoryManager;

class CropState : public CanvasViewState {
 public:
								CropState(CanvasView* parent);
	virtual						~CropState();

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
	virtual	void				ModifyClippingRegion(BView* view,
													 BRegion* region);

	virtual	bool				MessageReceived(BMessage* message);

	virtual	void				ModifiersChanged();
	virtual	bool				HandleKeyDown(uint32 key, uint32 modifiers);

	virtual	void				UpdateToolCursor();

 private:

	friend	class				ChangeCropRectAction;

			void				_SetRect(BRect newBounds, bool action = false,
														  bool forceNotify = false);

			void				_Perform(BRect newBounds);
			void				_Cancel();
			void				_SetConfirmationEnabled(bool enable) const;

			void				_DrawDragPoint(BView* into, BRect r) const;
			void				_InvalidateCanvasView(BRect r) const;
			BRect				_ViewRect(BRect canvasRect) const;
			uint32				_DragModeFor(BPoint where);
			void				_SetDragMode(uint32 mode);

	enum {
		DRAGGING_NONE			= 0,
		// corners
		DRAGGING_LEFT_TOP,
		DRAGGING_RIGHT_TOP,
		DRAGGING_LEFT_BOTTOM,
		DRAGGING_RIGHT_BOTTOM,
		// sides
		DRAGGING_LEFT_SIDE,
		DRAGGING_TOP_SIDE,
		DRAGGING_RIGHT_SIDE,
		DRAGGING_BOTTOM_SIDE,
		// or all
		DRAGGING_ALL,
	};

			BPoint				fTrackingStart;
			BPoint				fTrackingOffset;
			BPoint				fLeftTopStart;
			BPoint				fLastCanvasPos;

			BRect				fCropRect;
			BRect				fLastCropRect;
			Canvas*				fCanvas;
			HistoryManager*		fPrivateHistory;

			bool				fShiftDown;
			float				fAspect;
			uint32				fDraggingMode;
};

#endif	// CROP_STATE_H
