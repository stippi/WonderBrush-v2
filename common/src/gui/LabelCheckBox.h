// LabelCheckBox.h

#ifndef LABEL_CHECK_BOX_H
#define LABEL_CHECK_BOX_H

#include <layout.h>
#include <be/interface/CheckBox.h>

class BMessage;
class BHandler;

class LabelCheckBox : public MView, public BCheckBox
{
public:
								LabelCheckBox(const char *label, BMessage *msg,
											  BHandler *target, bool active);
	virtual						~LabelCheckBox();

	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect rect);
};

#endif // LABEL_CHECK_BOX_H
