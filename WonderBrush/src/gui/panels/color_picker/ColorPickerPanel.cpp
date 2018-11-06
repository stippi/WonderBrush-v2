// ColorPickerPanel.cpp

#include <stdio.h>

#include <Application.h>

#include <MBorder.h>
#include <HGroup.h>
#include <Space.h>
#include <MButton.h>
#include <VGroup.h>

#include "support.h"

#include "ColorPickerView.h"
#include "LanguageManager.h"

#include "ColorPickerPanel.h"

enum {
	MSG_CANCEL					= 'cncl',
	MSG_DONE					= 'done',
};

// constructor
ColorPickerPanel::ColorPickerPanel(BRect frame, rgb_color color,
								   selected_color_mode mode,
								   BMessage* message, BWindow* target)
	: Panel(frame, "Pick Color",
			B_FLOATING_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL,
			B_ASYNCHRONOUS_CONTROLS |
			B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_NOT_CLOSABLE),
	  fMessage(message),
	  fTarget(target)
{
	LanguageManager* manager = LanguageManager::Default();
	SetTitle(manager->GetString(PICK_COLOR, "Pick Color"));

	MButton* defaultButton = new MButton(manager->GetString(OK, "Ok"), new BMessage(MSG_DONE), this);

	// interface layout
	BView* topView = new VGroup
	(
		fColorPickerView = new ColorPickerView("color picker", color, mode),
		new MBorder
		(
			M_RAISED_BORDER, 5, "buttons",
			new HGroup
			(
				new Space(minimax(0.0, 0.0, 10000.0, 10000.0, 5.0)),
				new MButton(manager->GetString(CANCEL, "Cancel"),
							new BMessage(MSG_CANCEL), this),
				new Space(minimax(5.0, 0.0, 10.0, 10000.0, 1.0)),
				defaultButton,
				new Space(minimax(2.0, 0.0, 2.0, 10000.0, 0.0)),
				0
			)
		),
		0
	);

	SetDefaultButton(defaultButton);

	if (fTarget)
		AddToSubset(fTarget);
	else
		SetFeel(B_FLOATING_APP_WINDOW_FEEL);

	AddChild(topView);
}

// destructor
ColorPickerPanel::~ColorPickerPanel()
{
	delete fMessage;
}

// Cancel
void
ColorPickerPanel::Cancel()
{
	PostMessage(MSG_CANCEL);
}

// MessageReceived
void
ColorPickerPanel::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_CANCEL:
		case MSG_DONE: {
			BMessage msg('PSTE');
			BLooper* looper = fTarget ? static_cast<BLooper*>(fTarget)
									  : static_cast<BLooper*>(be_app);
			if (fMessage)
				msg = *fMessage;
			if (message->what == MSG_DONE)
				store_color_in_message(&msg, fColorPickerView->Color());
			msg.AddRect("panel frame", Frame());
			msg.AddInt32("panel mode", fColorPickerView->Mode());
			msg.AddBool("begin", true);
			looper->PostMessage(&msg);
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		default:
			Panel::MessageReceived(message);
			break;
	}
}

// SetColor
void
ColorPickerPanel::SetColor(rgb_color color)
{
	fColorPickerView->SetColor(color);
}
