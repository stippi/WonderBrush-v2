// FramedIconButton.h

#ifndef FRAMED_ICON_BUTTON_H
#define FRAMED_ICON_BUTTON_H

#include "IconButton.h"

class FramedIconButton : public IconButton {
 public:
								FramedIconButton(const char* name,
												 uint32 id,
												 const char* label = NULL,
												 BMessage* message = NULL,
												 BHandler* target = NULL);
	virtual						~FramedIconButton();

								// MView
	virtual	minimax				layoutprefs();

								// IconButton
	virtual	bool				DrawBorder() const;
	virtual	void				DrawNormalBorder(BRect r, rgb_color background,
														  rgb_color shadow,
														  rgb_color darkShadow,
														  rgb_color lightShadow,
														  rgb_color light);
	virtual	void				DrawPressedBorder(BRect r, rgb_color background,
														   rgb_color shadow,
														   rgb_color darkShadow,
														   rgb_color lightShadow,
														   rgb_color light);

};

#endif // FRAMED_ICON_BUTTON_H
