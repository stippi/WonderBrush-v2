// WonderBrushApp.cpp

#include <stdio.h>
#include <string.h>

#include <Alert.h>
#include <Bitmap.h>
#include <Entry.h>
#include <FilePanel.h>
#include <FindDirectory.h>
#include <Message.h>
#include <Mime.h>
#include <Path.h>
#include <Resources.h>
#include <Screen.h>
#include <String.h>

#include "AboutView.h"
#include "BubbleHelper.h"
#include "BBP.h"
#include "FontManager.h"
#include "GlobalSettings.h"
#include "LanguageManager.h"
#include "MainWindow.h"
#include "Panel.h"
#include "Strings.h"
#include "version.h"

#include "WonderBrushApp.h"

#ifndef B_LANGUAGE_CHANGED
	#define B_LANGUAGE_CHANGED '_BLC'
#endif

static char* kAppSig = "application/x.vnd-YellowBites.WonderBrush";

// ImageRefFilter
class ImageRefFilter : public BRefFilter {
public:
	virtual bool Filter(const entry_ref* ref, BNode* node, struct stat_beos* st,
		const char* filetype)
	{
		BString mimeRef(filetype);
		BString mimeOur("image/");
		BEntry entry(ref, true);
		if (entry.InitCheck() == B_OK && entry.IsDirectory())
			return true;
		else
			return (mimeRef.FindFirst(mimeOur) == 0);
	}
};

static ImageRefFilter kImageRefFilter;

// constructor
WonderBrushApp::WonderBrushApp()
	: MApplication(kAppSig),
	  fMainWindow(new MainWindow(BRect(50.0, 50.0, 850.0, 650.0))),
	  fOpenPanel(new BFilePanel(B_OPEN_PANEL)),
	  fRefsMessageSent(false),
	  fInitialRefsMessage(NULL),
	  fOpenPathAdjusted(false)
{
	fOpenPanel->SetTarget(BMessenger(this, this));
	fOpenPanel->SetRefFilter(&kImageRefFilter);

	FontManager::CreateDefault();
}

// destructor
WonderBrushApp::~WonderBrushApp()
{
	FontManager::DeleteDefault();
	delete fOpenPanel;
}

// AboutRequested
void
WonderBrushApp::AboutRequested()
{
	Panel* aboutPanel = new Panel(BRect(0.0, 0.0, 400.0, 300.0), "About WonderBrush",
								  B_MODAL_WINDOW, B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE);

	BString licenceeInfo;
#ifdef USE_SERIAL_NUMBERS
	const char* serialNumber = GlobalSettings::Default().SerialNumber();
	if (!serialNumber)
		licenceeInfo << "<invalid serial number>";
	else {
		licenceeInfo << "Serial Number: ";
		int32 c = 1;
		while (*serialNumber) {
			if (*serialNumber) {
				licenceeInfo << *serialNumber;
				serialNumber++;
			} else
				break;
			if (*serialNumber && c % 5 == 0)
				licenceeInfo << " - ";
			c++;
		}
	}
#else
	const char* licencee = fMainWindow->GetLicenceeName();
	if (!licencee)
		licenceeInfo = "<running in demo mode>";
	else
		licenceeInfo << "Registered to: " << licencee;
#endif

	int32 infoCount = 6;
	const char* infos[infoCount];
	infos[0] = "Colorpicker based on Colors! by Werner Freytag";
	infos[1] = "Text Tool uses libfreetype2 by the FreeType Team";
	infos[2] = "Anti-Grain Geometry - Version 2.3";
	infos[3] = "Copyright (C) 2002-2005 Maxim Shemanarev (McSeem)";
	infos[4] = "De-Noise filter uses CImg.h by David Tschumperlé";
	infos[5] = "see <http://cimg.sourceforge.net>";


	AboutView *aboutView = new AboutView("WonderBrush", kAppVersionString,
										 "by Stephan Aßmus (a.k.a. stippi)",
										 infos, infoCount,
										 licenceeInfo.String(),
										 B_UTF8_COPYRIGHT" 2003-2008 YellowBites, All Rights Reserved.");
	aboutPanel->AddChild(aboutView);
	BScreen screen(aboutPanel);
	aboutPanel->MoveTo(screen.Frame().Width() / 2.0 - aboutPanel->Frame().Width() / 2.0,
					   screen.Frame().Height() / 2.0 - aboutPanel->Frame().Height() / 2.0);
	aboutPanel->Show();
}

// QuitRequested
bool
WonderBrushApp::QuitRequested()
{
	return fMainWindow->QuitDocuments();
}

// ReadyToRun
void
WonderBrushApp::ReadyToRun()
{
	_InstallMimeTypes();
	fMainWindow->Show();
	FontManager::Default()->StartWatchingAll(fMainWindow);
	FontManager::Default()->PostMessage(MSG_UPDATE);

	snooze(300000);
	if (!fInitialRefsMessage && !fMainWindow->CountDocuments()) {
		GlobalSettings* settings = &GlobalSettings::Default();
		switch (settings->StartupAction()) {
			case STARTUP_ASK_OPEN:
				PostMessage(MSG_OPEN);
				break;
			case STARTUP_ASK_NEW:
				fMainWindow->PostMessage(MSG_NEW);
				break;
			case STARTUP_AUTO_SIZE: {
				uint32 width;
				uint32 height;
				settings->GetAutoSize(&width, &height);
				BMessage message(MSG_NEW);
				message.AddInt32("width", width);
				message.AddInt32("height", height);
				message.AddBool("no panel", true);
				fMainWindow->PostMessage(&message);
				break;
			}
			case STARTUP_DO_NOTHING:
			default:
				break;
		}
	}
	if (fInitialRefsMessage) {
		fMainWindow->PostMessage(fInitialRefsMessage);
		delete fInitialRefsMessage;
		fInitialRefsMessage = NULL;
	}
	fRefsMessageSent = true;
}

// RefsReceived
void
WonderBrushApp::RefsReceived(BMessage* message)
{
	if (fRefsMessageSent) {
		fMainWindow->PostMessage(message);
	} else {
		// this whole delaying thing works arround a problem with some of the controls
		// not receiving B_FRAME_RESIZED events (it appears) if the window processes
		// the refs message before Show() is called (which happens in ReadyToRun(),
		// which is called after this function here)
		fInitialRefsMessage = new BMessage(*message);
	}
}

// ArgvReceived
void
WonderBrushApp::ArgvReceived(int32 argc, char** argv)
{
	BMessage* message = new BMessage(B_REFS_RECEIVED);
	for (int32 i = 1; i < argc; i++) {
		entry_ref ref;
		if (get_ref_for_path(argv[i], &ref) == B_OK) {
			message->AddRef("refs", &ref);
		}
	}
	RefsReceived(message);
	delete message;
}

// MessageReceived
void
WonderBrushApp::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case BBP_OPEN_BBITMAP:
			RefsReceived(message);
			if (message->IsSourceWaiting()) {
				BMessage reply(B_MESSAGE_NOT_UNDERSTOOD);
				message->SendReply(&reply);
			}
			break;
		case MSG_OPEN: {
			entry_ref ref;
			if (!fOpenPathAdjusted &&
				message->FindRef("last path", &ref) >= B_OK) {
				BEntry entry(&ref, true);
				if (entry.GetParent(&entry) >= B_OK) {
					fOpenPathAdjusted = true;
					fOpenPanel->SetPanelDirectory(&entry);
				}
			}
			fOpenPanel->Refresh();
			fOpenPanel->Show();
			break;
		}
		// support for Zeta localization, doesn't harm the
		// program on R5
		case B_LANGUAGE_CHANGED: {
			BString systemLanguage;
			if (message->FindString("lang_name", &systemLanguage) >= B_OK) {
				BString wbLanguage;
				LanguageManager* m = LanguageManager::Default();
				for (int32 i = 0; m->GetNextLanguage(wbLanguage, i); i++) {
					// this does a fuzzy search as a work around
					// for the imprecise language description
					// in Zeta. It would be helpful if there was
					// a smarter matching using the international
					// two char language description.
					if (wbLanguage.CountChars() > 0 &&
						systemLanguage.IFindFirst(wbLanguage) >= 0) {
						m->SetLanguage(i);
						PostMessage(MSG_LANGUAGES_CHANGED, this);
						break;
					}
				}
			}
			break;
		}
		case MSG_LANGUAGES_CHANGED: {
			// distribute this message to every window
			for (int32 i = 0; BWindow* window = WindowAt(i); i++) {
				window->PostMessage(message);
			}
			// update our own strings
			LanguageManager* manager = LanguageManager::Default();
			fOpenPanel->SetButtonLabel(B_DEFAULT_BUTTON, manager->GetString(OPEN, "Open"));
			fOpenPanel->SetButtonLabel(B_CANCEL_BUTTON, manager->GetString(CANCEL, "Cancel"));
			BWindow* window = fOpenPanel->Window();
			if (window && window->Lock()) {
				BString helper("WonderBrush: ");
				helper << manager->GetString(OPEN, "Open");
				window->SetTitle(helper.String());
				window->Unlock();
			}
			break;
		}
		default:
			BApplication::MessageReceived(message);
			break;
	}
}


void
WonderBrushApp::_InstallMimeTypes()
{
	// install mime type of documents
	BMimeType mime("image/x-wonderbrush");
	if (mime.InitCheck() < B_OK)
		return;

	BString snifferRule;
	if (!(modifiers() & B_SHIFT_KEY) && mime.IsInstalled()
		&& mime.GetSnifferRule(&snifferRule) == B_OK && snifferRule.Length() > 0) 
		return;

	printf("installing image/x-wonderbrush MIME type\n");

	mime.Delete();

	status_t ret = mime.Install();
	if (ret < B_OK) {
		fprintf(stderr, "Could not install mime type "
			"\"image/x-wonderbrush\": %s.\n", strerror(ret));
		return;
	}

	// set preferred app
	if (mime.SetPreferredApp(kAppSig) < B_OK)
		fprintf(stderr, "Could not set preferred app!\n");

	// set descriptions
	if (mime.SetShortDescription("WonderBrush Image") < B_OK)
		fprintf(stderr, "Could not set short description of mime type!\n");
	if (mime.SetLongDescription("Image format containing layers and objects in "
		"flattened BMessage, WonderBrush native image.") != B_OK) {
		fprintf(stderr, "Could not set long description of mime type!\n");
	}

	// set extensions
	BMessage message('extn');
	message.AddString("extensions", "wbi");
	message.AddString("extensions", "WonderBrush");
	if (mime.SetFileExtensions(&message) < B_OK)
		fprintf(stderr, "Could not set extensions of mime type!\n");

	// set sniffer rule
	snifferRule = "0.9 ([0]'1BOF') ([0:32]\"layer\")";
	if (mime.SetSnifferRule(snifferRule.String()) < B_OK) {
		BString parseError;
		BMimeType::CheckSnifferRule(snifferRule.String(), &parseError);
		fprintf(stderr, "Could not set sniffer rule of mime type: %s\n", parseError.String());
	}

	// set document icons
	BResources* resources = AppResources();
		// does not need to be freed (belongs to BApplication base)
	if (resources == NULL) {
		fprintf(stderr, "Could not find app resources.\n");
		return;
	}

	size_t size;
	const void* iconData;

#if TARGET_PLATFORM_HAIKU
	iconData = resources->LoadResource('VICN', "BEOS:image/x-wonderbrush", &size);
	if (iconData && mime.SetIcon((uint8*)iconData, size) < B_OK)
		fprintf(stderr, "Could not set vector icon of mime type.\n");
#else
	iconData = resources->LoadResource('ICON', "WB:L:DOC_ICON", &size);
	BBitmap largeIcon(BRect(0, 0, 31, 31), 0, B_CMAP8);
	if (iconData && largeIcon.IsValid() && size == (size_t)largeIcon.BitsLength()) {
		memcpy(largeIcon.Bits(), iconData, size);
		if (mime.SetIcon(&largeIcon, B_LARGE_ICON) < B_OK)
			fprintf(stderr, "Could not set large icon of mime type.\n");
	} else
		fprintf(stderr, "Could not find large icon in app resources (data: %p, size: %ld).\n", iconData, size);
	iconData = resources->LoadResource('MICN', "WB:M:DOC_ICON", &size);
	BBitmap smallIcon(BRect(0, 0, 15, 15), 0, B_CMAP8);
	if (iconData && smallIcon.IsValid() && size == (size_t)smallIcon.BitsLength()) {
		memcpy(smallIcon.Bits(), iconData, size);
		if (mime.SetIcon(&smallIcon, B_MINI_ICON) < B_OK)
			fprintf(stderr, "Could not set small icon of mime type.\n");
	} else
		fprintf(stderr, "Could not find small icon in app resources (data: %p, size: %ld).\n", iconData, size);
#endif // !TARGET_PLATFORM_HAIKU

	// copy icon to ZETA
#if TARGET_PLATFORM_ZETA
	mime.Unset();
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) >= B_OK
		&& path.Append("beos_mime/image/x-wonderbrush") >= B_OK) {
		BNode mimeNode(path.Path());
		if (mimeNode.InitCheck() >= B_OK) {
			// SVG icon
			iconData = resources->LoadResource('zICO', "WB:V:DOC_ICON", &size);
			if (iconData) {
				if (mimeNode.WriteAttr("META:V:STD_ICON", 'zICO', 0, iconData, size) < B_OK)
					fprintf(stderr, "Could not write SVG icon to mime data base entry.\n");
			} else
				fprintf(stderr, "Could not find SVG icon in app resources (data: %p, size: %ld).\n", iconData, size);
			// funny extra data
			iconData = resources->LoadResource('iICO', "WB:D:DOC_ICON", &size);
			if (iconData) {
				if (mimeNode.WriteAttr("META:D:STD_ICON", 'iICO', 0, iconData, size) < B_OK)
					fprintf(stderr, "Could not write SVG icon description to mime data base entry.\n");
			} else
				fprintf(stderr, "Could not find SVG icon description in app resources (data: %p, size: %ld).\n", iconData, size);
		} else
			fprintf(stderr, "Could not get a BNode on the mime data base entry: %s\n", strerror(mimeNode.InitCheck()));
	} else
		fprintf(stderr, "Could not get path to the mime data base entry.\n");
#endif // TARGET_PLATFORM_ZETA
}

