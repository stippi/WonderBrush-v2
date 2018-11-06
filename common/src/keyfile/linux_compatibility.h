// linux_compatibility.h
#ifndef LINUX_COMPATIBILITY_H
#define LINUX_COMPATIBILITY_H

#ifdef TARGET_PLATFORM_LINUX

#include <stdint.h>

typedef int8_t		int8;
typedef int16_t		int16;
typedef int32_t		int32;
typedef int64_t		int64;
typedef uint8_t		uint8;
typedef uint16_t	uint16;
typedef uint32_t	uint32;
typedef uint64_t	uint64;

typedef long status_t;
#define	B_OK	0
#define	B_ERROR	-1

#endif	// TARGET_PLATFORM_LINUX

#endif	// LINUX_COMPATIBILITY_H

