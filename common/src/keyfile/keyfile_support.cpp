// keyfile.cpp

#include <algorithm>
#include <cstdio>

#include <File.h>
#include <Message.h>

#include "keyfile_support.h"
#include "LicenceeInfo.h"

static const char	kXORCharacter	= (unsigned char)0xff;
static const off_t	kMaxKeyFileSize	= 10240;

/*static
void
print_buffer(const void* buffer, size_t size)
{
	const unsigned char* b = (const unsigned char*)buffer;
	for (int i = 0; i < (int32)size; i++, b++)
		printf("%02x", *b);
	printf("\n");
}*/

// read_keyfile
LicenceeInfo*
read_keyfile(const char* filename, const rsa_key& key)
{
	status_t error = B_OK;
	if (!filename)
		error = B_ERROR;
	// read the file
	size_t encryptedSize = 0;
	char* encryptedInfo = NULL;
	if (error == B_OK) {
		BFile file(filename, B_READ_ONLY);
		off_t fileSize = 0;
		error = file.GetSize(&fileSize);
		if (error == B_OK) {
			if (fileSize > 0 && fileSize <= kMaxKeyFileSize)
				encryptedSize = (size_t)fileSize;
			else
				error = B_ERROR;
		}
		if (error == B_OK) {
			encryptedInfo = new char[encryptedSize];
			if (file.Read(encryptedInfo, encryptedSize)
				!= (ssize_t)encryptedSize) {
				error = B_ERROR;
			}
		}
	}
	// decrypt the info
	LicenceeInfo* info = NULL;
	if (error == B_OK) {
		BArchivable* object = rsa_decrypt_archivable(encryptedInfo,
													 encryptedSize, key);
		info = dynamic_cast<LicenceeInfo*>(object);
		if (!info)
			delete object;
	}
	delete[] encryptedInfo;
	if (!info)
		info = new LicenceeInfo();
	return info;
}

// write_keyfile
status_t
write_keyfile(const char* filename, LicenceeInfo* info, const rsa_key& key)
{
	status_t error = B_OK;
	if (!filename || !info)
		error = B_ERROR;
	// encrypt the info
	char* encryptedInfo = NULL;
	size_t encryptedSize = 0;
	if (error == B_OK) {
		encryptedInfo = rsa_encrypt_archivable(info, key, encryptedSize);
		if (!encryptedInfo)
			error = B_ERROR;
	}
	// write the encrypted data
	if (error == B_OK) {
		BFile file(filename, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
		error = file.InitCheck();
		if (error == B_OK) {
			error = file.Write(encryptedInfo, encryptedSize);
			if (error == (ssize_t)encryptedSize)
				error = B_OK;
		}
	}
	delete[] encryptedInfo;
	return error;
}

