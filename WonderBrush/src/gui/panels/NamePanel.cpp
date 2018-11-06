// NamePanel.cpp

#include <stdio.h>

#include <Screen.h>

#include <HGroup.h>
#include <MBorder.h>
#include <MButton.h>
#include <MTextControl.h>
#include <Space.h>
#include <VGroup.h>

#include "LanguageManager.h"

#include "NamePanel.h"

enum {
	MSG_PANEL_OK		= 'okok',
	MSG_PANEL_CANCEL	= 'cncl',
};

// constructor
NamePanel::NamePanel(const char* label,
					 const char* text,
					 BWindow *window,
					 BHandler* target,
					 BMessage* message,
					 BRect frame)
	: Panel(frame, "Name Panel",
			B_MODAL_WINDOW_LOOK, B_MODAL_SUBSET_WINDOW_FEEL,
			B_ASYNCHRONOUS_CONTROLS | B_NOT_V_RESIZABLE),
	  fWindow(window),
	  fTarget(target),
	  fMessage(message)
{
	LanguageManager* manager = LanguageManager::Default();
	MButton* defaultButton = new MButton(manager->GetString(OK, "Ok"), new BMessage(MSG_PANEL_OK), this);
	BView* topView = new VGroup
	(
		new MBorder
		(
			M_RAISED_BORDER, 10, "",
			new VGroup
			(
				new Space(),
				fNameTC = new MTextControl(NULL, NULL),
				new Space(),
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 5, "",
			new HGroup
			(
				new Space(minimax(-1.0, -1.0, 10000.0, 10000.0, 5.0)),
				new MButton(manager->GetString(CANCEL, "Cancel"), new BMessage(MSG_PANEL_CANCEL), this),
				new Space(minimax(10.0, 0.0, 10.0, 0.0, 1.0)),
				defaultButton,
				0
			)
		),
		0
	);

	fNameTC->SetLabel(label);
	fNameTC->SetText(text);
	
	AddChild(topView);
	SetDefaultButton(defaultButton);
	fNameTC->MakeFocus(true);

	if (fWindow && fWindow->Lock()) {
		fSavedTargetWindowFeel = fWindow->Feel();
		if (fSavedTargetWindowFeel != B_NORMAL_WINDOW_FEEL)
			fWindow->SetFeel(B_NORMAL_WINDOW_FEEL);
		fWindow->Unlock();
	}	

	AddToSubset(fWindow);
	Hide();
	Show();
	if (Lock()) {
		frame = _CalculateFrame(Frame());
		MoveTo(frame.LeftTop());
//		ResizeTo(frame.Width(), frame.Height());
		Show();
		Unlock();
	}
}

// destructor
NamePanel::~NamePanel()
{
	if (fWindow && fWindow->Lock()) {
		fWindow->SetFeel(fSavedTargetWindowFeel);
		fWindow->Unlock();
	}	
	delete fMessage;
}

// MessageReceived
void NamePanel::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_PANEL_CANCEL:
			Quit();
			break;
		case MSG_PANEL_OK: {
			if (!fTarget)
				fTarget = fWindow;
			BLooper* looper = fTarget ? fTarget->Looper() : NULL;
			if (fMessage && looper) {
				BMessage cloneMessage(*fMessage);
				cloneMessage.AddString("name", fNameTC->Text());
				looper->PostMessage(&cloneMessage, fTarget);
			}
			Quit();
			break;
		}
		default:
			Panel::MessageReceived(message);
	}
}

// _CalculateFrame
BRect
NamePanel::_CalculateFrame(BRect frame)
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
