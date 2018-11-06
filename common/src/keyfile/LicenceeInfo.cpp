// LicenceeInfo.cpp

#include <cstdio>

#include <Message.h>

#include "TimeSupport.h"

#include "LicenceeInfo.h"

// constants for default values
const char* const	kDefaultName				= "no name";
const char* const	kDefaultApplicationID		= "no application";
const uint32		kDefaultApplicationVersion	= 0;
const bigtime_t		kDefaultRegistrationTime	= 0;
const bigtime_t		kDefaultExpirationTime		= 0;
const int32			kDefaultLicenseType			= 0;

// constructor
LicenceeInfo::LicenceeInfo()
	: fName(kDefaultName),
	  fApplicationID(kDefaultApplicationID),
	  fApplicationVersion(kDefaultApplicationVersion),
	  fRegistrationTime(kDefaultRegistrationTime),
	  fExpirationTime(kDefaultExpirationTime),
	  fLicenseType(kDefaultLicenseType)
{
}

// unarchiving constructor
LicenceeInfo::LicenceeInfo(BMessage* archive)
	: fName(kDefaultName),
	  fApplicationID(kDefaultApplicationID),
	  fApplicationVersion(kDefaultApplicationVersion),
	  fRegistrationTime(kDefaultRegistrationTime),
	  fExpirationTime(kDefaultExpirationTime),
	  fLicenseType(kDefaultLicenseType)
{
	archive->FindString("MyClass::Name", &fName);
	archive->FindString("MyClass::ApplicationID", &fApplicationID);
	archive->FindInt32("MyClass::ApplicationVersion",
					   (int32*)&fApplicationVersion);
	archive->FindInt64("MyClass::RegistrationTime", &fRegistrationTime);
	archive->FindInt64("MyClass::ExpirationTime", &fExpirationTime);
	archive->FindInt32("MyClass::LicenseType", &fLicenseType);
}

// destructor
LicenceeInfo::~LicenceeInfo()
{
}

// SetName
void
LicenceeInfo::SetName(const char* name)
{
	fName = name;
}

// GetName
const char*
LicenceeInfo::GetName() const
{
	return fName.String();
}

// SetApplicationID
void
LicenceeInfo::SetApplicationID(const char* id)
{
	fApplicationID = id;
}

// GetApplicationID
const char*
LicenceeInfo::GetApplicationID() const
{
	return fApplicationID.String();
}

// SetApplicationVersion
void
LicenceeInfo::SetApplicationVersion(uint32 version)
{
	fApplicationVersion = version;
}

// GetApplicationVersion
uint32
LicenceeInfo::GetApplicationVersion() const
{
	return fApplicationVersion;
}

// SetRegistrationTime
void
LicenceeInfo::SetRegistrationTime(bigtime_t time)
{
	fRegistrationTime = time;
}

// GetRegistrationTime
bigtime_t
LicenceeInfo::GetRegistrationTime() const
{
	return fRegistrationTime;
}

// SetExpirationTime
void
LicenceeInfo::SetExpirationTime(bigtime_t time)
{
	fExpirationTime = time;
}

// GetExpirationTime
bigtime_t
LicenceeInfo::GetExpirationTime() const
{
	return fExpirationTime;
}

// SetLicenseType
void
LicenceeInfo::SetLicenseType(int32 type)
{
	fLicenseType = type;
}

// GetLicenseType
int32
LicenceeInfo::GetLicenseType() const
{
	return fLicenseType;
}

// PrintToStream
void
LicenceeInfo::PrintToStream() const
{
	printf("LicenceeInfo:\n");
	printf("  name               : `%s'\n", GetName());
	printf("  application ID     : `%s'\n", GetApplicationID());
	printf("  application version: %lu\n", GetApplicationVersion());
//	printf("  registration time  : %Ld\n", GetRegistrationTime());
//	printf("  expiration time    : %Ld\n", GetExpirationTime());
	printf("  registration time  : ");
	print_time(GetRegistrationTime());
	printf("\n");
	printf("  expiration time    : ");
	print_time(GetExpirationTime());
	printf("\n");
	const char* licenseString = "unknown";
	int32 licenseType = GetLicenseType();
	switch (licenseType) {
		case LICENSE_STUDENT:
			licenseString = "noncommercial, student";
			break;
		case LICENSE_COMMERCIAL_SINGLE:
			licenseString = "commercial, single user";
			break;
		case LICENSE_COMMERCIAL_MULTI:
			licenseString = "commercial, multiple users";
			break;
		default:
			break;
	}
	printf("  license type       : %ld (%s)\n", licenseType, licenseString);
}

// Archive
status_t
LicenceeInfo::Archive(BMessage* archive, bool deep) const
{
	status_t error = B_OK;
	error = BArchivable::Archive(archive, deep);
	if (error == B_OK)
		error = archive->AddString("MyClass::Name", fName);
	if (error == B_OK)
		error = archive->AddString("MyClass::ApplicationID", fApplicationID);
	if (error == B_OK) {
		error = archive->AddInt32("MyClass::ApplicationVersion",
								  fApplicationVersion);
	}
	if (error == B_OK) {
		error = archive->AddInt64("MyClass::RegistrationTime",
								  fRegistrationTime);
	}
	if (error == B_OK)
		error = archive->AddInt64("MyClass::ExpirationTime", fExpirationTime);
	if (error == B_OK)
		error = archive->AddInt32("MyClass::LicenseType", fLicenseType);
	return error;
}

// Instantiate
_EXPORT
BArchivable*
LicenceeInfo::Instantiate(BMessage* archive)
{
	if (!validate_instantiation(archive, "LicenceeInfo"))
		return NULL;
	return new LicenceeInfo(archive);
}

