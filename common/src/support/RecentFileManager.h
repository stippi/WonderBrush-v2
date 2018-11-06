// RecentFileManager.h

#ifndef RECENT_FILE_MANAGER_H
#define RECENT_FILE_MANAGER_H

#include <List.h>
#include <Locker.h>
#include <Messenger.h>
#include <StorageDefs.h>

enum {
	MSG_RECENT_FILES_CHANGED	= 'rcfc',
};

struct entry_ref;

class RecentFileManager : public BLocker {
 public:
								RecentFileManager(const char* settingsFilePath,
												  uint32 maxEntries = 10);
 	virtual						~RecentFileManager();

			status_t			InitCheck() const;

	static	RecentFileManager*	CreateDefault(const char* settingsFilePath,
											  uint32 maxEntries = 10);
	static	void				DeleteDefault();
	static	RecentFileManager*	Default();

			// if the maximum capacity of recent files is reached,
			// files will be removed from the beginning of the list
			bool				AddFile(const entry_ref* ref);
			// you must lock the object before looping this function!
			// start with cookie = 0
			const entry_ref*	GetNextFile(int32* cookie) const;

			// whenever the list of recent files of this object changes,
			// a message MSG_RECENT_FILES_CHANGED will be sent to the
			// supplied BMessenger (which contains all the entry_refs
			// in a field called "refs")
			bool				StartWatching(const BMessenger& messenger);
			bool				StopWatching(const BMessenger& messenger);

 private:
			BMessage*			fSettings;
			char				fSettingsFilePath[B_PATH_NAME_LENGTH];

			BList				fEntries;
			uint32				fMaxEntries;

			BList				fObservers;

	static	RecentFileManager*	fDefaultManager;
};

#endif	// RECENT_FILE_MANAGER_H
