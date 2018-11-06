// NewSizePanel.cpp

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

#include "NewSizePanel.h"

enum {
	MSG_CANCEL					= 'cncl',
	MSG_DONE					= 'done',

	MSG_SET_BOUNDS				= 'stbn',
	MSG_SET_WIDTH_SCALE			= 'stwd',
	MSG_SET_HEIGHT_SCALE		= 'stht',
};

// constructor
NewSizePanel::NewSizePanel(BRect frame, BRect currentSize,
						   BMessage* message, BWindow* target)
	: Panel(frame, "Resize Canvas",
			B_TITLED_WINDOW_LOOK, B_MODAL_SUBSET_WINDOW_FEEL,
			B_ASYNCHRONOUS_CONTROLS | B_NOT_ZOOMABLE),
	  fInitialSize(currentSize),
	  fMessage(message),
	  fTarget(target)
{
	LanguageManager* manager = LanguageManager::Default();
	SetTitle(manager->GetString(RESIZE_CANVAS, "Resize Canvas"));

	MButton* defaultButton = new MButton(manager->GetString(OK, "Ok"), new BMessage(MSG_DONE), this);

	// interface layout
	BView* topView = new VGroup
	(
		new MBorder
		(
			M_RAISED_BORDER, 5, "dimension",
			new VGroup
			(
				fWidthTC = new MTextControl("", "1.00"),
				fHeightTC = new MTextControl("", "1.00"),
				fDimensionsControl = new DimensionsControl(new BMessage(MSG_SET_BOUNDS),
														   this),
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

	fWidthTC->SetLabel(manager->GetString(X_SCALE, "X Scale"));
	fHeightTC->SetLabel(manager->GetString(Y_SCALE, "Y Scale"));

	fWidthTC->SetMessage(new BMessage(MSG_SET_WIDTH_SCALE));
	fHeightTC->SetMessage(new BMessage(MSG_SET_HEIGHT_SCALE));

	fDimensionsControl->SetWidthLimits(1, 4096);
	fDimensionsControl->SetHeightLimits(1, 4096);
	fDimensionsControl->SetDimensions(fInitialSize.IntegerWidth() + 1,
									  fInitialSize.IntegerHeight() + 1);
	fDimensionsControl->SetLabels(manager->GetString(WIDTH, "Width"),
								  manager->GetString(HEIGHT, "Height"));

	fDimensionsControl->SetProportionsLocked(GlobalSettings::Default().ProportionsLocked(PROP_LOCK_RESIZE));

	DivideSame(fWidthTC, fHeightTC,
			   fDimensionsControl->WidthControl(),
			   fDimensionsControl->HeightControl(),
			   NULL);

	SetDefaultButton(defaultButton);

	if (fTarget)
		AddToSubset(fTarget);
	else
		SetFeel(B_MODAL_APP_WINDOW_FEEL);

	AddChild(topView);
	fDimensionsControl->WidthControl()->MakeFocus(true);
}

// destructor
NewSizePanel::~NewSizePanel()
{
	delete fMessage;
}

// Cancel
void
NewSizePanel::Cancel()
{
	PostMessage(MSG_CANCEL);
}

// QuitRequested
bool
NewSizePanel::QuitRequested()
{
	_Finish(MSG_CANCEL);
	return true;
}

// MessageReceived
void
NewSizePanel::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_CANCEL:
		case MSG_DONE:
			_Finish(message->what);
			PostMessage(B_QUIT_REQUESTED);
			break;
		case MSG_SET_BOUNDS:
			_SyncScale();
			break;
		case MSG_SET_WIDTH_SCALE:
			_UpdateWidthFromScale();
			break;
		case MSG_SET_HEIGHT_SCALE:
			_UpdateHeightFromScale();
			break;
		default:
			Panel::MessageReceived(message);
			break;
	}
}

// _Finish
void
NewSizePanel::_Finish(uint32 what)
{
	// take care of pending updates
	if (fWidthTC->TextView()->IsFocus())
		_UpdateWidthFromScale();
	else if (fHeightTC->TextView()->IsFocus())
		_UpdateHeightFromScale();

	GlobalSettings::Default().SetProportionsLocked(PROP_LOCK_RESIZE,
												   fDimensionsControl->IsProportionsLocked());

	BMessage msg('size');
	BLooper* looper = fTarget ? static_cast<BLooper*>(fTarget)
							  : static_cast<BLooper*>(be_app);
	if (fMessage)
		msg = *fMessage;
	if (what == MSG_DONE) {
		msg.AddInt32("width", fDimensionsControl->Width());
		msg.AddInt32("height", fDimensionsControl->Height());
	}
	msg.AddRect("panel frame", Frame());
	looper->PostMessage(&msg);
}

// _UpdateWidthFromScale
void
NewSizePanel::_UpdateWidthFromScale() const
{
	float width = (fInitialSize.Width() + 1.0) * atof(fWidthTC->Text());
	fDimensionsControl->SetWidth(floorf(width + 0.5));
	_SyncScale();
}

// _UpdateHeightFromScale
void
NewSizePanel::_UpdateHeightFromScale() const
{
	float height = (fInitialSize.Height() + 1.0) * atof(fHeightTC->Text());
	fDimensionsControl->SetHeight(floorf(height + 0.5));
	_SyncScale();
}


// _SyncScale
void
NewSizePanel::_SyncScale() const
{
	float xScale = (float)fDimensionsControl->Width() / (fInitialSize.Width() + 1.0);
	float yScale = (float)fDimensionsControl->Height() / (fInitialSize.Height() + 1.0);

	BString helper;
	helper << xScale;
	fWidthTC->SetText(helper.String());

	helper = "";
	helper << yScale;
	fHeightTC->SetText(helper.String());
}

