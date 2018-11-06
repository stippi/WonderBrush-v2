// SettingsFile.h

#ifndef SETTINGS_FILE_H
#define SETTINGS_FILE_H

#include "Settings.h"

class SettingsFile : public Settings {
 public:
								SettingsFile(const char* settingsFilePath,
											 uint32 id);
	virtual						~SettingsFile();

 private:
			char*				fSettingsPath;
};

#endif // SETTINGS_FILE_H
