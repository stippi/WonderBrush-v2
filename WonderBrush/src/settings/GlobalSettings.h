// GlobalSettings.h

#ifndef GLOBAL_SETTINGS_H
#define GLOBAL_SETTINGS_H

#include "SettingsFile.h"

enum {
	STARTUP_DO_NOTHING	= 0,
	STARTUP_ASK_OPEN	= 1,
	STARTUP_ASK_NEW		= 2,
	STARTUP_AUTO_SIZE	= 3,	// creates a new canvas with specified size
};

enum {
	PROP_LOCK_NEW		= 0,
	PROP_LOCK_RESIZE	= 1,
	PROP_LOCK_START		= 2,
};

class GlobalSettings : public SettingsFile {
 public:
	static	GlobalSettings*		CreateDefault();
	static	void				DeleteDefault();
	static	GlobalSettings&		Default();

								// query settings
			void				SetStartupAction(uint32 action);
			uint32				StartupAction() const;

			void				SetAutoSize(uint32 width, uint32 height);
			void				GetAutoSize(uint32* width, uint32* height) const;

			void				SetLiveUpdates(bool update);
			bool				LiveUpdates() const;

			void				SetUseSystemLanguage(bool use);
			bool				UseSystemLanguage() const;

			void				SetLanguage(const char* language);
			const char*			Language() const;

			void				SetProportionsLocked(uint32 whichPanel,
													 bool locked);
			bool				ProportionsLocked(uint32 whichPanel) const;

			void				SetSerialNumber(const char* serialNumber);
			const char*			SerialNumber() const;

 private:
								GlobalSettings();
	virtual						~GlobalSettings();

	static	GlobalSettings*		fDefaultSettings;
};

#endif // GLOBAL_SETTINGS_H
