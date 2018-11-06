// EasyColumnListItem.h

#ifndef EASY_COLUMN_LIST_ITEM_H
#define EASY_COLUMN_LIST_ITEM_H

#include <List.h>

#include "ColumnListItem.h"

class BBitmap;

class ColumnItem;

class EasyColumnListItem : public ColumnListItem {
 public:
								EasyColumnListItem(float height);
	virtual						~EasyColumnListItem();

	virtual	void				Draw(BView* view, Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_list_item_colors* colors);

			void				SetContent(int32 index, ColumnItem* item);
			ColumnItem*			ColumnItemAt(int32 index) const;

	// convenience methods
			void				SetContent(int32 index, const BBitmap* bitmap);
			void				SetContent(int32 index, const char* text,
										   bool disabled = false);

	static	int 				StandardCompare(
										const ColumnListItem* item1,
										const ColumnListItem* item2,
										const Column* column);


 private:
			BList				fColumnItems;
};


#endif	// EASY_COLUMN_LIST_ITEM_H
