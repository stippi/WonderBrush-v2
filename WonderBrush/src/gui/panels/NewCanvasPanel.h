// NewCanvasPanel.h

#ifndef DIMENSIONS_PANEL_H
#define DIMENSIONS_PANEL_H

#include "Panel.h"

class DimensionsControl;
class MTextControl;

class NewCanvasPanel : public Panel {
 public:
								NewCanvasPanel(BRect frame,
												BMessage* message = NULL,
												BWindow* target = NULL);
	virtual						~NewCanvasPanel();

								// Panel
	virtual	void				Cancel();

	virtual	bool				QuitRequested();
	virtual	void				MessageReceived(BMessage* message);

 private:
			void				_Finish(uint32 what);

	DimensionsControl*			fDimensionsControl;
	MTextControl*				fNameTC;
	BMessage*					fMessage;
	BWindow*					fTarget;
};

#endif // DIMENSIONS_PANEL_H
