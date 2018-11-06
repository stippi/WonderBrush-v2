// NewCanvasPanel.cpp

#include <stdio.h>

#include <Application.h>

#include <MBorder.h>
#include <MButton.h>
#include <MTextControl.h>
#include <HGroup.h>
#include <Space.h>
#include <VGroup.h>

#include "support.h"

#include "DimensionsControl.h"
#include "GlobalSettings.h"
#include "LanguageManager.h"

#include "NewCanvasPanel.h"

enum {
	MSG_CANCEL					= 'cncl',
	MSG_DONE					= 'done',
};

// constructor
NewCanvasPanel::NewCanvasPanel(BRect frame, BMessage* message, BWindow* target)
	: Panel(frame, "New Canvas",
			B_TITLED_WINDOW_LOOK, B_MODAL_SUBSET_WINDOW_FEEL,
			B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE),
	  fMessage(message),
	  fTarget(target)
{
	LanguageManager* manager = LanguageManager::Default();
	SetTitle(manager->GetString(NEW_CANVAS, "New Canvas"));

	MButton* defaultButton = new MButton(manager->GetString(OK, "Ok"), new BMessage(MSG_DONE), this);

	// interface layout
	BView* topView = new VGroup
	(
		new MBorder
		(
			M_RAISED_BORDER, 5, "dimension",
			new VGroup
			(
				fNameTC = new MTextControl("Name", "<unnamed>"),
				fDimensionsControl = new DimensionsControl(),
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 5, "buttons",
			new HGroup
			(
				minimax(0.0, 0.0, 10000.0, 10000.0, 0.0),
				new Space(minimax(0.0, 0.0, 10000.0, 0.0, 5.0)),
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

	fNameTC->SetLabel(manager->GetString(NAME, "Name"));
	fNameTC->SetText(manager->GetString(UNNAMED, "<unnamed>"));

	fDimensionsControl->SetWidthLimits(1, 4096);
	fDimensionsControl->SetHeightLimits(1, 4096);
	fDimensionsControl->SetDimensions(800, 600);
	fDimensionsControl->SetLabels(manager->GetString(WIDTH, "Width"),
								  manager->GetString(HEIGHT, "Height"));

	DivideSame(fNameTC,
			   fDimensionsControl->WidthControl(),
			   fDimensionsControl->HeightControl(),
			   NULL);

	fDimensionsControl->SetProportionsLocked(GlobalSettings::Default().ProportionsLocked(PROP_LOCK_NEW));

	SetDefaultButton(defaultButton);

	if (fTarget)
		AddToSubset(fTarget);
	else
		SetFeel(B_MODAL_APP_WINDOW_FEEL);

	AddChild(topView);
	fNameTC->MakeFocus(true);
}

// destructor
NewCanvasPanel::~NewCanvasPanel()
{
	delete fMessage;
}

// Cancel
void
NewCanvasPanel::Cancel()
{
	PostMessage(MSG_CANCEL);
}

// QuitRequested
bool
NewCanvasPanel::QuitRequested()
{
	_Finish(MSG_CANCEL);
	return true;
}

// MessageReceived
void
NewCanvasPanel::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_CANCEL:
		case MSG_DONE:
			_Finish(message->what);
			PostMessage(B_QUIT_REQUESTED);
			break;
		default:
			Panel::MessageReceived(message);
			break;
	}
}

// _Finish
void
NewCanvasPanel::_Finish(uint32 what)
{
	GlobalSettings::Default().SetProportionsLocked(PROP_LOCK_NEW,
												   fDimensionsControl->IsProportionsLocked());

	BMessage msg('size');
	BLooper* looper = fTarget ? static_cast<BLooper*>(fTarget)
							  : static_cast<BLooper*>(be_app);
	if (fMessage)
		msg = *fMessage;
	if (what == MSG_DONE) {
		msg.AddInt32("width", fDimensionsControl->Width());
		msg.AddInt32("height", fDimensionsControl->Height());
		msg.AddString("name", fNameTC->Text());
	}
	msg.AddRect("panel frame", Frame());
	looper->PostMessage(&msg);
}


