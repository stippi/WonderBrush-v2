// TransformState.h

#ifndef TRANSFORM_STATE_H
#define TRANSFORM_STATE_H

#include "defines.h"

#include "CanvasViewState.h"
#include "Transformable.h"

class HistoryManager;
class TransformAction;
class TransformObjectsAction;
class TransformBox;

class TransformState : public CanvasViewState {
 public:
								TransformState(CanvasView* parent);
	virtual						~TransformState();

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
	virtual	bool				HandleKeyUp(uint32 key, uint32 modifiers);

	virtual	void				UpdateToolCursor();

	virtual	void				PrepareForObjectPropertyChange();

//			TransformAction*	ChangeTransformation(BPoint translation,
//													 double rotation,
//													 double xScale,
//													 double yScale,
			TransformAction*	ChangeTransformation(const Transformable& t,
													 BPoint centerOffset,
													 bool action = false,
													 bool notify = true);

			void				UpdateBox();

 private:
			friend class		ObjectSelection;
			friend class		EnterTransformObjectsAction;
			friend class		ExitTransformObjectsAction;
			friend class		TransformObjectsAction;

			void				_Perform();
			void				_Cancel();
			void				_SetConfirmationEnabled(bool enable) const;

			void				_SetTransformBox(ObjectSelection* box);

			void				_UpdateControls() const;

			void				_RebuildLayer(BRect area);

			void				_RefreshCaches() const;

			void				_Nudge(BPoint direction);
			void				_FinishNudging();

			BPoint				fLastCanvasPos;
			Layer*				fLayer;

			HistoryManager*		fPrivateHistory;
			Canvas*				fCanvas;

			bool				fItemsSelected;
			bool				fCanceling;
			ObjectSelection*	fTransformBox;

			bool				fShiftDown;
			bool				fCommandDown;

			TransformObjectsAction*	fTransformAction;

			BPoint				fNudgeOffset;
			bigtime_t			fLastNudgeTime;
			TransformAction*	fNudgeAction;
};

#endif	// TRANSFORM_STATE_H
