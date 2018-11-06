// IconProperty.cpp

#include <Message.h>

#include "IconValueView.h"

#include "IconProperty.h"

// constructor
IconProperty::IconProperty(const char* name, int32 id,
						   const uchar* icon,
						   uint32 width, uint32 height,
						   color_space format,
						   BMessage* message)
	: Property(name, id),
	  fMessage(message),
	  fIcon(icon),
	  fWidth(width),
	  fHeight(height),
	  fFormat(format)
{
}

// archive constructor
IconProperty::IconProperty(BMessage* archive)
	: Property(archive),
	  fMessage(new BMessage())
{
	if (archive->FindMessage("message", fMessage) < B_OK) {
		delete fMessage;
		fMessage = NULL;
	}
}

// destrucor
IconProperty::~IconProperty()
{
	delete fMessage;
}

// Archive
status_t
IconProperty::Archive(BMessage* into, bool deep) const
{
	status_t status = Property::Archive(into, deep);

	if (status >= B_OK && fMessage)
		status = into->AddMessage("message", fMessage);

	// finish off
	if (status >= B_OK)
		status = into->AddString("class", "IconProperty");

	return status;
}

// Instantiate
BArchivable*
IconProperty::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "IconProperty"))
		return new IconProperty(archive);
	return NULL;
}

// SetMessage
void
IconProperty::SetMessage(const BMessage* message)
{
	if (message && fMessage) {
		*fMessage = *message;
	}
}

// Editor
PropertyItemValueView*		
IconProperty::Editor()
{
	IconValueView* view = new IconValueView(this);
	view->SetIcon(fIcon, fWidth, fHeight, fFormat);
	return view;
}

