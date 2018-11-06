//	AboutView.cpp

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <Message.h>
#include <Messenger.h>
#include <MessageRunner.h>
#include <Window.h>
#include <Bitmap.h>
#include <Roster.h>

#include "BlurFilter.h"
#include "EdgesFilter.h"
#include "IFS.h"

#include "AboutView.h"

enum {
	MSG_TICK				= 'tick',
};

#if __MWERKS__
#define lrand48	rand
#define srand48	srand
#endif

// constructor
AboutView::AboutView(const char* app,
					 const char* developer,
					 const char* info[],
					 uint32 infoCount,
					 const char* licencee,
					 const char* copyright)
	: BView(BRect(0.0, 0.0, 400.0, 300.0), "about",
			B_FOLLOW_NONE, B_WILL_DRAW | B_PULSE_NEEDED),
	  fTicker(NULL),
	  fAppName(app),
	  fDevName(developer),
	  fInfos(new const char*[infoCount]),
	  fInfoCount(infoCount),
	  fLicencee(licencee),
	  fCopyright(copyright),
	  fTuneRect(60.0, 115.0, 175.0, 145.0),
	  fScottRect(100.0, 195.0, 245.0, 280.0),
	  fStippiRect(255.0, 175.0, 330.0, 230.0),
	  fCurrentBitmap(0),
	  fDirty(false),
	  fEdgesFilter(new EdgesFilter()),
	  fBlurFilter(new BlurFilter()),
	  fIFS(new IFS(Bounds()))
{
	for (uint32 i = 0; i < infoCount; i++)
		fInfos[i] = info[i];

	fBackBitmap[0] = NULL;
	fBackBitmap[1] = NULL;
	fBackView[0] = NULL;
	fBackView[1] = NULL;
	SetViewColor(B_TRANSPARENT_32_BIT);
	_AllocBackBitmaps(Bounds().Width(), Bounds().Height());

	srand48((long int)system_time());

	float width = Bounds().Width();
	float height = Bounds().Height();
	for (int32 i = 0; i < BACKGROUND_RECT_NUM; i++) {
		fBackgroundRects[i].Set(fmod(lrand48(), width),
								fmod(lrand48(), height),
								fmod(lrand48(), width),
								fmod(lrand48(), height));
	}
}

// destructor
AboutView::~AboutView()
{
	_FreeBackBitmaps();
	delete fTicker;
	delete fEdgesFilter;
	delete fBlurFilter;
	delete fIFS;
	delete[] fInfos;
}

// Draw
void
AboutView::Draw(BRect updateRect)
{
	if (fBackBitmap[fCurrentBitmap]) {
		if (fDirty) {
			uint32 nextBuffer = fCurrentBitmap + 1;
			if (nextBuffer >= 2)
				nextBuffer = 0;
			fEdgesFilter->Run(fBackBitmap[nextBuffer],
							  fBackBitmap[fCurrentBitmap]);
			// flip buffers
			if (fBackBitmap[fCurrentBitmap]->Lock()) {
				fIFS->Draw(fBackBitmap[fCurrentBitmap]);
				_DrawInto(fBackView[fCurrentBitmap]);
				fBackView[fCurrentBitmap]->Sync();
				fBackBitmap[fCurrentBitmap]->Unlock();
			}
			fCurrentBitmap = nextBuffer;
			fDirty = false;
		}
		SetDrawingMode(B_OP_COPY);
		DrawBitmap(fBackBitmap[fCurrentBitmap],
				   updateRect, updateRect);
	}
}

// MessageReceived
void
AboutView::MessageReceived(BMessage *msg)
{
	if (msg->what == MSG_TICK) {
		fDirty = true;
		Invalidate();
	} else
		BView::MessageReceived(msg);
}

// AttachedToWindow
void
AboutView::AttachedToWindow()
{
	BMessenger mess(this, Window());
	BMessage msg(MSG_TICK);
	fTicker = new BMessageRunner(mess, &msg, 40000LL);
}

// MouseDown
void
AboutView::MouseDown(BPoint where)
{
	if (Bounds().Contains(where)) {
/*		if (fTuneRect.Contains(where)) {
			char *argv = "http://www.beosradio.com/tunetracker/";
			be_roster->Launch("text/html", 1, &argv);
		} else if (fScottRect.Contains(where)) {
			char *argv = "mailto:tunetracker@centurytel.net";
			be_roster->Launch("text/x-email", 1, &argv);
		} else if (fStippiRect.Contains(where)) {
//			char *argv = "mailto:stippi@yellowbites.com";
//			be_roster->Launch("text/x-email", 1, &argv);
			char *argv = "http://www.yellowbites.com";
			be_roster->Launch("text/html", 1, &argv);
		}*/
		Window()->PostMessage(B_QUIT_REQUESTED);
	}
}

// MouseMoved
void
AboutView::MouseMoved(BPoint where, uint32 transit, const BMessage *message)
{
/*	if (fTuneRect.Contains(where))
		fHotRect = 1;
	else if (fScottRect.Contains(where))
		fHotRect = 2;
	else if (fStippiRect.Contains(where))
		fHotRect = 3;
	else
		fHotRect = 0;*/
}

// _AllocBackBitmaps
void
AboutView::_AllocBackBitmaps(float width, float height)
{
	// sanity check
	if (width <= 0.0 || height <= 0.0)
		return;

	BRect b(0.0, 0.0, width, height);
	fBackBitmap[0] = new BBitmap(b, B_BITMAP_ACCEPTS_VIEWS, B_RGB32);
	fBackBitmap[1] = new BBitmap(b, B_BITMAP_ACCEPTS_VIEWS, B_RGB32);
	if (!fBackBitmap[0] || !fBackBitmap[0]->IsValid()
		|| !fBackBitmap[1] || !fBackBitmap[1]->IsValid()) {
		_FreeBackBitmaps();
		fprintf(stderr, "AboutView::_AllocBackBitmaps(): bitmaps invalid\n");
		return;
	}
	// init bitmap 1
	fBackView[0] = new BView(b, 0, B_FOLLOW_NONE, B_WILL_DRAW);
	fBackBitmap[0]->AddChild(fBackView[0]);
	memset(fBackBitmap[0]->Bits(), 0, fBackBitmap[0]->BitsLength());
	// init bitmap 2
	fBackView[1] = new BView(b, 0, B_FOLLOW_NONE, B_WILL_DRAW);
	fBackBitmap[1]->AddChild(fBackView[1]);
	memset(fBackBitmap[1]->Bits(), 0, fBackBitmap[1]->BitsLength());
}

// _FreeBackBitmaps
void
AboutView::_FreeBackBitmaps()
{
	if (fBackBitmap[0]) {
		delete fBackBitmap[0];
		fBackBitmap[0] = NULL;
		fBackView[0] = NULL;
	}
	if (fBackBitmap[1]) {
		delete fBackBitmap[1];
		fBackBitmap[1] = NULL;
		fBackView[1] = NULL;
	}
}

// randomize_color
rgb_color
randomize_color(rgb_color color, uint8 howMuch)
{
	color.red = max_c(0, color.red - lrand48() % howMuch);
	color.green = max_c(0, color.green - lrand48() % howMuch);
	color.blue = max_c(0, color.blue - lrand48() % howMuch);
	return color;
}

// randomize_rect
BRect
randomize_rect(BRect r, float howMuch)
{
	return BRect(r.left + fmod(lrand48(), howMuch),
				 r.top + fmod(lrand48(), howMuch),
				 r.right + fmod(lrand48(), howMuch),
				 r.bottom + fmod(lrand48(), howMuch));
}

// DrawInto
void
AboutView::_DrawInto(BView *view)
{
	rgb_color lightShadow = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT);
	rgb_color light = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_MAX_TINT);
	BRect r(view->Bounds());
	// background
	rgb_color backGround = { 0, 0, 0, 20 };
	view->SetHighColor(backGround);
	view->SetDrawingMode(B_OP_ALPHA);
	view->SetBlendingMode(B_CONSTANT_ALPHA, B_ALPHA_OVERLAY);
	r.bottom = 80.0;
	view->FillRect(r);
	r.top = r.bottom + 1.0;
	r.bottom = view->Bounds().bottom;
	backGround.alpha = 51;
	view->SetHighColor(backGround);
	view->FillRect(r);

/*	rgb_color backgroundBase = (rgb_color){ 18, 165, 241, 30 };
	for (int32 i = 0; i < BACKGROUND_RECT_NUM; i++) {
		view->SetHighColor(randomize_color(backgroundBase, 40));
		view->SetPenSize(fmod(lrand48(), 15.0));
		view->StrokeRect(randomize_rect(fBackgroundRects[i], 30.0));
	}*/

/*	view->SetHighColor((rgb_color){ 20, 82, 241, 50 });
	view->SetPenSize(fmod(lrand48(), 15.0));
	view->StrokeRect(randomize_rect(BRect(50.0, -35.0, 180.0, 100.0), 40.0));
	view->SetHighColor((rgb_color){ 30, 30, 216, 50 });
	view->SetPenSize(fmod(lrand48(), 30.0));
	view->StrokeRect(randomize_rect(BRect(380.0, 255.0, 410.0, 290.0), 15.0));
	view->SetHighColor((rgb_color){ 49, 11, 136, 50 });
	view->SetPenSize(fmod(lrand48(), 8.0));
	view->StrokeRect(randomize_rect(BRect(90.0, 200.0, 210.0, 320.0), 35.0));
	view->SetHighColor((rgb_color){ 59, 59, 198, 50 });
	view->SetPenSize(fmod(lrand48(), 12.0));
	view->StrokeRect(randomize_rect(BRect(-50.0, 80.0, 230.0, 160.0), 25.0));*/

	// about text
	view->SetLowColor(backGround);
	BPoint offset(15.0, 70.0);
	_DrawJigglingAndRotatingString(view, &fAppName, offset, 30.0,
								   (rgb_color){ 255, 255, 255, 255 });
	offset.y += 30;
	_DrawJigglingString(view, &fDevName, offset, 15.0, (rgb_color){ 210, 220, 255, 255 });
	offset.y += 50;
	
	for (uint32 i = 0; i < fInfoCount; i++) {
		BString string(fInfos[i]);
		_DrawJigglingString(view, &string, offset, 11.0, (rgb_color){ 180, 185, 200, 255 });
		offset.y += 16.0;
	}

	offset.y += 10;
	_DrawJigglingString(view, &fLicencee, offset, 9.0, (rgb_color){ 150, 160, 180, 255 });
	offset.y += 40;
	_DrawJigglingString(view, &fCopyright, offset, 10.0, (rgb_color){ 150, 160, 180, 255 });

	// clickables
/*	if (fHotRect == 1)
		view->SetHighColor((rgb_color){ 255, 15, 150, 150 });
	else
		view->SetHighColor((rgb_color){ 255, 15, 150, 55 });
	view->SetPenSize(fmod(lrand48(), 7.0));
	view->StrokeRect(randomize_rect(fTuneRect, 30.0));
	if (fHotRect == 2)
		view->SetHighColor((rgb_color){ 255, 15, 150, 150 });
	else
		view->SetHighColor((rgb_color){ 255, 15, 150, 55 });
	view->SetPenSize(fmod(lrand48(), 5.0));
	view->StrokeRect(randomize_rect(fScottRect, 20.0));
	if (fHotRect == 3)
		view->SetHighColor((rgb_color){ 255, 15, 150, 150 });
	else
		view->SetHighColor((rgb_color){ 255, 15, 150, 55 });
	view->SetPenSize(fmod(lrand48(), 9.0));
	view->StrokeRect(randomize_rect(fStippiRect, 30.0));
	view->SetPenSize(1.0);*/
}

// DrawJigglingString
void
AboutView::_DrawJigglingString(BView *view, BString *string,
							  BPoint start, float baseSize, rgb_color baseColor)
{
	char letter[3];
	letter[1] = 0;
	letter[2] = 0;
	BPoint textPoint(start);
	int32 chars = string->Length();
	float *fontSizes = new float[chars];
	float *charSizes = new float[chars];
	float stringWidth = 0.0;
	BFont font;
	view->GetFont(&font);
	for (int32 i = 0; i < chars; i++) {
		fontSizes[i] = floorf(baseSize + fmod(lrand48(), baseSize * 0.30));
		font.SetSize(fontSizes[i]);
		letter[0] = (*string)[i];
		if ((uint8)letter[0] > 127) {
			letter[1] = (*string)[++i];
		}
		charSizes[i] = font.StringWidth(letter) + 1.0;
		stringWidth += charSizes[i];
		letter[1] = 0;
	}
	textPoint.x = view->Bounds().Width() / 2.0 - stringWidth / 2.0;
	for (int32 i = 0; i < chars; i++) {
		view->SetFontSize(fontSizes[i]);
		letter[0] = (*string)[i];
		if ((uint8)letter[0] > 127) {
			letter[1] = (*string)[++i];
		}
		view->SetHighColor(baseColor.red - fmod(lrand48(), baseColor.red * 0.15),
						   baseColor.green - fmod(lrand48(), baseColor.green * 0.15),
						   baseColor.blue - fmod(lrand48(), baseColor.blue * 0.15));
		view->DrawString(letter, textPoint);
		textPoint.x += charSizes[i];
		letter[1] = 0;
	}
	delete[] charSizes;
	delete[] fontSizes;
}

// _DrawJigglingAndRotatingString
void
AboutView::_DrawJigglingAndRotatingString(BView *view, BString *string,
										  BPoint start, float baseSize,
										  rgb_color baseColor)
{
	char letter[2];
	letter[1] = 0;
	BPoint textPoint(start);
	int32 chars = string->Length();
	float *fontSizes = new float[chars];
	float *fontAngles = new float[chars];
	float *charSizes = new float[chars];
	float stringWidth = 0.0;
	BFont font(be_bold_font);
//	view->GetFont(&font);
	for (int32 i = 0; i < chars; i++) {
		fontSizes[i] = floorf(baseSize + fmod(lrand48(), baseSize * 0.50));
		fontAngles[i] = floorf(-15.0 + fmod(lrand48(), 30.0));
		font.SetSize(fontSizes[i]);
		font.SetRotation(fontAngles[i]);
		letter[0] = (*string)[i];
		charSizes[i] = font.StringWidth(letter) + 1.0;
		stringWidth += charSizes[i];
	}
	textPoint.x = view->Bounds().Width() / 2.0 - stringWidth / 2.0;
	for (int32 i = 0; i < chars; i++) {
		if (lrand48() % 16 > 10) {
			font.SetSize(fontSizes[i]);
			font.SetRotation(fontAngles[i]);
			view->SetFont(&font);
			letter[0] = (*string)[i];
			view->SetHighColor(baseColor.red - fmod(lrand48(), baseColor.red * 0.25),
							   baseColor.green - fmod(lrand48(), baseColor.green * 0.25),
							   baseColor.blue - fmod(lrand48(), baseColor.blue * 0.25));
			view->DrawString(letter, textPoint);
		}
		textPoint.x += charSizes[i];
	}
	view->SetFont(be_plain_font);
	delete[] charSizes;
	delete[] fontAngles;
	delete[] fontSizes;
}

