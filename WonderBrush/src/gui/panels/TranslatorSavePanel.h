// TranslatorSavePanel.h

#ifndef TRANSLATOR_SAVE_PANEL_H
#define TRANSLATOR_SAVE_PANEL_H

#include <FilePanel.h>
#include <MenuItem.h>
#include <TranslationDefs.h>
#include <String.h>

class BMenuField;
class BPopUpMenu;
class BWindow;

class TranslatorItem : public BMenuItem {
 public:
								TranslatorItem(const char* name,
											   const char* shortName,
											   BMessage* message,
											   translator_id id,
											   uint32 format);
		
		translator_id			id;
		uint32					format;

		const char*				ShortName() const;

 private:
		BString					fShortName;
};

class TranslatorSavePanel : public BFilePanel,
							public BHandler {
 public:
								TranslatorSavePanel(const char* name,
													BMessenger* target = NULL,
													entry_ref* startDirectory = NULL,
													uint32 nodeFlavors = 0,
													bool allowMultipleSelection = true,
													BMessage* message = NULL,
													BRefFilter *filter = NULL,
													bool modal = false,
													bool hideWhenDone = true);
		virtual					~TranslatorSavePanel();

		// BFilePanel
		virtual	void			SendMessage(const BMessenger* messenger,
											BMessage* message);
		// BHandler
		virtual	void			MessageReceived(BMessage* message);

		// TranslatorSavePanel
				void			SetTranslatorMode(bool translator);

								// setting and retrieving settings
				translator_id	TranslatorID() const;
				uint32			TranslatorFormat() const;
				void			SetTranslator(translator_id id,
											  uint32 format);

				void			UpdateStrings();

 private:
				TranslatorItem*	_GetCurrentMenuItem() const;
				void			_BuildMenu();
				void			_TranslatorSettings();
				void			_UpdateMenuName();

	BWindow*					fConfigWindow;
	BMenu*						fFormatM;
	BMenuItem*					fSettingsMI;
};

#endif // TRANSLATOR_SAVE_PANEL_H
