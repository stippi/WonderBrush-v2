// GradientControl.h

#ifndef GRADIENT_CONTROL_H
#define GRADIENT_CONTROL_H

#include <View.h>

#include <layout.h>

class Gradient;

enum {
	MSG_GRADIENT_CONTROL_FOCUS_CHANGED	= 'gcfc',
};

class GradientControl : public MView,
						public BView {

 public:
								GradientControl();
	virtual						~GradientControl();

								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);

								// BView
	virtual	void				WindowActivated(bool active);
	virtual	void				MakeFocus(bool focus);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
										   const BMessage* dragMessage);

	virtual	void				MessageReceived(BMessage* message);
	virtual	void				KeyDown(const char* bytes, int32 numBytes);

	virtual	void				Draw(BRect updateRect);
	virtual	void				FrameResized(float width, float height);

								// GradientControl
			void				SetGradient(const Gradient* gradient);
			Gradient*			GetGradient() const
									{ return fGradient; }

			void				SetCurrentStop(const rgb_color& color);
			bool				GetCurrentStop(rgb_color* color) const;

			void				SetEnabled(bool enabled);
			bool				IsEnabled() const
									{ return fEnabled; }

 private:
			void				_UpdateColors();
			void				_AllocBitmap(int32 width, int32 height);
			BRect				_GradientBitmapRect() const;
			int32				_StepIndexFor(BPoint where) const;
			float				_OffsetFor(BPoint where) const;
			void				_UpdateCanvasColor() const;

 			Gradient*			fGradient;
			BBitmap*			fGradientBitmap;
			int32				fDraggingStepIndex;
			int32				fCurrentStepIndex;

			float				fDropOffset;
			int32				fDropIndex;

			bool				fEnabled;
};

#endif // GRADIENT_CONTROL_H
