// Settings.h

#ifndef SETTINGS_H
#define SETTINGS_H

#include <Handler.h>
#include <Message.h>

enum {
	SETTINGS_CHANGED	= 0x1,
};

class Settings : public BMessage, public BHandler {
 public:
								Settings(const char* name,
										 uint32 id);
	virtual						~Settings();

			status_t			SetValue(const char* key, int32 value);
			status_t			GetValue(const char* key, int32* value) const;

			status_t			SetValue(const char* key, uint32 value);
			status_t			GetValue(const char* key, uint32* value) const;

			status_t			SetValue(const char* key, const char* value);
			status_t			GetValue(const char* key, const char** value) const;

			status_t			SetValue(const char* key, float value);
			status_t			GetValue(const char* key, float* value) const;

			status_t			SetValue(const char* key, bool value);
			status_t			GetValue(const char* key, bool* value) const;

			status_t			SetValue(const char* key, const BMessage& value);
			status_t			GetValue(const char* key, BMessage* value) const;

			bool				Lock();
			void				Unlock();

 private:
			uint32				fLooperLocked;
};

#endif // SETTINGS_H
