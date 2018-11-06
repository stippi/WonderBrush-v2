// SettingsFile.cpp

#include <malloc.h>
#include <new>
#include <stdio.h>
#include <string.h>

#include <File.h>
#include <FindDirectory.h>
#include <Path.h>

#include "SettingsFile.h"

// constructor
SettingsFile::SettingsFile(const char* settingsFilePath, uint32 id)
	: Settings(settingsFilePath, id),
	  fSettingsPath(strdup(settingsFilePath))
{
	// automatically restore ourselfs from a settings file
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) >= B_OK
		&& path.Append(fSettingsPath) >= B_OK) {
		BFile file(path.Path(), B_READ_ONLY);
		if (file.InitCheck() == B_OK)
			Unflatten(&file);
	}
}

// destructor
SettingsFile::~SettingsFile()
{
	// automatically save to settings file
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) >= B_OK
		&& path.Append(fSettingsPath) >= B_OK) {
		BFile file(path.Path(), B_WRITE_ONLY | B_ERASE_FILE | B_CREATE_FILE);
		status_t err = file.InitCheck();
		if (err == B_OK)
			err = Flatten(&file);
		if (err != B_OK)
			fprintf(stderr, "ERROR: could not save settings in file '%s'! (%s)\n",
					path.Path(),
					strerror(err));
	}
	free(fSettingsPath);
}

