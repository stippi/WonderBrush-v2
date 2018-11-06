// MenuBar.h

#ifndef MENU_BAR_H
#define MENU_BAR_H

/*
#include <MenuBar.h>

#include <layout.h>

class MenuBar : public MView, public BMenuBar {
 public:
								MenuBar(const char* name);*/

#include <MMenuBar.h>

class MenuBar : public MMenuBar {
 public:
								MenuBar(const char* name);
	virtual						~MenuBar();

								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect rect);

 private:
};

#endif // MENU_BAR_H
