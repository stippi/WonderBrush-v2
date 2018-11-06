// PathManipulator.h

#ifndef SHAPE_STATE_H
#define SHAPE_STATE_H

#include <String.h>

#include "defines.h"

#include "CanvasView.h"

class AddPointAction;
class ChangePointAction;
class EditModifierAction;
class HistoryManager;
class InsertPointAction;
class PointSelection;
class Selection;
class ShapeStroke;
class VectorPath;


class PathRenderer {
 public:
								PathRenderer();
	virtual						~PathRenderer();

	virtual	void				Invalidate(BRect oldPathArea,
										   BRect oldControlPointArea,
										   bool forceRedraw = false);

	virtual	void				TriggerRedraw();
};

class PathSelection {
 public:
								PathSelection();
	virtual						~PathSelection();

	virtual	PathSelection*		Clone() const;
	virtual	bool				SetTo(const PathSelection* other);

	virtual	Action*				Delete();
};

class PathManipulator {
 public:
								PathManipulator(CanvasView* parent,
												PathRenderer* renderer);
	virtual						~PathManipulator();

			void				SetTo(VectorPath* path, bool clickToClose);
			void				Unset();

	virtual	void				MouseDown(BPoint where, Point canvasWhere,
										  bool eraser = false);
	virtual	void				MouseUp(BPoint where, Point canvasWhere);
	virtual	void				MouseMoved(BPoint where, Point canvasWhere,
										   uint32 transit,
										   const BMessage* dragMessage);

	virtual	void				Draw(BView* into, BRect updateRect);
	virtual	bool				MessageReceived(BMessage* message);

	virtual	void				ModifiersChanged(uint32 modifiers);
	virtual	bool				HandleKeyDown(uint32 key, uint32 modifiers);
	virtual	bool				HandleKeyUp(uint32 key, uint32 modifiers);

	virtual	void				UpdateToolCursor();

			uint32				ControlFlags() const;

			PathSelection*		Selection() const;

			// path manipulation
			void				ReversePath();

 private:
			friend	class		PathAction;
			friend	class		PointSelection;
			friend	class		EnterTransformPointsAction;
			friend	class		ExitTransformPointsAction;
			friend	class		TransformPointsAction;
//			friend	class		NewPathAction;
			friend	class		NudgePointsAction;
//			friend	class		RemovePathAction;
			friend	class		ReversePathAction;
//			friend	class		SelectPathAction;

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

			void				_Select(BRect canvasRect);
			void				_Select(int32 index, bool extend = false);
			void				_Select(const int32* indices, int32 count, bool extend = false);
			void				_Deselect(int32 index);
			void				_ShiftSelection(int32 startIndex, int32 direction);
			bool				_IsSelected(int32 index) const;
			// END functions that need to be undoable

			BRect				_ControlPointRect() const;
			BRect				_ControlPointRect(int32 index, uint32 mode) const;
			void				_GetChangableAreas(BRect* pathArea,
												   BRect* controlPointArea) const;

			void				_SetModeForMousePos(BPoint canvasWhere);

			void				_InvalidateHighlightPoints(int32 newIndex, uint32 newMode);

			void				_UpdateSelection() const;

			void				_Nudge(BPoint direction);
			void				_FinishNudging();

			void				_Delete();

			CanvasView*			fCanvasView;
			PathRenderer*		fRenderer;

			bool				fCommandDown;
			bool				fOptionDown;
			bool				fShiftDown;
			bool				fAltDown;

			bool				fClickToClose;

			uint32				fMode;
			uint32				fFallBackMode;

			bool				fMouseDown;
			BPoint				fTrackingStart;
			BPoint				fLastCanvasPos;

			VectorPath*			fPath;
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
