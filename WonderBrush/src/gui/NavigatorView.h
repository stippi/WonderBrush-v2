// NavigatorView.h

#ifndef NAVIGATOR_VIEW_H
#define NAVIGATOR_VIEW_H

#include <View.h>

#include <layout.h>

enum {
	MSG_CENTER_CANVAS		= 'cntc',
};

class BBitmap;

class NavigatorView : public MView, public BView {
 public:
								NavigatorView(const char* name);
	virtual						~NavigatorView();

								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);

								// BView
	virtual	void				Draw(BRect updateRect);
	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
										   const BMessage* dragMessage);

								// NavigatorView
			void				Update(const BBitmap* bitmap,
									   BRect bounds);

			void				SetTarget(BHandler* target);

 private:
			void				_CenterCanvas(BPoint where);

	const	BBitmap*			fBitmap;
			BRect				fBounds;

			bool				fTracking;
			BHandler*			fTarget;
};

#endif // NAVIGATOR_VIEW_H
