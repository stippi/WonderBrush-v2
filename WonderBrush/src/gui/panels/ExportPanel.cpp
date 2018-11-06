// ExportPanel.cpp

#include <stdio.h>
#include <string.h>

#include <Alert.h>
#include <Button.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <ScrollBar.h>
#include <TextControl.h>
#include <TranslationKit.h>
#include <View.h>
#include <Window.h>

#include "Exporter.h"
#include "LanguageManager.h"
#include "Panel.h"
#include "Strings.h"

#include "ExportPanel.h"

enum {
	MSG_FORMAT		= 'sfpf',
	MSG_SETTINGS	= 'sfps',
};

// ExportItem class
ExportItem::ExportItem(const char* name,
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
ExportItem::ShortName() const
{
	return fShortName.String();
}

// ExportPanel class
ExportPanel::ExportPanel(const char* name,
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
		
		// find a couple of important views
		BView* background = Window()->ChildAt(0);
		BButton* cancel = dynamic_cast<BButton*>(background->FindView("cancel button"));
		BView* textview = background->FindView("text view");
		BScrollBar* hscrollbar = dynamic_cast<BScrollBar*>(background->FindView("HScrollBar"));

		if (!background || !cancel || !textview || !hscrollbar) {
			printf("ExportPanel::ExportPanel() - couldn't find necessary controls.\n");
			return;
		}

		_BuildMenu();
	
		BRect rect = textview->Frame();
		rect.top = cancel->Frame().top;
		font_height fh;
		be_plain_font->GetHeight(&fh);
		rect.bottom = rect.top + fh.ascent + fh.descent + 5.0;

		fFormatMF = new BMenuField(rect, "format popup", "Format", fFormatM, true,
								   B_FOLLOW_LEFT | B_FOLLOW_BOTTOM,
								   B_WILL_DRAW | B_NAVIGABLE);
		fFormatMF->SetDivider(be_plain_font->StringWidth("Format") + 7);
		fFormatMF->MenuBar()->ResizeToPreferred();
		fFormatMF->ResizeToPreferred();

		float height = fFormatMF->Bounds().Height() + 8.0;

		// find all the views that are in the way and
		// move up them up the height of the menu field
		BView *poseview = background->FindView("PoseView");
		if (poseview) poseview->ResizeBy(0, -height);
		BButton *insert = (BButton *)background->FindView("default button");
		if (hscrollbar) hscrollbar->MoveBy(0, -height);
		BScrollBar *vscrollbar = (BScrollBar *)background->FindView("VScrollBar");
		if (vscrollbar) vscrollbar->ResizeBy(0, -height);
		BView *countvw = (BView *)background->FindView("CountVw");
		if (countvw) countvw->MoveBy(0, -height);
		textview->MoveBy(0, -height);

#if TARGET_PLATFORM_ZETA
		fFormatMF->MoveTo(textview->Frame().left, fFormatMF->Frame().top + 2);
#else
		fFormatMF->MoveTo(textview->Frame().left, fFormatMF->Frame().top);
#endif

		background->AddChild(fFormatMF);

		// Build the "Settings" button relative to the format menu
		rect = cancel->Frame();
		rect.OffsetTo(fFormatMF->Frame().right + 5.0, rect.top);
		fSettingsB = new BButton(rect, "settings", "Settings"B_UTF8_ELLIPSIS,
								 new BMessage(MSG_SETTINGS),
								 B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_NAVIGABLE);
		fSettingsB->ResizeToPreferred();
		background->AddChild(fSettingsB);
		fSettingsB->SetTarget(this);

		textview->ResizeTo(fSettingsB->Frame().right - fFormatMF->Frame().left,
						   textview->Frame().Height());

		// Make sure the smallest window won't draw the "Settings" button over anything else
		float minWindowWidth = textview->Bounds().Width()
								+ cancel->Bounds().Width()
								+ (insert ? insert->Bounds().Width() : 0.0)
								+ 90;
		Window()->SetSizeLimits(minWindowWidth, 10000, 250, 10000);
		if (Window()->Bounds().IntegerWidth() + 1 < minWindowWidth)
			Window()->ResizeTo(minWindowWidth, Window()->Bounds().Height());


		window->Unlock();
	}
}

// destructor
ExportPanel::~ExportPanel()
{
}

// SendMessage
void
ExportPanel::SendMessage(const BMessenger* messenger,
								 BMessage* message)
{
	// add the current translator information to the message
	if (message) {
		int32 mode = ExportMode();
		message->AddInt32("export mode", mode);
		ExportItem* exportItem = dynamic_cast<ExportItem*>(_GetCurrentMenuItem());
		if (mode == EXPORT_TRANSLATOR && exportItem) {
			message->AddData("translator_id", B_RAW_TYPE, &(exportItem->id),
							 sizeof(translator_id));
			message->AddInt32("translator_format", exportItem->format);
		}
	}
	// let the original file panel code handle the rest
	BFilePanel::SendMessage(messenger, message);
}

// MessageReceived
void
ExportPanel::MessageReceived(BMessage* message)
{
	// Handle messages from controls we've added
	switch (message->what) {
		case MSG_FORMAT:
			// TODO: make this behaviour a setting
			AdjustExtension();
			_EnableSettings();
			break;
		case MSG_SETTINGS:
			_TranslatorSettings();
			break;
		default:
			BHandler::MessageReceived(message);
			break;
	}
}

// SetMode
void
ExportPanel::SetMode(bool exportMode)
{
	BWindow* window = Window();
	if (window && window->Lock()) {
		LanguageManager* manager = LanguageManager::Default();
		BString helper("WonderBrush: ");
		// TODO support "WonderBrush image"
		if (exportMode) {
			fFormatMF->SetEnabled(true);
			_EnableSettings();
			helper << manager->GetString(EXPORT_CANVAS, "Export Canvas");
		} else {
			fFormatMF->SetEnabled(false);
			fSettingsB->SetEnabled(false);
			helper << manager->GetString(SAVE_CANVAS, "Save Canvas");
		}
		window->SetTitle(helper.String());
		window->Unlock();
	}
}

// SetExportMode
void
ExportPanel::SetExportMode(int32 mode)
{
	BWindow* window = Window();
	if (window && window->Lock()) {
		switch (mode) {
			case EXPORT_CURSOR_SOURCE:
				fCursorMI->SetMarked(true);
				break;
			case EXPORT_BITMAP_SOURCE:
				fSourceMI->SetMarked(true);
				break;
			case EXPORT_SVG:
				fSVGMI->SetMarked(true);
				break;
			case EXPORT_ILLUSTRATOR:
				fIllustratorMI->SetMarked(true);
				break;
			case EXPORT_RDEF_SOURCE:
				fRDefMI->SetMarked(true);
				break;
		}
		_EnableSettings();
		window->Unlock();
	}
}

// AdjustExtension
void
ExportPanel::AdjustExtension()
{
	if (Window()->Lock()) {
		BView* background = Window()->ChildAt(0);
		BTextControl* textview = dynamic_cast<BTextControl*>(background->FindView("text view"));
	
		if (textview) {
	
			translator_id id = 0;
			uint32 format = 0;
			int32 mode = ExportMode();
			ExportItem* exportItem = dynamic_cast<ExportItem*>(_GetCurrentMenuItem());
			if (mode == EXPORT_TRANSLATOR && exportItem) {
				id = exportItem->id;
				format = exportItem->format;
			}
	
			Exporter* exporter = Exporter::ExporterFor(mode, id, format);

			if (exporter) {
				BString name(textview->Text());
		
				// adjust the name extension
				const char* extension = exporter->Extension();
				if (strlen(extension) > 0) {
					int32 cutPos = name.FindLast('.');
					int32 cutCount = name.Length() - cutPos;
					if (cutCount > 0 && cutCount <= 4) {
						name.Remove(cutPos, cutCount);
					}
					name << "." << extension;
				}
		
				SetSaveText(name.String());
			}

			delete exporter;
		}
		Window()->Unlock();
	}
}

// ExportMode
int32
ExportPanel::ExportMode() const
{
	int32 mode = EXPORT_NATIVE;
	BWindow* window = Window();
	if (window && window->Lock()) {
		if (fFormatMF->IsEnabled()) {
			BMenuItem* item = _GetCurrentMenuItem();
			if (ExportItem* exportItem = dynamic_cast<ExportItem*>(item)) {
				mode = EXPORT_TRANSLATOR;
			} else if (item == fCursorMI) {
				mode = EXPORT_CURSOR_SOURCE;
			} else if (item == fSourceMI) {
				mode = EXPORT_BITMAP_SOURCE;
			} else if (item == fSVGMI) {
				mode = EXPORT_SVG;
			} else if (item == fIllustratorMI) {
				mode = EXPORT_ILLUSTRATOR;
			} else if (item == fRDefMI) {
				mode = EXPORT_RDEF_SOURCE;
			}
		}
		window->Unlock();
	}
	return mode;
}

// TranslatorID
translator_id
ExportPanel::TranslatorID() const
{
	translator_id id = 0;
	BWindow* window = Window();
	if (window && window->Lock()) {
		if (ExportItem* item = dynamic_cast<ExportItem*>(_GetCurrentMenuItem()))
			id = item->id;
		window->Unlock();
	}
	return id;
}

// TranslatorFormat
uint32
ExportPanel::TranslatorFormat() const
{
	uint32 format = 0;
	BWindow* window = Window();
	if (window && window->Lock()) {
		if (ExportItem* item = dynamic_cast<ExportItem*>(_GetCurrentMenuItem()))
			format = item->format;
		window->Unlock();
	}
	return format;
}

// SetTranslator
void
ExportPanel::SetTranslator(translator_id id, uint32 format)
{
	if (id > 0) {
		BWindow* window = Window();
		if (window && window->Lock()) {
			for (int32 i = 0;
				 ExportItem* item = dynamic_cast<ExportItem*>(fFormatM->ItemAt(i));
				 i++) {
				item->SetMarked(item->id == id && item->format == format);
			}
			window->Unlock();
		}
	}
}

// UpdateStrings
void
ExportPanel::UpdateStrings()
{
	BWindow* window = Window();
	bool locked = false;
	if (window && window->Lock())
		locked = true;

	LanguageManager* manager = LanguageManager::Default();
	SetButtonLabel(B_DEFAULT_BUTTON, manager->GetString(SAVE, "Save"));
	SetButtonLabel(B_CANCEL_BUTTON, manager->GetString(CANCEL, "Cancel"));

	// Window title
	BString helper("WonderBrush: ");
	if (fFormatMF->IsEnabled()) {
		helper << manager->GetString(EXPORT_CANVAS, "Export Canvas");
	} else {
		helper << manager->GetString(SAVE_CANVAS, "Save Canvas");
	}
	window->SetTitle(helper.String());

	helper = manager->GetString(SETTINGS, "Settings");
	helper << B_UTF8_ELLIPSIS;
	fSettingsB->SetLabel(helper.String());
	// relayout format menufield and settings button
	BRect r = fSettingsB->Frame();
	fSettingsB->ResizeToPreferred();
	fSettingsB->MoveTo(r.right - fSettingsB->Bounds().Width(), r.top);
	fFormatMF->ResizeTo((fSettingsB->Frame().left - 5.0) - fFormatMF->Frame().left, fFormatMF->Frame().Height());
	BMenuBar* menuBar = fFormatMF->MenuBar();
	if (menuBar)
#if TARGET_PLATFORM_ZETA
		menuBar->ResizeTo(fFormatMF->Bounds().right - menuBar->Frame().left - 7, menuBar->Frame().Height());
#else
		menuBar->ResizeTo(fFormatMF->Bounds().right - menuBar->Frame().left - 1, menuBar->Frame().Height());
#endif

	fCursorMI->SetLabel(manager->GetString(CURSOR_SOURCE, "Cursor Source Code"));
	fSourceMI->SetLabel(manager->GetString(BITMAP_SOURCE, "Bitmap Source Code"));
	fSVGMI->SetLabel(manager->GetString(SVG_FORMAT, "SVG (Not all Object types)"));
	fIllustratorMI->SetLabel(manager->GetString(AI_FORMAT, "Adobe Illustrator (Not all Object types)"));
	fRDefMI->SetLabel(manager->GetString(RDEF_FORMAT, "RDef Source Code"));

	fFormatMF->SetLabel(manager->GetString(EXPORT_FORMAT, "Format"));
	fFormatMF->SetDivider(be_plain_font->StringWidth(fFormatMF->Label()) + 7.0);

	// refresh the item label as well
	if (BMenuItem* item = fFormatMF->Menu()->FindMarked())
		item->SetMarked(true);

	if (locked)
		window->Unlock();
}

// _GetCurrentMenuItem
BMenuItem*
ExportPanel::_GetCurrentMenuItem() const
{
	return fFormatM->FindMarked();
}

// _TranslatorSettings
void
ExportPanel::_TranslatorSettings()
{
	ExportItem *item = dynamic_cast<ExportItem*>(_GetCurrentMenuItem());
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
		view->ResizeToPreferred();
		fConfigWindow->MoveTo(100, 100);
		fConfigWindow->Show();
	}
}

// _BuildMenu
void
ExportPanel::_BuildMenu()
{
	fFormatM = new BPopUpMenu("Translator");
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
				ExportItem *item = new ExportItem(format[y].name,
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

	fFormatM->AddSeparatorItem();
	fCursorMI = new BMenuItem("Cursor Source Code", new BMessage(MSG_FORMAT));
	fFormatM->AddItem(fCursorMI);
	fSourceMI = new BMenuItem("Bitmap Source Code", new BMessage(MSG_FORMAT));
	fFormatM->AddItem(fSourceMI);
	fRDefMI = new BMenuItem("RDef Source Code", new BMessage(MSG_FORMAT));
	fFormatM->AddItem(fRDefMI);
	fFormatM->AddSeparatorItem();
	fSVGMI = new BMenuItem("SVG (Shape Objects only)", new BMessage(MSG_FORMAT));
	fFormatM->AddItem(fSVGMI);
	fIllustratorMI = new BMenuItem("Adobe Illustrator (Shape Objects only)", new BMessage(MSG_FORMAT));
	fFormatM->AddItem(fIllustratorMI);

	fFormatM->SetTargetForItems(this);
}

// _EnableSettings
void
ExportPanel::_EnableSettings() const
{
	BMenuItem* item = _GetCurrentMenuItem();
	fSettingsB->SetEnabled(dynamic_cast<ExportItem*>(item) != NULL);
}

