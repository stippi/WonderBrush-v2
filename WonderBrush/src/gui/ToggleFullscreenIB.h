// ToggleFullscreenIB.h

#ifndef TOGGLE_FULLSCREEN_ICON_BUTTON_H
#define TOGGLE_FULLSCREEN_ICON_BUTTON_H

#include "FramedIconButton.h"

class ToggleFullscreenIB : public FramedIconButton {
 public:
								ToggleFullscreenIB(BMessage* message,
												   BHandler* target);
	virtual						~ToggleFullscreenIB();

			void				SetFullscreen(bool full);
};

#endif // TOGGLE_FULLSCREEN_ICON_BUTTON_H
