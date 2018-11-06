// CanvasViewState.h

#ifndef CANVAS_VIEW_STATE_H
#define CANVAS_VIEW_STATE_H

#include <Rect.h>

#include "defines.h"

class BBitmap;
class BMessage;
class BRegion;
class BView;
class Canvas;
class CanvasView;
class Layer;
class Stroke;

class CanvasViewState {
 public:
								CanvasViewState(CanvasView* parent);
	virtual						~CanvasViewState();

	virtual	void				Init(Canvas* canvas,
									 Layer* layer,
									 BPoint currentMousePos);
	virtual	void				CleanUp();
	virtual	void				SetTool(uint32 tool);

	virtual	bool				EditModifier(Stroke* modifier);

	virtual	void				MouseDown(BPoint where, Point canvasWhere,
										  bool erasor = false);
	virtual	void				MouseUp(BPoint where, Point canvasWhere);
	virtual	void				MouseMoved(BPoint where, Point canvasWhere,
										   uint32 transit,
										   const BMessage* dragMessage);

	virtual	void				ModifierSelectionChanged(bool itemsSelected);

	virtual	void				FinishLayer(BRect dirtyRect);
	virtual	void				Draw(BView* into, BRect updateRect);
	virtual	void				ModifyClippingRegion(BView* view,
													 BRegion* region);

	virtual	bool				MessageReceived(BMessage* message);
	
	virtual	void				ModifiersChanged();
	virtual	bool				HandleKeyDown(uint32 key, uint32 modifiers);
	virtual	bool				HandleKeyUp(uint32 key, uint32 modifiers);

	virtual	void				UpdateToolCursor();
	virtual	void				DrawCursor(BPoint where, bool invisible = false);
	virtual	void				ClearCursor(BRect* updateRect = NULL);

			void				SetPrecise(bool precise);
			bool				Precise() const
									{ return fPrecise; }

			bool				IsTracking() const
									{ return fMouseDown; }

	virtual	bool				RebuildLayer(Layer* layer, BRect area) const
									{ return false; }

	virtual	void				PrepareForObjectPropertyChange() {}
	virtual	void				ObjectPropertyChangeDone() {}

 protected:
			void				DrawCross(BView* into, BPoint where,
										  float innerRadius, float outerRadius);

			void				DrawCircle(BView* into, BPoint where,
										   float maxPressureRadius,
										   float minPressureRadius,
										   float tiltX = 0.0, 
										   float tiltY = 0.0);

			void				_ApplyStroke(BBitmap* dest, BRect area,
											 Stroke* stroke) const;
			void				_ApplyStroke(BBitmap* from, BBitmap* to,
											 BRect area, Stroke* stroke) const;

			void				_InvalidateCanvasRect(BRect r, bool deep = false) const;


			CanvasView*			fCanvasView;
	static	bool				fMouseDown;
			bool				fPrecise;
};

#endif	// CANVAS_VIEW_STATE_H
