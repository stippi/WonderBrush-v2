// GlobalSettings.cpp

#include <stdio.h>
#include <string.h>

#include "GlobalSettings.h"

static const char* kGlobalSettingsPath		= "WonderBrush/gobal_settings";
// message data member keys
static const char* kStartupActionKey		= "startup action";
static const char* kAutoWidthKey			= "auto width";
static const char* kAutoHeightKey			= "auto height";
static const char* kLiveUpdatesKey			= "live updates";
static const char* kUseSystemLanguageKey	= "use system language";
static const char* kLanguageKey				= "language";

static const char* kPropLockNewKey			= "prop lock new";
static const char* kPropLockResizeKey		= "prop lock resize";
static const char* kPropLockStartKey		= "prop lock start";
static const char* kSerialNumberKey			= "serial number";

// static variable initialization
GlobalSettings*
GlobalSettings::fDefaultSettings = NULL;


// constructor
GlobalSettings::GlobalSettings()
	: SettingsFile(kGlobalSettingsPath, 'gbst')
{
}

// destructor
GlobalSettings::~GlobalSettings()
{
}

// CreateDefault
GlobalSettings*
GlobalSettings::CreateDefault()
{
	if (!fDefaultSettings)
		fDefaultSettings = new GlobalSettings;
	return fDefaultSettings;
}

// DeleteDefault
void
GlobalSettings::DeleteDefault()
{
	if (fDefaultSettings) {
		delete fDefaultSettings;
		fDefaultSettings = NULL;
	}
}

// Default
GlobalSettings&
GlobalSettings::Default()
{
	return *fDefaultSettings;
}


// SetStartupAction
void
GlobalSettings::SetStartupAction(uint32 action)
{
	SetValue(kStartupActionKey, action);
}

// StartupAction
uint32
GlobalSettings::StartupAction() const
{
	uint32 action;
	if (GetValue(kStartupActionKey, &action) < B_OK)
		action = STARTUP_AUTO_SIZE;
	return action;
}

// SetAutoSize
void
GlobalSettings::SetAutoSize(uint32 width, uint32 height)
{
	SetValue(kAutoWidthKey, width);
	SetValue(kAutoHeightKey, height);
}

// GetAutoSize
void
GlobalSettings::GetAutoSize(uint32* width, uint32* height) const
{
	if (GetValue(kAutoWidthKey, width) < B_OK)
		*width = 800;
	if (GetValue(kAutoHeightKey, height) < B_OK)
		*height = 600;
}

// SetLiveUpdates
void
GlobalSettings::SetLiveUpdates(bool update)
{
	SetValue(kLiveUpdatesKey, update);
}

// LiveUpdates
bool
GlobalSettings::LiveUpdates() const
{
	bool update;
	if (GetValue(kLiveUpdatesKey, &update) < B_OK)
		update = true;
	return update;
}

// SetUseSystemLanguage
void
GlobalSettings::SetUseSystemLanguage(bool use)
{
	SetValue(kUseSystemLanguageKey, use);
}

// UseSystemLanguage
bool
GlobalSettings::UseSystemLanguage() const
{
	bool use;
	if (GetValue(kUseSystemLanguageKey, &use) < B_OK)
#ifdef TARGET_PLATFORM_ZETA
		use = true;
#else
		use = false;
#endif
	return use;
}

// SetLanguage
void
GlobalSettings::SetLanguage(const char* language)
{
	SetValue(kLanguageKey, language);
}

// Language
const char*
GlobalSettings::Language() const
{
	const char* language;
	if (GetValue(kLanguageKey, &language) < B_OK)
		language = "English";
	return language;
}

// SetProportionsLocked
void
GlobalSettings::SetProportionsLocked(uint32 whichPanel, bool locked)
{
	switch (whichPanel) {
		case PROP_LOCK_NEW:
			SetValue(kPropLockNewKey, locked);
			break;
		case PROP_LOCK_RESIZE:
			SetValue(kPropLockResizeKey, locked);
			break;
		case PROP_LOCK_START:
			SetValue(kPropLockStartKey, locked);
			break;
	}
}

// ProportionsLocked
bool
GlobalSettings::ProportionsLocked(uint32 whichPanel) const
{
	bool locked = true;
	switch (whichPanel) {
		case PROP_LOCK_NEW:
			if (GetValue(kPropLockNewKey, &locked) < B_OK)
				locked = true;
			break;
		case PROP_LOCK_RESIZE:
			if (GetValue(kPropLockResizeKey, &locked) < B_OK)
				locked = true;
			break;
		case PROP_LOCK_START:
			if (GetValue(kPropLockStartKey, &locked) < B_OK)
				locked = true;
			break;
	}
	return locked;
}

// SetSerialNumber
void
GlobalSettings::SetSerialNumber(const char* serialNumber)
{
	SetValue(kSerialNumberKey, serialNumber);
}

// SerialNumber
const char*
GlobalSettings::SerialNumber() const
{
	const char* serialNumber;
	if (GetValue(kSerialNumberKey, &serialNumber) < B_OK)
		serialNumber = NULL;
	return serialNumber;
}
