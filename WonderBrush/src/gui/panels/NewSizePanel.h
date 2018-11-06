// NewSizePanel.h

#ifndef NEW_SIZE_PANEL_H
#define NEW_SIZE_PANEL_H

#include "Panel.h"

class DimensionsControl;
class MTextControl;

class NewSizePanel : public Panel {
 public:
								NewSizePanel(BRect frame,
											 BRect currentSize,
											 BMessage* message = NULL,
											 BWindow* target = NULL);
	virtual						~NewSizePanel();

								// Panel
	virtual	void				Cancel();

	virtual	bool				QuitRequested();
	virtual	void				MessageReceived(BMessage* message);

 private:
			void				_Finish(uint32 what);

			void				_UpdateWidthFromScale() const;
			void				_UpdateHeightFromScale() const;
			void				_SyncScale() const;

	DimensionsControl*			fDimensionsControl;
	MTextControl*				fWidthTC;
	MTextControl*				fHeightTC;

	BRect						fInitialSize;

	BMessage*					fMessage;
	BWindow*					fTarget;
};

#endif // NEW_SIZE_PANEL_H
