// PopupView.h

#ifndef POPUP_VIEW_H
#define POPUP_VIEW_H

#include <View.h>

#include <layout.h>

class PopupWindow;

class PopupView : public MView, public BView {
 public:
								PopupView(const char* name);
	virtual						~PopupView();

								// MView
	virtual	minimax				layoutprefs() = 0;
	virtual	BRect				layout(BRect frame) = 0;

	virtual	void				SetPopupWindow(PopupWindow* window);

	virtual	void				PopupDone(bool canceled);

 private:
			PopupWindow*		fWindow;
};


#endif	// POPUP_CONTROL_H
