// DualSlider.h

#ifndef DUAL_SLIDER_H
#define DUAL_SLIDER_H

#include <View.h>
#include <String.h>

#include <layout.h>

class DualSlider : public MView, public BView {
 public:
								DualSlider(const char* name,
										   const char* label = NULL,
										   BMessage* valueMessage = NULL,
										   BMessage* controlMessage = NULL,
										   BHandler* target = NULL,
										   float minValue = 0.0,
										   float maxValue = 1.0);
	virtual						~DualSlider();

								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);

								// BView
	virtual	void				Draw(BRect updateRect);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
										   const BMessage* dragMessage);

								// DualSlider
			void				SetLabel(const char* label);

			void				SetValues(float min, float max);

			void				SetMinValue(float value);
			float				MinValue() const
									{ return fMinValue; }

			void				SetMaxValue(float value);
			float				MaxValue() const
									{ return fMaxValue; }

			void				SetEnabled(bool enable);
			bool				IsEnabled() const;

			void				SetMinEnabled(bool enable, 
											  bool sendMessage = false);
			bool				IsMinEnabled() const;

			void				SetMaxEnabled(bool enable);
			bool				IsMaxEnabled() const;

			void				SetPressureControlTip(const char* text);

 private:
	enum {
		MOUSE_DOWN,
		MOUSE_MOVED,
		MOUSE_UP,
	};


			void				_Invoke(BMessage* message,
										uint32 mode = MOUSE_MOVED);
			void				_Invalidate();
			void				_StrokeRect(BRect frame, rgb_color leftTop,
										   rgb_color rightBottom);

			float				_ValueFor(BPoint where) const;
			BRect				_BarFrame() const;
			BRect				_SliderFrame() const;
			BRect				_PressureBoxFrame() const;
			float				_LabelHeight() const;

			float				fMinValue;
			float				fMaxValue;
			float				fLastFactor;

			uint32				fFlags;

			BMessage*			fValueMessage;
			BMessage*			fControlMessage;
			BHandler*			fTarget;

			BString				fLabel;
			BString				fPressureControlTip;
};

#endif // DUAL_SLIDER_H
