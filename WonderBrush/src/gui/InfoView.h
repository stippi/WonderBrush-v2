// InfoView.h

#ifndef INFO_VIEW_H
#define INFO_VIEW_H

#include <InterfaceDefs.h>
#include <View.h>

#include <layout.h>

#include <String.h>

class BMessageRunner;
class CanvasView;

class InfoView : public MView, public BView {
 public:
								InfoView(const char* name,
										 border_style border = B_PLAIN_BORDER);

	virtual						~InfoView();

								// BView
	virtual void				Draw(BRect updateRect);
	virtual void				AttachedToWindow();
	virtual void				MessageReceived(BMessage* message);

								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect rect);

								// InfoView
			void				SetInfo(const char* info);
			void				SetCanvasView(CanvasView* view);

								// You should either start a job
								// and show how much of it is done
								// or use the more general busy
								// indication, don't use both at the
								// same time
			void				JobStarted();
			void				JobProgress(float percent);
			void				JobDone();
			bool				IsIndicatingProgress() const;

			void				SetIndicateBusy(bool busy);
			bool				IsIndicatingBusy() const;

 private:
			void				_DrawInto(BView *view, BRect updateRect);
			void				_BusyTick();
			void				_CursorTick();
			BRect				_ProgressBarFrame() const;

			BString				fInfoText;

			border_style		fBorderStyle;

			// general busy indication
			BMessageRunner*		fBusyPulse;
			int32				fBusyStep;

			// job progress indication
			float				fJobProgress;

			// polling of cursor position
			CanvasView*			fCanvasView;
			BMessageRunner*		fCursorPulse;
			int32				fCursorIdle;
			BPoint				fCursorPos;
};

#endif // INFO_VIEW_H
