// LayerConfigView.cpp

#include <stdio.h>

#include <Message.h>
#include <Window.h>

#include <HGroup.h>
#include <MCheckBox.h>
#include <Space.h>

#include "BubbleHelper.h"
#include "CanvasView.h"
#include "LanguageManager.h"
#include "Layer.h"
#include "PopupSlider.h"

#include "ChangeLayerSettingsAction.h"

#include "LayerConfigView.h"

enum {
	MSG_ALLOW_UPDATES	= 'alup',
};

// constructor
LayerConfigView::LayerConfigView(CanvasView* canvasView)
	: MBorder( M_NO_BORDER, 0, "layer config", NULL),
	  fLayer(NULL),
	  fCanvasView(canvasView),
	  fUpdate(true),
	  fOldAlpha(-1.0)
{
	BView* topView = new HGroup
	(
		new Space(),
//		fVisibleCB = new MCheckBox("", new BMessage(MSG_SET_VISIBLE), this, true),
		fAlphaPS = new PopupSlider("layer alpha", "Alpha",
								   new BMessage(MSG_SET_ALPHA), this,
								   0, 100, 100, "%ld%%"),
		0
	);
	fAlphaPS->SetPressedMessage(new BMessage(MSG_START_ALPHA_CHANGE));
	fAlphaPS->SetReleasedMessage(new BMessage(MSG_STOP_ALPHA_CHANGE));
	AddChild(topView);
	SetLayer(NULL);

	BubbleHelper* helper = BubbleHelper::Default();

	helper->SetHelp(fAlphaPS, "Sets the active layer's global alpha value.");
//	helper->SetHelp(fVisibleCB, "Toggles the active layer's visibility.");
}

// destructor
LayerConfigView::~LayerConfigView()
{
}

// AttachtedToWindow
void
LayerConfigView::AttachtedToWindow()
{
//	fVisibleCB->SetTarget(this);
}

// MessageReceived
void
LayerConfigView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_SET_VISIBLE:
/*			if (fLayer && fUpdate) {
				uint32 flags = fLayer->Flags();
				if (fVisibleCB->Value())
					flags &= ~FLAG_INVISIBLE;
				else
					flags |= FLAG_INVISIBLE;
				fCanvasView->Perform(new ChangeLayerSettingsAction(fLayer,
																   fLayer->Name(),
																   fLayer->Alpha(),
																   fLayer->Mode(),
																   flags));
				Window()->PostMessage(MSG_ALLOW_UPDATES, this);
				fUpdate = false;
			}*/
			break;
		case MSG_START_ALPHA_CHANGE:
			fOldAlpha = fLayer->Alpha();
			// fall through
		case MSG_SET_ALPHA:
			if (fOldAlpha >= 0.0) {
				if (fLayer && fUpdate) {
					float alpha = fAlphaPS->Value() / 100.0;
					if (alpha != fLayer->Alpha()) {
						fLayer->SetAlpha(alpha);
						fCanvasView->InvalidateCanvas(fLayer->ActiveBounds(), true);
						Window()->PostMessage(MSG_ALLOW_UPDATES, this);
						fUpdate = false;
					}
				}
			}
			break;
		case MSG_STOP_ALPHA_CHANGE:
			if (fOldAlpha >= 0.0)
				fCanvasView->Perform(new ChangeLayerSettingsAction(fLayer, fOldAlpha));
			fOldAlpha = -1.0;
			break;
		case MSG_ALLOW_UPDATES:
			fUpdate = true;
			if (fLayer) {
				float alpha = fAlphaPS->Value() / 100.0;
				if (alpha != fLayer->Alpha())
					Window()->PostMessage(MSG_SET_ALPHA, this);
/*				uint32 flags = fLayer->Flags();
				if (fVisibleCB->Value())
					flags &= ~FLAG_INVISIBLE;
				else
					flags |= FLAG_INVISIBLE;
				if (flags != fLayer->Flags())
					Window()->PostMessage(MSG_SET_VISIBLE, this);*/
			}
			break;
		default:
			MBorder::MessageReceived(message);
	}
}

// SetLayer
void
LayerConfigView::SetLayer(Layer* layer)
{
	fLayer = layer;
	if (fLayer) {
//		fVisibleCB->SetValue(!(fLayer->Flags() & FLAG_INVISIBLE));
//		fVisibleCB->SetEnabled(true);
		fAlphaPS->SetValue(int32(fLayer->Alpha() * 100.0));
		fAlphaPS->SetEnabled(true);
	} else {
//		fVisibleCB->SetValue(true);
//		fVisibleCB->SetEnabled(false);
		fAlphaPS->SetValue(100);
		fAlphaPS->SetEnabled(false);
	}
}

// UpdateStrings
void
LayerConfigView::UpdateStrings()
{
	LanguageManager* manager = LanguageManager::Default();
	BubbleHelper* helper = BubbleHelper::Default();

	fAlphaPS->SetLabel(manager->GetString(ALPHA, "Opacity"));

	helper->SetHelp(fAlphaPS, manager->GetString(ALPHA_TIP, "Sets the active layer's global alpha value."));
//	helper->SetHelp(fVisibleCB, manager->GetString(VISIBILITY_TIP, "Toggles the active layer's visibility."));
}
