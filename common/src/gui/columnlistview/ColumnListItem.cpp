// ColumnListItem.cpp

#include <stdio.h>

#include <View.h>

#include "ColumnListItem.h"
#include "Column.h"
#include "ColumnListView.h"
#include "ColumnListViewColors.h"

// constructor
ColumnListItem::ColumnListItem(float height)
	: fParent(NULL),
	  fParentItem(NULL),
	  fChildItems(NULL),
	  fHeight(height),
	  fFlags(0),
	  fYOffset(0)
{
}

// destructor
ColumnListItem::~ColumnListItem()
{
	delete fChildItems;
}

// SetParent
void
ColumnListItem::SetParent(ColumnListView* parent)
{
	if (!parent) {
		if (fParentItem)
			fParentItem->ItemRemoved(this);
		if (!IsCollapsed())
			_RemoveSubitems();
	} else {
		if (!IsCollapsed())
			_AddSubitems();
	}
	
	fParent = parent;
	// ...
}

// Parent
ColumnListView*
ColumnListItem::Parent() const
{
	return fParent;
}

// SetParentItem
void
ColumnListItem::SetParentItem(ColumnListItem* parent)
{
	fParentItem = parent;
}

// ParentItem
ColumnListItem*
ColumnListItem::ParentItem() const
{
	return fParentItem;
}

// AddItem
bool
ColumnListItem::AddItem(ColumnListItem* item)
{
	bool success = false;
	if (item) {
		if (!fChildItems)
			fChildItems = new BList(20);
		if ((success = fChildItems->AddItem((void*)item))) {
			item->SetParentItem(this);
			// add item to column list view
			if (fParent && !IsCollapsed()) {
				int32 index = fChildItems->IndexOf((void*)item);
				index += fParent->IndexOf(this);
//				if (!(success = fParent->AddItem(item, index)));
//					fChildItems->RemoveItem((void*)item);
fParent->AddItem(item, index);
success = true;
			}
		}
	}
	return success;
}

// AddItem
bool
ColumnListItem::AddItem(ColumnListItem* item, int32 index)
{
	// if we no sub items yet, simply call above method
	if (!fChildItems)
		return AddItem(item);

	bool success = false;
	if (item) {
		if ((success = fChildItems->AddItem((void*)item, index))) {
			item->SetParentItem(this);
			// add item to column list view
			if (fParent && !IsCollapsed()) {
				int32 index = fChildItems->IndexOf((void*)item);
				index += fParent->IndexOf(this);
//				if (!(success = fParent->AddItem(item, index)));
//					fChildItems->RemoveItem((void*)item);
fParent->AddItem(item, index);
success = true;
			}
		}
	}
	return success;
}

// ItemRemoved
void
ColumnListItem::ItemRemoved(ColumnListItem* item)
{
	if (fChildItems) {
		fChildItems->RemoveItem((void*)item);
		item->SetParentItem(NULL);
	}
}

// CountItemSlots
int32
ColumnListItem::CountItemSlots() const
{
	int32 count = 1; // one for this item
	if (!IsCollapsed() && fChildItems) {
		// deep ("recursive") count of all sub items
		for (int32 i = 0; ColumnListItem* item = (ColumnListItem*)fChildItems->ItemAt(i); i++)
			count += item->CountItemSlots();
	}
	return count;
}

// SetHeight
void
ColumnListItem::SetHeight(float height)
{
	fHeight = height;
	// notify parent
	// ...
}

// Height
float
ColumnListItem::Height() const
{
	return fHeight;
}

// Draw
void
ColumnListItem::Draw(BView* view, Column* column, BRect frame,
					 BRect updateRect, uint32 flags,
					 const column_list_item_colors* colors)
{
}

// DrawBackground
void
ColumnListItem::DrawBackground(BView* view, Column* column, BRect frame,
							   BRect rect, uint32 flags,
							   const column_list_item_colors* colors)
{
	bool selected = (flags & COLUMN_LIST_ITEM_SELECTED);
	// set the colors according to the selection state
	if (selected)
		view->SetLowColor(colors->selected_background);
	else
		view->SetLowColor(colors->background);
	view->FillRect(rect, B_SOLID_LOW);
}


// AddFlags
void
ColumnListItem::AddFlags(uint32 flags)
{
	fFlags |= flags;
}

// ClearFlags
void
ColumnListItem::ClearFlags(uint32 flags)
{
	fFlags &= ~flags;
}

// SetFlags
void
ColumnListItem::SetFlags(uint32 flags)
{
	fFlags = flags;
}

// Flags
uint32
ColumnListItem::Flags() const
{
	return fFlags;
}

// SetSelected
void
ColumnListItem::SetSelected(bool selected)
{
	if (selected)
		AddFlags(COLUMN_LIST_ITEM_SELECTED);
	else
		ClearFlags(COLUMN_LIST_ITEM_SELECTED);
}

// IsSelected
bool
ColumnListItem::IsSelected() const
{
	return (fFlags & COLUMN_LIST_ITEM_SELECTED);
}

// SetCollapsed
void
ColumnListItem::SetCollapsed(bool collapse)
{
	if (collapse) {
		if (!IsCollapsed()) {
			AddFlags(COLUMN_LIST_ITEM_COLLAPSED);
			_RemoveSubitems();
		}
	} else {
		if (IsCollapsed()) {
			ClearFlags(COLUMN_LIST_ITEM_COLLAPSED);
			_AddSubitems();
		}
	}
}

// IsCollapsed
bool
ColumnListItem::IsCollapsed() const
{
	return (fFlags & COLUMN_LIST_ITEM_COLLAPSED);
}

// SetYOffset
void
ColumnListItem::SetYOffset(float offset)
{
	fYOffset = offset;
}

// YOffset
float
ColumnListItem::YOffset() const
{
	return fYOffset;
}

// _AddSubitems
void
ColumnListItem::_AddSubitems()
{
	if (fParent && fChildItems) {
		int32 index = fParent->IndexOf(this) + 1;
		for (int32 i = 0; ColumnListItem* item = (ColumnListItem*)fChildItems->ItemAt(i); i++) {
			fParent->AddItem(item, index);
			// adding the item to the list view
			// might cause its subitems to be added as well,
			// add the next item after all subitems of this one
			index += item->CountItemSlots();
		}
	}
}

// _RemoveSubitems
void
ColumnListItem::_RemoveSubitems()
{
	if (fParent && fChildItems) {
		for (int32 i = 0; ColumnListItem* item = (ColumnListItem*)fChildItems->ItemAt(i); i++) {
			fParent->RemoveItem(item);
		}
	}
}


