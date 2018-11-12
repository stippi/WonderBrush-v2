// AutoShapeState.h

#ifndef AUTOSHAPE_STATE_H
#define AUTOSHAPE_STATE_H

#include "CanvasViewState.h"
#include "ObjectObserver.h"
#include "VectorPath.h"

class HistoryManager;
class ShapeStroke;
class TransformBox;
class AutoShapeBox;

class AutoShapeState : public CanvasViewState,
					 public ObjectObserver {
 public:
								AutoShapeState(CanvasView* parent,
											   uint32 opacityWhat,
											   uint32 outlineWhat,
											   uint32 outlineWidthWhat);
	virtual						~AutoShapeState();

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

	virtual	void				ModifierSelectionChanged(bool itemsSelected);

	virtual	void				Draw(BView* into, BRect updateRect);

	virtual	bool				MessageReceived(BMessage* message);

	virtual	void				ModifiersChanged();
	virtual	bool				HandleKeyDown(uint32 key, uint32 modifiers);

	virtual	void				UpdateToolCursor();

	virtual	bool				RebuildLayer(Layer* layer, BRect area) const;

	virtual	void				PrepareForObjectPropertyChange();

								// ObjectObserver
	virtual	void				ObjectChanged(const Observable* object);

								// AutoShapeState
			template<class V, class F>
			void				SetSomething(V value, V control, F func,
											 bool ignorBusy = false);


			void				SetAlpha(uint8 opacity,
										 bool action = false,
										 bool notify = true);
			uint8				Alpha() const
									{ return fAlpha; }

			void				SetColor(rgb_color color,
										 bool action = false,
										 bool notify = true);

			void				SetOutline(bool outline,
										   bool action = false,
										   bool notify = true);
			bool				Outline() const
									{ return fOutline; }

			void				SetOutlineWidth(float width,
												bool action = false,
												bool notify = true);
			float				OutlineWidth() const
									{ return fOutlineWidth; }


 private:

			friend	class		AutoShapeBox;
			friend	class		AutoShapeAction;
			friend	class		AutoShapeColorAction;
			friend	class		AutoShapeTransformAction;

	// round rect state needs this
	virtual	bool				GetPoints(control_point* points,
										  int32 count,
										  double xScale,
										  double yScale) const;

			void				_Perform();
			void				_Cancel();
			void				_SetConfirmationEnabled(bool enable) const;

			void				_SetModifier(ShapeStroke* modifier);
			void				_SetBox(AutoShapeBox* box);

	virtual	void				_UpdateToolCursor() = 0;
	virtual	ShapeStroke*		_MakeAutoShape(BPoint where) const = 0;

			void				_RedrawStroke(ShapeStroke* object,
											  BRect oldStrokeBounds,
											  BRect oldControlPointsBounds,
											  bool rebuild,
											  bool forceStrokeDrawing = false,
											  bool ignorBusy = false) const;

			BRect				_ControlPointRect() const;

			Canvas*				fCanvas;
			Layer*				fLayer;
			HistoryManager*		fPrivateHistory;

	protected:
			ShapeStroke*		fShapeStroke;
			AutoShapeBox*		fBox;
	private:
			BPoint				fLastMousePos;

			uint8				fAlpha;
			bool				fOutline;
			float				fOutlineWidth;
			rgb_color			fColor;

			bool				fCancelling;

			// message what fields
			uint32				fOpacityWhat;
			uint32				fOutlineWhat;
			uint32				fOutlineWidthWhat;

};

#endif	// AUTOSHAPE_STATE_H
