// StrokeState.h

#ifndef STROKE_STATE_H
#define STROKE_STATE_H

#include "defines.h"

#include "CanvasView.h"
#include "CanvasViewState.h"

class Stroke;

class StrokeState : public CanvasViewState {
 public:
								StrokeState(CanvasView* parent);
	virtual						~StrokeState();

								// CanvasViewState
	virtual	void				Init(Canvas* canvas,
									 Layer* layer,
									 BPoint currentMousePos);
	virtual	void				CleanUp();
	virtual	void				SetTool(uint32 tool);

	virtual	void				MouseDown(BPoint where, Point canvasWhere,
										  bool eraser = false);
	virtual	void				MouseUp(BPoint where, Point canvasWhere);
	virtual	void				MouseMoved(BPoint where, Point canvasWhere,
										   uint32 transit,
										   const BMessage* dragMessage);

	virtual	void				FinishLayer(BRect dirtyRect);
	virtual	void				Draw(BView* into, BRect updateRect);

//	virtual	bool				MessageReceived(BMessage* message);

	virtual	void				ModifiersChanged();
	virtual	bool				HandleKeyDown(uint32 key, uint32 modifiers);
	virtual	bool				HandleKeyUp(uint32 key, uint32 modifiers);

	virtual	void				UpdateToolCursor();
	virtual	void				DrawCursor(BPoint where, bool invisible = false);
	virtual	void				ClearCursor(BRect* updateRect = NULL);

								// StrokeState
			void				SetMode(uint32 mode);
//	inline	uint32				Mode() const { return fMode; }

 private:
			BRect				_LayerBounds() const;

			Stroke*				_NewStroke(bool eraser) const;

			void				_StrokeLastLine(bool forceStrokeDrawing = false);

			Stroke*				fCurrentStroke;
			Layer*				fLayer;

			bool				fEraser;
			uint32				fMode;

			uint32				fCloneStep;
			BPoint				fCloneOffset;

	struct cursor {
		BPoint		position;
		BPoint		mouse;
		float		radius;
		float		min;
		float		max;
		float		tilt_x;
		float		tilt_y;
		BRect		bounds;
	};

			cursor				fCursor;
			bool				fCursorShowing;
};

#endif	// STROKE_STATE_H
