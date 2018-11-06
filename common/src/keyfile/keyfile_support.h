// keyfile_support.h

#ifndef KEY_FILE_SUPPORT_H
#define KEY_FILE_SUPPORT_H

#include "rsa.h"

class LicenceeInfo;

LicenceeInfo*	read_keyfile(const char* filename, const rsa_key& key);
status_t		write_keyfile(const char* filename, LicenceeInfo* info,
							  const rsa_key& key);

#endif	// KEY_FILE_SUPPORT_H
