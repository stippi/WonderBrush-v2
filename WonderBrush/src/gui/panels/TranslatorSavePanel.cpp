// TranslatorSavePanel.cpp

#include <stdio.h>
#include <string.h>

#include <Alert.h>
#include <MenuBar.h>
#include <TranslationKit.h>
#include <View.h>
#include <Window.h>

#include "LanguageManager.h"
#include "Panel.h"
#include "Strings.h"

#include "TranslatorSavePanel.h"

enum {
	MSG_FORMAT		= 'sfpf',
	MSG_SETTINGS	= 'sfps',
};

// TranslatorItem class
TranslatorItem::TranslatorItem(const char* name,
							   const char* shortName,
							   BMessage* message,
							   translator_id id,
							   uint32 format)
	: BMenuItem(name, message),
	  fShortName(shortName)
{
	this->id = id;
	this->format = format;
}

// ShortName
const char*
TranslatorItem::ShortName() const
{
	return fShortName.String();
}

// TranslatorSavePanel class
TranslatorSavePanel::TranslatorSavePanel(const char* name,
										 BMessenger* target,
										 entry_ref* startDirectory,
										 uint32 nodeFlavors,
										 bool allowMultipleSelection,
										 BMessage* message,
										 BRefFilter* filter,
										 bool modal,
										 bool hideWhenDone)
	: BFilePanel(B_SAVE_PANEL, target, startDirectory,
				 nodeFlavors, allowMultipleSelection,
				 message, filter, modal, hideWhenDone),
	  BHandler(name),
	  fConfigWindow(NULL),
	  fFormatM(NULL)
{
	BWindow* window = Window();
	if (window && window->Lock()) {
		window->SetTitle("Save Image");
		
		// add the new BHandler to the window's looper
		window->AddHandler(this);
		
		if (BMenuBar* menuBar = window->KeyMenuBar()) {
			_BuildMenu();
			menuBar->AddItem(fFormatM);
			_UpdateMenuName();
		}

		window->Unlock();
	}
}

// destructor
TranslatorSavePanel::~TranslatorSavePanel()
{
}

// SendMessage
void
TranslatorSavePanel::SendMessage(const BMessenger* messenger,
								 BMessage* message)
{
	// add the current translator information to the message
	if (message) {
		if (TranslatorItem* item = _GetCurrentMenuItem()) {
			message->AddData("translator_id", B_RAW_TYPE, &(item->id),
							 sizeof(translator_id));
			message->AddInt32("translator_format", item->format);
		}
	}
	// let the original file panel code handle the rest
	BFilePanel::SendMessage(messenger, message);
}

// MessageReceived
void
TranslatorSavePanel::MessageReceived(BMessage* message)
{
	// Handle messages from controls we've added
	switch (message->what) {
		case MSG_FORMAT: {
			// mark the item from which this message came
			BMenuItem* newItem;
			if (message->FindPointer("source", (void**)&newItem) == B_OK) {
				TranslatorItem* lastItem = _GetCurrentMenuItem();
				if (newItem != lastItem) {
					// unmark any other
					if (lastItem)
						lastItem->SetMarked(false);
					if (newItem)
						newItem->SetMarked(true);
				}
				_UpdateMenuName();
			}
			break;
		}
		case MSG_SETTINGS:
			_TranslatorSettings();
			break;
		default:
			BHandler::MessageReceived(message);
			break;
	}
}

// SetTranslatorMode
void
TranslatorSavePanel::SetTranslatorMode(bool translator)
{
	BWindow* window = Window();
	if (window && window->Lock()) {
		if (BMenuBar* menuBar = window->KeyMenuBar()) {
			LanguageManager* manager = LanguageManager::Default();
			if (translator) {
				if (menuBar->IndexOf(fFormatM) < 0) {
					menuBar->AddItem(fFormatM);
					_UpdateMenuName();
				}
				window->SetTitle(manager->GetString(SAVE_IMAGE, "Save Image"));
			} else {
				if (menuBar->IndexOf(fFormatM) >= 0)
					menuBar->RemoveItem(fFormatM);
				window->SetTitle(manager->GetString(SAVE_PROJECT, "Save Project"));
			}
		}
		window->Unlock();
	}
}

// TranslatorID
translator_id
TranslatorSavePanel::TranslatorID() const
{
	translator_id id = 0;
	if (Window()->Lock()) {
		if (TranslatorItem* item = _GetCurrentMenuItem())
			id = item->id;
		Window()->Unlock();
	}
	return id;
}

// TranslatorFormat
uint32
TranslatorSavePanel::TranslatorFormat() const
{
	uint32 format = 0;
	if (Window()->Lock()) {
		if (TranslatorItem* item = _GetCurrentMenuItem())
			format = item->format;
		Window()->Unlock();
	}
	return format;
}

// SetTranslator
void
TranslatorSavePanel::SetTranslator(translator_id id, uint32 format)
{
	if (id > 0) {
		if (Window()->Lock()) {
			for (int32 i = 0;
				 TranslatorItem* item = dynamic_cast<TranslatorItem*>(fFormatM->ItemAt(i));
				 i++) {
				item->SetMarked(item->id == id && item->format == format);
			}
			_UpdateMenuName();
			Window()->Unlock();
		}
	}
}

// UpdateStrings
void
TranslatorSavePanel::UpdateStrings()
{
	LanguageManager* manager = LanguageManager::Default();
	SetButtonLabel(B_DEFAULT_BUTTON, manager->GetString(SAVE, "Save"));
	SetButtonLabel(B_CANCEL_BUTTON, manager->GetString(CANCEL, "Cancel"));
	BString helper = manager->GetString(SETTINGS, "Settings");
	helper << B_UTF8_ELLIPSIS;
	fSettingsMI->SetLabel(helper.String());
}

// _GetCurrentMenuItem
TranslatorItem*
TranslatorSavePanel::_GetCurrentMenuItem() const
{
	return (TranslatorItem*)fFormatM->FindMarked();
}

// _TranslatorSettings
void
TranslatorSavePanel::_TranslatorSettings()
{
	TranslatorItem *item = _GetCurrentMenuItem();
	if (item == NULL)
		return;

	BTranslatorRoster *roster = BTranslatorRoster::Default();
	BView *view;
	BRect rect(0, 0, 239, 239);
	
	// Build a window around this translator's configuration view
	status_t err = roster->MakeConfigurationView(item->id, NULL, &view, &rect);
	if (err < B_OK || view == NULL) {
		BAlert *alert = new BAlert(NULL, strerror(err), "OK");
		alert->Go();
	} else {
		if (fConfigWindow != NULL) {
			if (fConfigWindow->Lock())
				fConfigWindow->Quit();
		}
		fConfigWindow = new Panel(rect, "Translator Settings",
								  B_TITLED_WINDOW_LOOK,
								  B_NORMAL_WINDOW_FEEL,
								  B_NOT_ZOOMABLE | B_NOT_RESIZABLE);
		fConfigWindow->AddChild(view);
		// Just to make sure
		view->MoveTo(0, 0);
		view->ResizeTo(rect.Width(), rect.Height());
		fConfigWindow->MoveTo(100, 100);
		fConfigWindow->Show();
	}
}

// _BuildMenu
void
TranslatorSavePanel::_BuildMenu()
{
	fFormatM = new BMenu("Translator");
	BTranslatorRoster* roster = BTranslatorRoster::Default();
	translator_id* translators;
	int32 count;
	// find all translators on the system
	roster->GetAllTranslators(&translators, &count);
	const translation_format* format;
	int32 format_count;
	
	for (int32 x = 0; x < count; x++) {
		// determine which formats this one can write
		roster->GetOutputFormats(translators[x], &format, &format_count);
		for (int32 y = 0; y < format_count; y++) {
			// check if this is an image translator
			if (format[y].group == B_TRANSLATOR_BITMAP) {
				// if this format saves to some native
				// format build a menu item for it
				if (format[y].type == B_TRANSLATOR_BITMAP)
					continue;
				TranslatorItem *item = new TranslatorItem(format[y].name,
														  format[y].MIME,
														  new BMessage(MSG_FORMAT),
														  translators[x],
														  format[y].type);
				fFormatM->AddItem(item);
				break;
			}
		}
	}
	delete[] translators;
	
	// pick the first translator in the list
	if (BMenuItem* item = fFormatM->ItemAt(0))
		item->SetMarked(true);

	// add settings item
	fFormatM->AddSeparatorItem();
	fSettingsMI = new BMenuItem("Settings"B_UTF8_ELLIPSIS,
								new BMessage(MSG_SETTINGS));
	fFormatM->AddItem(fSettingsMI);

	fFormatM->SetTargetForItems(this);
}

// _UpdateMenuName
void
TranslatorSavePanel::_UpdateMenuName()
{
	if (TranslatorItem* item = _GetCurrentMenuItem()) {
		BString helper("Translator: ");
		helper << item->ShortName();
		fFormatM->Superitem()->SetLabel(helper.String());
	}
}
