// ExportPanel.h

#ifndef EXPORT_SAVE_PANEL_H
#define EXPORT_SAVE_PANEL_H

#include <FilePanel.h>
#include <MenuItem.h>
#include <TranslationDefs.h>
#include <String.h>

class BButton;
class BMenuField;
class BPopUpMenu;
class BWindow;

class ExportItem : public BMenuItem {
 public:
								ExportItem(const char* name,
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

class ExportPanel : public BFilePanel,
					public BHandler {
 public:
								ExportPanel(const char* name,
													BMessenger* target = NULL,
													entry_ref* startDirectory = NULL,
													uint32 nodeFlavors = 0,
													bool allowMultipleSelection = true,
													BMessage* message = NULL,
													BRefFilter *filter = NULL,
													bool modal = false,
													bool hideWhenDone = true);
		virtual					~ExportPanel();

		// BFilePanel
		virtual	void			SendMessage(const BMessenger* messenger,
											BMessage* message);
		// BHandler
		virtual	void			MessageReceived(BMessage* message);

		// ExportPanel
				void			SetMode(bool exportMode);

								// setting and retrieving settings
				void			SetExportMode(int32 mode);
				int32			ExportMode() const;

				void			AdjustExtension();

				translator_id	TranslatorID() const;
				uint32			TranslatorFormat() const;
				void			SetTranslator(translator_id id,
											  uint32 format);

				void			UpdateStrings();

 private:
				BMenuItem*		_GetCurrentMenuItem() const;
				void			_BuildMenu();
				void			_TranslatorSettings();
				void			_EnableSettings() const;

	BWindow*					fConfigWindow;
	BPopUpMenu*					fFormatM;
	BMenuField*					fFormatMF;

	BButton*					fSettingsB;

	BMenuItem*					fCursorMI;
	BMenuItem*					fSourceMI;
	BMenuItem*					fSVGMI;
	BMenuItem*					fIllustratorMI;
	BMenuItem*					fRDefMI;
};

#endif // EXPORT_SAVE_PANEL_H
