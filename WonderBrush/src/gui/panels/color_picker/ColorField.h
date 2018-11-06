#ifndef _COLOR_FIELD_H
#define _COLOR_FIELD_H

#include <Control.h>

#include <layout.h>

#include "selected_color_mode.h"

enum {
	MSG_COLOR_FIELD		= 'ColF',
};

class BBitmap;

class ColorField : public MView, public BControl {
 public:
								ColorField(BPoint offset_point,
										   selected_color_mode mode,
										   float fixed_value,
										   orientation orient = B_VERTICAL);
	virtual						~ColorField();

								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);

								// BControl
	virtual	status_t			Invoke(BMessage *msg = NULL);

	virtual	void				AttachedToWindow();
	virtual	void				Draw(BRect updateRect);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 code,
										   const BMessage* message);

								// ColorField
			void				Update(int depth);

			void				SetModeAndValue(selected_color_mode mode, float fixed_value);
			void				SetFixedValue(float fixed_value);
			float				FixedValue() const
									{ return fFixedValue; }
			
			void				SetMarkerToColor( rgb_color color );
			void				PositionMarkerAt( BPoint where );

			float				Width() const;
			float				Height() const;
			bool				IsTracking() const
									{ return fMouseDown; }

 private:
	static	int32				_UpdateThread(void* data);
			void				_DrawBorder();

	selected_color_mode			fMode;
	float						fFixedValue;
	orientation					fOrientation;

	BPoint						fMarkerPosition;
	BPoint						fLastMarkerPosition;

	bool						fMouseDown;

	BBitmap*					fBgBitmap[2];
	BView*						fBgView[2];

	thread_id					fUpdateThread;
	port_id						fUpdatePort;
};

#endif
