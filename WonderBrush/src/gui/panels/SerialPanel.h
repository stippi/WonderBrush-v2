// SerialPanel.h

#ifndef SERIAL_PANEL_H
#define SERIAL_PANEL_H

#include "Panel.h"

#define TEXT_FIELD_COUNT 5
#define MAX_SECTION_LENGTH 5

class BString;
class MStringView;
class MTextControl;

class SerialPanel : public Panel {
 public:
							SerialPanel();
	virtual					~SerialPanel();

	virtual void			MessageReceived(BMessage *message);

	// Panel
	virtual void			Cancel();

			bool			Go(BString* serialNumber);

 private:
			BRect			_CalculateFrame(BRect frame);

	MStringView*			fStringView;
	MTextControl*			fSerialTC[TEXT_FIELD_COUNT];

	sem_id					fExitSem;
	bool					fResult;

};

#endif // SERIAL_PANEL_H
