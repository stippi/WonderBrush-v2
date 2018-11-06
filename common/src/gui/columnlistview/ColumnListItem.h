// ColumnListItem.h

#ifndef COLUMN_LIST_ITEM_H
#define COLUMN_LIST_ITEM_H

#include <Rect.h>

// flags
enum {
	COLUMN_LIST_ITEM_USER_FLAGS	= 0x00,
	COLUMN_LIST_ITEM_SELECTED	= 0x01,
	COLUMN_LIST_ITEM_COLLAPSED	= 0x02,
};

class BList;
class BView;

class Column;
class ColumnListView;
struct column_list_item_colors;

class ColumnListItem {
 public:
								ColumnListItem(float height);
	virtual						~ColumnListItem();

			void				SetParent(ColumnListView* parent);
			ColumnListView*		Parent() const;

			void				SetParentItem(ColumnListItem* parent);
			ColumnListItem*		ParentItem() const;

			bool				AddItem(ColumnListItem* item);
			bool				AddItem(ColumnListItem* item,
										int32 index);
			void				ItemRemoved(ColumnListItem* item);

			int32				CountItemSlots() const;

			void				SetHeight(float height);
			float				Height() const;

	virtual	void				Draw(BView* view, Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_list_item_colors* colors);

			void				DrawBackground(BView* view, Column* column,
									BRect frame, BRect rect, uint32 flags,
									const column_list_item_colors* colors);

	// service methods for the ColumnListView implementation
			void				AddFlags(uint32 flags);
			void				ClearFlags(uint32 flags);
			void				SetFlags(uint32 flags);
			uint32				Flags() const;

			void				SetSelected(bool selected);
			bool				IsSelected() const;

			void				SetCollapsed(bool collapse);
			bool				IsCollapsed() const;

			void				SetYOffset(float offset);
			float				YOffset() const;

 private:
			void				_AddSubitems();
			void				_RemoveSubitems();

			ColumnListView*		fParent;
			ColumnListItem*		fParentItem;
			BList*				fChildItems;
			float				fHeight;
			uint32				fFlags;
			float				fYOffset;
};

#endif	// COLUMN_LIST_ITEM_H
