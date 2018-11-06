// rsa.cpp

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#include <Archivable.h>
#include <ByteOrder.h>
#include <File.h>
#include <Message.h>
#include <OS.h>

#include "rsa.h"
#include "BigIntGenerator.h"
#include "BitQueue.h"

static const size_t	kHeaderSize		= sizeof(size_t);
static const off_t	kMaxKeyFileSize	= 10240;
static const char	kXORCharacter	= (unsigned char)0xff;

rsa_key dbgKey;

// bit_filter
static inline
unsigned long
bit_filter(int count)
{
	return (~0UL >> (sizeof(unsigned long) * 8 - count));
}

static inline
void
print_queue(const BitQueue& queue)
{
	printf("queue (%d): ", queue.CountBits());
	queue.PrintToStream();
	printf("\n");
}

// mod_pow
unsigned
mod_pow(unsigned b, unsigned e, unsigned m)
{
	unsigned long long result = 1;
	unsigned long long bx = b;
	while (e) {
		if (e & 1) {
			result = result * bx;
			result %= m;
		}
		bx *= bx;
		bx %= m;
		e >>= 1;
	}
	return (unsigned)result;
}

// rsa_generate_key
void
rsa_generate_key(rsa_key& publicKey, rsa_key& privateKey, int bits)
{
	// two prime numbers p, q
	bigint p = BigIntGenerator::GeneratePrime(bits / 2);
	bigint q = BigIntGenerator::GeneratePrime(bits - bits / 2);
	bigint n = p * q;
	bigint pn = (p - 1) * (q - 1);				// Euler's totient function
	bigint ln =  pn / gcd((p - 1), (q - 1));	// Carmichael's function
	// a number d relatively prime to ln
	int cookie;
	bigint d = BigIntGenerator::GeneratePrimeCandidate(ln.ld() + 1, cookie);
	while (gcd(d, ln) != 1)
		BigIntGenerator::GetNextPrimeCandidate(d, cookie);
	d %= ln;
	// a number e so that (d * e) % ln = 1:
	// since d and ln are rel. prime, we can abuse the gcd:
	// 1 = gcd(d, ln) = u * d + v * ln
	bigint e, m;
	gcd(d, ln, e, m);
	// e must be > 0
	if (e < 0)
		e = e % ln + ln;
//bigint g = gcd(d, ln, e, m);
//cout << "gcd: " << g << endl;
//cout << "e * d + m * ln: " << (e * d + m * ln) << endl;
//cout << "(e * d) % ln: " << ((e * d) % ln) << endl;
//cout << "(e * d) % ln + ln: " << ((e * d) % ln + ln) << endl;
	// set the return values
	publicKey.n = n;
	publicKey.k = d;
	privateKey.n = n;
	privateKey.k = e;
}

// rsa_flattened_key_size
size_t
rsa_flattened_key_size(const rsa_key& key)
{
	size_t size = 0;
	unsigned long nLen = key.n.ld() + 1;
	unsigned long kLen = key.k.ld() + 1;
	size = 2 * sizeof(unsigned long) + (nLen + kLen + 7) / 8;
	return size;
}

// rsa_flatten_key
void
rsa_flatten_key(const rsa_key& key, void* buffer)
{
	BitQueue queue;
	unsigned long nLen = key.n.ld() + 1;
	unsigned long kLen = key.k.ld() + 1;
	queue.PushBits(nLen, sizeof(unsigned long) * 8);
	queue.PushBits(kLen, sizeof(unsigned long) * 8);
	queue.PushBits(key.n, nLen);
	queue.PushBits(key.k, kLen);
	queue.PopBytes(buffer, (queue.CountBits() + 7) / 8);
}

// rsa_unflatten_key
status_t
rsa_unflatten_key(const void* buffer, size_t length, rsa_key& key)
{
	status_t error = B_ERROR;
	if (buffer && length > 0) {
		error = B_OK;
		BitQueue queue;
		queue.PushBytes(buffer, length);
		unsigned long nLen = queue.PopBits(sizeof(unsigned long) * 8);
		unsigned long kLen = queue.PopBits(sizeof(unsigned long) * 8);
//printf("rsa_unflatten_key(): n: %lu, k: %lu\n", nLen, kLen);
		queue.PopBits(key.n, nLen);
//cout << "key.n: " << key.n << endl;
		queue.PopBits(key.k, kLen);
//cout << "key.k: " << key.k << endl;
	}
	return error;
}

// rsa_read_key
status_t
rsa_read_key(const char* filename, rsa_key& key)
{
	status_t error = B_OK;
	if (!filename)
		error = B_ERROR;

	// open the file
	int fd = -1;
	if (error == B_OK) {
		fd = open(filename, O_RDONLY);
		if (fd < 0)
			error = errno;
	}

	// read the file
	size_t flattenedSize = 0;
	char* flattenedKey = NULL;
	if (error == B_OK) {
		off_t fileSize = 0;
		struct stat st;
		if (fstat(fd, &st) == 0)
			fileSize = st.st_size;
		else
			error = errno;
			
		if (error == B_OK) {
			if (fileSize > 0 && fileSize <= kMaxKeyFileSize)
				flattenedSize = (size_t)fileSize;
			else
				error = B_ERROR;
		}

		if (error == B_OK) {
			flattenedKey = new char[flattenedSize];
			if (read(fd, flattenedKey, flattenedSize) != (long)flattenedSize)
				error = errno;
		}
	}

	// unflatten the key
	if (error == B_OK)
		error = rsa_unflatten_key(flattenedKey, flattenedSize, key);
	delete[] flattenedKey;

	if (fd >= 0)
		close(fd);

	return error;
}

// rsa_write_key
status_t
rsa_write_key(const char* filename, const rsa_key& key)
{
	status_t error = B_OK;
	if (!filename)
		error = B_ERROR;

	// flatten the key
	size_t flattenedSize = 0;
	char* flattenedKey = NULL;
	if (error == B_OK) {
		flattenedSize = rsa_flattened_key_size(key);
		flattenedKey = new char[flattenedSize];
		rsa_flatten_key(key, flattenedKey);
	}

	// create the file
	int fd = -1;
	if (error == B_OK) {
		fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if (fd < 0)
			error = errno;
	}

	// write the flattened data
	if (error == B_OK) {
		if (write(fd, flattenedKey, flattenedSize) != (long)flattenedSize)
			error = errno;
	}

	if (fd >= 0)
		close(fd);

	delete[] flattenedKey;

	return error;
}

// write_line
static
status_t
write_line(int fd, const char* line)
{
	status_t error = B_OK;
	size_t toWrite = strlen(line);
	ssize_t written = write(fd, line, toWrite);
	if (written < 0)
		error = errno;
	else if (written != (ssize_t)toWrite)
		error = B_ERROR;
	return error;
}

// rsa_write_key_to_c
status_t
rsa_write_key_to_c(const char* filename, const rsa_key& key)
{
	status_t error = B_OK;
	if (!filename)
		error = B_ERROR;
	// flatten the key
	size_t flattenedSize = 0;
	char* flattenedKey = NULL;
	if (error == B_OK) {
		flattenedSize = rsa_flattened_key_size(key);
		flattenedKey = new char[flattenedSize];
		rsa_flatten_key(key, flattenedKey);
	}

	// create the file
	int fd = -1;
	if (error == B_OK) {
		fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC,
			S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if (fd < 0)
			error = errno;
	}

	// write the flattened data as c code
	if (error == B_OK) {
		char* line = new char[256];

		// write variable declaration
		error = write_line(fd, "const unsigned char* key = {\n");
		const unsigned char* buffer = (const unsigned char*)flattenedKey;
		size_t bytesRemaining = flattenedSize;
		while (error == B_OK && bytesRemaining > 0) {
			line[0] = '\0';
			// compose a line (10 bytes at maximum)
			size_t toWrite = min((unsigned long)bytesRemaining, 10UL);
			char* dest = line;
			sprintf(dest, "\t");
			dest += strlen(dest);
			for (int i = 0; i < (int)toWrite; i++) {
				if (i > 0) {
					sprintf(dest, ", ");
					dest += strlen(dest);
				}
				sprintf(dest, "0x%02x", *buffer);
				dest += strlen(dest);
				buffer++;
			}
			bytesRemaining -= toWrite;
			if (bytesRemaining > 0)
				sprintf(dest, ",\n");
			else
				sprintf(dest, "\n");
			error = write_line(fd, line);
		}
		if (error == B_OK)
			error = write_line(fd, "};\n");
		delete line;
	}
	
	if (fd >= 0)
		close(fd);

	delete[] flattenedKey;

	return error;
}


// rsa_bytes_needed_for_encryption
size_t
rsa_bytes_needed_for_encryption(size_t bytesToEncrypt, const rsa_key& key)
{
	// additional space for a header containing the actual number of bytes
	bytesToEncrypt += kHeaderSize;
	int inBlockSize = key.n.ld();
	int outBlockSize = inBlockSize + 1;
	int numBlocks = (bytesToEncrypt * 8 + inBlockSize - 1) / inBlockSize;
	return (numBlocks * outBlockSize + 7) / 8;
}

// rsa_bytes_needed_for_decryption
//
// Returns the maximal number of bytes needed for the decryption output.
// rsa_decrypt() may return a smaller number.
size_t
rsa_bytes_needed_for_decryption(size_t bytesToDecrypt, const rsa_key& key)
{
	int outBlockSize = key.n.ld();
	int inBlockSize = outBlockSize + 1;
	int numBlocks = (bytesToDecrypt * 8) / inBlockSize;
	return (numBlocks * outBlockSize + 7) / 8 - kHeaderSize;
}

// rsa_encrypt
ssize_t
rsa_encrypt(const void* message, size_t bytesToEncrypt,
			void* buffer, size_t bufferSize, const rsa_key& key)
{
//printf("rsa_encrypt()\n");
	int inBlockSize = key.n.ld();
	int outBlockSize = inBlockSize + 1;
	// check the key -- we don't allow keys shorter than the header
	if (inBlockSize < (int)kHeaderSize * 8)
		return B_ERROR;
	// check the buffer size
	if (bufferSize < rsa_bytes_needed_for_encryption(bytesToEncrypt, key))
		return B_ERROR;

	const unsigned char* inBuffer = (const unsigned char*)message;
	unsigned char* outBuffer = (unsigned char*)buffer;
	int remainingBytes = bytesToEncrypt;
	int bytesWritten = 0;
	BitQueue inQueue, outQueue;
	inQueue.PushBits(bytesToEncrypt, kHeaderSize * 8);
//bool firstBlock = true;
	while (inQueue.CountBits() > 0 || remainingBytes > 0) {
		// get material for the next block
		if (inQueue.CountBits() < inBlockSize && remainingBytes > 0) {
			int bytesNeeded = (inBlockSize - inQueue.CountBits() + 7) / 8;
			bytesNeeded = min(bytesNeeded, remainingBytes);
			inQueue.PushBytes(inBuffer, bytesNeeded);
			inBuffer += bytesNeeded;
			remainingBytes -= bytesNeeded;
		}
		// encrypt the block
//printf("  encrypt block: %d -> %d\n", inBlockSize, outBlockSize);
		bigint text;
		inQueue.PopBits(text, inBlockSize);
//if (firstBlock) {
//cout << "first block: " << text << endl;
//}
//bigtime_t startTime = system_time();
		text.mod_pow(key.k, key.n);
//printf("encrypting block took %Ld\n", system_time() - startTime);
		outQueue.PushBits(text, outBlockSize);
//if (firstBlock) {
//cout << "encrypted block: " << text << endl;
//text.mod_pow(dbgKey->k, dbgKey->n);
//cout << "decrypted block: " << text << endl;
//print_queue(outQueue);
//firstBlock = false;
//}
		// write the complete bytes of the encrypted text
		int bytesToWrite = outQueue.CountBits() / 8;
		outQueue.PopBytes(outBuffer, bytesToWrite);
		outBuffer += bytesToWrite;
		bytesWritten += bytesToWrite;
//printf("  popped %d bytes: written: %d\n", bytesToWrite, bytesWritten);
	}
	// if an incomplete byte remains, pad it
	int bytesToWrite = (outQueue.CountBits() + 7) / 8;
	if (bytesToWrite > 0) {
		outQueue.PopBytes(outBuffer, bytesToWrite);
		outBuffer += bytesToWrite;
		bytesWritten += bytesToWrite;
	}
//printf("rsa_encrypt() done\n");
	return bytesWritten;
}

// rsa_decrypt
ssize_t
rsa_decrypt(const void* message, size_t bytesToDecrypt,
			void* buffer, size_t bufferSize, const rsa_key& key)
{
//printf("rsa_decrypt()\n");
	int outBlockSize = key.n.ld();
	int inBlockSize = outBlockSize + 1;
	// check the key -- we don't allow keys shorter than the header
	if (outBlockSize < (int)kHeaderSize * 8)
		return B_ERROR;
	// check the buffer size
	size_t bytesForDecryption = rsa_bytes_needed_for_decryption(bytesToDecrypt, key);
	if (bufferSize < bytesForDecryption)
		return B_ERROR;

	const unsigned char* inBuffer = (const unsigned char*)message;
	unsigned char* outBuffer = (unsigned char*)buffer;
	int remainingBytes = bytesToDecrypt;
	int bytesWritten = 0;
	size_t decryptedSize = 0;
	bool firstBlock = true;
	BitQueue inQueue, outQueue;
	// as long as complete blocks are left...
//int blockNum = 0;
	while (inQueue.CountBits() + remainingBytes * 8 >= inBlockSize) {
		// get material for the next block
		if (inQueue.CountBits() < inBlockSize && remainingBytes > 0) {
			int bytesNeeded = (inBlockSize - inQueue.CountBits() + 7) / 8;
			bytesNeeded = min(bytesNeeded, remainingBytes);
			inQueue.PushBytes(inBuffer, bytesNeeded);
			inBuffer += bytesNeeded;
			remainingBytes -= bytesNeeded;
		}
		// decrypt the block
		bigint text;
//printf("\n  block %d:\n", blockNum++);
//printf("  encrypt block: %d -> %d\n", inBlockSize, outBlockSize);
//if (firstBlock) {
//print_queue(inQueue);
//}
		inQueue.PopBits(text, inBlockSize);
//if (firstBlock) {
//cout << "first block: " << text << endl;
//}
//bigtime_t startTime = system_time();
		text.mod_pow(key.k, key.n);
//printf("decrypting block took %Ld\n", system_time() - startTime);
		outQueue.PushBits(text, outBlockSize);
//printf("  outQueue: ");
//print_queue(outQueue);
		// the first block contains a header
		if (firstBlock) {
//cout << "decrypted block: " << text << endl;
			firstBlock = false;
			decryptedSize = (size_t)outQueue.PopBits(kHeaderSize * 8);
			if (decryptedSize > bytesForDecryption)
				return B_ERROR;
		}
		// write the complete bytes of the encrypted text
		int bytesToWrite = outQueue.CountBits() / 8;
		outQueue.PopBytes(outBuffer, bytesToWrite);
//printf("  bytes written: ");
//for (int32 i = 0; i < bytesToWrite; i++)
//printf("[%02x]", outBuffer[i]);
//printf("\n");
		outBuffer += bytesToWrite;
		bytesWritten += bytesToWrite;
//printf("  popped %d bytes: written: %d\n", bytesToWrite, bytesWritten);
	}
	// if an incomplete byte remains, pad it
	int bytesToWrite = (outQueue.CountBits() + 7) / 8;
	if (bytesToWrite > 0) {
		outQueue.PopBytes(outBuffer, bytesToWrite);
		outBuffer += bytesToWrite;
		bytesWritten += bytesToWrite;
	}
//printf("rsa_decrypt() done\n");
	return min((ssize_t)bytesWritten, (ssize_t)decryptedSize);
}


// rsa_encrypt_archivable
char*
rsa_encrypt_archivable(const BArchivable* object, const rsa_key& key,
					   size_t& encryptedSize)
{
	status_t error = B_OK;
	if (!object)
		return NULL;
	// archive the object
	BMessage archive;
	if (error == B_OK)
		error = object->Archive(&archive);
	// flatten the archive
	char* flattenedObject = NULL;
	ssize_t flattenedSize = 0;
	if (error == B_OK) {
		flattenedSize = archive.FlattenedSize();
		if (flattenedSize < 0)
			error = flattenedSize;
		if (error == B_OK) {
			flattenedObject = new char[flattenedSize];
			error = archive.Flatten(flattenedObject, flattenedSize);
		}
	}
	// reorder the buffer and xor it with a special character
	if (error == B_OK) {
		for (int i = 0; i < flattenedSize / 2; i += 2)
			swap(flattenedObject[i], flattenedObject[flattenedSize - i - 1]);
		for (int i = 0; i < flattenedSize; i++)
			flattenedObject[i] ^= kXORCharacter;
	}
	// encrypt the buffer
	char* encryptedObject = NULL;
	encryptedSize = 0;
	if (error == B_OK) {
		encryptedSize = rsa_bytes_needed_for_encryption(flattenedSize, key);
		encryptedObject = new char[encryptedSize];
		encryptedSize = rsa_encrypt(flattenedObject, flattenedSize,
									encryptedObject, encryptedSize, key);
		if (encryptedSize < 0)
			error = encryptedSize;
	}
	delete[] flattenedObject;
	return encryptedObject;
}

// rsa_decrypt_archivable
BArchivable*
rsa_decrypt_archivable(const void* buffer, size_t size, const rsa_key& key)
{
	status_t error = B_OK;
	if (!buffer)
		return NULL;;
	// decrypt the buffer
	ssize_t decryptedSize = 0;
	char* decryptedObject = NULL;
	if (error == B_OK) {
		decryptedSize = rsa_bytes_needed_for_decryption(size, key);
		decryptedObject = new char[decryptedSize];
		decryptedSize = rsa_decrypt(buffer, size,
									decryptedObject, decryptedSize, key);
		if (decryptedSize < 0)
			error = decryptedSize;
	}
	// xor the decrypted buffer and reorder it
	if (error == B_OK) {
		for (int i = 0; i < decryptedSize; i++)
			decryptedObject[i] ^= kXORCharacter;
		for (int i = 0; i < decryptedSize / 2; i += 2)
			swap(decryptedObject[i], decryptedObject[decryptedSize - i - 1]);
	}
	// unflatten the archive
	BMessage archive;
	if (error == B_OK)
		error = archive.Unflatten(decryptedObject);
	delete[] decryptedObject;
	// unarchive the object
	BArchivable* object = NULL;
	if (error == B_OK)
		object = instantiate_object(&archive);
	return object;
}
