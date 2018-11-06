#ifndef _COLOR_PREVIEW_H
#define _COLOR_PREVIEW_H

#include <Control.h>

#define	MSG_COLOR_PREVIEW	'ColP'
#define MSG_MESSAGERUNNER 	'MsgR'

class BMessageRunner;

class ColorPreview : public BControl {

public:

			ColorPreview( BRect frame, rgb_color color );
void		Draw( BRect updateRect );

void		MessageReceived( BMessage *message );
void		MouseDown( BPoint where );
void		MouseUp(BPoint where);
void		MouseMoved(	BPoint where, uint32 code, const BMessage *a_message);

status_t	Invoke( BMessage *msg = NULL );

void		SetColor( rgb_color color );	// ändert die aktuelle Farbe
void		SetNewColor( rgb_color color );	// ändert auch die alte Farbe

private:

void			DragColor( BPoint where );
rgb_color		fColor, fOldColor;
bool			fMouseDown;
BMessageRunner	*fMessageRunner;
};

#endif

