// BrushPanel.cpp

#include <stdio.h>

#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Message.h>
#include <Path.h>

#include <HGroup.h>
#include <MBorder.h>
#include <MButton.h>
#include <Space.h>
#include <VGroup.h>

#include "support.h"

#include "Brush.h"
#include "GlobalFunctions.h"
#include "LanguageManager.h"
#include "BetterMScrollView.h"
#include "BrushListView.h"
#include "BrushStroke.h"
#include "MenuBar.h"
#include "NamePanel.h"

#include "BrushPanel.h"


// constructor
BrushPanel::BrushPanel(BRect frame,
					   BWindow *window,
					   CanvasView* view)
	: Panel(frame, "Brush Panel",
			B_FLOATING_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL,
			B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE),
	  fSettings(new BMessage('sett')),
	  fWindow(window)
{
	fBrushLV = new BrushListView(view, BRect(0.0, 0.0, 50.0, 50.0));
	BView* topView = new VGroup
	(
		_CreateMenu(),
		new MBorder
		(
			M_RAISED_BORDER, 5, "",
			new VGroup
			(
				new BetterMScrollView(fBrushLV, false, true),
/*				new HGroup
				(
					new Space(minimax(0.0, 0.0, 10000.0, 10000.0, 5.0)),
					fNewB = new MButton("New", new BMessage(MSG_NEW_BRUSH), fBrushLV),
					0
				),*/
				0
			)
		),
		0
	);

	AddChild(topView);
	AddToSubset(fWindow);
	fBrushLV->SetBrushMenu(fBrushM);

	LoadSettings();
	_BuildBrushSetMenu();

	UpdateStrings();

	Show();
}

// destructor
BrushPanel::~BrushPanel()
{
	SaveSettings();
	_EmptyListView();
	delete fSettings;
}

// QuitRequested
bool
BrushPanel::QuitRequested()
{
	if (fWindow) {
		fWindow->PostMessage(MSG_BRUSH_PANEL_QUIT);
		return false;
	} else
		return true;
}

// MessageReceived
void
BrushPanel::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_SET_BRUSH_SET:
			_LoadBrushSet(message);
			break;

		case MSG_SAVE_BRUSH_SET: {
			entry_ref ref;
			const char* name;
			if (message->FindRef("ref", &ref) >= B_OK) {
				_SaveBrushSet(message, &ref);
			} else if (message->FindString("name", &name) >= B_OK) {
				// message contains name
				BPath path;
				if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) >= B_OK
					&& path.Append("WonderBrush/brush_sets") >= B_OK
					&& path.Append(name) >= B_OK) {
					BEntry entry(path.Path());
					if (entry.GetRef(&ref) >= B_OK)
						_SaveBrushSet(message, &ref);
				}
			} else {
				// ask name from user
				LanguageManager* manager = LanguageManager::Default();
				new NamePanel(manager->GetString(NAME, "Name"), NULL, this, this,
												 new BMessage(*message));
			}
			break;
		}
		case MSG_DELETE_BRUSH_SET: {
			entry_ref ref;
			if (message->FindRef("ref", &ref) >= B_OK) {
				BEntry entry(&ref);
				if (entry.Remove() >= B_OK)
					_BuildBrushSetMenu();
			}
			break;
		}
		default:
			Panel::MessageReceived(message);
	}
}

// LoadSettings
void
BrushPanel::LoadSettings()
{
	load_settings(fSettings, "brush_set_settings", "WonderBrush");
	// restore window frame
	BRect frame;
	if (fSettings->FindRect("window frame", &frame) == B_OK) {
		make_sure_frame_is_on_screen(frame, this);
		MoveTo(frame.LeftTop());
		ResizeTo(frame.Width(), frame.Height());
	}
	_LoadBrushSet(fSettings);
}

// SaveSettings
void
BrushPanel::SaveSettings()
{
	_SaveBrushSet(fSettings, NULL);

	if (fSettings->ReplaceRect("window frame", Frame()) < B_OK) {
		fSettings->AddRect("window frame", Frame());
	}

	save_settings(fSettings, "brush_set_settings", "WonderBrush");
}

// UpdateStrings
void
BrushPanel::UpdateStrings()
{
	LanguageManager* manager = LanguageManager::Default();

	SetTitle(manager->GetString(BRUSH_PANEL, "Brush Panel"));

	fBrushM->Superitem()->SetLabel(manager->GetString(BRUSH, "Brush"));
	fBrushSetM->Superitem()->SetLabel(manager->GetString(SET, "Set"));
	fLoadM->Superitem()->SetLabel(manager->GetString(LOAD, "Load"));
	fSaveM->Superitem()->SetLabel(manager->GetString(SAVE, "Save"));
	fDeleteM->Superitem()->SetLabel(manager->GetString(DELETE, "Delete"));

	fSaveAsMI->SetLabel(manager->GetString(NEW_BRUSH, "New"));

	fBrushLV->UpdateStrings();
}

// _CreateMenu
MenuBar*
BrushPanel::_CreateMenu()
{
	MenuBar* menuBar = new MenuBar("brush menu bar");

	fBrushM = new BMenu("Brush");

	menuBar->AddItem(fBrushM);

	fBrushSetM = new BMenu("Set");

	fLoadM = new BMenu("Load");
	fBrushSetM->AddItem(fLoadM);
	fSaveM = new BMenu("Save As");
	fBrushSetM->AddItem(fSaveM);

	fBrushSetM->AddSeparatorItem();

	fDeleteM = new BMenu("Delete");
	fBrushSetM->AddItem(fDeleteM);

	menuBar->AddItem(fBrushSetM);

	return menuBar;
}

// _BuildBrushSetMenu
void
BrushPanel::_BuildBrushSetMenu()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) >= B_OK
		&& path.Append("WonderBrush/brush_sets") >= B_OK
		&& create_directory(path.Path(), 0777) >= B_OK) {
		BDirectory dir(path.Path());
		if (dir.InitCheck() >= B_OK) {
			// empty the menus
			while (BMenuItem* item = fLoadM->RemoveItem(0L)) {
				delete item;
			}
			while (BMenuItem* item = fSaveM->RemoveItem(0L)) {
				delete item;
			}
			while (BMenuItem* item = fDeleteM->RemoveItem(0L)) {
				delete item;
			}
			LanguageManager* manager = LanguageManager::Default();
			fSaveAsMI = new BMenuItem(manager->GetString(NEW, "New"),
									  new BMessage(MSG_SAVE_BRUSH_SET));
			fSaveAsMI->SetTarget(this);
			fSaveM->AddItem(fSaveAsMI);
			bool separator = false;
			// traverse files in swatches folder
			entry_ref ref;
			while (dir.GetNextRef(&ref) >= B_OK) {
				BFile file(&ref, B_READ_ONLY);
				BMessage* loadMessage = new BMessage();
				if (loadMessage->Unflatten(&file) >= B_OK
					&& loadMessage->HasBool("brush set")) {
					loadMessage->what = MSG_SET_BRUSH_SET;
					// load
					BMenuItem* loadItem = new BMenuItem(ref.name, loadMessage);
					loadItem->SetTarget(this);
					fLoadM->AddItem(loadItem);
					// save
					if (!separator) {
						fSaveM->AddSeparatorItem();
						separator = true;
					}
					BMessage* saveMessage = new BMessage(MSG_SAVE_BRUSH_SET);
					saveMessage->AddRef("ref", &ref);
					BMenuItem* saveItem = new BMenuItem(ref.name, saveMessage);
					saveItem->SetTarget(this);
					fSaveM->AddItem(saveItem);
					// delete
					BMessage* deleteMessage = new BMessage(MSG_DELETE_BRUSH_SET);
					deleteMessage->AddRef("ref", &ref);
					BMenuItem* deleteItem = new BMenuItem(ref.name, deleteMessage);
					deleteItem->SetTarget(this);
					fDeleteM->AddItem(deleteItem);
				} else
					delete loadMessage;
			}
		}
	}
}

// _LoadBrushSet
void
BrushPanel::_LoadBrushSet(const BMessage* message)
{
	// restore swatches
	if (message && message->HasBool("brush set")) {
		_EmptyListView();
		BMessage strokeArchive;
		const char* name;
		int32 i = 0;
		while (message->FindMessage("brush", i, &strokeArchive) >= B_OK
			   && message->FindString("name", i, &name) >= B_OK) {
			BrushStroke* stroke = new BrushStroke(&strokeArchive);
			// scale brush if scale was not 1.0
			float scale;
			if (message->FindFloat("brush scale", i, &scale) < B_OK)
				scale = 1.0;
			if (Brush* brush = stroke->GetBrush()) {
				range radius = brush->Radius();
				radius.min *= scale;
				radius.max *= scale;
				brush->SetRadius(radius);
			}
			fBrushLV->AddItem(new BrushItem(stroke, name));
			i++;
		}
	}
}

// _SaveBrushSet
void
BrushPanel::_SaveBrushSet(BMessage* message, const entry_ref* ref)
{
	message->MakeEmpty();
	message->AddBool("brush set", true);
	// store brush strokes
	for (int32 i = 0; BrushItem* item = (BrushItem*)fBrushLV->ItemAt(i); i++) {
		if (BrushStroke* stroke = item->GetBrushStroke()) {
			BMessage archive;
			if (stroke->Archive(&archive) >= B_OK) {
				message->AddMessage("brush", &archive);
				message->AddString("name", item->Text());
				message->AddFloat("brush scale", item->BrushScale());
			}
		}
	}
	if (ref) {
		BFile file(ref, B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
		if (message->Flatten(&file) >= B_OK) {
			// rebuild the swatches menu
			_BuildBrushSetMenu();
		}
	}
}

// _EmptyListView
void
BrushPanel::_EmptyListView() const
{
	int32 count = fBrushLV->CountItems();
	for (int32 i = count - 1; BrushItem* item = (BrushItem*)fBrushLV->RemoveItem(i); i--)
		delete item;
}
