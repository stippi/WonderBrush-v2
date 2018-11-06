// PropertyObject.h

#include <ClassInfo.h>
#include <Message.h>

#include "BoolProperty.h"
#include "IntProperty.h"
#include "FloatProperty.h"
#include "Property.h"

#include "PropertyObject.h"

// constructor
PropertyObject::PropertyObject()
	: BArchivable(),
	  BList(20)
{
}

// archivable constructor
PropertyObject::PropertyObject(BMessage* archive)
	: BArchivable(archive),
	  BList(20)
{
	if (archive) {
		BMessage propertyArchive;
		if (archive->FindMessage("property", &propertyArchive) >= B_OK) {
			BArchivable* archivable = instantiate_object(&propertyArchive);
			if (Property* property = cast_as(archivable, Property))
				if (!AddProperty(property))
					delete property;
			else
				delete archivable;
		}
	}
}

// destructor
PropertyObject::~PropertyObject()
{
	int32 count = CountProperties();
	for (int32 i = 0; i < count; i++)
		delete PropertyAt(i);
}

// Archive
status_t
PropertyObject::Archive(BMessage* archive, bool deep) const
{
	status_t ret = BArchivable::Archive(archive, deep);
	if (ret >= B_OK) {
		for (int32 i = 0; Property* property = PropertyAt(i); i++) {
			BMessage propertyArchive;
			ret = property->Archive(&propertyArchive, deep);
			if (ret >= B_OK)
				ret = archive->AddMessage("property", &propertyArchive);
			if (ret < B_OK)
				break;
		}
	}
	return ret;
}

// AddProperty
bool
PropertyObject::AddProperty(Property* property)
{
	if (property)
		return BList::AddItem((void*)property);
	return false;
}

// RemoveProperty
Property*
PropertyObject::RemoveProperty(int32 index)
{
	return (Property*)BList::RemoveItem(index);
}

// RemoveProperty
bool
PropertyObject::RemoveProperty(Property* property)
{
	return BList::RemoveItem((void*)property);
}

// PropertyAt
Property*
PropertyObject::PropertyAt(int32 index) const
{
	return (Property*)BList::ItemAt(index);
}

// PropertyAtFast
Property*
PropertyObject::PropertyAtFast(int32 index) const
{
	return (Property*)BList::ItemAtFast(index);
}

// CountProperties
int32
PropertyObject::CountProperties() const
{
	return BList::CountItems();
}

// ContainsSameProperties
bool
PropertyObject::ContainsSameProperties(const PropertyObject& other) const
{
	bool equal = false;
	int32 count = CountProperties();
	if (count == other.CountProperties()) {
		equal = true;
		for (int32 i = 0; i < count; i++) {
			Property* ownProperty = PropertyAtFast(i);
			Property* otherProperty = other.PropertyAtFast(i);
			if (ownProperty->ID() != otherProperty->ID()) {
				equal = false;
				break;
			}
		}
	}
	return equal;
}

// FindProperty
Property*
PropertyObject::FindProperty(const char* name) const
{
	for (int32 i = 0; Property* property = PropertyAt(i); i++) {
		if (strcmp(property->Name(), name) == 0)
			return property;
	}
	return NULL;
}

// FindProperty
Property*
PropertyObject::FindProperty(int32 id) const
{
	for (int32 i = 0; Property* property = PropertyAt(i); i++) {
		if (property->ID() == id)
			return property;
	}
	return NULL;
}

// FindIntProperty
int32
PropertyObject::FindIntProperty(int32 id, int32 defaultValue) const
{
	IntProperty* property = dynamic_cast<IntProperty*>(FindProperty(id));
	if (property)
		return property->Value();
	return defaultValue;
}

// FindFloatProperty
float
PropertyObject::FindFloatProperty(int32 id, float defaultValue) const
{
	FloatProperty* property = dynamic_cast<FloatProperty*>(FindProperty(id));
	if (property)
		return property->Value();
	return defaultValue;
}

// FindBoolProperty
bool
PropertyObject::FindBoolProperty(int32 id, bool defaultValue) const
{
	BoolProperty* property = dynamic_cast<BoolProperty*>(FindProperty(id));
	if (property)
		return property->Value();
	return defaultValue;
}

// FindStringProperty
const char*
PropertyObject::FindStringProperty(int32 id, const char* defaultValue) const
{
	return defaultValue;
}


