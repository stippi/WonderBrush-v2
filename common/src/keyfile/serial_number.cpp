// serial_number.cpp

#include <ctype.h>
#include <string.h>

#include "bigint.h"
#include "rsa.h"
#include "serial_number.h"

// the characters the user is allowed to enter
const char* kValidSerialNumberChars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

// the characters actually used int the serial number string -- the letters
// "I" and "O" are filtered out to avoid confusion with the digits "1" and "0".
static const char* kUsedSerialNumberChars
	= "0123456789ABCDEFGHJKLMNPQRSTUVWXYZ";

// length of a serial number string
static const int kSerialNumberLength = 25;
	// 25 digits means the max representable number is max = 34^25 - 1, for
	// which holds 2^127 < max < 2^128; hence we need a 127 bit serial number
	// key

// Format of the (unencrypted) serial number:
//
// bits  n ... 48   47 ... 40   39 ... 32   31 ... 0
//       0 ... 0      major      middle      serial
//
// major, middle: the major and middle part of the application version the
//                serial number was created for
// serial:        an arbitrary 32 bit number (ideally a number written into
//                executable on CD, we can check against)

static bool
convert_serial_number_string_to_number(const char* serialNumber, bigint& number)
{
	number = 0;

	int base = strlen(kUsedSerialNumberChars);

	int len = strlen(serialNumber);
	for (int i = 0; i < len; i++) {
		// check if valid at all
		char c = toupper(serialNumber[i]);
		if (!strchr(kValidSerialNumberChars, c))
			return false;

		// replace the letters "O" and "I" by the digits "0" and "1"
		if (c == 'O')
			c = '0';
		if (c == 'I')
			c = '1';

		int digit = strchr(kUsedSerialNumberChars, c) - kUsedSerialNumberChars;
		number *= base;
		number += digit;
	}

	return true;
}

// convert_serial_number_to_string
static void
convert_serial_number_to_string(const bigint& _number,
	std::string& serialString)
{
	bigint number(_number);
	int base = strlen(kUsedSerialNumberChars);
	bigint bigBase(base);

	char buffer[kSerialNumberLength + 1];
	for (int i = 0; i < kSerialNumberLength; i++) {
		bigint remainder;
		number.divide(bigBase, remainder);
		char c = kUsedSerialNumberChars[remainder.toi()];
		buffer[kSerialNumberLength - i - 1] = c;
	}

	buffer[kSerialNumberLength] = '\0';
	serialString = buffer;
}

// check_serial_number
bool
check_serial_number(const char* serialNumber, int32 currentMajorVersion,
	int32 currentMiddleVersion, int32 firstAcceptedMajorVersion,
	int32 firstAcceptedMiddleVersion, const rsa_key& key)
{
	// convert the serial number string into a number
	bigint number;
	if (!convert_serial_number_string_to_number(serialNumber, number))
		return false;

	// the serial number must not be greater than the key
	if (number > key.n)
		return false;

	// decrypt the serial number
	number.mod_pow(key.k, key.n);

	// decompose the serial number

	// check, whether the upper part is 0
	if ((number >> 48) != 0)
		return false;

	uint64 shortNumber = number.toull();
	int32 major = (shortNumber >> 40) & 0xff;
	int32 middle = (shortNumber >> 32) & 0xff;
	uint32 serial = shortNumber & 0xffffffffUL;

	// check the version
	// to small?
	if (major < firstAcceptedMajorVersion)
		return false;
	if (major == firstAcceptedMajorVersion
		&& middle < firstAcceptedMiddleVersion) {
		return false;
	}

	// too big?
	if (major > currentMajorVersion)
		return false;
	if (major == currentMajorVersion && middle > currentMiddleVersion)
		return false;

	// check serial
	(void)serial;

	return true;
}

// generate_serial_number
void
generate_serial_number(int32 major, int32 middle, uint32 serial,
	const rsa_key& key, std::string& serialNumber)
{
	// compose the serial number
	uint64 shortNumber
		= ((uint64)major << 40) | ((uint64)middle << 32) | serial;
	bigint number(shortNumber);

	// encrypt the number
	number.mod_pow(key.k, key.n);

	// convert to serial number string
	convert_serial_number_to_string(number, serialNumber);
}

