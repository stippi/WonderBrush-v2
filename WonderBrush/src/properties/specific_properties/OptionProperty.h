// OptionProperty.h

#ifndef OPTION_PROPERTY_H
#define OPTION_PROPERTY_H

#include <List.h>
#include <String.h>

#include "Property.h"

class OptionProperty : public Property {
 public:
								OptionProperty(const char* name,
											   int32 id);
								OptionProperty(BMessage* archive);
	virtual						~OptionProperty();

								// BArchivable
	virtual	status_t			Archive(BMessage* archive, bool deep = true) const;
	static	BArchivable*		Instantiate(BMessage* archive);

								// Property
	virtual	PropertyItemValueView*		Editor();

								// OptionProperty
			void				AddOption(int32 id, const char* name);

			int32				CurrentOptionID() const;
			void				SetCurrentOptionID(int32 id);

			bool				GetOption(int32 index, BString* string, int32* id) const;
			bool				GetCurrentOption(BString* string) const;

			bool				SetOptionAtOffset(int32 indexOffset);

 private:

	struct option {
		int32		id;
		BString		name;
	};

			BList				fOptions;
			int32				fCurrentOptionID;
};


#endif // OPTION_PROPERTY_H


