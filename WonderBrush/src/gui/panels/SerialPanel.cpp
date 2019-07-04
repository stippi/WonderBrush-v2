// SerialPanel.cpp

#include <stdio.h>

#include <Beep.h>
#include <Screen.h>
#include <String.h>

#include <HGroup.h>
#include <MBorder.h>
#include <MButton.h>
#include <MStringView.h>
#include <MTextControl.h>
#include <Space.h>
#include <VGroup.h>

#include "support.h"

#include "GlobalFunctions.h"
#include "LanguageManager.h"

#include "SerialPanel.h"

enum {
	MSG_PANEL_OK		= 'okok',
	MSG_PANEL_CANCEL	= 'cncl',
	MSG_TEXT_MODIFIED	= 'txmd',
};

// constructor
SerialPanel::SerialPanel()
	: Panel(BRect(-1000, -1000, -900, -900), "Serial Number",
			B_MODAL_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
			B_ASYNCHRONOUS_CONTROLS | B_NOT_V_RESIZABLE),
	  fExitSem(create_sem(0, "serial number panel")),
	  fResult(false)
{
	LanguageManager* m = LanguageManager::Default();

	MButton* defaultButton = new MButton(m->GetString(OK, "Ok"), new BMessage(MSG_PANEL_OK), this);

	BString allowedChars = kValidSerialNumberChars;
	BString allowedCharsLower = kValidSerialNumberChars;
	allowedCharsLower.ToLower();
	allowedChars << allowedCharsLower;

	for (int32 i = 0; i < TEXT_FIELD_COUNT; i++) {
		fSerialTC[i] = new MTextControl(NULL, NULL);

		BMessage* message = new BMessage(MSG_TEXT_MODIFIED);
		message->AddInt32("control", i);
		fSerialTC[i]->SetModificationMessage(message);

		BTextView* textView = fSerialTC[i]->TextView();
		for (uint32 j = 0; j < 256; j++)
			textView->DisallowChar(j);
		const char* c = allowedChars.String();
		while (*c) {
			textView->AllowChar((uint32)*c);
			c++;
		}
	}

	fStringView = new MStringView(m->GetString(ENTER_SERIAL_NUMBER, "Please enter the Serial Number which "
											  "came with your copy of WonderBrush."));

	BView* topView = new VGroup
	(
		new MBorder
		(
			M_RAISED_BORDER, 10, "",
			new VGroup
			(
				fStringView,
				vertical_space(),
				new HGroup
				(
					fSerialTC[0],
					fSerialTC[1],
					fSerialTC[2],
					fSerialTC[3],
					fSerialTC[4],
					0
				),
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 5, "",
			new HGroup
			(
				new Space(minimax(-1.0, -1.0, 10000.0, 10000.0, 5.0)),
				new MButton(m->GetString(CANCEL, "Cancel"), new BMessage(MSG_PANEL_CANCEL), this),
				new Space(minimax(10.0, 0.0, 10.0, 0.0, 1.0)),
				defaultButton,
				0
			)
		),
		0
	);

	AddChild(topView);
	SetDefaultButton(defaultButton);
	fSerialTC[0]->MakeFocus(true);

	// enlarge the text control's font
	BFont font(*be_fixed_font);
	font.SetSize(font.Size() * 1.5);
	for (int32 i = 0; i < TEXT_FIELD_COUNT; i++) {
		BTextView* textView = fSerialTC[i]->TextView();
		textView->SetFontAndColor(&font);

// Grrr
//		fSerialTC[i]->SetDivider(0);
	}

	// trigger window size calculation
	Hide();
	Show();

	// center window on screen
	if (Lock()) {
		BRect frame = _CalculateFrame(Frame());
		make_sure_frame_is_on_screen(frame, this);
		MoveTo(frame.LeftTop());
		Unlock();
	}
}

// destructor
SerialPanel::~SerialPanel()
{
}

// MessageReceived
void SerialPanel::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_PANEL_CANCEL:
			Cancel();
			break;
		case MSG_PANEL_OK:
			fResult = true;
			release_sem(fExitSem);
			break;
		case MSG_TEXT_MODIFIED: {
			// handle cycling to the next control
			int32 control;
			if (message->FindInt32("control", &control) >= B_OK) {
				const char* text = fSerialTC[control]->Text();
				if (text) {
					int32 length = strlen(text);
					if (length > 0) {
						BString upper = text;
						upper.ToUpper();
						if (upper != BString(text)) {
							fSerialTC[control]->SetText(upper.String());
							fSerialTC[control]->TextView()->Select(length,
																   length);
						}
						if (length == MAX_SECTION_LENGTH) {
							if (control < TEXT_FIELD_COUNT - 1)
								fSerialTC[control + 1]->MakeFocus(true);
						} else if (length > MAX_SECTION_LENGTH) {
							char oldText[MAX_SECTION_LENGTH + 1];
							memcpy(oldText, text, MAX_SECTION_LENGTH);
							oldText[MAX_SECTION_LENGTH] = 0;
							fSerialTC[control]->SetText(oldText);
							fSerialTC[control]->TextView()->Select(MAX_SECTION_LENGTH,
																   MAX_SECTION_LENGTH);
							beep();
						}
					}
				}
			}
			break;
		}
		default:
			Panel::MessageReceived(message);
	}
}

// Cancel
void
SerialPanel::Cancel()
{
	fResult = false;
	release_sem(fExitSem);
}

// Go
bool
SerialPanel::Go(BString* serial)
{
	if (Lock()) {
		Show();
		Unlock();
	}

	acquire_sem(fExitSem);

	bool result = fResult;

	if (serial) {
		serial->SetTo("");
		if (result) {
			for (int32 i = 0; i < TEXT_FIELD_COUNT; i++)
				*serial << fSerialTC[i]->Text();
			serial->ToUpper();
		}
	}

	Lock();
	Quit();

	return result;
}

// _CalculateFrame
BRect
SerialPanel::_CalculateFrame(BRect frame)
{
	BScreen screen(this);
	BRect screenFrame = screen.Frame();
	if (!screenFrame.Contains(frame)) {
		float width = frame.Width();
		float height = frame.Height();
		BPoint center;
		center.x = screenFrame.left + screenFrame.Width() / 2.0;
		center.y = screenFrame.top + screenFrame.Height() / 4.0;
		frame.left = center.x - width / 2.0;
		frame.right = frame.left + width;
		frame.top = center.y - height / 2.0;
		frame.bottom = frame.top + height;
	}
	return frame;
}

