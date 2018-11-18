// SettingsPanel.cpp

#include <stdio.h>

#include <Application.h>
#include <MenuItem.h>

#include <MBorder.h>
#include <MButton.h>
#include <MCheckBox.h>
#include <MTextControl.h>
#include <HGroup.h>
#include <Space.h>
#include <VGroup.h>

#include "support.h"

#include "LabelPopup.h"
#include "LanguageManager.h"
#include "GlobalSettings.h"
#include "DimensionsControl.h"
#include "LanguageManager.h"
#include "Seperator.h"

#include "SettingsPanel.h"

enum {
	MSG_CANCEL					= 'cncl',
	MSG_DONE					= 'done',
	MSG_REVERT					= 'rvrt',

	MSG_USE_SYSTEM_LANGUAGE		= 'usyl',

	MSG_STARTUP_ACTION			= 'stup',
};

// constructor
SettingsPanel::SettingsPanel(BRect frame, BMessage* msg, BWindow* target)
	: Panel(frame, "Settings",
			B_FLOATING_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL,
			B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE),
	  fMessage(msg),
	  fTarget(target)
{
	fStartupActionPU = new LabelPopup("On Startup");

	BMessage* message = new BMessage(MSG_STARTUP_ACTION);
	message->AddInt32("action", STARTUP_DO_NOTHING);
	fDoNothingMI = new BMenuItem("Do Nothing", message);

	message = new BMessage(MSG_STARTUP_ACTION);
	message->AddInt32("action", STARTUP_ASK_OPEN);
	fAskOpenMI = new BMenuItem("Display Open Panel", message);

	message = new BMessage(MSG_STARTUP_ACTION);
	message->AddInt32("action", STARTUP_ASK_NEW);
	fAskNewMI = new BMenuItem("Display New Panel", message);

	message = new BMessage(MSG_STARTUP_ACTION);
	message->AddInt32("action", STARTUP_AUTO_SIZE);
	fAutoSizeMI = new BMenuItem("Create New Canvas of Size", message);

	fStartupActionPU->Menu()->AddItem(fDoNothingMI);
	fStartupActionPU->Menu()->AddItem(fAskOpenMI);
	fStartupActionPU->Menu()->AddItem(fAskNewMI);
	fStartupActionPU->Menu()->AddItem(fAutoSizeMI);

	fStartupSizeDC = new DimensionsControl();

	fLiveUpdatesCB = new MCheckBox("Live Updates in Navigator and Layer Icons", 0, true);

	fUseSystemLanguageCB = new MCheckBox("Use Same Language as the System",
										 new BMessage(MSG_USE_SYSTEM_LANGUAGE),
										 this, true);

	fLanguagePU = new LabelPopup("Language");

	// interface layout
	BView* topView = new VGroup
	(
		new MBorder
		(
			M_RAISED_BORDER, 5, "dimension",
			new VGroup (
				fStartUpS = new Seperator("Startup Action"),
				fStartupActionPU,
				fStartupSizeDC,
				new Space(minimax(1.0, 8.0, 10000.0, 10000.0, 1.0)),

				fInterfaceS = new Seperator("Interface"),
				fLiveUpdatesCB,
#ifdef TARGET_PLATFORM_ZETA
				fUseSystemLanguageCB,
#endif
				fLanguagePU,
				new Space(minimax(1.0, 8.0, 10000.0, 10000.0, 1.0)),
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 5, "buttons",
			new HGroup
			(
				minimax(0.0, 0.0, 10000.0, 10000.0, 0.0),
//				new Space(minimax(0.0, 0.0, 10000.0, 0.0, 5.0)),
				fRevertB = new MButton("Revert", new BMessage(MSG_REVERT), this),
				new Space(minimax(40.0, 0.0, 10.0, 10000.0, 3.0)),
				fCancelB = new MButton("Cancel", new BMessage(MSG_CANCEL), this),
				new Space(minimax(5.0, 0.0, 10.0, 10000.0, 1.0)),
				fOkB = new MButton("Ok", new BMessage(MSG_DONE), this),
				new Space(minimax(2.0, 0.0, 2.0, 10000.0, 0.0)),
				0
			)
		),
		0
	);

	fStartupSizeDC->SetWidthLimits(1, 4096);
	fStartupSizeDC->SetHeightLimits(1, 4096);
	fStartupSizeDC->SetDimensions(800, 600);
	fStartupSizeDC->SetLabels("Width", "Height");

	DivideSame(fStartupActionPU,
			   fStartupSizeDC->WidthControl(),
			   fStartupSizeDC->HeightControl(),
			   fLanguagePU,
			   NULL);

	SetDefaultButton(fOkB);

	if (fTarget)
		AddToSubset(fTarget);
	else
		SetFeel(B_FLOATING_APP_WINDOW_FEEL);

	AddChild(topView);
	fStartupActionPU->Menu()->SetTargetForItems(this);

	_BuildLanguageMenu(fLanguagePU->Menu());
	_SetToSettings();

	UpdateStrings();
}

// destructor
SettingsPanel::~SettingsPanel()
{
	delete fMessage;
}

// Cancel
void
SettingsPanel::Cancel()
{
	PostMessage(MSG_CANCEL);
}

// QuitRequested
bool
SettingsPanel::QuitRequested()
{
	_Finish(MSG_CANCEL);
	return true;
}

// MessageReceived
void
SettingsPanel::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_CANCEL:
		case MSG_DONE:
			_Finish(message->what);
			PostMessage(B_QUIT_REQUESTED);
			break;
		case MSG_REVERT:
			_SetToSettings();
			break;
		case MSG_STARTUP_ACTION: {
			int32 action;
			if (message->FindInt32("action", &action) >= B_OK) {
				fStartupSizeDC->SetEnabled(action == STARTUP_AUTO_SIZE);
			}
			break;
		}
		case MSG_USE_SYSTEM_LANGUAGE: {
			// turn off language menu if we use the system language
			fLanguagePU->SetEnabled(!fUseSystemLanguageCB->Value());
			// update to system language now!
			if (fUseSystemLanguageCB->Value()) {
				LanguageManager* manager = LanguageManager::Default();
				_SetLanguage(manager->PosixSystemLanguage());
			}
			break;
		}
		case MSG_SET_LANGUAGE: {
			int32 cookie;
			if (message->FindInt32("index", &cookie) >= B_OK)
				_SetLanguage(cookie);
			break;
		}
		case MSG_LANGUAGES_CHANGED: {
			// message comes from the be_app
			_BuildLanguageMenu(fLanguagePU->Menu());
			UpdateStrings();
			break;
		}
		default:
			Panel::MessageReceived(message);
			break;
	}
}

// UpdateStrings
void
SettingsPanel::UpdateStrings()
{
	LanguageManager* manager = LanguageManager::Default();

	SetTitle(manager->GetString(PROGRAM_SETTINGS, "Program Settings"));

	int32 cookie = manager->Language();
	for (int32 i = 0; BMenuItem* item = fLanguagePU->Menu()->ItemAt(i); i++) {
		BMessage* message = item->Message();
		int32 itemCookie;
		if (message && message->FindInt32("index", &itemCookie) >= B_OK && itemCookie == cookie)
			item->SetMarked(true);
		else
			item->SetMarked(false);
	}

	fStartupActionPU->SetLabel(manager->GetString(ON_STARTUP, "On Startup"));
	fDoNothingMI->SetLabel(manager->GetString(DO_NOTHING, "Do Nothing"));
	fAskOpenMI->SetLabel(manager->GetString(ASK_OPEN, "Display Open Panel"));
	fAskNewMI->SetLabel(manager->GetString(ASK_NEW, "Display New Panel"));
	fAutoSizeMI->SetLabel(manager->GetString(AUTO_SIZE, "Create New Canvas of Size"));
	fStartupActionPU->RefreshItemLabel();

	fStartupSizeDC->SetLabels(manager->GetString(WIDTH, "Width"),
							  manager->GetString(HEIGHT, "Height"));

	fLiveUpdatesCB->SetLabel(manager->GetString(LIVE_UPDATES, "Live Updates in Navigator and Layer Icons"));
	fUseSystemLanguageCB->SetLabel(manager->GetString(USE_SYSTEM_LANGUAGE, "Use Same Language as the System"));
	fLanguagePU->SetLabel(manager->GetString(LANGUAGE, "Language"));

	fStartUpS->SetLabel(manager->GetString(STARTUP_ACTION, "Startup Action"));
	fInterfaceS->SetLabel(manager->GetString(INTERFACE, "Interface"));

	fOkB->SetLabel(manager->GetString(OK, "Ok"));
	fCancelB->SetLabel(manager->GetString(CANCEL, "Cancel"));
	fRevertB->SetLabel(manager->GetString(REVERT, "Revert"));

	RecalcSize();
}


// _SetLanguage
void
SettingsPanel::_SetLanguage(int32 cookie)
{
	LanguageManager* manager = LanguageManager::Default();
	if (!manager->SetLanguage(cookie)) {
		// an error happened and the language menu will not be current
		_BuildLanguageMenu(fLanguagePU->Menu());
	}
	be_app->PostMessage(MSG_LANGUAGES_CHANGED);
}

// _SetLanguage
void
SettingsPanel::_SetLanguage(const char* language)
{
	if (language) {
		LanguageManager* manager = LanguageManager::Default();
		if (!manager->SetLanguage(language)) {
			// an error happened and the language menu will not be current
			_BuildLanguageMenu(fLanguagePU->Menu());
		}
	}
	be_app->PostMessage(MSG_LANGUAGES_CHANGED);
}

// _Finish
void
SettingsPanel::_Finish(uint32 what)
{
	BMessage msg('size');
	BLooper* looper = fTarget ? static_cast<BLooper*>(fTarget)
							  : static_cast<BLooper*>(be_app);
	if (fMessage)
		msg = *fMessage;
	if (what == MSG_DONE) {
		_ApplySettings();
	}
	msg.AddRect("panel frame", Frame());
	looper->PostMessage(&msg);
}

// _SetToSettings
void
SettingsPanel::_SetToSettings()
{
	GlobalSettings* settings = GlobalSettings::CreateDefault();
	// start up action settings
	bool enableAutoSize = false;
	switch (settings->StartupAction()) {
		case STARTUP_DO_NOTHING:
			fDoNothingMI->SetMarked(true);
			break;
		case STARTUP_ASK_OPEN:
			fAskOpenMI->SetMarked(true);
			break;
		case STARTUP_ASK_NEW:
			fAskNewMI->SetMarked(true);
			break;
		case STARTUP_AUTO_SIZE:
			fAutoSizeMI->SetMarked(true);
			enableAutoSize = true;
			break;
	}
	// startup auto "create new" size
	fStartupSizeDC->SetEnabled(enableAutoSize);
	uint32 width, height;
	settings->GetAutoSize(&width, &height);
	fStartupSizeDC->SetDimensions(width, height);
	// live updates
	fLiveUpdatesCB->SetValue(settings->LiveUpdates());
	// system language
	fUseSystemLanguageCB->SetValue(settings->UseSystemLanguage());
	// turn off language menu if we use the system language
	fLanguagePU->SetEnabled(!fUseSystemLanguageCB->Value());
}

// _ApplySettings
void
SettingsPanel::_ApplySettings()
{
	GlobalSettings* settings = GlobalSettings::CreateDefault();
	// startup action
	if (BMenuItem* item = fStartupActionPU->Menu()->FindMarked()) {
		int32 index = fStartupActionPU->Menu()->IndexOf(item);
		if (index >= 0)
			settings->SetStartupAction(index);
	}
	// startup auto "create new" size
	settings->SetAutoSize(fStartupSizeDC->Width(), fStartupSizeDC->Height());
	// live updates
	settings->SetLiveUpdates(fLiveUpdatesCB->Value() == B_CONTROL_ON);
	// use system language
	settings->SetUseSystemLanguage(fUseSystemLanguageCB->Value() == B_CONTROL_ON);
	// language
	settings->SetLanguage(LanguageManager::Default()->LanguageName());
}

// _BuildLanguageMenu
void
SettingsPanel::_BuildLanguageMenu(BMenu* menu)
{
	// whipe menu clean
	while (BMenuItem* item = menu->RemoveItem((int32)0))
		delete item;
	// add the languages the language manager tells us he got...
	int32 index = 0;
	BString string;
	LanguageManager* manager = LanguageManager::Default();
	while (manager->GetNextLanguage(string, index)) {
		BMessage* message = new BMessage(MSG_SET_LANGUAGE);
		message->AddInt32("index", index);
		BMenuItem* item = new BMenuItem(string.String(), message);
		if (index == manager->Language())
			item->SetMarked(true);
		menu->AddItem(item);
		index++;
	}
	menu->SetTargetForItems(this);
}



