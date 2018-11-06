//	AboutView.cpp

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <ByteOrder.h>
#include <Entry.h>
#include <Message.h>
#include <Messenger.h>
#include <MessageRunner.h>
#include <Path.h>
#include <Window.h>
#include <Bitmap.h>
#include <Roster.h>
#include <UTF8.h>

#include <agg_basics.h>
#include <agg_bounding_rect.h>
#include <agg_conv_segmentator.h>
#include <agg_conv_transform.h>
#include <agg_conv_clip_polygon.h>
#include <agg_image_filters.h>
#include <agg_pixfmt_rgba.h>
#include <agg_rasterizer_scanline_aa.h>
#include <agg_renderer_mclip.h>
#include <agg_renderer_primitives.h>
#include <agg_renderer_scanline.h>
#include <agg_rendering_buffer.h>
#include <agg_scanline_u.h>
#include <agg_scanline_bin.h>
#include <agg_trans_bilinear.h>
#include <agg_trans_perspective.h>

#include "FontManager.h"
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

// random_number_between
float
random_number_between(float v1, float v2)
{
	if (v1 < v2)
		return v1 + fmod(lrand48() / 1000.0, (v2 - v1));
	else if (v2 < v1)
		return v2 + fmod(lrand48() / 1000.0, (v1 - v2));
	return v1;
}

#define SPEED 3.0

// constructor
AboutView::AboutView(const char* app,
					 const char* version,
					 const char* developer,
					 const char* info[],
					 uint32 infoCount,
					 const char* licencee,
					 const char* copyright)
	: BView(BRect(0.0, 0.0, 400.0, 300.0), "about",
			B_FOLLOW_NONE, B_WILL_DRAW | B_PULSE_NEEDED),
	  fTicker(NULL),
	  fAppName(app),
	  fVersionName(version),
	  fDevName(developer),
	  fInfos(new const char*[infoCount]),
	  fInfoCount(infoCount),
	  fLicencee(licencee),
	  fCopyright(copyright),
	  fInfoBitmap(NULL),
	  fTuneRect(60.0, 115.0, 175.0, 145.0),
	  fScottRect(100.0, 195.0, 245.0, 280.0),
	  fStippiRect(255.0, 175.0, 330.0, 230.0),
	  fCurrentBitmap(0),
	  fDirty(false),
	  fEdgesFilter(new EdgesFilter()),
	  fBlurFilter(new BlurFilter()),
	  fIFS(new IFS(Bounds())),
	  fDistortionPhase(0.0),
	  fWBPolygon2MorphPause(10),
	  fWBPolygon3MorphPause(20),
	  fVPolygon2MorphPause(10),
	  fFontEngine(),
	  fFontManager(fFontEngine)
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
	// init font engine
	fFontEngine.flip_y(true);
	FontManager* fm = FontManager::Default();
	font_family family;
	font_style style;
	be_bold_font->GetFamilyAndStyle(&family, &style);
	BPath path(fm->FontFileFor(family, style));
//	BPath path(fm->FontFileFor("CommercialScript BT", "Regular"));
	if (path.InitCheck() >= B_OK) {
		if (!fFontEngine.load_font(path.Path(), 0, agg::glyph_ren_outline))
			fprintf(stderr, "failed to load font file\n");
	} else
		fprintf(stderr, "failed to get path to font file\n");

	// init perspective polygon
	BRect b(Bounds());
	b.InsetBy(50.0, 60.0);
	fInitialLeft = b.left;
	fInitialTop = b.top;
	fInitialRight = b.right;
	fInitialBottom = b.bottom;

	_InitPolygon(b, fWBPolygon1);
	memcpy(fWBPolygon2, fWBPolygon1, sizeof(point) * 4);
	memcpy(fWBPolygon3, fWBPolygon1, sizeof(point) * 4);

	_InitPolygon(b, fVPolygon1);
	memcpy(fVPolygon2, fVPolygon1, sizeof(point) * 4);
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
			uint32 previousBuffer = fCurrentBitmap;
			fCurrentBitmap = fCurrentBitmap + 1;
			if (fCurrentBitmap >= 2)
				fCurrentBitmap = 0;

			memset(fBackBitmap[previousBuffer]->Bits(), 0,
				fBackBitmap[previousBuffer]->BitsLength());
			fIFS->Draw(fBackBitmap[previousBuffer]);
			fBlurFilter->Run(fBackBitmap[previousBuffer],
							  fBackBitmap[fCurrentBitmap]);

			// flip buffers
			if (fBackBitmap[fCurrentBitmap]->Lock()) {
				_DrawInto(fBackView[fCurrentBitmap]);
				fBackView[fCurrentBitmap]->Sync();
				fBackBitmap[fCurrentBitmap]->Unlock();
			}
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

		fDistortionPhase += 15.0 * agg::pi / 180.0;
        if (fDistortionPhase > agg::pi * 200.0)
        	fDistortionPhase -= agg::pi * 200.0;

		// animate all polygons
		_MorphPolygon(fWBPolygon1);

		if (fWBPolygon2MorphPause > 0)
			fWBPolygon2MorphPause--;
		else
			_MorphPolygon(fWBPolygon2);

		if (fWBPolygon3MorphPause > 0)
			fWBPolygon3MorphPause--;
		else
			_MorphPolygon(fWBPolygon3);

		_MorphPolygon(fVPolygon1);

		if (fVPolygon2MorphPause > 0)
			fVPolygon2MorphPause--;
		else
			_MorphPolygon(fVPolygon2);

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

// center_string
static void
center_string(BView* view, float size, const char* string, BPoint& offset)
{
	view->SetFontSize(size);
	float stringWidth = view->StringWidth(string);
	offset.x = view->Bounds().Width() / 2.0 - (stringWidth * 1.05) / 2.0;
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
//	view->FillRect(r);
	r.top = r.bottom + 1.0;
	r.bottom = view->Bounds().bottom;
	backGround.alpha = 51;
	view->SetHighColor(backGround);
//	view->FillRect(r);

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
	BPoint offset(0.0, 140.0);
	agg::trans_affine transform;
//	_DrawJigglingAndRotatingString(view, &fAppName, offset, 30.0,
//								   (rgb_color){ 255, 255, 255, 255 });
	// app name
	BPoint wbOffset = offset + BPoint(-30.0, 0);
	_DrawPerspectiveText(fBackBitmap[fCurrentBitmap],
						 wbOffset, fAppName.String(), 80.0,
						 (rgb_color){ 255, 50, 80, 120 },
						 fWBPolygon1);
	_DrawPerspectiveText(fBackBitmap[fCurrentBitmap],
						 wbOffset + BPoint(-5.0, 5.0), fAppName.String(), 90.0,
						 (rgb_color){ 200, 10, 120, 70 },
						 fWBPolygon2);
	_DrawPerspectiveText(fBackBitmap[fCurrentBitmap],
						 wbOffset + BPoint(-10.0, 10.0), fAppName.String(), 100.0,
						 (rgb_color){ 180, 70, 150, 40 },
						 fWBPolygon3);

	// version
	_DrawPerspectiveText(fBackBitmap[fCurrentBitmap],
						 offset, fVersionName.String(), 50.0,
						 (rgb_color){ 100, 70, 150, 180 },
						 fVPolygon1);
	_DrawPerspectiveText(fBackBitmap[fCurrentBitmap],
						 offset + BPoint(-10.0, 5.0), fVersionName.String(), 60.0,
						 (rgb_color){ 80, 50, 180, 80 },
						 fVPolygon2);
	offset.y = 70;
//	_DrawJigglingString(view, &fDevName, offset, 15.0, (rgb_color){ 210, 220, 255, 255 });
//	view->SetFontSize(15.0);
//	view->SetHighColor(210, 220, 255, 100);
//	view->DrawString(fDevName.String(), offset);

	float fontSize = 15.0;
	center_string(view, fontSize, fDevName.String(), offset);

	transform *= agg::trans_affine_rotation(-0.02);
	_DrawSwirlText(fBackBitmap[fCurrentBitmap],
				   offset, fDevName.String(), fontSize,
				   (rgb_color){ 255, 220, 210, 200 },
				   transform);

	center_string(view, fontSize, fDevName.String(), offset);
	offset.x -= 50;
	offset.y += 10;
	transform *= agg::trans_affine_rotation(-0.03);
	transform *= agg::trans_affine_scaling(1.9, 2.6);
	transform *= agg::trans_affine_translation(-30.0, -100.0);
	_DrawSwirlText(fBackBitmap[fCurrentBitmap],
				   offset, fDevName.String(), fontSize,
				   (rgb_color){ 255, 220, 210, 30 },
				   transform);

	offset.y += 20;
	
	fontSize = 13.0;
	for (uint32 i = 0; i < fInfoCount; i++) {
//		BString string(fInfos[i]);
//		_DrawJigglingString(view, &string, offset, 11.0, (rgb_color){ 180, 185, 200, 255 });
		center_string(view, fontSize, fInfos[i], offset);
		_DrawSwirlText(fBackBitmap[fCurrentBitmap],
					   offset, fInfos[i], fontSize,
					   (rgb_color){ 200, 185, 180, 190 },
					   agg::trans_affine_rotation(0.013 * (float)(i + 1)));
		offset.y += fontSize * 1.1;
	}

	fontSize = 10.0;
	center_string(view, fontSize, fLicencee.String(), offset);
	offset.y += 40;
//	_DrawJigglingString(view, &fLicencee, offset, 9.0, (rgb_color){ 150, 160, 180, 200 });
	_DrawSwirlText(fBackBitmap[fCurrentBitmap],
				   offset, fLicencee.String(), fontSize,
				   (rgb_color){ 180, 160, 150, 200 },
				   agg::trans_affine_rotation(-0.02));
	fontSize = 11.0;
	center_string(view, fontSize, fCopyright.String(), offset);
	offset.y += 30;
//	_DrawJigglingString(view, &fCopyright, offset, 10.0, (rgb_color){ 150, 160, 180, 160 });
	_DrawSwirlText(fBackBitmap[fCurrentBitmap],
				   offset, fCopyright.String(), fontSize,
				   (rgb_color){ 180, 160, 150, 160 },
				   agg::trans_affine_rotation(0.002));

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

/*	view->SetHighColor(255, 0, 0, 200);
	view->StrokeLine(BPoint(fWBPolygon1[0].x, fWBPolygon1[0].y),
					 BPoint(fWBPolygon1[1].x, fWBPolygon1[1].y));
	view->StrokeLine(BPoint(fWBPolygon1[1].x, fWBPolygon1[1].y),
					 BPoint(fWBPolygon1[2].x, fWBPolygon1[2].y));
	view->StrokeLine(BPoint(fWBPolygon1[2].x, fWBPolygon1[2].y),
					 BPoint(fWBPolygon1[3].x, fWBPolygon1[3].y));
	view->StrokeLine(BPoint(fWBPolygon1[3].x, fWBPolygon1[3].y),
					 BPoint(fWBPolygon1[0].x, fWBPolygon1[0].y));*/
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

inline void calculate_wave(double* x, double* y, 
                           double cx, double cy, 
                           double period, double amplitude, double phase)
{
    double xd = *x - cx;
    double yd = *y - cy;
    double d = sqrt(xd*xd + yd*yd);
    if(d > 1)
    {
        double a = cos(d / (16.0 * period) - phase) * (1.0 / (amplitude * d)) + 1.0; 
        *x = xd * a + cx;
        *y = yd * a + cy;
    }
}

class periodic_distortion
{
public:
    periodic_distortion() :
        m_cx(0.0),
        m_cy(0.0),
        m_period(0.5),
        m_amplitude(0.5),
        fDistortionPhase(0.0)
    {}

    void center(double x, double y) { m_cx = x; m_cy = y; }
    void period(double v)           { m_period = v; }
    void amplitude(double v)        { m_amplitude = 1.0 / v; }
    void phase(double v)            { fDistortionPhase = v; }

    void transform(double* x, double* y) const
    {
    	calculate_wave(x, y, m_cx, m_cy, m_period, m_amplitude, fDistortionPhase);
    	m_transform.transform(x, y);
    }

	void set_additional_transform(const agg::trans_affine& transform)
	{
		m_transform = transform;
	}

protected:
    double m_cx;
    double m_cy;
    double m_period;
    double m_amplitude;
    double fDistortionPhase;

	agg::trans_affine	m_transform;
};




// _DrawSwirlText
void
AboutView::_DrawSwirlText(const BBitmap* bitmap, const BPoint& offset,
						  const char* string, float size, rgb_color color,
						  const agg::trans_affine& transform)
{
	fFontEngine.height((int32)(size));

	typedef agg::pixfmt_bgra32												pixfmt_type;
	typedef agg::renderer_base<pixfmt_type>									base_ren_type;
	typedef agg::renderer_scanline_aa_solid<base_ren_type>					renderer_solid;
//	typedef agg::renderer_scanline_u_solid<base_ren_type>					renderer_solid;
	typedef agg::renderer_scanline_bin_solid<base_ren_type>					renderer_bin;
	
	typedef agg::conv_curve<font_manager_type::path_adaptor_type>			conv_font_curve_type;
//	typedef agg::conv_segmentator<conv_font_curve_type>						conv_font_segm_type;
//	typedef agg::conv_transform<conv_font_segm_type, agg::trans_affine>		conv_font_trans_type;
//	typedef agg::conv_transform<conv_font_curve_type, agg::trans_affine>	conv_font_trans_type;
	typedef agg::conv_transform<conv_font_curve_type, periodic_distortion>	conv_font_trans_type;
	

	// set up the renderser
	agg::rendering_buffer ren_buffer;
	ren_buffer.attach((uint8*)bitmap->Bits(),
					  bitmap->Bounds().IntegerWidth() + 1,
					  bitmap->Bounds().IntegerHeight() + 1,
					  bitmap->BytesPerRow());

	pixfmt_type pf(ren_buffer);
	base_ren_type ren_base(pf);
	renderer_solid ren_solid(ren_base);
	renderer_bin ren_bin(ren_base);
	
	agg::scanline_u8 sl;
	agg::rasterizer_scanline_aa<> ras;

	periodic_distortion dist;
	dist.phase(fDistortionPhase);
	dist.period(2.22);
	dist.amplitude(1.35);
//	dist.center(cx, cy);
	dist.set_additional_transform(transform);

	conv_font_curve_type fcurves(fFontManager.path_adaptor());
//	fcurves.approximation_scale(transform.scale());
//	conv_font_segm_type  fsegm(fcurves);
//	conv_font_trans_type ftrans(fsegm, transform);
//	conv_font_trans_type ftrans(fcurves, transform);
	conv_font_trans_type ftrans(fcurves, dist);


	// do a UTF8 -> Unicode conversion
	int32 srcLength = strlen(string);
	int32 dstLength = srcLength * 4;

	char* buffer = new char[dstLength];

	int32 state = 0;
	status_t ret;
	if ((ret = convert_from_utf8(B_UNICODE_CONVERSION, 
								 string, &srcLength,
								 buffer, &dstLength,
								 &state, B_SUBSTITUTE)) >= B_OK
		&& (ret = swap_data(B_INT16_TYPE, buffer, dstLength,
							B_SWAP_BENDIAN_TO_HOST)) >= B_OK) {

		uint16* p = (uint16*)buffer;

		double x  = offset.x;//0.0;
		double y0 = offset.y;//0.0;
		double y  = y0;

		double advanceX = 0.0;
		double advanceY = 0.0;

		agg::rgba8 c(color.red, color.green, color.blue, color.alpha);
		ren_bin.color(c);
		ren_solid.color(c);

		for (int32 i = 0; i < dstLength / 2; i++) {

			// line break
			if (*p == '\n') {
				y0 += size;
				x = offset.x;//0.0;
				y = y0;
				advanceX = 0.0;
				advanceY = 0.0;
				++p;
				continue;
			}

			const agg::glyph_cache* glyph = fFontManager.glyph(*p);

			if (glyph) {
				fFontManager.add_kerning(&advanceX, &advanceY);
				x += advanceX;
				y += advanceY;

				fFontManager.init_embedded_adaptors(glyph, x, y);
	
				switch(glyph->data_type) {
					case agg::glyph_data_mono:
						agg::render_scanlines(fFontManager.mono_adaptor(), 
											  fFontManager.mono_scanline(), 
											  ren_bin);
						break;
		
					case agg::glyph_data_gray8:
						agg::render_scanlines(fFontManager.gray8_adaptor(), 
											  fFontManager.gray8_scanline(), 
											  ren_solid);
						break;
		
					case agg::glyph_data_outline:
						ras.reset();
						if(fabs(0.0) <= 0.01) {
							// For the sake of efficiency skip the
							// contour converter if the weight is about zero.
							//-----------------------
//							ras.add_path(fCurves);
							ras.add_path(ftrans);
						} else {
//							ras.add_path(fContour);
							ras.add_path(ftrans);
						}
						agg::render_scanlines(ras, sl, ren_solid);
						break;
					default:
						break;
				}
	
				// increment pen position
				advanceX = glyph->advance_x;
				advanceY = glyph->advance_y;
			}
			++p;
		}
	} else {
		fprintf(stderr, "UTF8 -> Unicode conversion failed: %s\n", strerror(ret));
	}
	delete[] buffer;
}

// _DrawPerspectiveText
void
AboutView::_DrawPerspectiveText(const BBitmap* bitmap, const BPoint& offset,
								const char* string, float size, rgb_color color,
								const point* polygon)
{
	fFontEngine.height((int32)(size));

	typedef agg::pixfmt_bgra32												pixfmt_type;
	typedef agg::renderer_base<pixfmt_type>									base_ren_type;
	typedef agg::renderer_scanline_aa_solid<base_ren_type>					renderer_solid;
//	typedef agg::renderer_scanline_u_solid<base_ren_type>					renderer_solid;
	typedef agg::renderer_scanline_bin_solid<base_ren_type>					renderer_bin;
	
	typedef agg::conv_curve<font_manager_type::path_adaptor_type>			conv_font_curve_type;
//	typedef agg::conv_segmentator<conv_font_curve_type>						conv_font_segm_type;
//	typedef agg::conv_transform<conv_font_segm_type, agg::trans_affine>		conv_font_trans_type;
//	typedef agg::conv_transform<conv_font_curve_type, agg::trans_affine>	conv_font_trans_type;
	typedef agg::conv_transform<conv_font_curve_type, agg::trans_perspective>	conv_font_trans_type;
	

	// set up the renderser
	agg::rendering_buffer ren_buffer;
	ren_buffer.attach((uint8*)bitmap->Bits(),
					  bitmap->Bounds().IntegerWidth() + 1,
					  bitmap->Bounds().IntegerHeight() + 1,
					  bitmap->BytesPerRow());

	pixfmt_type pf(ren_buffer);
	base_ren_type ren_base(pf);
	renderer_solid ren_solid(ren_base);
	renderer_bin ren_bin(ren_base);
	
	agg::scanline_u8 sl;
	agg::rasterizer_scanline_aa<> ras;

	double quad[8];
	quad[0] = polygon[0].x;
	quad[1] = polygon[0].y;
	quad[2] = polygon[1].x;
	quad[3] = polygon[1].y;
	quad[4] = polygon[2].x;
	quad[5] = polygon[2].y;
	quad[6] = polygon[3].x;
	quad[7] = polygon[3].y;
//	agg::trans_bilinear tr(fInitialLeft, fInitialTop, fInitialRight, fInitialBottom,
	agg::trans_perspective tr(fInitialLeft, fInitialTop, fInitialRight, fInitialBottom,
						   quad);
	if (!tr.is_valid())
		return;

	conv_font_curve_type fcurves(fFontManager.path_adaptor());
	fcurves.approximation_scale(3.0);
//	conv_font_segm_type  fsegm(fcurves);
//	conv_font_trans_type ftrans(fsegm, transform);
//	conv_font_trans_type ftrans(fcurves, transform);
	conv_font_trans_type ftrans(fcurves, tr);
	BRect b(bitmap->Bounds());
	ras.clip_box(b.left, b.top, b.right + 1, b.bottom + 1);


	// do a UTF8 -> Unicode conversion
	int32 srcLength = strlen(string);
	int32 dstLength = srcLength * 4;

	char* buffer = new char[dstLength];

	int32 state = 0;
	status_t ret;
	if ((ret = convert_from_utf8(B_UNICODE_CONVERSION, 
								 string, &srcLength,
								 buffer, &dstLength,
								 &state, B_SUBSTITUTE)) >= B_OK
		&& (ret = swap_data(B_INT16_TYPE, buffer, dstLength,
							B_SWAP_BENDIAN_TO_HOST)) >= B_OK) {

		uint16* p = (uint16*)buffer;

		double x  = offset.x;//0.0;
		double y0 = offset.y;//0.0;
		double y  = y0;

		double advanceX = 0.0;
		double advanceY = 0.0;

		agg::rgba8 c(color.red, color.green, color.blue, color.alpha);
		ren_bin.color(c);
		ren_solid.color(c);

		for (int32 i = 0; i < dstLength / 2; i++) {

			// line break
			if (*p == '\n') {
				y0 += size;
				x = offset.x;//0.0;
				y = y0;
				advanceX = 0.0;
				advanceY = 0.0;
				++p;
				continue;
			}

			const agg::glyph_cache* glyph = fFontManager.glyph(*p);

			if (glyph) {
				fFontManager.add_kerning(&advanceX, &advanceY);
				x += advanceX;
				y += advanceY;

				fFontManager.init_embedded_adaptors(glyph, x, y);
	
				switch(glyph->data_type) {
					case agg::glyph_data_mono:
						agg::render_scanlines(fFontManager.mono_adaptor(), 
											  fFontManager.mono_scanline(), 
											  ren_bin);
						break;
		
					case agg::glyph_data_gray8:
						agg::render_scanlines(fFontManager.gray8_adaptor(), 
											  fFontManager.gray8_scanline(), 
											  ren_solid);
						break;
		
					case agg::glyph_data_outline:
						ras.reset();
						if(fabs(0.0) <= 0.01) {
							// For the sake of efficiency skip the
							// contour converter if the weight is about zero.
							//-----------------------
//							ras.add_path(fCurves);
							ras.add_path(ftrans);
						} else {
//							ras.add_path(fContour);
							ras.add_path(ftrans);
						}
						agg::render_scanlines(ras, sl, ren_solid);
						break;
					default:
						break;
				}
	
				// increment pen position
				advanceX = glyph->advance_x;
				advanceY = glyph->advance_y;
			}
			++p;
		}
	} else {
		fprintf(stderr, "UTF8 -> Unicode conversion failed: %s\n", strerror(ret));
	}
	delete[] buffer;
}

// _InitPolygon
void
AboutView::_InitPolygon(const BRect& b, point* polygon) const
{
	polygon[0].x = b.left;
	polygon[0].y = b.top;
	polygon[0].direction_x = random_number_between(-SPEED, SPEED);
	polygon[0].direction_y = random_number_between(-SPEED, SPEED);
	polygon[0].velocity_x = 0.0;
	polygon[0].velocity_y = 0.0;
	polygon[1].x = b.right;
	polygon[1].y = b.top;
	polygon[1].direction_x = random_number_between(-SPEED, SPEED);
	polygon[1].direction_y = random_number_between(-SPEED, SPEED);
	polygon[1].velocity_x = 0.0;
	polygon[1].velocity_y = 0.0;
	polygon[2].x = b.right;
	polygon[2].y = b.bottom;
	polygon[2].direction_x = random_number_between(-SPEED, SPEED);
	polygon[2].direction_y = random_number_between(-SPEED, SPEED);
	polygon[2].velocity_x = 0.0;
	polygon[2].velocity_y = 0.0;
	polygon[3].x = b.left;
	polygon[3].y = b.bottom;
	polygon[3].direction_x = random_number_between(-SPEED, SPEED);
	polygon[3].direction_y = random_number_between(-SPEED, SPEED);
	polygon[3].velocity_x = 0.0;
	polygon[3].velocity_y = 0.0;
}

// morph
inline void
morph(double* value, double* direction, double* velocity, double min, double max)
{
	*value += *velocity;

	// flip direction if necessary
	if (*value < min && *direction < 0.0) {
		*direction = -*direction;
	} else if (*value > max && *direction > 0.0) {
		*direction = -*direction;
	}

	// accelerate velocity
	if (*direction < 0.0) {
		if (*velocity > *direction)
			*velocity += *direction / 10.0;
		// truncate velocity
		if (*velocity < *direction)
			*velocity = *direction;
	} else {
		if (*velocity < *direction)
			*velocity += *direction / 10.0;
		// truncate velocity
		if (*velocity > *direction)
			*velocity = *direction;
	}
}

// _MorphPolygon
void
AboutView::_MorphPolygon(point* polygon)
{
	BRect b(Bounds());
//	b.InsetBy(50.0, 60.0);

	float centerX = (b.left + b.right) / 2.0;
	float centerY = (b.top + b.bottom) / 2.0;

	morph(&polygon[0].x, &polygon[0].direction_x, &polygon[0].velocity_x, b.left, centerX);
	morph(&polygon[1].x, &polygon[1].direction_x, &polygon[1].velocity_x, centerX, b.right);
	morph(&polygon[2].x, &polygon[2].direction_x, &polygon[2].velocity_x, centerX, b.right);
	morph(&polygon[3].x, &polygon[3].direction_x, &polygon[3].velocity_x, b.left, centerX);
	morph(&polygon[0].y, &polygon[0].direction_y, &polygon[0].velocity_y, b.top, centerY);
	morph(&polygon[1].y, &polygon[1].direction_y, &polygon[1].velocity_y, b.top, centerY);
	morph(&polygon[2].y, &polygon[2].direction_y, &polygon[2].velocity_y, centerY, b.bottom);
	morph(&polygon[3].y, &polygon[3].direction_y, &polygon[3].velocity_y, centerY, b.bottom);
}
