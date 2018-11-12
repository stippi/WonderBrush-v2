// AboutView.h

#ifndef ABOUT_VIEW_H
#define ABOUT_VIEW_H

#include <View.h>
#include <String.h>

#include <agg_trans_affine.h>

#include <agg_font_freetype.h>

class BBitmap;
class BlurFilter;
class BMessageRunner;
class EdgesFilter;
class IFS;

#define BACKGROUND_RECT_NUM 10

class AboutView : public BView {
public:
						AboutView(const char* app,
								  const char* version,
								  const char* developer,
								  const char* info[],
								  uint32 infoCount,
								  const char* licencee,
								  const char* copyright);
						~AboutView();

						// BView
	virtual	void		Draw(BRect area);
	virtual	void		MessageReceived(BMessage *msg);
	virtual	void		AttachedToWindow();
	virtual	void		MouseDown(BPoint where);
	virtual	void		MouseMoved(BPoint where, uint32 transit,
								   const BMessage *message);

private:
			void		_AllocBackBitmaps(float width, float height);
			void		_FreeBackBitmaps();
			void		_DrawInto(BView *view);
			void		_DrawJigglingString(BView *into,
											BString *string,
											BPoint start,
											float baseSize,
											rgb_color baseColor);
			void		_DrawJigglingAndRotatingString(BView *into,
													   BString *string,
													   BPoint start,
													   float baseSize,
													   rgb_color baseColor);

			void		_DrawSwirlText(const BBitmap* bitmap,
									   const BPoint& offset,
									   const char* text,
									   float size,
									   rgb_color color,
									   const agg::trans_affine& transform);

	struct point {
		double x;
		double y;
		double direction_x;
		double direction_y;
		double velocity_x;
		double velocity_y;
	};

			void		_DrawPerspectiveText(const BBitmap* bitmap,
											 const BPoint& offset,
											 const char* text,
											 float size,
											 rgb_color color,
											 const point* polygon);


			void		_InitPolygon(const BRect& b,
									 point* polygon) const;
			void		_MorphPolygon(point* polygon);

	BMessageRunner*		fTicker;
	BString				fAppName;
	BString				fVersionName;
	BString				fDevName;
	const char**		fInfos;
	uint32				fInfoCount;
	BString				fLicencee;
	BString				fCopyright;
	BBitmap*			fBackBitmap[2];
	BView*				fBackView[2];

	BBitmap*			fInfoBitmap;

	BRect				fTuneRect;
	BRect				fScottRect;
	BRect				fStippiRect;
	int32				fHotRect;
	uint32				fCurrentBitmap;
	bool				fDirty;
	EdgesFilter*		fEdgesFilter;
	BlurFilter*			fBlurFilter;
	IFS*				fIFS;
	BRect				fBackgroundRects[BACKGROUND_RECT_NUM];

    double				fDistortionCenterX;
    double				fDistortionCenterY;
    double				fDistortionPhase;

	typedef agg::font_engine_freetype_int32				font_engine_type;
	typedef agg::font_cache_manager<font_engine_type>	font_manager_type;

	point				fWBPolygon1[4];
	point				fWBPolygon2[4];
	int32				fWBPolygon2MorphPause;
	point				fWBPolygon3[4];
	int32				fWBPolygon3MorphPause;

	point				fVPolygon1[4];
	point				fVPolygon2[4];
	int32				fVPolygon2MorphPause;

	double				fInitialLeft;
	double				fInitialTop;
	double				fInitialRight;
	double				fInitialBottom;

	font_engine_type	fFontEngine;
	font_manager_type	fFontManager;
};

#endif // ABOUT_VIEW_H
