// LabelPopup.cpp

#include <stdio.h>

#include <MenuBar.h>
#include <MenuItem.h>
#include <OS.h>
#include <PopUpMenu.h>

#include "LabelPopup.h"
#include "Colors.h"

#define LABEL_DIST 8.0
#define DIVIDER_DIST 3.0

enum {
	SYSTEM_R5			= 0,
	SYSTEM_DANO			= 1,
};
/*
static int32
get_system_version()
{
	system_info info;
	if (get_system_info(&info) >= B_OK) {
		printf("kernel_build_date: %s\n", info.kernel_build_date);
		if (info.kernel_version >= 11305)
			return SYSTEM_DANO;
	}
	return SYSTEM_R5;
}*/

int32
LabelPopup::sSystemVersion = SYSTEM_R5;

// constructor
LabelPopup::LabelPopup(const char* label, BMenu* menu,
					   bool fixedSize, bool asLabel)
	: MDividable(),
	  BMenuField(_PreferredSize(label, menu, asLabel),
				 label, label, menu ? menu : new BPopUpMenu("popup"), fixedSize,
				 B_FOLLOW_NONE, B_WILL_DRAW | B_NAVIGABLE),
	  fIsLabel(asLabel)
{
	if (Menu()) {
		Menu()->SetRadioMode(true);
		MenuItem()->SetMarked(false);
	}
	const BFont* labelFont = fIsLabel ? be_bold_font : be_plain_font;
	SetFont(labelFont);
	SetDivider(labelFont->StringWidth(label) + LABEL_DIST);
	if (MenuBar())
		MenuBar()->ResizeToPreferred();
	ResizeToPreferred();
}

// destructor
LabelPopup::~LabelPopup()
{
}

// layoutprefs
minimax
LabelPopup::layoutprefs()
{
	BRect r = _PreferredSize(Label(), Menu(), fIsLabel);
	mpm.mini.x = r.Width() + 1.0;
	mpm.maxi.x = mpm.mini.x + 10000.0;
	mpm.maxi.y = mpm.mini.y = r.Height() + 2.0;
	mpm.weight = fIsLabel ? 0.0 : 1.0;
	return mpm;
}

// layout
BRect
LabelPopup::layout(BRect rect)
{
	ResizeTo(rect.Width(), rect.Height());
	MoveTo(rect.LeftTop());
	_LayoutMenuBar();
	return Frame();
}

// LabelWidth
float
LabelPopup::LabelWidth()
{
	float width = StringWidth(Label());
	if (width > 0.0)
		width += LABEL_DIST - DIVIDER_DIST;
	return width;
}

// AttachedToWindow
void
LabelPopup::AttachedToWindow()
{
	BMenuField::AttachedToWindow();
//	SetViewColor(255, 0, 0, 255);
//	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
//	SetLowColor(ViewColor());
}

// SetLabel
void
LabelPopup::SetLabel(const char* label)
{
	BMenuField::SetLabel(label);
	float width = LabelWidth();
	if (rolemodel)
		labelwidth = rolemodel->LabelWidth() > labelwidth ?
						rolemodel->LabelWidth() : labelwidth;

	labelwidth = width > labelwidth ? width : labelwidth;

	_LayoutMenuBar();
	ResizeToPreferred();
}

// RefreshItemLabel
void
LabelPopup::RefreshItemLabel()
{
	if (BMenuItem* item = Menu()->FindMarked())
		item->SetMarked(true);
	MenuBar()->ResizeToPreferred();
}

// _PreferredSize
BRect
LabelPopup::_PreferredSize(const char* label, BMenu *menu, bool asLabel)
{
	float width = 0.0;
	float height = 0.0;
	// figure out height
	font_height fh;
	be_plain_font->GetHeight(&fh);
	// complicated formular to make a BMenuField look good
	height = floorf(1.065 * (fh.ascent + fh.descent) + 0.5) + 7.0;
	// go through all items in the menu and figure out their max width
	if (menu) {
		for (int32 i = 0; BMenuItem *item = menu->ItemAt(i); i++) {
			float itemWidth = be_plain_font->StringWidth(item->Label());
			if (itemWidth > width)
				width = itemWidth;
		}
	}
	// figure out the width of the label and add it to width
	float labelWidth = asLabel ? be_bold_font->StringWidth(label)
							   : be_plain_font->StringWidth(label);
	if (labelWidth > 0.0)
		width += labelWidth + LABEL_DIST;
	return BRect(0.0, 0.0, width + 40.0, height);
}

// _LayoutMenuBar
void
LabelPopup::_LayoutMenuBar()
{
	if (BMenuBar* m = MenuBar()) {
		BRect r = m->Frame();
		// sanity checks
		if (rolemodel)
			labelwidth = rolemodel->LabelWidth();
		if (labelwidth < LabelWidth())
			labelwidth = LabelWidth();
		// layout menu bar
		r.left = labelwidth + DIVIDER_DIST;
		r.right = Bounds().right - 2.0;

//		if (sSystemVersion == SYSTEM_DANO) {
//			r.left += 13.0;
//			r.right -= 3.0;
//		}
#if TARGET_PLATFORM_ZETA
			r.left += 13.0;
			r.right -= 7.0;
#endif

		SetDivider(r.left);
		m->MoveTo(r.LeftTop());
		m->ResizeTo(r.Width(), r.Height());
		// account for little handle
		r.right -= 8.0;
		m->SetMaxContentWidth(r.Width());
		m->InvalidateLayout();
		Menu()->InvalidateLayout();
	}
}
