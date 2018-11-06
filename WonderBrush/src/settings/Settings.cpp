// Settings.cpp

#include <stdio.h>

#include "Settings.h"

// constructor
Settings::Settings(const char* name, uint32 id)
	: BMessage(id),
	  BHandler(name),
	  fLooperLocked(0)
{
}

// destructor
Settings::~Settings()
{
}

// SetValue (int32)
status_t
Settings::SetValue(const char* key, int32 value)
{
	status_t ret = ReplaceInt32(key, value);
	if (ret < B_OK)
		ret = AddInt32(key, value);
	return ret;
}

// GetValue (int32)
status_t
Settings::GetValue(const char* key, int32* value) const
{
	int32 foundValue;
	status_t ret = FindInt32(key, &foundValue);
	if (ret >= B_OK)
		*value = foundValue;
	return ret;
}

// SetValue (uint32)
status_t
Settings::SetValue(const char* key, uint32 value)
{
	status_t ret = ReplaceInt32(key, (int32)value);
	if (ret < B_OK)
		ret = AddInt32(key, (int32)value);
	return ret;
}

// GetValue (uint32)
status_t
Settings::GetValue(const char* key, uint32* value) const
{
	uint32 foundValue;
	status_t ret = FindInt32(key, (int32*)&foundValue);
	if (ret >= B_OK)
		*value = foundValue;
	return ret;
}

// SetValue (const char*)
status_t
Settings::SetValue(const char* key, const char* value)
{
	status_t ret = ReplaceString(key, value);
	if (ret < B_OK)
		ret = AddString(key, value);
	return ret;
}

// GetValue (const char*)
status_t
Settings::GetValue(const char* key, const char** value) const
{
	const char* foundValue;
	status_t ret = FindString(key, &foundValue);
	if (ret >= B_OK)
		*value = foundValue;
	return ret;
}

// SetValue (float)
status_t
Settings::SetValue(const char* key, float value)
{
	status_t ret = ReplaceFloat(key, value);
	if (ret < B_OK)
		ret = AddFloat(key, value);
	return ret;
}

// GetValue (float)
status_t
Settings::GetValue(const char* key, float* value) const
{
	float foundValue;
	status_t ret = FindFloat(key, &foundValue);
	if (ret >= B_OK)
		*value = foundValue;
	return ret;
}

// SetValue (bool)
status_t
Settings::SetValue(const char* key, bool value)
{
	status_t ret = B_BAD_VALUE;
	if (key) {
		ret = ReplaceBool(key, value);
		if (ret < B_OK)
			ret = AddBool(key, value);
	}
	return ret;
}

// GetValue (bool)
status_t
Settings::GetValue(const char* key, bool* value) const
{
	bool foundValue;
	status_t ret = FindBool(key, &foundValue);
	if (ret >= B_OK)
		*value = foundValue;
	return ret;
}

// SetValue (BMessage)
status_t
Settings::SetValue(const char* key, const BMessage& value)
{
	status_t ret = B_BAD_VALUE;
	if (key) {
		ret = ReplaceMessage(key, &value);
		if (ret < B_OK)
			ret = AddMessage(key, &value);
	}
	return ret;
}

// GetValue (BMessage)
status_t
Settings::GetValue(const char* key, BMessage* value) const
{
	BMessage foundValue;
	status_t ret = FindMessage(key, &foundValue);
	if (ret >= B_OK)
		*value = foundValue;
	return ret;
}

// Lock
bool
Settings::Lock()
{
	if (LockLooper()) {
		fLooperLocked++;
		return true;
	}
	return false;
}

// Unlock
void
Settings::Unlock()
{
	if (fLooperLocked > 0) {
		UnlockLooper();
		fLooperLocked--;
	}
}
