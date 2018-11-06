// LanguageManager.h

#ifndef LANGUAGE_MANAGER_H
#define LANGUAGE_MANAGER_H

#define USE_HASH_MAP 0

#if USE_HASH_MAP
#include <hash_map.h>
#include <string.h>
#endif // USE_HASH_MAP

#include <Handler.h>
#include <List.h>
#include <OS.h>
#include <String.h>

#include "Strings.h"

enum {
	MSG_SET_LANGUAGE			= 'stln',
	MSG_LANGUAGES_CHANGED		= 'lngc',
};

struct language_addon;
struct entry_ref;

struct language_item {
	char				name[B_OS_NAME_LENGTH];
	char				international[3];
	char				path[B_PATH_NAME_LENGTH];
	language_addon*		addon;
};

class LanguageManager : public BHandler {
 public:
								LanguageManager();
	virtual						~LanguageManager();

								// BHandler
	virtual	void				MessageReceived(BMessage* message);

								// LanguageManager
	static	LanguageManager*	Default();

			void				AttachedToLooper(BLooper* looper);

			bool				SetLanguage(int32 index);
			int32				Language() const;
			int32				FindLanguage(const char* language) const;

			bool				SetLanguage(const char* name);
			const char*			LanguageName() const;

			const char*			GetString(uint32 key,
										  const char* defaultString) const;

			bool				GetNextLanguage(BString& string,
												int32 index) const;
			int32				PosixSystemLanguage() const;

			void				DumpMissingStrings() const;

 private:
			void				_LoadLanguages(const char* path);
			void				_MakeEmpty();
			void				_InitNames(const BString& name,
										   language_item* item) const;

	entry_ref*					fLanguageFolder;
	int32						fLanguage;
	BList						fLanguages;
#if USE_HASH_MAP
	typedef hash_map<string, const char*> string_map;

	string_map					fStrings;
#else // USE_HASH_MAP
	const char*					fStrings[STRING_COUNT];
#endif // USE_HASH_MAP
	static	LanguageManager*	fDefaultManager;
};

# endif // LANGUAGE_MANAGER_H
