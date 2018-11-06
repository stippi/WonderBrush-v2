// SettingsPanel.h

#ifndef SETTINGS_PANEL_H
#define SETTINGS_PANEL_H

#include "Panel.h"

class BMenu;
class BMenuItem;
class DimensionsControl;
class LabelPopup;
class MButton;
class MCheckBox;
class MTextControl;
class Seperator;

class SettingsPanel : public Panel {
 public:
								SettingsPanel(BRect frame,
											  BMessage* message = NULL,
											  BWindow* target = NULL);
	virtual						~SettingsPanel();

								// Panel
	virtual	void				Cancel();

	virtual	bool				QuitRequested();
	virtual	void				MessageReceived(BMessage* message);

			void				UpdateStrings();
 private:
			void				_SetLanguage(int32 cookie);
			void				_SetLanguage(const char* name);
			void				_Finish(uint32 what);

			void				_SetToSettings();
			void				_ApplySettings();
			void				_BuildLanguageMenu(BMenu* menu);

	Seperator*					fStartUpS;
	Seperator*					fInterfaceS;

	LabelPopup*					fStartupActionPU;
	BMenuItem*					fDoNothingMI;
	BMenuItem*					fAskOpenMI;
	BMenuItem*					fAskNewMI;
	BMenuItem*					fAutoSizeMI;

	DimensionsControl*			fStartupSizeDC;

	MCheckBox*					fLiveUpdatesCB;

	MCheckBox*					fUseSystemLanguageCB;
	LabelPopup*					fLanguagePU;

	MButton*					fOkB;
	MButton*					fCancelB;
	MButton*					fRevertB;

	BMessage*					fMessage;
	BWindow*					fTarget;
};

#endif // SETTINGS_PANEL_H
