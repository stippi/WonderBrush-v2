// ColumnListViewColors.h

#ifndef COLUMN_LIST_VIEW_COLORS_H
#define COLUMN_LIST_VIEW_COLORS_H

#include <GraphicsDefs.h>

// item colors
struct column_list_item_colors {
	rgb_color					foreground;
	rgb_color					background;
	rgb_color					highlight;
	rgb_color					shadow;
	rgb_color					selected_foreground;
	rgb_color					selected_background;
	rgb_color					selected_highlight;
	rgb_color					selected_shadow;
};

// header colors
struct column_header_colors {
	rgb_color					foreground;
	rgb_color					background;
	rgb_color					highlight;
	rgb_color					light_shadow;
	rgb_color					shadow;
	rgb_color					pressed_foreground;
	rgb_color					pressed_background;
	rgb_color					pressed_highlight;
	rgb_color					pressed_shadow;
};

// header view colors
struct column_header_view_colors {
	rgb_color					background;
	column_header_colors		header_colors;
};

// list view colors
struct column_list_view_colors{
	rgb_color					background;
	column_list_item_colors		item_colors;
	column_header_view_colors	header_view_colors;
};

// defaults
extern const column_list_item_colors	kDefaultColumnListItemColors;
extern const column_header_colors		kDefaultColumnHeaderColors;
extern const column_header_view_colors	kDefaultColumnHeaderViewColors;
extern const column_list_view_colors	kDefaultColumnListViewColors;


#endif	// COLUMN_LIST_VIEW_COLORS_H
