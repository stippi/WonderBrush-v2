// LanguageManager.cpp

#include <parsedate.h>
#include <stdio.h>
#include <stdlib.h>

#include <Application.h>
#include <Directory.h>
#include <Entry.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <Roster.h>

#include "addon.h"

#include "LanguageManager.h"

#define ENGLISH -1

LanguageManager* LanguageManager::fDefaultManager = NULL;

// constructor
LanguageManager::LanguageManager()
	: BHandler("language manager"),
	  fLanguageFolder(NULL),
	  fLanguage(ENGLISH),
	  fLanguages(10)
{
	memset(fStrings, 0, sizeof(fStrings));

	app_info info;
	status_t status;
	if ((status = be_app->GetAppInfo(&info)) >= B_OK) {
		BEntry entry(&info.ref);

		BPath path(&entry);
		if ((status = path.InitCheck()) >= B_OK
			&& (status = path.GetParent(&path)) >= B_OK
			&& (status = path.Append("Languages")) >= B_OK) {
			_LoadLanguages(path.Path());
			if (entry.SetTo(path.Path()) >= B_OK) {
				fLanguageFolder = new entry_ref;
				if (entry.GetRef(fLanguageFolder) < B_OK) {
					delete fLanguageFolder;
					fLanguageFolder = NULL;
				}
			}
		} else
			fprintf(stderr, "Creating path to \"Languages\" folder failed: %s\n", strerror(status));
	} else
		fprintf(stderr, "GetAppInfo() failed: %s\n", strerror(status));
}

// destructor
LanguageManager::~LanguageManager()
{
	_MakeEmpty();
	clean_temporary_files();
	delete fLanguageFolder;
}

// MessageReceived
void
LanguageManager::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_NODE_MONITOR:
			// TODO: be a little smarter about what actually
			// needs to be done depending on "be:op_code"
			if (fLanguageFolder) {
				BPath path(fLanguageFolder);
				if (path.InitCheck() >= B_OK) {
					// rebuild list of languages
					_LoadLanguages(path.Path());
					// rebuild strings
					SetLanguage(fLanguage + 1);
					// inform looper
					if (BLooper* looper = Looper())
						looper->PostMessage(MSG_LANGUAGES_CHANGED);
				}
			}
			break;
		default:
			BHandler::MessageReceived(message);
	}
}

// Default
LanguageManager*
LanguageManager::Default()
{
	if (!fDefaultManager)
		fDefaultManager = new LanguageManager();
	return fDefaultManager;
}

// AttachedToLooper
void
LanguageManager::AttachedToLooper(BLooper* looper)
{
	if (fLanguageFolder) {
		BEntry entry(fLanguageFolder);
		node_ref nref;
		if (entry.GetNodeRef(&nref) >= B_OK)
			watch_node(&nref, B_WATCH_DIRECTORY, this, looper);
	}
}

// SetLanguage
bool
LanguageManager::SetLanguage(int32 cookie)
{
	// first valid language is -1 (build-in English), but cookie starts at 0
	cookie--;

	bool success = true;
	if (cookie >= fLanguages.CountItems()) {
		// out of range
		cookie = ENGLISH;
		success = false;
	}
	if (cookie > ENGLISH) {
		if (language_item* item = (language_item*)fLanguages.ItemAt(cookie)) {
			fLanguage = cookie;
			// compile addon if not already done!
			if (!item->addon) {
				item->addon = build_addon(item->path);
				if (item->addon && fLanguageFolder) {
					BLooper* looper = Looper();
					// stop node watching
					if (looper)
						stop_watching(this, looper);
					// move this items addon to Language folder
					BPath path(fLanguageFolder);
					sprintf(item->addon->name, "%s.%s.addon", item->name, item->international);
					status_t err = save_addon(item->addon, path.Path());
					if (err < B_OK)
						fprintf(stderr, "failed to save addon: %s\n", strerror(err));
					// reenable node watching
					if (looper) {
						BEntry entry(fLanguageFolder);
						node_ref nref;
						if (entry.GetNodeRef(&nref) >= B_OK)
							watch_node(&nref, B_WATCH_DIRECTORY, this, looper);
					}
				}
			}
			if (!item->addon) {
				// failed to compile
				fprintf(stderr, "failed to build addon for \"%s\"\n", item->name);
				fLanguages.RemoveItem((void*)item);
				delete item;
				fLanguage = ENGLISH;
				success = false;
			} else {
				// init this languages strings
				memset(fStrings, 0, sizeof(fStrings));
				item->addon->function(fStrings);
			}
		}
	} else
		fLanguage = cookie;
	return success;
}

// SetLanguage
bool
LanguageManager::SetLanguage(const char* name)
{
	return SetLanguage(FindLanguage(name));
}

// LanguageName
const char*
LanguageManager::LanguageName() const
{
	const char* name = "English";
	if (fLanguage > ENGLISH) {
		language_item* language = (language_item*)fLanguages.ItemAt(fLanguage);
		if (language)
			name = language->name;
		else
			name = "<unknown>";
	}
	return name;
}

// Language
int32
LanguageManager::Language() const
{
	return fLanguage + 1;
}

// FindLanguage
int32
LanguageManager::FindLanguage(const char* language) const
{
	int32 index = ENGLISH;
	if (language) {
		for (int32 i = 0; language_item* item = (language_item*)fLanguages.ItemAt(i); i++) {
			if (item->name && (strcmp(language, item->name) == 0
							   || strcmp(language, item->international) == 0)) {
				index = i;
				break;
			}
		}
	}
	// account for ENLISH at -1
	return index + 1;
}

// GetString
const char*
LanguageManager::GetString(uint32 key, const char* defaultString) const
{
	const char* string = defaultString;
	if (fLanguage > ENGLISH) {
		language_item* language = (language_item*)fLanguages.ItemAt(fLanguage);
		if (key < STRING_COUNT) {
			if (language) {
				if (fStrings[key])
					string = fStrings[key];
				else
					fprintf(stderr, "no %s string for \"%s\"\n", language->name, defaultString);
			} else
				fprintf(stderr, "no language at index %ld\n!", fLanguage);
		} else
			fprintf(stderr, "index (%ld) out of range! (max = %ld)\n!", key, (int32)STRING_COUNT - 1);
	}
	return string;
}

// GetNextLanguage
bool
LanguageManager::GetNextLanguage(BString& string, int32 index) const
{
	// the first valid index is -1, which means
	// the build-in English language, loaded languages
	// start at 0
	index--;
	int32 count = fLanguages.CountItems();
	if (index < count) {
		if (index == ENGLISH)
			string = "English";
		else {
			language_item* language = (language_item*)fLanguages.ItemAt(index);
			if (language)
				string = language->name;
			else
				string = "<unkown language>";
		}
	}
	return index < count;
}

// PosixSystemLanguage
int32
LanguageManager::PosixSystemLanguage() const
{
	char* l = getenv("LC_MESSAGES");
	if (l) {
		BString language(l);
		int32 cutOffPos = language.FindFirst('_');
		language.Remove(cutOffPos, language.Length() - cutOffPos);
//		free(l); // irritating posix globals...
		return FindLanguage(language.String());
	}
	return ENGLISH;
}

// DumpMissingStrings
void
LanguageManager::DumpMissingStrings() const
{
	if (fLanguage > ENGLISH) {
		printf("\nmissing strings for \"%s\"...\n", LanguageName());
		bool noMissing = true;
//		language_item* language = (language_item*)fLanguages.ItemAt(fLanguage);
//		printf("missing strings for language '%s':\n\n", ??);

		const char* defines[STRING_COUNT];
		memset(defines, 0, sizeof(defines));
		init_defines(defines);

		for (int32 key = 0; key < STRING_COUNT; key++) {
			if (!fStrings[key]) {
				noMissing = false;
				if (defines[key])
					printf("%s\n", defines[key]);
				else
					printf("index: %ld\n", key);
			}
		}
		if (noMissing)
			printf("no strings missing.\n");
	}
}

// _LoadLanguages
void
LanguageManager::_LoadLanguages(const char* path)
{
bigtime_t now = system_time();
	BDirectory dir(path);
	if (dir.InitCheck() >= B_OK) {
		// remember current language
		BString previousLanguage("English");
		bool foundPreviousLanguage = false;
		if (language_item* language = (language_item*)fLanguages.ItemAt(fLanguage)) {
			previousLanguage = language->name;
		} else
			foundPreviousLanguage = true;
		// whipe us clean
		_MakeEmpty();
		// populate the list with what we find
		entry_ref ref;
		while (dir.GetNextRef(&ref) >= B_OK) {
			BEntry entry(&ref, true);
			if (!entry.IsDirectory()) {
				language_item* item = new language_item;
				// see if the file is an addon
				BString origName(ref.name);
				BString name = origName;
				name.RemoveLast(".addon");
				if (name != origName) {
					// found a file which is already an addon
					_InitNames(name, item);
					// copy path
					sprintf(item->path, "%s/%s", path, ref.name);
					item->addon = load_addon(item->path);
					if (item->addon) {
						// see if we already have a language with that name
						// if yes, see if this file is newer
						// remove the text file in that case
						time_t addonTime;
						BEntry addonEntry(&ref);
						addonEntry.GetModificationTime(&addonTime);
						char compileTimeString[256];
						sprintf(compileTimeString, "%s, %s", __DATE__, __TIME__);
						time_t compileTime = parsedate(compileTimeString, real_time_clock());
						if (addonTime < compileTime) {
							fprintf(stderr, "%s addon invalid, because it was compiled by old WonderBrush version\n", name.String());
							// this addon has been compiled earlier then we,
							// so it cannot work correctly (and needs to be recompiled)
							delete item;
							item = NULL;
						} else {
							for (int32 i = 0; language_item* l = (language_item*)fLanguages.ItemAt(i); i++) {
								if (item->name && l->name && strcmp(item->name, l->name) == 0) {
									BEntry textEntry(l->path);
									time_t textTime;
									textEntry.GetModificationTime(&textTime);
									if (addonTime >= textTime) {
										fLanguages.RemoveItem((void*)l);
										destroy_addon(l->addon);
										delete l;
									} else {
										delete item;
										item = NULL;
									}
								}
							}
						}
					} else {
						// error loading addon, delete item
						delete item;
						item = NULL;
					}
				} else {
					// found a file which we need to compile
					_InitNames(name, item);
					// remember path
					sprintf(item->path, "%s/%s", path, ref.name);
					item->addon = NULL;
					// see if we already have a language with that name
					// if yes, see if this file is newer
					// remove the addon in that case
					BEntry textEntry(&ref);
					time_t textTime;
					textEntry.GetModificationTime(&textTime);
					for (int32 i = 0; language_item* l = (language_item*)fLanguages.ItemAt(i); i++) {
						if (item->name && l->name && strcmp(item->name, l->name) == 0) {
							BEntry addonEntry(l->path);
							time_t addonTime;
							addonEntry.GetModificationTime(&addonTime);
							if (addonTime < textTime) {
								fLanguages.RemoveItem((void*)l);
								destroy_addon(l->addon);
								delete l;
							} else {
								delete item;
								item = NULL;
							}
						}
					}
				}
				// add the item to our list
				if (item) {
					if (fLanguages.AddItem((void*)item)) {
						// see if it is the previous language
						if (strcmp(item->name, previousLanguage.String()) == 0) {
							int32 index = fLanguages.IndexOf((void*)item);
							fLanguage = index;
							foundPreviousLanguage = true;
						}
					} else {
						destroy_addon(item->addon);
						delete item;
					}	
				}
			}
		}
		if (!foundPreviousLanguage) {
			fprintf(stderr, "previous language (%s) not found!\n", previousLanguage.String());
			fLanguage = ENGLISH;
		}
	}
printf("looking through language files: %lld µsecs\n", system_time() - now);
}

// _MakeEmpty
void
LanguageManager::_MakeEmpty()
{
	int32 count = fLanguages.CountItems();
	for (int32 i = count - 1; language_item* language = (language_item*)fLanguages.ItemAt(i); i--) {
		destroy_addon(language->addon);
		delete language;
	}
	fLanguages.MakeEmpty();
}

// _InitNames
void
LanguageManager::_InitNames(const BString& name, language_item* item) const
{
	// cut off international name
	int32 dotPos = name.FindLast('.');
	int32 length = name.Length();
	if (dotPos < 0)
		dotPos = length;
	// copy name
	strncpy(item->name, name.String(), dotPos);
	item->name[dotPos] = 0;
	// copy international name
	if (dotPos < length) {
		int32 remaining = length - (dotPos + 1);
		if (remaining > 2)
			remaining = 2;
		strncpy(item->international, name.String() + dotPos + 1, remaining);
		item->international[remaining] = 0;
	} else
		item->international[0] = 0;
}


