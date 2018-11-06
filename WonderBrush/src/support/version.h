// version.h

#ifndef VERSION_H
#define VERSION_H

// The build defines the following number macros:
//
// MAJOR_APP_VERSION
// MIDDLE_APP_VERSION
// MINOR_APP_VERSION
// APP_VERSION_SUFFIX

#define _PLAIN_STRING(x)	#x
#define _STRING(x)			_PLAIN_STRING(x)

// The app version string. E.g. "2.0.0 WIP".
#define APP_VERSION_STRING			\
	_STRING(MAJOR_APP_VERSION) "."	\
	_STRING(MIDDLE_APP_VERSION) "."	\
	_STRING(MINOR_APP_VERSION) 		\
	APP_VERSION_SUFFIX

// Identical to APP_VERSION_STRING.
extern const char* kAppVersionString;

#endif	// VERSION_H
