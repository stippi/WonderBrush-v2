// rsa.h

#ifndef RSA_H
#define RSA_H

#include "bigint.h"

class BArchivable;

#include <SupportDefs.h>

struct rsa_key {
	bigint	n;	// the number to `%' with 
	bigint	k;	// the number to `pow' with
};


void		rsa_generate_key(rsa_key& publicKey, rsa_key& privateKey,
							 int bits);

size_t 		rsa_flattened_key_size(const rsa_key& key);
void 		rsa_flatten_key(const rsa_key& key, void* buffer);
status_t	rsa_unflatten_key(const void* buffer, size_t length, rsa_key& key);

status_t	rsa_read_key(const char* filename, rsa_key& key);
status_t	rsa_write_key(const char* filename, const rsa_key& key);
status_t	rsa_write_key_to_c(const char* filename, const rsa_key& key);

size_t		rsa_bytes_needed_for_encryption(size_t bytesToEncrypt,
											const rsa_key& key);
size_t		rsa_bytes_needed_for_decryption(size_t bytesToDecrypt,
											const rsa_key& key);

ssize_t		rsa_encrypt(const void* message, size_t bytesToEncrypt,
						void* buffer, size_t bufferSize, const rsa_key& key);
ssize_t		rsa_decrypt(const void* message, size_t bytesToDecrypt,
						void* buffer, size_t bufferSize, const rsa_key& key);

char*		rsa_encrypt_archivable(const BArchivable* object,
								   const rsa_key& key, size_t& encryptedSize);
BArchivable* rsa_decrypt_archivable(const void* buffer, size_t size,
								   const rsa_key& key);

#endif	// RSA_H
