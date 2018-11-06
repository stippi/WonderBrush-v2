// NamePanel.h

#ifndef NAME_PANEL_H
#define NAME_PANEL_H

#include "Panel.h"

class MTextControl;

class NamePanel : public Panel {
 public:
							NamePanel(const char* label,
									  const char* text,
									  BWindow* window,
									  BHandler* target,
									  BMessage* message,
									  BRect frame = BRect(-1000.0, -1000.0, -900.0, -900.0));
	virtual					~NamePanel();

	virtual void			MessageReceived(BMessage *message);

 private:
			BRect			_CalculateFrame(BRect frame);

	MTextControl*			fNameTC;
	BWindow*				fWindow;
	BHandler*				fTarget;
	BMessage*				fMessage;

	window_feel				fSavedTargetWindowFeel;
};

#endif // NAME_PANEL_H
