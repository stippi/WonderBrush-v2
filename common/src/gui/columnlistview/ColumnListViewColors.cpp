// ColumnListViewColors.cpp

#include <InterfaceDefs.h>

#include "ColumnListViewColors.h"

const rgb_color kPanelBackground = ui_color(B_PANEL_BACKGROUND_COLOR);
const rgb_color kLighten1 = tint_color(kPanelBackground, B_LIGHTEN_1_TINT);
const rgb_color kLighten2 = tint_color(kPanelBackground, B_LIGHTEN_2_TINT);
const rgb_color kLightenMax = tint_color(kPanelBackground, B_LIGHTEN_MAX_TINT);
const rgb_color kDarken1 = tint_color(kPanelBackground, B_DARKEN_1_TINT);
const rgb_color kDarken2 = tint_color(kPanelBackground, B_DARKEN_2_TINT);
const rgb_color kDarken3 = tint_color(kPanelBackground, B_DARKEN_3_TINT);

const rgb_color kListBackGround = kLightenMax;//tint_color(kPanelBackground, B_DARKEN_2_TINT);
const rgb_color kListSelectedBackGround = tint_color(kListBackGround, 1.2);
const rgb_color kBlack = (rgb_color){ 0, 0, 0, 255 };

const column_list_item_colors	kDefaultColumnListItemColors = {
	kBlack,													// foreground
	kListBackGround,										// background
	kLightenMax,											// highlight
	kDarken2,												// shadow
	kBlack,													// selected_foreground
	(rgb_color){											// selected_background
		kListSelectedBackGround.red,
		kListSelectedBackGround.green,
		(uint8)(1.03 * kListSelectedBackGround.blue),
		kListSelectedBackGround.alpha
	},
	tint_color(kListSelectedBackGround, B_LIGHTEN_2_TINT),	// selected_highlight
	tint_color(kListSelectedBackGround, B_DARKEN_2_TINT)	// selected_shadow
};

const column_header_colors		kDefaultColumnHeaderColors = {
	kBlack,													// foreground
	kPanelBackground,										// background
	kLightenMax,											// highlight
	kDarken1,												// light_shadow
	kDarken2,												// shadow
	kBlack,													// pressed_foreground
	kDarken2,												// pressed_background
	kPanelBackground,										// pressed_highlight
	kDarken3												// pressed_shadow
};

const column_header_view_colors	kDefaultColumnHeaderViewColors = {
	kPanelBackground,										// background
	kDefaultColumnHeaderColors								// header_colors
};

const column_list_view_colors	kDefaultColumnListViewColors = {
	kListBackGround,										// background
	kDefaultColumnListItemColors,							// item_colors
	kDefaultColumnHeaderViewColors							// header_view_colors
};

