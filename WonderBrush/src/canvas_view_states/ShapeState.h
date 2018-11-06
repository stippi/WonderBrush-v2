// ShapeState.h

#ifndef SHAPE_STATE_H
#define SHAPE_STATE_H

#include <String.h>

#include "defines.h"

#include "CanvasView.h"
#include "CanvasViewState.h"
#include "ObjectObserver.h"

class AddPointAction;
class ChangePointAction;
class EditModifierAction;
class HistoryManager;
class InsertPointAction;
class PointSelection;
class Selection;
class ShapeStroke;
class VectorPath;

class ShapeState : public CanvasViewState,
				   public ObjectObserver {
 public:
								ShapeState(CanvasView* parent);
	virtual						~ShapeState();

								// CanvasViewState
	virtual	void				Init(Canvas* canvas,
									 Layer* layer,
									 BPoint currentMousePos);
	virtual	void				CleanUp();

	virtual	bool				EditModifier(Stroke* modifier);

	virtual	void				MouseDown(BPoint where, Point canvasWhere,
										  bool eraser = false);
	virtual	void				MouseUp(BPoint where, Point canvasWhere);
	virtual	void				MouseMoved(BPoint where, Point canvasWhere,
										   uint32 transit,
										   const BMessage* dragMessage);

	virtual	void				ModifierSelectionChanged(bool itemsSelected);

/*	virtual	void				FinishLayer(BRect dirtyRect);*/
	virtual	void				Draw(BView* into, BRect updateRect);
	virtual	bool				MessageReceived(BMessage* message);

	virtual	void				ModifiersChanged();
	virtual	bool				HandleKeyDown(uint32 key, uint32 modifiers);
	virtual	bool				HandleKeyUp(uint32 key, uint32 modifiers);

	virtual	void				UpdateToolCursor();

	virtual	bool				RebuildLayer(Layer* layer, BRect area) const;

	virtual	void				PrepareForObjectPropertyChange();

								// ObjectObserver
	virtual	void				ObjectChanged(const Observable* object);

								// ShapeState
			void				StoreNonGUISettings(BMessage* message) const;
			void				RestoreNonGUISettings(const BMessage* message);

			template<class V, class F>
			void				SetSomething(V value, V control, F func,
											 bool ignorBusy = false);


			void				SetAlpha(uint8 opacity,
										 bool action = false,
										 bool notify = true);
			uint8				Alpha() const
									{ return fAlpha; }

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

			void				SetCapMode(uint32 mode,
										   bool action = false,
										   bool notify = true);
			uint32				CapMode() const
									{ return fCapMode; }

			void				SetJoinMode(uint32 mode,
											bool action = false,
											bool notify = true);
			uint32				JoinMode() const
									{ return fJoinMode; }

			void				SetColor(rgb_color color,
										 bool action = false,
										 bool notify = true);

			void				SetClosed(bool closed,
										  bool action = false,
										  bool notify = true);

//			bool				WidthEnabled() const;
//			bool				TransformEnabled() const;
			uint32				ControlFlags() const;


 private:
			friend	class		PathAction;
			friend	class		PointSelection;
			friend	class		EnterTransformPointsAction;
			friend	class		ExitTransformPointsAction;
			friend	class		TransformPointsAction;
			friend	class		NewPathAction;
			friend	class		NudgePointsAction;
			friend	class		RemovePathAction;
			friend	class		ReversePathAction;
			friend	class		SelectPathAction;

			void				_SetModifier(ShapeStroke* modifier, bool edit = false);
			void				_SetPath(VectorPath* path);

			BRect				_LayerBounds() const;

			void				_RedrawStroke(BRect oldStrokeBounds,
											  BRect oldControlPointsBounds,
											  bool forceStrokeDrawing = false,
											  bool ignorBusy = false) const;

			void				_Perform();
			void				_Cancel();
			void				_SetConfirmationEnabled(bool enable) const;

			void				_SetMode(uint32 mode);

			void				_SetPointSelection(PointSelection* selection);

			// BEGIN functions that need to be undoable
			void				_AddPoint(Point where);
			void				_InsertPoint(Point where, int32 index);
			void				_SetInOutConnected(int32 index, bool connected);
			void				_SetSharp(int32 index);

			void				_RemoveSelection();
			void				_RemovePoint(int32 index);
			void				_RemovePointIn(int32 index);
			void				_RemovePointOut(int32 index);

			void				_ReversePath();

			void				_Select(BRect canvasRect);
			void				_Select(int32 index, bool extend = false);
			void				_Select(const int32* indices, int32 count, bool extend = false);
			void				_Deselect(int32 index);
			void				_ShiftSelection(int32 startIndex, int32 direction);
			bool				_IsSelected(int32 index) const;
			// END functions that need to be undoable

			BRect				_ControlPointRect() const;

			void				_SetModeForMousePos(BPoint canvasWhere);

			BRect				_ControlPointRect(int32 index, uint32 mode) const;
			void				_InvalidateHighlightPoints(int32 newIndex, uint32 newMode);

			void				_UpdateSelection() const;

			void				_Nudge(BPoint direction);
			void				_FinishNudging();

			void				_Delete();

			HistoryManager*		fPrivateHistory;
			Canvas*				fCanvas;

			ShapeStroke*		fShapeStroke;
			EditModifierAction*	fEditAction;
			bool				fCanceling;

			bool				fCommandDown;
			bool				fOptionDown;
			bool				fShiftDown;
			bool				fAltDown;

			Layer*				fLayer;

			uint32				fMode;
			uint32				fFallBackMode;

			uint8				fAlpha;
			bool				fOutline;
			float				fOutlineWidth;
			bool				fClosed;
			uint32				fCapMode;
			uint32				fJoinMode;
			rgb_color			fColor;
			uint32				fFillingRule;
			bool				fSolid;

			BPoint				fTrackingStart;
			BPoint				fLastCanvasPos;

			VectorPath*			fPath;
			VectorPath*			fPathUnderMouse;
			int32				fCurrentPathPoint;
			ChangePointAction*	fChangePointAction;
			InsertPointAction*	fInsertPointAction;
			AddPointAction*		fAddPointAction;

			Selection*			fSelection;
			Selection*			fOldSelection;
			PointSelection*		fTransformBox;

			// stuff needed for nudging
			BPoint				fNudgeOffset;
			bigtime_t			fLastNudgeTime;
			NudgePointsAction*	fNudgeAction;
};

#endif	// SHAPE_STATE_H
