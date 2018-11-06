// PropertyObject.h

#ifndef PROPERTY_OBJECT_H
#define PROPERTY_OBJECT_H

#include <Archivable.h>
#include <List.h>

class Property;

class PropertyObject : public BArchivable,
					   private BList {
 public:
								PropertyObject();
								PropertyObject(BMessage* archive);
	virtual						~PropertyObject();

								// BArchivable
	virtual	status_t			Archive(BMessage* archive, bool deep = true) const;

								// PropertyObject
			bool				AddProperty(Property* property);
			Property*			RemoveProperty(int32 index);
			bool				RemoveProperty(Property* property);

			Property*			PropertyAt(int32 index) const;
			Property*			PropertyAtFast(int32 index) const;

			int32				CountProperties() const;

			bool				ContainsSameProperties(const PropertyObject& other) const;

			Property*			FindProperty(const char* name) const;
			Property*			FindProperty(int32 id) const;

								// some convinience functions
			int32				FindIntProperty(int32 id,
												int32 defaultValue) const;
			float				FindFloatProperty(int32 id,
												  float defaultValue) const;
			bool				FindBoolProperty(int32 id,
												 bool defaultValue) const;
			const char*			FindStringProperty(int32 id,
												   const char* defaultValue) const;

};

#endif // PROPERTY_OBJECT_H


