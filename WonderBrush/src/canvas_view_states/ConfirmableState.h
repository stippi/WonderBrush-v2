// ConfirmableState.h

#ifndef CONFIRMABLE_STATE_H
#define CONFIRMABLE_STATE_H

#include "defines.h"

#include "CanvasViewState.h"

class ConfirmableState : public CanvasViewState {
 public:
								ConfirmableState(CanvasView* parent);
	virtual						~ConfirmableState();

								// CanvasViewState
	virtual	void				Init(Canvas* canvas,
									 Layer* layer,
									 BPoint currentMousePos);
	virtual	void				CleanUp();

	virtual	bool				MessageReceived(BMessage* message);
	
	virtual	bool				HandleKeyDown(uint32 key, uint32 modifiers);

								// ConfirmableState
	virtual	bool				Perform(Action* action) const;

	virtual	void				Confirm() {}
	virtual	void				Cancel() {}

 protected:
	HistoryManager*				fTemporaryHistory;
	Canvas*						fCanvas;
};

#endif	// CONFIRMABLE_STATE_H
