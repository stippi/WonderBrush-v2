// GuidesState.h

#ifndef GUIDES_STATE_H
#define GUIDES_STATE_H

#include <String.h>

#include "defines.h"

#include "CanvasView.h"
#include "CanvasViewState.h"
#include "ObjectObserver.h"

class Guide;

class GuidesState : public CanvasViewState,
					public ObjectObserver {
 public:
								GuidesState(CanvasView* parent);
	virtual						~GuidesState();

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

//	virtual	void				ModifiersChanged();
	virtual	bool				HandleKeyDown(uint32 key, uint32 modifiers);
//	virtual	bool				HandleKeyUp(uint32 key, uint32 modifiers);

	virtual	void				UpdateToolCursor();

								// ObjectObserver
	virtual	void				ObjectChanged(const Observable* object);

								// GuidesState
			void				SetHPos(float hPos, bool action = false,
										bool notify = true);

			void				SetVPos(float vPos, bool action = false,
										bool notify = true);

			void				SetAngle(double angle, bool action = false,
										 bool notify = true);

 private:
			void				_SetGuide(Guide* modifier);

			void				_Perform();
			void				_Cancel();
			void				_SetConfirmationEnabled(bool enable) const;

			void				_Nudge(BPoint point);
			void				_NudgeRotation(float diff);

			void				_SetUnits(uint32 units);
			float				_ConvertToUnits(float value,
												float start,
												float size) const;
			float				_ConvertFromUnits(float value,
												  float start,
												  float size) const;
			void				_UpdateHPosControl() const;
			void				_UpdateVPosControl() const;

			void				_DeleteGuide();

			HistoryManager*		fPrivateHistory;
			Canvas*				fCanvas;

			Guide*				fGuide;
			Guide*				fGuideUnderMouse;
			bool				fCanceling;

			float				fHPos;
			float				fVPos;
			double				fAngle;
			uint32				fUnitsMode;

			bool				fShowGuides;

			BPoint				fTrackingStart;
			BPoint				fLastCanvasPos;
			BPoint				fStartOffset;

			bigtime_t			fLastNudgeTime;
			bigtime_t			fLastNudgeAngleTime;
};

#endif	// GUIDES_STATE_H
