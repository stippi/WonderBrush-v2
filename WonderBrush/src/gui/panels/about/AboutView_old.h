// AboutView.h

#ifndef ABOUT_VIEW_H
#define ABOUT_VIEW_H

#include <View.h>
#include <String.h>

class BBitmap;
class BlurFilter;
class BMessageRunner;
class EdgesFilter;
class IFS;

#define BACKGROUND_RECT_NUM 10

class AboutView : public BView {
public:
						AboutView(const char* app,
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

	BMessageRunner*		fTicker;
	BString				fAppName;
	BString				fDevName;
	const char**		fInfos;
	uint32				fInfoCount;
	BString				fLicencee;
	BString				fCopyright;
	BBitmap*			fBackBitmap[2];
	BView*				fBackView[2];
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
};

#endif // ABOUT_VIEW_H
