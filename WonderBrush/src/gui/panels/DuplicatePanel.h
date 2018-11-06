// DuplicatePanel.h

#ifndef DUPLICATE_PANEL_H
#define DUPLICATE_PANEL_H

#include "Panel.h"

class NumericalTextControl;

class DuplicatePanel : public Panel {
 public:
							DuplicatePanel(BWindow* window,
										   BHandler* target,
										   BMessage* message,
										   BRect frame = BRect(-1000.0, -1000.0, -900.0, -900.0));
	virtual					~DuplicatePanel();

	virtual void			MessageReceived(BMessage *message);

 private:
			BRect			_CalculateFrame(BRect frame);

			void			_RestoreSettings(BRect panelFrame);
			void			_StoreSettings();

	NumericalTextControl*	fCopyCountTC;

	NumericalTextControl*	fXOffsetTC;
	NumericalTextControl*	fYOffsetTC;

	NumericalTextControl*	fRotationTC;

	NumericalTextControl*	fXScaleTC;
	NumericalTextControl*	fYScaleTC;

	BWindow*				fWindow;
	BHandler*				fTarget;
	BMessage*				fMessage;

	BMessage				fSettings;
};

#endif // DUPLICATE_PANEL_H
