// ColumnTreeViewColors.cpp

#include <InterfaceDefs.h>

#include "ColumnTreeViewColors.h"
#include "Colors.h"

#define BLUE_PERCENTAGE 0.03

const rgb_color treeBackGround = (rgb_color){ 200, 200, 200, 255 };
const rgb_color treeSelectedBackGround = tint_color(treeBackGround, 1.2);

const column_tree_item_colors	kDefaultColumnTreeItemColors = {
	Black,							// foreground
	treeBackGround,					// background
	BeHighlight,					// highlight
	BeShadow,						// shadow
	Black,							// selected_foreground
	(rgb_color){					// selected_background
		treeSelectedBackGround.red,
		treeSelectedBackGround.green,
		treeSelectedBackGround.blue + (uint8)(treeSelectedBackGround.blue * BLUE_PERCENTAGE),
		treeSelectedBackGround.alpha
	},
	tint_color(treeSelectedBackGround, B_LIGHTEN_2_TINT),	// selected_highlight
	tint_color(treeSelectedBackGround, B_DARKEN_2_TINT)	// selected_shadow
};

const column_header_colors		kDefaultColumnHeaderColors = {
	Black,							// foreground
	ui_color(B_PANEL_BACKGROUND_COLOR),				// background
	tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_LIGHTEN_MAX_TINT),					// highlight
	tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT),	// light_shadow
	tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_2_TINT),						// shadow
	Black,							// pressed_foreground
	MedXSheetSelectedColor,			// pressed_background
	LightXSheetSelectedColor,		// pressed_highlight
	DarkXSheetSelectedColor			// pressed_shadow
};

const column_header_view_colors	kDefaultColumnHeaderViewColors = {
	BeBackgroundGrey,				// background
	kDefaultColumnHeaderColors		// header_colors
};

const column_tree_view_colors	kDefaultColumnTreeViewColors = {
	treeBackGround,					// background
	kDefaultColumnTreeItemColors,	// item_colors
	kDefaultColumnHeaderViewColors	// header_view_colors
};

