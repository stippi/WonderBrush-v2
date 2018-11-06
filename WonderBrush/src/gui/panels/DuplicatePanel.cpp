// DuplicatePanel.cpp

#include <stdio.h>

#include <Screen.h>

#include <HGroup.h>
#include <MBorder.h>
#include <MButton.h>
#include <Space.h>
#include <VGroup.h>

#include "support.h"

#include "GlobalFunctions.h"
#include "GlobalSettings.h"
#include "LanguageManager.h"
#include "NumericalTextControl.h"
#include "Seperator.h"

#include "DuplicatePanel.h"

enum {
	MSG_PANEL_OK		= 'okok',
	MSG_PANEL_CANCEL	= 'cncl',
};

// constructor
DuplicatePanel::DuplicatePanel(BWindow *window,
							   BHandler* target,
							   BMessage* message,
							   BRect frame)
	: Panel(frame, "Duplicate",
			B_FLOATING_WINDOW_LOOK, B_FLOATING_SUBSET_WINDOW_FEEL,
			B_ASYNCHRONOUS_CONTROLS | B_NOT_V_RESIZABLE),
	  fWindow(window),
	  fTarget(target),
	  fMessage(message)
{
	LanguageManager* m = LanguageManager::Default();

	SetTitle(m->GetString(DUPLICATE, "Duplicate"));

	MButton* defaultButton = new MButton(m->GetString(DUPLICATE, "Duplicate"), new BMessage(MSG_PANEL_OK), this);

	fCopyCountTC = new NumericalTextControl(NULL, NULL);
	fCopyCountTC->SetLabel(m->GetString(COUNT, "Count"));
	fCopyCountTC->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_CENTER);
	fCopyCountTC->SetFloatMode(false);

	fXOffsetTC = new NumericalTextControl(NULL, NULL);
	fXOffsetTC->SetLabel(m->GetString(TRANSLATION_X, "X"));
	fXOffsetTC->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_CENTER);
	fYOffsetTC = new NumericalTextControl(NULL, NULL);
	fYOffsetTC->SetLabel(m->GetString(TRANSLATION_Y, "Y"));
	fYOffsetTC->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_CENTER);

	fRotationTC = new NumericalTextControl(NULL, NULL);
	fRotationTC->SetLabel(m->GetString(ANGLE, "Angle"));
	fRotationTC->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_CENTER);

	fXScaleTC = new NumericalTextControl(NULL, NULL);
	fXScaleTC->SetLabel(m->GetString(SCALE_X, "X"));
	fXScaleTC->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_CENTER);
	fYScaleTC = new NumericalTextControl(NULL, NULL);
	fYScaleTC->SetLabel(m->GetString(SCALE_Y, "Y"));
	fYScaleTC->SetAlignment(B_ALIGN_RIGHT, B_ALIGN_CENTER);

	DivideSame(fCopyCountTC,
			   fXOffsetTC,
			   fYOffsetTC,
			   fRotationTC,
			   fXScaleTC,
			   fYScaleTC,
			   NULL);


	BView* topView = new VGroup
	(
		new MBorder
		(
			M_RAISED_BORDER, 10, "",
			new VGroup
			(
				fCopyCountTC,
				vertical_space(),
				new Seperator(m->GetString(TRANSLATE, "Translate")),
				fXOffsetTC,
				fYOffsetTC,
				vertical_space(),
				new Seperator(m->GetString(ROTATE, "Rotate")),
				fRotationTC,
				vertical_space(),
				new Seperator(m->GetString(SCALE, "Scale")),
				fXScaleTC,
				fYScaleTC,
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
	AddToSubset(fWindow);

	_RestoreSettings(frame);

	fCopyCountTC->MakeFocus(true);

	Show();
}

// destructor
DuplicatePanel::~DuplicatePanel()
{
	_StoreSettings();
	delete fMessage;
}

// MessageReceived
void DuplicatePanel::MessageReceived(BMessage* message)
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
				cloneMessage.AddInt32("count", fCopyCountTC->IntValue());
				cloneMessage.AddFloat("x offset", fXOffsetTC->FloatValue());
				cloneMessage.AddFloat("y offset", fYOffsetTC->FloatValue());
				cloneMessage.AddFloat("rotation", fRotationTC->FloatValue());
				cloneMessage.AddFloat("x scale", fXScaleTC->FloatValue());
				cloneMessage.AddFloat("y scale", fYScaleTC->FloatValue());
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
DuplicatePanel::_CalculateFrame(BRect frame)
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

// _RestoreSettings
void
DuplicatePanel::_RestoreSettings(BRect frame)
{
	GlobalSettings::Default().GetValue("duplicate panel", &fSettings);

	int32 copyCount;
	if (fSettings.FindInt32("copy count", &copyCount) < B_OK)
		copyCount = 1;
	fCopyCountTC->SetValue(copyCount);

	float value;

	if (fSettings.FindFloat("x offset", &value) < B_OK)
		value = 0.0;
	fXOffsetTC->SetValue(value);

	if (fSettings.FindFloat("y offset", &value) < B_OK)
		value = 0.0;
	fYOffsetTC->SetValue(value);

	if (fSettings.FindFloat("rotation", &value) < B_OK)
		value = 0.0;
	fRotationTC->SetValue(value);

	if (fSettings.FindFloat("x scale", &value) < B_OK)
		value = 1.0;
	fXScaleTC->SetValue(value);

	if (fSettings.FindFloat("y scale", &value) < B_OK)
		value = 1.0;
	fYScaleTC->SetValue(value);

	BRect panelFrame;
	if (fSettings.FindRect("panel frame", &panelFrame) < B_OK)
		panelFrame = _CalculateFrame(frame);

	make_sure_frame_is_on_screen(panelFrame, this);

	MoveTo(panelFrame.LeftTop());
	ResizeTo(panelFrame.Width(), panelFrame.Height());
}

// _StoreSettings
void
DuplicatePanel::_StoreSettings()
{
	if (fSettings.ReplaceInt32("copy count", fCopyCountTC->IntValue()) < B_OK)
		fSettings.AddInt32("copy count", fCopyCountTC->IntValue());

	if (fSettings.ReplaceFloat("x offset", fXOffsetTC->FloatValue()) < B_OK)
		fSettings.AddFloat("x offset", fXOffsetTC->FloatValue());

	if (fSettings.ReplaceFloat("y offset", fYOffsetTC->FloatValue()) < B_OK)
		fSettings.AddFloat("y offset", fYOffsetTC->FloatValue());

	if (fSettings.ReplaceFloat("rotation", fRotationTC->FloatValue()) < B_OK)
		fSettings.AddFloat("rotation", fRotationTC->FloatValue());

	if (fSettings.ReplaceFloat("x scale", fXScaleTC->FloatValue()) < B_OK)
		fSettings.AddFloat("x scale", fXScaleTC->FloatValue());

	if (fSettings.ReplaceFloat("y scale", fYScaleTC->FloatValue()) < B_OK)
		fSettings.AddFloat("y scale", fYScaleTC->FloatValue());

	if (fSettings.ReplaceRect("panel frame", Frame()) < B_OK)
		fSettings.AddRect("panel frame", Frame());

	GlobalSettings::Default().SetValue("duplicate panel", fSettings);
}
