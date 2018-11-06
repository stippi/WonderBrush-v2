/* (c) 2001 Werner Freytag - please read to the LICENSE file */

#include <stdio.h>

#include <Bitmap.h>
#include <Cursor.h>
#include <MessageRunner.h>
#include <String.h>
#include <Window.h>

#include "support.h"

#include "ColorPreview.h"

const unsigned char kDropperCursor[] = { 16, 1, 14, 1,
										 0x00, 0x0e,
										 0x00, 0x1f,
										 0x00, 0x1f,
										 0x00, 0xff,
										 0x00, 0x7e,
										 0x00, 0xb8,
										 0x01, 0x18,
										 0x03, 0x28,
										 0x04, 0x40,
										 0x0c, 0x80,
										 0x11, 0x00,
										 0x32, 0x00,
										 0x44, 0x00,
										 0x48, 0x00,
										 0x30, 0x00,
										 0x00, 0x00,
										  
										 0x00, 0x0e,
										 0x00, 0x1f,
										 0x00, 0x1f,
										 0x00, 0xff,
										 0x00, 0x7e,
										 0x00, 0xf8,
										 0x01, 0xf8,
										 0x03, 0xe8,
										 0x07, 0xc0,
										 0x0f, 0x80,
										 0x1f, 0x00,
										 0x3e, 0x00,
										 0x7c, 0x00,
										 0x78, 0x00,
										 0x30, 0x00,
										 0x00, 0x00 };

// constructor
ColorPreview::ColorPreview( BRect frame, rgb_color color )
:	BControl( frame, "colorpreview", "", new BMessage(MSG_COLOR_PREVIEW),
		B_FOLLOW_TOP|B_FOLLOW_LEFT,	B_WILL_DRAW ),
	fMouseDown( false ),
	fMessageRunner( 0 )
{
	fColor = color;
	fOldColor = color;
}

// Draw
void
ColorPreview::Draw(BRect updateRect)
{
		
	rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color shadow = tint_color(background, B_DARKEN_1_TINT);
	rgb_color darkShadow = tint_color(background, B_DARKEN_3_TINT);
	rgb_color light = tint_color(background, B_LIGHTEN_MAX_TINT);

	BRect r(Bounds());
	stroke_frame(this, r, shadow, shadow, light, light);
	r.InsetBy(1.0, 1.0);
	stroke_frame(this, r, darkShadow, darkShadow, background, background);
	r.InsetBy(1.0, 1.0);

	r.bottom = r.top + r.Height() / 2.0;
	SetHighColor(fColor);
	FillRect(r);

	r.top = r.bottom + 1;
	r.bottom = Bounds().bottom - 2.0;
	SetHighColor(fOldColor);
	FillRect(r);
}

void ColorPreview::MessageReceived( BMessage *message ) {

	if (message->what==MSG_MESSAGERUNNER) {

		BPoint	where;
		uint32	buttons;

		GetMouse( &where, &buttons );

		DragColor( where );

	}
	else {
#ifdef TARGET_PLATFORM_ZETA
		const char *nameFound;
#else
		char *nameFound;
#endif
		type_code typeFound;
		
		if (message->GetInfo(B_RGB_COLOR_TYPE, 0, &nameFound, &typeFound)!=B_OK) {
			BControl::MessageReceived( message );
			return;
		}
		
	   	rgb_color	*color;
		ssize_t		numBytes;
		message->FindData(nameFound, typeFound, (const void **)&color, &numBytes);
		
		BPoint where;
		bool drop_new = false;
		if (message->FindPoint("_drop_point_", &where)==B_OK) {
			ConvertFromScreen(&where);
			if (where.y > Bounds().top + (Bounds().IntegerHeight()>>1) )
				drop_new = true;
		}
	
		if (drop_new) SetNewColor( *color ); else SetColor( *color );
		Invoke();
	}
}

void ColorPreview::MouseDown( BPoint where ) {

	Window()->Activate();
	
	fMouseDown = true;

	fMessageRunner = new BMessageRunner(this, new BMessage(MSG_MESSAGERUNNER), 300000, 1);

	SetMouseEventMask(B_POINTER_EVENTS, B_SUSPEND_VIEW_FOCUS|B_LOCK_WINDOW_FOCUS );

	BRect rect = Bounds().InsetByCopy(2.0, 2.0);
	rect.top = rect.bottom/2 + 1;
	
	if (rect.Contains( where ) ) {
		fColor = fOldColor;
		Draw( Bounds() );
		Invoke();
	}
	
}
			
void ColorPreview::MouseUp( BPoint where ) {

	delete fMessageRunner;
	fMessageRunner = 0;

	fMouseDown = false;
	BControl::MouseUp( where );

}

// MouseMoved
void 
ColorPreview::MouseMoved( BPoint where, uint32 transit, const BMessage *a_message)
{
	if (transit == B_ENTERED_VIEW) {
		BCursor cursor(kDropperCursor);
		SetViewCursor(&cursor, true);
	}
	if (fMouseDown)
		DragColor( where );
}

// DragColor
void
ColorPreview::DragColor(BPoint where)
{
	BBitmap *bitmap = new BBitmap(BRect(0.0, 0.0, 15.0, 15.0), B_RGB32);
	BMessage message = make_color_drop_message(fColor, bitmap);

	DragMessage(&message, bitmap, B_OP_ALPHA, BPoint(9.0, 9.0));

	MouseUp(where);
}

status_t ColorPreview::Invoke( BMessage *msg ) {
	if (!msg) msg = Message();
	
	msg->RemoveName("color");
	msg->AddData("color", B_RGB_COLOR_TYPE, &fColor, sizeof(fColor));
	
	return BControl::Invoke( msg );
}

void ColorPreview::SetColor( rgb_color color ) {
	color.alpha = 255;
	fColor = color;
	Draw(Bounds());
}

void ColorPreview::SetNewColor( rgb_color color ) {
	fColor = color;
	fOldColor = color;
	Draw(Bounds());
}
