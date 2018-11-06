// StatusBar.h

/*-----------------------------------------------------------
PURPOSE:
A StatusBar can live on the bottom of a window and can be used
to display messages to the user with optional color code to indicate
the importance of the message

FEATURES:
- smoothly fades back into the default message in order to limit
distraction
- can be used on the left side of a BScrollBar. It then behaves
a little bit different (allows resizing of itslef and the scrollbar).
 -> if scrollBar != NULL, border has no effect, the border will then
be drawn to match the look of a BScrollBar

USAGE:
SetStatus() will display a temporary (error-) message with optional
color code to indicate the type of message (info, warning, error)
-----------------------------------------------------------*/

#ifndef STATUSBAR_H
#define STATUSBAR_H

#include <InterfaceDefs.h>
#include <View.h>

#include <layout.h>

#include <String.h>

class BMessageRunner;

enum status_type {
	STATUS_INFO			= 0,
	STATUS_WARNING		= 1,
	STATUS_ERROR		= 2,
};

class StatusBar : public MView, public BView {
 public:
								StatusBar(const char* defaultMessage = NULL,
										  bool useDoubleBuffering = false,
										  border_style border = B_PLAIN_BORDER,
										  bool fixedHeight = true,
										  BScrollBar* scrollBar = NULL);

	virtual						~StatusBar();

								// BView
	virtual void				AttachedToWindow();
	virtual void				Draw(BRect updateRect);
	virtual void				FrameResized(float width, float height);
	virtual void				MessageReceived(BMessage* message);
	virtual void				MouseDown(BPoint where);
	virtual void				MouseUp(BPoint where);
	virtual void				MouseMoved(BPoint where, uint32 transit,
										   const BMessage* message);

								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect rect);
	virtual	void				reloadfont(BFont* font[]);

								// StatusBar
			void				SetStatus(const char* message,
										  status_type type = STATUS_INFO);
			void				SetDefaultMessage(const char* message);

			void				SetTextColor(const rgb_color& color);

								// You should either start a job
								// and show how much of it is done
								// or use the more general busy
								// indication, don't use both at the
								// same time
			void				JobStarted(const char* message = NULL);
			void				JobProgress(float percent,
											const char* message = NULL);
			void				JobDone();
			bool				IsIndicatingProgress() const;

			void				SetIndicateBusy(bool busy);
			bool				IsIndicatingBusy() const;

 private:
			void				_DrawInto(BView *view, BRect updateRect);
			void				_StartFade();
			void				_FadeTick();
			void				_BusyTick();
			void				_AllocBackBitmap(float width, float height);
			void				_FreeBackBitmap();
			BRect				_ProgressBarFrame() const;
			float				_WidthForMessage() const;
			void				_TruncateString(BString* inOutString,
												uint32 mode,
												float maxWidth);

			BString				fDefaultMessage;
			BString				fTruncatedDefaultMessage;
			BString				fFullMessage;
			BString				fTruncatedMessage;
			rgb_color			fTextColor;
			rgb_color			fBaseColor;
			rgb_color			fFadedColor;

			border_style		fBorderStyle;
			bool				fFixedHeight;

			// the sibling scrollbar which should be resized
			// by this status bar
			BScrollBar*			fScrollBar;

			// from 0.0 to 1.0
			float				fOpacity;
			int32				fDecayDelay;
			BMessageRunner*		fFadePulse;
			BMessageRunner*		fBusyPulse;
			int32				fBusyStep;

			bool				fDragging;
			bool				fHadJobMessage;

			// job progress indication
			float				fJobProgress;
	
			// offscreen buffer
			BBitmap*			fBackBitmap;
			BView*				fBackView;
			bool				fDirty;
};

#endif // STATUSBAR_H
