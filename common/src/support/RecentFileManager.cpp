// RecentFileManager.cpp

#include <new>
#include <stdio.h>
#include <string.h>

#include <Entry.h>
#include <File.h>
#include <Message.h>

#include "RecentFileManager.h"

using std::nothrow;

// static variables

// fDefaultManager
RecentFileManager*	RecentFileManager::fDefaultManager = NULL;


// constructor
RecentFileManager::RecentFileManager(const char* settingsFilePath,
									 uint32 maxEntries)
	: BLocker("recent files"),
	  fSettings(new BMessage('sett')),
	  fEntries(maxEntries),
	  fMaxEntries(maxEntries),
	  fObservers(10)
{
	Lock();
	if (settingsFilePath) {
		// remember path for later
		sprintf(fSettingsFilePath, "%s", settingsFilePath);
		// load settings file
		BFile file(fSettingsFilePath, B_READ_ONLY);
		status_t status = file.InitCheck();
		if (status >= B_OK) {
			status = fSettings->Unflatten(&file);
			if  (status >= B_OK) {
				entry_ref ref;
				for (uint32 i = 0; fSettings->FindRef("refs", i, &ref) >= B_OK; i++) {
					BEntry entry(&ref, true);
					if (entry.Exists()) {
						entry_ref* copy = new(nothrow) entry_ref(ref);
						if (!copy || i >= fMaxEntries || !fEntries.AddItem((void*)copy))
							break;
					}
				}
			} else
				fprintf(stderr, "RecentFileManager::RecentFileManager(): failed to unflatten settings: %s\n", strerror(status));
		} else
			fprintf(stderr, "RecentFileManager::RecentFileManager(): failed to open settings file: %s\n", strerror(status));
	} else {
		fSettingsFilePath[0] = 0;
		fprintf(stderr, "RecentFileManager::RecentFileManager(): no settings path specified\n");
	}
	Unlock();
}

// destructor
RecentFileManager::~RecentFileManager()
{
	Lock();
	// save settings file
	if (fSettings) {
		BFile file(fSettingsFilePath, B_CREATE_FILE | B_ERASE_FILE | B_READ_WRITE);
		status_t status = file.InitCheck();
		if (status >= B_OK) {
			status = fSettings->RemoveName("refs");
			if (status < B_OK)
				fprintf(stderr, "RecentFileManager::~RecentFileManager(): failed to remove \"refs\" entries from settings: %s\n", strerror(status));
			for (int32 i = 0; entry_ref* ref = (entry_ref*)fEntries.ItemAt(i); i++) {
				if ((status = fSettings->AddRef("refs", ref)) < B_OK) {
					fprintf(stderr, "RecentFileManager::~RecentFileManager(): Could not add entry to settings: %s\n", strerror(status));
					break;
				}
			}
			status = fSettings->Flatten(&file);
			if (status < B_OK)
				fprintf(stderr, "RecentFileManager::~RecentFileManager(): error flattening settings: %s\n", strerror(status));
			file.Unset();
		} else
			fprintf(stderr, "RecentFileManager::~RecentFileManager(): error opening settings file: %s\n", strerror(status));
		delete fSettings;
	} else {
		fprintf(stderr, "RecentFileManager::~RecentFileManager(): no settings!\n");
	}
	for (int32 i = 0; entry_ref* ref = (entry_ref*)fEntries.ItemAt(i); i++)
		delete ref;
	fEntries.MakeEmpty();
	for (int32 i = 0; BMessenger* messenger = (BMessenger*)fObservers.ItemAt(i); i++)
		delete messenger;
	fObservers.MakeEmpty();
	Unlock();
}

// InitCheck
status_t
RecentFileManager::InitCheck() const
{
	return fSettings ? B_OK : B_NO_MEMORY;
}

// CreateDefault
RecentFileManager*
RecentFileManager::CreateDefault(const char* settingsFilePath, uint32 maxEntries)
{
	if (!fDefaultManager) {
		fDefaultManager = new(nothrow) RecentFileManager(settingsFilePath,
														 maxEntries);
		if (fDefaultManager && fDefaultManager->InitCheck() != B_OK)
			DeleteDefault();
	}
	return fDefaultManager;
}

// DeleteDefault
void
RecentFileManager::DeleteDefault()
{
	delete fDefaultManager;
	fDefaultManager = NULL;
}

// Default
RecentFileManager*
RecentFileManager::Default()
{
	return fDefaultManager;
}

// AddFile
bool
RecentFileManager::AddFile(const entry_ref* ref)
{
	bool success = false;
	if (ref && Lock()) {
		entry_ref* copy = new(nothrow) entry_ref(*ref);
		if (copy) {
			// remove any (old) duplicates
			for (int32 i = 0; entry_ref* oldRef = (entry_ref*)fEntries.ItemAt(i); i++) {
				if (*oldRef == *ref) {
					if (fEntries.RemoveItem((void*)oldRef)) {
						delete oldRef;
						i--;
					}
				}
			}
			// remove old entries from beginning of the list
			while ((uint32)fEntries.CountItems() >= fMaxEntries) {
				if (entry_ref* oldRef = (entry_ref*)fEntries.RemoveItem(0L))
					delete oldRef;
				else
					break;
			}
			success = fEntries.AddItem((void*)copy);
			if (success) {
				// inform every watching BMessenger
				// put the items into the message already, should save some work
				BMessage message(MSG_RECENT_FILES_CHANGED);
				for (int32 i = 0; entry_ref* recentRef = (entry_ref*)fEntries.ItemAt(i); i++)
					message.AddRef("refs", recentRef);
				for (int32 i = 0; BMessenger* messenger = (BMessenger*)fObservers.ItemAt(i); i++) {
					if (messenger->IsValid())
						messenger->SendMessage(&message);
				}
			} else
				delete copy;
		}
		Unlock();
	}
	return success;
}

// GetNextFile
const entry_ref*
RecentFileManager::GetNextFile(int32* cookie) const
{
	const entry_ref* ref = NULL;
	if (cookie) {
		ref = (const entry_ref*)fEntries.ItemAt(*cookie);
		(*cookie)++;
	}
	return ref;
}

// StartWatching
bool
RecentFileManager::StartWatching(const BMessenger& messenger)
{
	bool success = false;
	if (Lock()) {
		BMessenger* copy = new(nothrow) BMessenger(messenger);
		if (copy) {
			// don't add if messenger is already watching
			for (int32 i = 0; BMessenger* old = (BMessenger*)fObservers.ItemAt(i); i++) {
				if (*old == messenger) {
					success = true;
					break;
				}
			}
			if (!success)
				success = fObservers.AddItem((void*)copy);
			if (!success)
				delete copy;
		}
		Unlock();
	}
	return success;
}

// StopWatching
bool
RecentFileManager::StopWatching(const BMessenger& messenger)
{
	bool success = false;
	if (Lock()) {
		// look through the messengers and remove it if found
		for (int32 i = 0; BMessenger* old = (BMessenger*)fObservers.ItemAt(i); i++) {
			if (*old == messenger) {
				if ((success = fObservers.RemoveItem(i))) {
					delete old;
				}
				break;
			}
		}
		Unlock();
	}
	return success;
}

