// BrushPanel.h

#ifndef BRUSH_PANEL_H
#define BRUSH_PANEL_H

#include "Panel.h"

enum {
	MSG_BRUSH_PANEL_QUIT	= 'bpqt',

	MSG_SET_BRUSH_SET		= 'stbs',
	MSG_SAVE_BRUSH_SET		= 'svbs',
	MSG_DELETE_BRUSH_SET	= 'dlbs',
};

class BMenu;
class BMenuItem;
class BrushListView;
class MButton;
class MenuBar;

class BrushPanel : public Panel {
 public:
								BrushPanel(BRect frame,
										   BWindow* window,
										   CanvasView* view);
	virtual						~BrushPanel();

	virtual bool				QuitRequested();
	virtual void				MessageReceived(BMessage* message);

								// BrushPanel
			void				LoadSettings();
			void				SaveSettings();

			void				UpdateStrings();

 private:
			MenuBar*			_CreateMenu();
			void				_BuildBrushSetMenu();
			void				_LoadBrushSet(const BMessage* message);
			void				_SaveBrushSet(BMessage* message,
											  const entry_ref* ref);

			void				_EmptyListView() const;

			BMessage*			fSettings;

			BWindow*			fWindow;

			BrushListView*		fBrushLV;

			BMenu*				fBrushM;

			BMenu*				fBrushSetM;
			BMenu*				fLoadM;
			BMenu*				fSaveM;
			BMenuItem*			fSaveAsMI;
			BMenu*				fDeleteM;
};

#endif // BRUSH_PANEL_H
