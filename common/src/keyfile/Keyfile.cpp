// Keyfile.cpp

#include <stdio.h>

#include <OS.h>
#include <Path.h>

#include "LicenceeInfo.h"

#include "keyfile_support.h"

#include "Keyfile.h"

// constructor
Keyfile::Keyfile(const char* path, const unsigned char* publicKey,
				 size_t keySize, const char* appName,
				 const BMessenger* notifyTarget)
	: fKeyfileLocker("keyfile"),
	  fLicenceeInfo(NULL),
	  fPublicKey(publicKey),
	  fPublicKeySize(keySize),
	  fAppName(appName),
	  fNotifyTarget(notifyTarget ? new BMessenger(*notifyTarget) : NULL)
{
	if (get_ref_for_path(path, &fRef) >= B_OK) {
		// start thread to read keyfile
		thread_id keyfileValidator = spawn_thread(_validate_keyfile, "keyfile validator",
												  B_LOW_PRIORITY, this);
		if (keyfileValidator >= B_OK)
			resume_thread(keyfileValidator);
	}
}

// destructor
Keyfile::~Keyfile()
{
	delete fLicenceeInfo;
	delete fNotifyTarget;
}

// GetLicenceeInfo
const LicenceeInfo*
Keyfile::GetLicenceeInfo() const
{
	const LicenceeInfo* info = NULL;
	if (fKeyfileLocker.Lock()) {
		info = fLicenceeInfo;
		fKeyfileLocker.Unlock();
	}
	return info;
}

// _validate_keyfile
int32
Keyfile::_validate_keyfile(void* cookie)
{
	Keyfile* app = (Keyfile*)cookie;
	return app->_ValidateKeyfile();
}

// _ValidateKeyfile
int32
Keyfile::_ValidateKeyfile()
{
	// figure out if we have a valid keyfile
	if (fKeyfileLocker.Lock()) {
		rsa_key key;
		BPath path(&fRef);
		if (path.InitCheck() >= B_OK
			&& rsa_unflatten_key(fPublicKey, fPublicKeySize, key) >= B_OK) {
			fLicenceeInfo = read_keyfile(path.Path(), key);
			if (strcmp(fLicenceeInfo->GetApplicationID(), fAppName.String()) != 0
				|| fLicenceeInfo->GetExpirationTime() < real_time_clock_usecs()) {
				delete fLicenceeInfo;
				fLicenceeInfo = NULL;
			}
		}
		fKeyfileLocker.Unlock();
	}
	if (fNotifyTarget)
		fNotifyTarget->SendMessage(MSG_KEYFILE_VALIDATED);
	return B_OK;
}

