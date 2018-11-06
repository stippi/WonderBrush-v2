// LicenceeInfo.h

#ifndef LICENCEE_INFO_H
#define LICENCEE_INFO_H

#include <Archivable.h>
#include <String.h>

// license versions
enum {
	LICENSE_STUDENT					= 0,
	LICENSE_COMMERCIAL_SINGLE		= 1,
	LICENSE_COMMERCIAL_MULTI		= 2,
};

class LicenceeInfo : public BArchivable {
 public:
								LicenceeInfo();
								LicenceeInfo(BMessage* archive);
	virtual						~LicenceeInfo();

			void				SetName(const char* name);
			const char*			GetName() const;
			void				SetApplicationID(const char* id);
			const char*			GetApplicationID() const;
			void				SetApplicationVersion(uint32 version);
			uint32				GetApplicationVersion() const;
			void				SetRegistrationTime(bigtime_t time);
			bigtime_t			GetRegistrationTime() const;
			void				SetExpirationTime(bigtime_t time);
			bigtime_t			GetExpirationTime() const;
			void				SetLicenseType(int32 type);
			int32				GetLicenseType() const;
			// ...

			void				PrintToStream() const;

	virtual	status_t			Archive(BMessage* archive,
										bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

 private:
			BString				fName;
			BString				fApplicationID;
			uint32				fApplicationVersion;
			bigtime_t			fRegistrationTime;
			bigtime_t			fExpirationTime;
			int32				fLicenseType;
};

#endif	// LICENCEE_INFO_H
