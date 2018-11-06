// Keyfile.h
//
// this class spawns a thread in its constructor to validate the given keyfile

#ifndef KEYFILE_H
#define KEYFILE_H

#include <Entry.h>
#include <Locker.h>
#include <Messenger.h>
#include <String.h>

class LicenceeInfo;

enum {
	MSG_KEYFILE_VALIDATED = 'kfvd',
};

class Keyfile {
 public:
								Keyfile(const char* path,
										const unsigned char* publicKey,
										size_t keySize,
										const char* appName,
										const BMessenger* notifyTarget = NULL);
	virtual						~Keyfile();

	const	LicenceeInfo*		GetLicenceeInfo() const;

 private:
	static	int32				_validate_keyfile(void *cookie);
			int32				_ValidateKeyfile();

	mutable BLocker				fKeyfileLocker;
	LicenceeInfo*				fLicenceeInfo;
	entry_ref					fRef;
	const unsigned char*		fPublicKey;
	size_t						fPublicKeySize;
	BString						fAppName;

	BMessenger*					fNotifyTarget;
};

#endif // KEYFILE_H
