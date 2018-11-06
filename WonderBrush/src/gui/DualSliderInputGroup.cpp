// DualSliderInputGroup.cpp

#include <stdio.h>

#include <Message.h>
#include <Window.h>

#include <MTextControl.h>

#include "BubbleHelper.h"
#include "DualSlider.h"
#include "LanguageManager.h"

#include "DualSliderInputGroup.h"

class SmallTextControl : public MTextControl {
 public:
					SmallTextControl(char* label, char* text)
					: MTextControl(label, text)
						{ }
	virtual			~SmallTextControl()
						{ }

	virtual	void	AllAttached()
					{
						MTextControl::AttachedToWindow();
						BFont font;
						GetFont(&font);
						font.SetSize(font.Size() * 0.8);
						SetFont(&font);
						TextView()->SetFontAndColor(&font);
					}
};



// constructor
DualSliderInputGroup::DualSliderInputGroup(DualSlider* slider)
	: MGroup(),
	  BView(BRect(0.0, 0.0, 10.0, 10.0), "slider input group",
	  		B_FOLLOW_NONE, B_WILL_DRAW),
	  fSlider(slider),
	  fMinTC(new SmallTextControl("Min", NULL)),
	  fMaxTC(new SmallTextControl("Max", NULL))
{
	AddChild(fSlider);
	AddChild(fMinTC);
	AddChild(fMaxTC);

//	BubbleHelper* helper = BubbleHelper::Default();

//	helper->SetHelp(fMinTC, "");
//	helper->SetHelp(fMaxTC, "");

	DivideSame(fMinTC, fMaxTC, NULL);

	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

// destructor
DualSliderInputGroup::~DualSliderInputGroup()
{
}

// layoutprefs
minimax
DualSliderInputGroup::layoutprefs()
{
	minimax sliderPrefs = fSlider->layoutprefs();
	minimax minPrefs = fMinTC->layoutprefs();
	minimax maxPrefs = fMaxTC->layoutprefs();

	mpm.mini.y = sliderPrefs.mini.y
				 + (minPrefs.mini.y > maxPrefs.mini.y ?
				 	minPrefs.mini.y : maxPrefs.mini.y);

	mpm.maxi.y = sliderPrefs.maxi.y
				 + (minPrefs.maxi.y < maxPrefs.maxi.y ?
				 	minPrefs.maxi.y : maxPrefs.maxi.y);

	mpm.mini.y += 5.0;
	mpm.maxi.y += 5.0;

	mpm.mini.x = sliderPrefs.mini.x;
	if (minPrefs.mini.x + maxPrefs.mini.x + 5.0 > mpm.mini.x)
		mpm.mini.x = minPrefs.mini.x + maxPrefs.mini.x + 5.0;

	mpm.maxi.x = sliderPrefs.maxi.x;
	if (minPrefs.maxi.x + maxPrefs.maxi.x + 5.0 < mpm.maxi.x)
		mpm.maxi.x = minPrefs.maxi.x + maxPrefs.maxi.x + 5.0;

	mpm.weight = 1.0;
	return mpm;
}

// layout
BRect
DualSliderInputGroup::layout(BRect frame)
{
	MoveTo(frame.LeftTop());
	ResizeTo(frame.Width(), frame.Height());
	BRect childRect(Bounds());
	minimax sliderPrefs = fSlider->layoutprefs();
	childRect.bottom = childRect.top + sliderPrefs.mini.y;
	childRect = fSlider->layout(childRect);
	
	childRect.top = childRect.bottom + 5.0;
	childRect.bottom = Bounds().bottom;
	childRect.right = childRect.left + childRect.Width() / 2.0 - 2.0;

	fMinTC->layout(childRect);

	childRect.left = childRect.right + 5.0;
	childRect.right = Bounds().right;

	fMaxTC->layout(childRect);

	return Frame();
}

// AllAttachted
void
DualSliderInputGroup::AllAttachted()
{
/*	BFont font;
	fMinTC->GetFont(&font);
	font.SetSize(font.Size() * 0.5);
	fMinTC->SetFont(&font);
	fMaxTC->SetFont(&font);
	fMinTC->TextView()->SetFont(&font);
	fMaxTC->TextView()->SetFont(&font);*/
}

// MessageReceived
void
DualSliderInputGroup::MessageReceived(BMessage* message)
{
	switch (message->what) {
		default:
			BView::MessageReceived(message);
	}
}

