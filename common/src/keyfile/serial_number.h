// serial_number.h

#ifndef SERIAL_NUMBER_H
#define SERIAL_NUMBER_H

#include <string>

#include <SupportDefs.h>

struct rsa_key;

extern const char* kValidSerialNumberChars;

bool check_serial_number(const char* serialNumber, int32 currentMajorVersion,
	int32 currentMiddleVersion, int32 firstAcceptedMajorVersion,
	int32 firstAcceptedMiddleVersion, const rsa_key& key);

void generate_serial_number(int32 major, int32 middle, uint32 serial,
	const rsa_key& key, std::string& serialNumber);

#endif	// SERIAL_NUMBER_H
