// Property.h

#ifndef PROPERTY_H
#define PROPERTY_H

#include <Archivable.h>
#include <String.h>

class PropertyItemValueView;

class Property : public BArchivable {
 public:
								Property(const char* name,
										 int32 id);
								Property(BMessage* archive);
	virtual						~Property();

								// BArchivable
	virtual	status_t			Archive(BMessage* archive,
										bool deep = true) const;

								// Property
	virtual	PropertyItemValueView*		Editor() = 0;

	// name is the human readable GUI name,
	// since Properties are generated on the fly,
	// they can be translated at the time they are generated
			const char*			Name() const;

	// the global property id
	inline	int32				ID() const
									{ return fID; }

 private:
			BString				fName;
			int32				fID;
};

#endif // PROPERTY_H


