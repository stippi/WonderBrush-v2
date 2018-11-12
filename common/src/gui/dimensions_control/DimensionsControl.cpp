// DimensionsControl.cpp

#include <stdio.h>

#include <Handler.h>
#include <Looper.h>
#include <MenuItem.h>
#include <Message.h>
#include <String.h>
#include <PopUpMenu.h>

#include <HGroup.h>
#include <MTextControl.h>
#include <MPopup.h>
#include <Space.h>
#include <VGroup.h>

#include "BubbleHelper.h"
#include "LockView.h"

#include "DimensionsControl.h"

// debugging
#include "Debug.h"
//#define ldebug	debug
#define ldebug	nodebug

enum {
	MSG_WIDTH_CHANGED		= 'wdtc',
	MSG_HEIGHT_CHANGED		= 'hgtc',
	MSG_COMMON_FORMAT		= 'cfmt',

	FORMAT_160_120,
	FORMAT_320_240,
	FORMAT_400_300,
	FORMAT_640_480,
	FORMAT_800_600,
	FORMAT_1024_768,
	FORMAT_1152_864,
	FORMAT_1280_800,
	FORMAT_1280_960,
	FORMAT_1280_1024,
	FORMAT_1400_1050,
	FORMAT_1600_1200,
	FORMAT_1920_1200,
	FORMAT_2048_1536,

	FORMAT_384_288,
	FORMAT_768_576,
	FORMAT_720_540,

	FORMAT_1280_720,
	FORMAT_1920_1080,

	FORMAT_352_240,
	FORMAT_768_524,
	FORMAT_720_492,

	FORMAT_720_480,
	FORMAT_720_576,

	FORMAT_384_216,
	FORMAT_768_432,
	FORMAT_720_405,

	FORMAT_384_164,
	FORMAT_768_327,
	FORMAT_720_306,
};

class ArrowPopup : public MPopup {
public:
			ArrowPopup(BMessage* message, BHandler* handler)
				: MPopup(NULL, message, handler, NULL)
			{
				Menu()->SetLabelFromMarked(false);
				if (BMenuItem *superItem = Menu()->Superitem())
					superItem->SetLabel("");
				Menu()->SetRadioMode(false);
			}

	virtual	~ArrowPopup() {}

	virtual	minimax	layoutprefs()
			{
				float width, height;
				GetPreferredSize(&width, &height);
				mpm = MPopup::layoutprefs();
				mpm.mini.x = mpm.maxi.x = 21.0;//width + 1.0;
				mpm.weight = 1.0;
				return mpm;
			}
};

// constructor
DimensionsControl::DimensionsControl(BMessage* _message,
									 BHandler* target)
	: MBorder(M_NO_BORDER, 0, "dimensions control", 0),
	  fMessage(_message),
	  fTarget(target),
	  fWidthTC(NULL),
	  fHeightTC(NULL),
	  fLockView(NULL),
	  fCommonFormatsPU(NULL),
	  fPreviousWidth(0),
	  fPreviousHeight(0),
	  fMinWidth(0),
	  fMaxWidth(0),
	  fMinHeight(0),
	  fMaxHeight(0)
{
	// build common formats menu
	fCommonFormatsPU = new ArrowPopup(NULL, this);

	BMessage* message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_160_120);
	BMenuItem* item = new BMenuItem("160x120", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_320_240);
	item = new BMenuItem("320x240", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_400_300);
	item = new BMenuItem("400x300", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_640_480);
	item = new BMenuItem("640x480", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_800_600);
	item = new BMenuItem("800x600", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_1024_768);
	item = new BMenuItem("1024x768", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_1152_864);
	item = new BMenuItem("1152x864", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_1280_800);
	item = new BMenuItem("1280x800", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_1280_960);
	item = new BMenuItem("1280x960 (4:3)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_1280_1024);
	item = new BMenuItem("1280x1024 (5:4)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_1400_1050);
	item = new BMenuItem("1400x1050", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_1600_1200);
	item = new BMenuItem("1600x1200", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_1920_1200);
	item = new BMenuItem("1920x1200", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_2048_1536);
	item = new BMenuItem("2048x1536", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	fCommonFormatsPU->Menu()->AddSeparatorItem();

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_384_288);
	item = new BMenuItem("384x288 (1/4 PAL)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_768_576);
	item = new BMenuItem("768x576 (PAL)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_720_540);
	item = new BMenuItem("720x540 (PAL TV cropped)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	fCommonFormatsPU->Menu()->AddSeparatorItem();

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_1280_720);
	item = new BMenuItem("1280x720 (HDTV 16:9)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_1920_1080);
	item = new BMenuItem("1920x1080 (HDTV 16:9)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

/*	fCommonFormatsPU->Menu()->AddSeparatorItem();

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_352_240);
	item = new BMenuItem("352x240 (1/4 NTSC)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_768_524);
	item = new BMenuItem("768x524 (NTSC)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_720_492);
	item = new BMenuItem("720x492 (TV cropped NTSC)", message);
	fCommonFormatsPU->Menu()->AddItem(item);*/

	fCommonFormatsPU->Menu()->AddSeparatorItem();

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_720_576);
	item = new BMenuItem("720x576 (DV PAL)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_720_480);
	item = new BMenuItem("720x480 (DV NTSC)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	fCommonFormatsPU->Menu()->AddSeparatorItem();

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_384_216);
	item = new BMenuItem("384x216 (1/4 PAL 16:9)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_768_432);
	item = new BMenuItem("768x432 (PAL 16:9)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_720_405);
	item = new BMenuItem("720x405 (PAL 16:9 TV cropped)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	fCommonFormatsPU->Menu()->AddSeparatorItem();

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_384_164);
	item = new BMenuItem("384x164 (1/4 PAL 2.35:1)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_768_327);
	item = new BMenuItem("768x327 (PAL 2.35:1)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	message = new BMessage(MSG_COMMON_FORMAT);
	message->AddInt32("format", FORMAT_720_306);
	item = new BMenuItem("720x306 (PAL 2.35:1 TV cropped)", message);
	fCommonFormatsPU->Menu()->AddItem(item);

	// build gui layout
	HGroup *topView = new HGroup
	(
		new VGroup
		(
			fWidthTC = new MTextControl("Width:", 0, new BMessage(MSG_WIDTH_CHANGED)),
			fHeightTC = new MTextControl("Height:", 0, new BMessage(MSG_HEIGHT_CHANGED)),
			0
		),
		fLockView = new LockView(),
		new Space(minimax(5.0f, 5.0f, 10000.0f, 10000.0f, 0.1f)),
		new VGroup
		(
			fCommonFormatsPU,
			new Space(minimax(5.0f, 20.0f, 5.0f, 20.0f, 1.0f)),
			0
		),
		0
	);

	// some last tweaks
	DivideSame(fHeightTC, fWidthTC, NULL);
	// only accept numbers in text views
	for (uint32 i = 0; i < '0'; i++) {
		fWidthTC->TextView()->DisallowChar(i);
		fHeightTC->TextView()->DisallowChar(i);
	}
	for (uint32 i = '9' + 1; i < 255; i++) {
		fWidthTC->TextView()->DisallowChar(i);
		fHeightTC->TextView()->DisallowChar(i);
	}
	// set bubble help texts
//	helper->SetHelp(fWidthTC, "");
//	helper->SetHelp(fHeightTC, "");
//	helper->SetHelp(fLockView, "Lock proportion of Width and Height.");
//	helper->SetHelp(fCommonFormatsPU, "Pick one of many common formats.");

	AddChild(topView);

	// put something in the textviews
	_SetDimensions(fMinWidth, fMinHeight, false);
}

// destructor
DimensionsControl::~DimensionsControl()
{
	delete fMessage;
}

// AttachedToWindow
void
DimensionsControl::AttachedToWindow()
{
	fWidthTC->SetTarget(this);
	fHeightTC->SetTarget(this);
}

// MessageReceiced
void
DimensionsControl::MessageReceived(BMessage *message)
{
	switch (message->what) {
		case MSG_WIDTH_CHANGED:
			SetWidth(Width());
			break;
		case MSG_HEIGHT_CHANGED:
			SetHeight(Height());
			break;
		case MSG_COMMON_FORMAT: {
			int32 format;
			if (message->FindInt32("format", &format) == B_OK) {
				uint32 width;
				uint32 height;
				switch (format) {
					case FORMAT_160_120:
						width = 160;
						height = 120;
						break;
					case FORMAT_320_240:
						width = 320;
						height = 240;
						break;
					case FORMAT_400_300:
						width = 400;
						height = 300;
						break;
					case FORMAT_640_480:
						width = 640;
						height = 480;
						break;
					case FORMAT_800_600:
						width = 800;
						height = 600;
						break;

					case FORMAT_1024_768:
						width = 1024;
						height = 768;
						break;
					case FORMAT_1152_864:
						width = 1152;
						height = 864;
						break;
					case FORMAT_1280_800:
						width = 1280;
						height = 800;
						break;
					case FORMAT_1280_960:
						width = 1280;
						height = 960;
						break;
					case FORMAT_1280_1024:
						width = 1280;
						height = 1024;
						break;
					case FORMAT_1400_1050:
						width = 1400;
						height = 1050;
						break;
					case FORMAT_1600_1200:
						width = 1600;
						height = 1200;
						break;
					case FORMAT_1920_1200:
						width = 1920;
						height = 1200;
						break;
					case FORMAT_2048_1536:
						width = 2048;
						height = 1536;
						break;

					case FORMAT_384_288:
						width = 384;
						height = 288;
						break;
					case FORMAT_768_576:
						width = 768;
						height = 576;
						break;
					case FORMAT_720_540:
						width = 720;
						height = 540;
						break;

					case FORMAT_1280_720:
						width = 1280;
						height = 720;
						break;
					case FORMAT_1920_1080:
						width = 1920;
						height = 1080;
						break;

					case FORMAT_352_240:
						width = 352;
						height = 240;
						break;
					case FORMAT_768_524:
						width = 768;
						height = 524;
						break;
					case FORMAT_720_492:
						width = 720;
						height = 492;
						break;
					case FORMAT_720_480:
						width = 720;
						height = 480;
						break;
					case FORMAT_720_576:
						width = 720;
						height = 576;
						break;
					case FORMAT_384_216:
						width = 384;
						height = 216;
						break;
					case FORMAT_768_432:
						width = 768;
						height = 432;
						break;
					case FORMAT_720_405:
						width = 720;
						height = 405;
						break;
					case FORMAT_384_164:
						width = 384;
						height = 164;
						break;
					case FORMAT_768_327:
						width = 768;
						height = 327;
						break;
					case FORMAT_720_306:
						width = 720;
						height = 306;
						break;
					default:
						width = 384;
						height = 288;
						break;
				}
				_SetDimensions(width, height);
			}
			break;
		}
		default:
			MBorder::MessageReceived(message);
			break;
	}
}

// layoutprefs
minimax
DimensionsControl::layoutprefs()
{
	mpm = MBorder::layoutprefs();
	mpm.mini.x *= 1.5;
	return mpm;
}

// SetDimensions
void
DimensionsControl::SetDimensions(uint32 width, uint32 height)
{
	_SetDimensions(width, height, false);
}

// SetWidth
void
DimensionsControl::SetWidth(uint32 width)
{
	uint32 height = Height();
	if (fLockView->IsLocked()) {
		height = GetLockedHeightFor(width);
	}
	_SetDimensions(width, height);
}

// SetHeight
void
DimensionsControl::SetHeight(uint32 height)
{
	uint32 width = Width();
	if (fLockView->IsLocked()) {
		width = GetLockedWidthFor(height);
	}
	_SetDimensions(width, height);
}

// Width
uint32
DimensionsControl::Width() const
{
	return atoi(fWidthTC->Text());
}

// Height
uint32
DimensionsControl::Height() const
{
	return atoi(fHeightTC->Text());
}

// SetWidthLimits
void
DimensionsControl::SetWidthLimits(uint32 min, uint32 max)
{
	if (max < min)
		max = min;
	fMinWidth = min;
	fMaxWidth = max;
	_SetDimensions(Width(), Height(), false);
}

// SetHeightLimits
void
DimensionsControl::SetHeightLimits(uint32 min, uint32 max)
{
	if (max < min)
		max = min;
	fMinHeight = min;
	fMaxHeight = max;
	_SetDimensions(Width(), Height(), false);
}

// SetProportionsLocked
void
DimensionsControl::SetProportionsLocked(bool lock)
{
	fLockView->SetLocked(lock);
}

// IsProportionsLocked
bool
DimensionsControl::IsProportionsLocked() const
{
	return fLockView->IsLocked();
}

// SetEnabled
void
DimensionsControl::SetEnabled(bool enabled)
{
	fWidthTC->SetEnabled(enabled);
	fHeightTC->SetEnabled(enabled);
	fLockView->SetEnabled(enabled);
	fCommonFormatsPU->SetEnabled(enabled);
}

// IsEnabled
bool
DimensionsControl::IsEnabled() const
{
	return fLockView->IsEnabled();
}

// SetLabels
void
DimensionsControl::SetLabels(const char* width, const char* height)
{
	fWidthTC->SetLabel(width);
	fHeightTC->SetLabel(height);
}

// WidthControl
MTextControl*
DimensionsControl::WidthControl() const
{
	return fWidthTC;
}

// HeightControl
MTextControl*
DimensionsControl::HeightControl() const
{
	return fHeightTC;
}

// GetLockedWidthFor
uint32
DimensionsControl::GetLockedWidthFor(uint32 newHeight)
{
	float proportion = (float)fPreviousHeight / (float)fPreviousWidth; // using old height
	return (uint32)((float)newHeight / proportion);
}

// GetLockedHeightFor
uint32
DimensionsControl::GetLockedHeightFor(uint32 newWidth)
{
	float proportion = (float)fPreviousWidth / (float)fPreviousHeight; // using old width
	return (uint32)((float)newWidth / proportion);
}

// _SetDimensions
void
DimensionsControl::_SetDimensions(uint32 width, uint32 height, bool sendMessage)
{
	// check limits
	if (width < fMinWidth)
		width = fMinWidth;
	if (width > fMaxWidth)
		width = fMaxWidth;
	if (height < fMinHeight)
		height = fMinHeight;
	if (height > fMaxHeight)
		height = fMaxHeight;
	// put this into the text controls
	BString helper("");
	helper << width;
	fWidthTC->SetText(helper.String());
	if (fWidthTC->TextView()->IsFocus())
		fWidthTC->TextView()->SelectAll();
	helper.SetTo("");
	helper << height;
	fHeightTC->SetText(helper.String());
	if (fHeightTC->TextView()->IsFocus())
		fHeightTC->TextView()->SelectAll();
	// reset previous width and height
	fPreviousWidth = width;
	fPreviousHeight = height;
	// notify target
	if (sendMessage && fTarget && fMessage) {
		if (BLooper* looper = fTarget->Looper()) {
			BMessage message(*fMessage);
			message.AddPointer("source", this);
			message.AddInt64("when", system_time());
			message.AddInt32("width", (int32)Width());
			message.AddInt32("height", (int32)Height());
			looper->PostMessage(&message, fTarget);
		}
	}
}

