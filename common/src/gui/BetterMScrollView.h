// BetterMScrollView.h

#ifndef BETTER_M_SCROLL_VIEW_H
#define BETTER_M_SCROLL_VIEW_H

#include <MScrollView.h>

class BetterMScrollView : public MScrollView {
 public:
								BetterMScrollView(MView* target,
												  bool horizontal = false,
												  bool vertical = false,
												  border_style border = B_FANCY_BORDER,
												  minimax size = 0);


								// BView
	virtual	void				AttachedToWindow();

#ifdef TARGET_PLATFORM_ZETA

	virtual void				WindowActivated(bool state);
	virtual	void				Draw(BRect updateRect);

 private:
			bool				fWindowActive;

#endif // TARGET_PLATFORM_ZETA
};



#endif	// BETTER_M_SCROLL_VIEW_H
