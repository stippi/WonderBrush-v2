// ColumnListView.cpp

#include <algorithm>
#include <stdio.h>

#include <Message.h>
#include <Window.h>

#include "ColumnListView.h"
#include "Column.h"
#include "ColumnHeaderView.h"
#include "ColumnListItem.h"
#include "ColumnListViewColors.h"
#include "ScrollView.h"
#include "Sort.h"

using namespace ColumnListViewStates;

// compare item function prototype
static inline
int
item_cmp(column_list_compare_function* cmpFunc,
		 Column* primary, Column* secondary,
		 int inverse1, int inverse2,
		 ColumnListItem* item1, ColumnListItem* item2);


// ItemGreater

class ItemGreater {
 public:
								ItemGreater(
									column_list_compare_function* cmpFunc,
			 						Column* primary, Column* secondary);

	inline	bool				operator()(ColumnListItem* item1,
										   ColumnListItem* item2) const;

 private:
			column_list_compare_function* fCmpFunc;
			Column*				fPrimary;
			Column*				fSecondary;
			int					fInverse1;
			int					fInverse2;
};

// constructor
ItemGreater::ItemGreater(column_list_compare_function* cmpFunc,
						 Column* primary, Column* secondary)
	: fCmpFunc(cmpFunc),
	  fPrimary(primary),
	  fSecondary(secondary),
	  fInverse1(1),
	  fInverse2(1)
{
	// make clear what key(s) to sort
	if (fSecondary && (fSecondary->Flags() & COLUMN_SORT_INVERSE))
		fInverse2 = -1;
	if (fPrimary) {
		if ((fPrimary->Flags() & COLUMN_SORT_INVERSE))
			fInverse1 = -1;
	} else {
		fPrimary = fSecondary;
		fInverse1 = fInverse2;
		fSecondary = NULL;
	}
}

// ()
bool
ItemGreater::operator()(ColumnListItem* item1, ColumnListItem* item2) const
{
	int cmp = (*fCmpFunc)(item1, item2, fPrimary) * fInverse1;
	if (cmp == 0) {
		if (fSecondary)
			cmp = (*fCmpFunc)(item1, item2, fSecondary) * fInverse2;
	}
	return (cmp > 0);
}



// ColumnListView

// constructor
ColumnListView::ColumnListView()
	: BView(BRect(0.0, 0.0, 100.0, 100.0), NULL, B_FOLLOW_NONE,
			B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE),
	  BInvoker(new BMessage(B_SIMPLE_DATA), this),
	  Scrollable(),	// DataRect == (0, 0, 0, 0), ScrollOffset = (0, 0)
	  fColumns(20),
	  fVisibleColumns(20),
	  fItems(100),
	  fSelectedItems(100),
	  fHeaderView(NULL),
	  fCurrentScrollOffset(0, 0),
	  fScrollingEnabled(true),
	  fPrimarySortColumn(NULL),
	  fSecondarySortColumn(NULL),
	  fInvocationMessage(NULL),
	  fSelectionMessage(NULL),
	  fSelectionDepth(0),
	  fState(new OutsideState(this)),
	  fColors(new column_list_view_colors(kDefaultColumnListViewColors)),
	  fCompareFunction(NULL),
	  fSelectionMode(CLV_MULTIPLE_SELECTION),
	  fLastClickedIndex(-1),
	  fLastClickTime(system_time()),
	  fDoubleClickTime(500000),
	  fMouseInside(false)
{
	fHeaderView = new ColumnHeaderView();
	fHeaderView->SetParentView(this);
	fHeaderView->SetColors(&fColors->header_view_colors);
	AddChild(fHeaderView);
	_Layout();
	SetViewColor(B_TRANSPARENT_32_BIT);
	if (get_click_speed(&fDoubleClickTime) != B_OK)
		fDoubleClickTime = 500000;
}

// destructor
ColumnListView::~ColumnListView()
{
	_ChangeState(NULL);
	delete fColors;
	delete fInvocationMessage;
	delete fSelectionMessage;
	// delete columns
	for (int32 i = 0; Column* column = _OrderedColumnAt(i); i++)
		delete column;
}

// AttachedToWindow
void
ColumnListView::AttachedToWindow()
{
	SetTarget(Window());
}

// Draw
void
ColumnListView::Draw(BRect updateRect)
{
	Draw(this, updateRect);
}

// Draw
void
ColumnListView::Draw(BView* view, BRect updateRect)
{
	if (view) {
		BRect itemsRect(_ActualItemsRect());
		// let the items draw themselves
		BRect itemsUpdateRect(updateRect & itemsRect);
		if (itemsUpdateRect.IsValid()) {
			// some items have to be updated
			int32 first = IndexOf(itemsUpdateRect.top);
			int32 last = IndexOf(itemsUpdateRect.bottom);
			int32 firstColumn = _VisibleColumnIndexOf(itemsUpdateRect.left);
			int32 lastColumn = _VisibleColumnIndexOf(itemsUpdateRect.right);
			for (int32 i = first; i <= last; i++) {
				ColumnListItem* item = ItemAt(i);
				if (item) {
					BRect itemRect(_ItemFrame(item));
					for (int32 c = firstColumn; c <= lastColumn; c++) {
						Column* column = _VisibleColumnAt(c);
						BRect columnRect(_VisibleColumnFrame(column));
						item->Draw(view, column, itemRect & columnRect,
								   itemRect & columnRect & itemsUpdateRect,
								   item->Flags(), &fColors->item_colors);
					}
				}
			}
		}
		// draw the background
		if (updateRect.right > itemsRect.right) {
			BRect rect(updateRect);
			rect.left = itemsRect.right + 1.0f;
			if (rect.IsValid()) {
				view->SetHighColor(fColors->background);
				view->FillRect(rect);
			}
		}
		if (updateRect.bottom > itemsRect.bottom) {
			BRect rect(updateRect);
			rect.top = itemsRect.bottom + 1.0f;
			// don't draw more than necessary:
			rect.right = std::min(rect.right, itemsRect.right);
			if (rect.IsValid()) {
				view->SetHighColor(fColors->background);
				view->FillRect(rect);
			}
		}
	}
}

// FrameResized
void
ColumnListView::FrameResized(float width, float height)
{
	SetVisibleSize(width, height - _HeaderViewRect().Height() - 1.0f);
	_Layout();
	BRect itemsRect(_ItemsRect());
	SetVisibleSize(itemsRect.Width(), itemsRect.Height());
}

// KeyDown
void
ColumnListView::KeyDown(const char* bytes, int32 numBytes)
{
	int32 count = CountItems();
	int32 firstSelected = CurrentSelection(0);
	int32 lastSelected = CurrentSelection(fSelectedItems.CountItems() - 1);
	uint32 modifiers = 0;
	Window()->CurrentMessage()->FindInt32("modifiers", (int32 *)&modifiers);
	bool extend = (modifiers & B_SHIFT_KEY) || (modifiers & B_OPTION_KEY);
	switch (bytes[0]) {
		case B_DOWN_ARROW:
		{
			if (fSelectedItems.IsEmpty()) {
				if (count > 0) {
					Select(0, false);
					ScrollToItem(0);
				}
			} else {
				int32 index = lastSelected + 1;
				if (index >= count)
					index--;
				Select(index, extend);
				ScrollToItem(index);
			}
			break;
		}
		case B_UP_ARROW:
		{
			if (fSelectedItems.IsEmpty()) {
				if (count > 0) {
					Select(count - 1, false);
					ScrollToItem(count - 1);
				}
			} else {
				int32 index = firstSelected - 1;
				if (index < 0)
					index = 0;
				Select(index, extend);
				ScrollToItem(index);
			}
			break;
		}
		case B_PAGE_DOWN:
			SetScrollOffset(fCurrentScrollOffset
							+ BPoint(0, _ItemsRect().Height()));
			break;
		case B_PAGE_UP:
			SetScrollOffset(fCurrentScrollOffset
							- BPoint(0, _ItemsRect().Height()));
			break;
		case B_ENTER:
//		case B_RETURN:
			if (!fSelectedItems.IsEmpty())
				_InternalInvoke(fInvocationMessage);
			break;
		default:
			BView::KeyDown(bytes, numBytes);
			break;
	}
}

// MakeFocus
void
ColumnListView::MakeFocus(bool focusState)
{
	if (ScrollView* scrollView = dynamic_cast<ScrollView*>(ScrollSource())) {
		if (scrollView->Child() == this)
			scrollView->ChildFocusChanged(focusState);
	}
	BView::MakeFocus(focusState);
}

// MouseDown
void
ColumnListView::MouseDown(BPoint point)
{
	uint32 buttons = 0;
	uint32 modifiers = 0;
	int32 clicks = 1;
	Window()->CurrentMessage()->FindInt32("buttons", (int32 *)&buttons);
	Window()->CurrentMessage()->FindInt32("modifiers", (int32 *)&modifiers);
	Window()->CurrentMessage()->FindInt32("clicks", &clicks);
	// begin double click feature
	int32 index = IndexOf(point);
	bigtime_t clickTime = system_time();
	if (index == fLastClickedIndex
		&& clickTime - fLastClickTime <= fDoubleClickTime) {
		ItemDoubleClicked(fLastClickedIndex);
	}
	fLastClickedIndex = index;
	fLastClickTime = clickTime;
	// end double click feature
	fState->Pressed(point, buttons, modifiers, clicks);
}

// MouseMoved
void
ColumnListView::MouseMoved(BPoint point, uint32 transit,
							 const BMessage* message)
{
	// work arround a problem on ZETA, don't know why it is triggered
	if (Bounds().Contains(point)) {
		if (!fMouseInside) {
			transit = B_ENTERED_VIEW;
		}
	} else {
		if (fMouseInside)
			transit = B_EXITED_VIEW;
	}

	switch (transit) {
		case B_ENTERED_VIEW:
			fMouseInside = true;
			fState->Entered(point, message);
			break;
		case B_EXITED_VIEW:
			fMouseInside = false;
			fState->Exited(point, message);
			break;
		default:
			fState->Moved(point, transit, message);
			break;
	}
}

// MouseUp
void
ColumnListView::MouseUp(BPoint point)
{
	uint32 buttons = 0;
	uint32 modifiers = 0;
	Window()->CurrentMessage()->FindInt32("buttons", (int32 *)&buttons);
	Window()->CurrentMessage()->FindInt32("modifiers", (int32 *)&modifiers);
	fState->Released(point, buttons, modifiers);
}

// Invoke
/*status_t
ColumnListView::Invoke(BMessage* message)
{
	BMessage clone(InvokeKind());
	if (message)
		clone = *message;
	clone.AddInt64("when", system_time());
	clone.AddPointer("source", (void*)this);
	int32 count = fSelectedItems.CountItems();
	for (int32 i = 0; i < count; i++)
		clone.AddInt32("index", CurrentSelection(i));
	return BInvoker::Invoke(&clone);
}*/

// ScrollOffsetChanged
void
ColumnListView::ScrollOffsetChanged(BPoint oldOffset, BPoint newOffset)
{
	if (fScrollingEnabled) {
		fCurrentScrollOffset = newOffset;
		_Layout();
		BRect itemsRect(_ItemsRect());
		CopyBits(itemsRect.OffsetByCopy(newOffset - oldOffset), itemsRect);
	}
}

// AddColumn
void
ColumnListView::AddColumn(Column* column, int32 index)
{
	if (column && column->Header()) {
		int32 count = CountColumns();
		// check index
		if (index < 0 || index > count)
			index = count;
		_ReindexColumns(index, 1);
		// add column
		fColumns.AddItem((void*)column, index);
		column->SetParent(this);
		column->SetIndex(index);
		// visible?
		if (column->IsVisible())
			_AddVisibleColumn(column);
	}
}

// RemoveColumn
Column*
ColumnListView::RemoveColumn(int32 index)
{
	Column* column = NULL;
	int32 orderedIndex = OrderedColumnIndexOf(index);
	if (orderedIndex >= 0)
		column = _RemoveOrderedColumn(orderedIndex);
	return column;
}

// RemoveColumn
bool
ColumnListView::RemoveColumn(Column* column)
{
	int32 index = OrderedColumnIndexOf(column);
	if (index >= 0)
		return _RemoveOrderedColumn(index);
	return false;
}

// FindColumn
Column*
ColumnListView::FindColumn(const char* name) const
{
	Column* column = NULL;
	for (int32 i = 0; Column* candidate = (Column*)fColumns.ItemAt(i); i++) {
		if (strcmp(candidate->Name(), name) == 0) {
			column = candidate;
			break;
		}
	}
	return column;
}

// ColumnAt
Column*
ColumnListView::ColumnAt(int32 index) const
{
	return _OrderedColumnAt(OrderedColumnIndexOf(index));
}

// ColumnIndexOf
int32
ColumnListView::ColumnIndexOf(Column* column) const
{
	if (fColumns.HasItem((void*)column))
		return column->Index();
	return -1;
}

// CountColumns
int32
ColumnListView::CountColumns() const
{
	return fColumns.CountItems();
}

// HideColumn
void
ColumnListView::HideColumn(int32 index)
{
	_HideOrderedColumn(OrderedColumnIndexOf(index));
}

// HideColumn
void
ColumnListView::HideColumn(Column* column)
{
	if (fColumns.HasItem((void*)column))
		_HideOrderedColumn(column);
}

// ShowColumn
void
ColumnListView::ShowColumn(int32 index)
{
	_ShowOrderedColumn(OrderedColumnIndexOf(index));
}

// ShowColumn
void
ColumnListView::ShowColumn(Column* column)
{
	if (fColumns.HasItem((void*)column))
		_ShowOrderedColumn(column);
}

// InvalidateHeader
void
ColumnListView::InvalidateHeader(int32 index)
{
	fHeaderView->_InvalidateHeaders(index, -1);
}

// AddItem
void
ColumnListView::AddItem(ColumnListItem* item, int32 index)
{
	if (item) {
		// check index
		int32 count = CountItems();
		if (index < 0 || index > count)
			index = count;
		// if sorted, get insertion idex
		if (fCompareFunction && (fPrimarySortColumn || fSecondarySortColumn))
			index = _FindSortedInsertionIndex(item);
		// keep the selection in sync
		_ReindexSelectedItems(index, 1);
		// add item -- clear non-user flags before
		item->ClearFlags(~COLUMN_LIST_ITEM_USER_FLAGS);
		if (fItems.AddItem((void*)item, index)) {
			_UpdateItemYOffsets(index, false);
			_InvalidateItems(index, -1);
			item->SetParent(this);
		}
	}
}

// RemoveItem
ColumnListItem*
ColumnListView::RemoveItem(int32 index)
{
	ColumnListItem* item = ItemAt(index);
	if (item) {
		// keep the selection in sync
		if (item->IsSelected())
			Deselect(index);
		_ReindexSelectedItems(index + 1, -1);
		// remove
		fItems.RemoveItem(index);
		_UpdateItemYOffsets(index, false);
		_InvalidateItems(index, -1);
		item->SetParent(NULL);
	}
	return item;
}

// RemoveItem
bool
ColumnListView::RemoveItem(ColumnListItem* item)
{
	int32 index = IndexOf(item);
	if (index >= 0) {
		item->SetParent(NULL);
		return RemoveItem(index);
	}
	return false;
}

// RemoveItems
bool
ColumnListView::RemoveItems(int32 index, int32 count)
{
	bool result = false;
	int32 itemCount = CountItems();
	if (index >= 0 && count > 0 && index + count <= itemCount) {
		// keep the selection in sync
		int32 first = _FindSelectionInsertionIndex(index);
		int32 last = _FindSelectionInsertionIndex(index + count);
		if (first < last)
			fSelectedItems.RemoveItems(first, last - first);
		// remove
		for (int32 i = index; i < index + count; i++) {
			if (ColumnListItem* item = ItemAt(i))
				item->SetParent(NULL);
		}
		fItems.RemoveItems(index, count);
		_UpdateItemYOffsets(index, false);
		_InvalidateItems(index, -1);
	}
	return result;
}

// HasItem
bool
ColumnListView::HasItem(ColumnListItem* item) const
{
	return fItems.HasItem((void*)item);
}

// ItemAt
ColumnListItem*
ColumnListView::ItemAt(int32 index) const
{
	return (ColumnListItem*)fItems.ItemAt(index);
}

// IndexOf
int32
ColumnListView::IndexOf(ColumnListItem* item) const
{
	return fItems.IndexOf((void*)item);
}

// IndexOf
//
// Returns the index of the item that contains the supplied coordinates
// (measured in view coordinates).
// Returns -1 in case no such item exists.
int32
ColumnListView::IndexOf(BPoint point) const
{
	if (DataRect().OffsetBySelf(_ItemsRect().LeftTop() - fCurrentScrollOffset)
		.Contains(point)) {
		return IndexOf(point.y);
	}
	return -1;
}

// IndexOf
//
// Returns the index of the item that contains the supplied Y coordinate
// (measured in view coordinates).
// Returns -1 in case no such item exists.
int32
ColumnListView::IndexOf(float y) const
{
	// binary search
	int32 lower = 0;
	int32 upper = CountItems() - 1;
	if (upper >= 0 && _ItemPosition(lower).y <= y &&
		_ItemFrame(upper).bottom >= y) {
		while (lower < upper) {
			int32 mid = (lower + upper + 1) / 2;
			if (_ItemPosition(mid).y <= y)
				lower = mid;
			else
				upper = mid - 1;
		}
		return lower;
	}
	return -1;
}

// CountItems
int32
ColumnListView::CountItems() const
{
	return fItems.CountItems();
}

// IsEmpty
bool
ColumnListView::IsEmpty() const
{
	return fItems.IsEmpty();
}

// MakeEmpty
void
ColumnListView::MakeEmpty()
{
	if (!IsEmpty()) {
		BRect itemsRect(_ActualItemsRect());
		fItems.MakeEmpty();
		fSelectedItems.MakeEmpty();
		_UpdateItemYOffsets(0, false);
		Invalidate(itemsRect);
	}
}

// InvalidateItem
void
ColumnListView::InvalidateItem(int32 index)
{
	if (ColumnListItem* item = ItemAt(index))
		Invalidate(_ItemFrame(item));
}

// InvalidateItem
void
ColumnListView::InvalidateItem(ColumnListItem* item)
{
	if (item)
		Invalidate(_ItemFrame(item));
}

// ItemChanged
//
// To be called, when an item has changed. The sort position is checked
// and the item is invalidated.
void
ColumnListView::ItemChanged(int32 index)
{
	if (ColumnListItem* item = ItemAt(index)) {
		if (fCompareFunction && (fPrimarySortColumn || fSecondarySortColumn)) {
			int32 count = CountItems();
			ItemGreater greater(fCompareFunction, fPrimarySortColumn,
								fSecondarySortColumn);
			// compare the item with its predecessor and successor
			if (index > 0 && greater(ItemAt(index - 1), item)
				|| index < count - 1
				   && greater(item, ItemAt(index + 1)) > 0) {
				// the item is misplaced -- remove and reinsert it
				bool selected = item->IsSelected();
				RemoveItem(index);
				AddItem(item, index);
				if (selected)
					Select(IndexOf(item));
			}
		}
		InvalidateItem(item);
	}
}

// ScrollToItem
void
ColumnListView::ScrollToItem(int32 index)
{
	if (ColumnListItem* item = ItemAt(index)) {
		BRect itemFrame(_ItemFrame(item));
		BRect itemsRect(_ItemsRect());
		if (itemFrame.top < itemsRect.top) {
			SetScrollOffset(fCurrentScrollOffset +
							BPoint(0, itemFrame.top - itemsRect.top));
		} else if (itemFrame.bottom > itemsRect.bottom) {
			SetScrollOffset(fCurrentScrollOffset +
							BPoint(0, itemFrame.bottom - itemsRect.bottom));
		}
	}
}

// InitiateDrag
bool
ColumnListView::InitiateDrag(BPoint point, int32 index, bool wasSelected)
{
	return false;
}

// ItemDoubleClicked
void
ColumnListView::ItemDoubleClicked(int32 index)
{
}

// SetSelectionMode
void
ColumnListView::SetSelectionMode(selection_mode mode)
{
	fSelectionMode = mode;
}

// CurrentSelection
int32
ColumnListView::CurrentSelection(int32 index) const
{
	return (int32)fSelectedItems.ItemAt(index) - 1;
}

// Deselect
void
ColumnListView::Deselect(int32 index)
{
	ColumnListItem* item = ItemAt(index);
	if (item && item->IsSelected()) {
		item->SetSelected(false);
		fSelectedItems.RemoveItem(_FindSelectedItem(index));
		InvalidateItem(index);
		_InternalSelectionChanged();
	}
}

// DeselectAll
void
ColumnListView::DeselectAll()
{
	if (!fSelectedItems.IsEmpty()) {
		for (int32 i = 0;
			 ColumnListItem* item = ItemAt(CurrentSelection(i));
			 i++) {
			item->SetSelected(false);
		}
		fSelectedItems.MakeEmpty();
		Invalidate();
		_InternalSelectionChanged();
	}
}

// DeselectExcept
void
ColumnListView::DeselectExcept(int32 start, int32 finish)
{
	bool changed = false;
	if (!fSelectedItems.IsEmpty()) {
		for (int32 i = fSelectedItems.CountItems() - 1; i >= 0; i--) {
			int32 index = CurrentSelection(i);
			if (index < start || index > finish) {
				ColumnListItem* item = ItemAt(index);
				item->SetSelected(false);
				fSelectedItems.RemoveItem(i);
				changed = true;
			}
		}
		if (changed) {
			Invalidate();
			_InternalSelectionChanged();
		}
	}
}

// IsItemSelected
bool
ColumnListView::IsItemSelected(int32 index) const
{
	if (ColumnListItem* item = ItemAt(index))
		return item->IsSelected();
	return false;
}

// ScrollToSelection
void
ColumnListView::ScrollToSelection()
{
	ScrollToItem(CurrentSelection(0));
}

// Select
void
ColumnListView::Select(int32 index, bool extend)
{
	if (ColumnListItem* item = ItemAt(index)) {
		if (!extend) {
			fSelectionDepth++;
			DeselectAll();
			fSelectionDepth--;
		}
		if (!item->IsSelected()) {
			int32 insertionIndex = _FindSelectionInsertionIndex(index);
			fSelectedItems.AddItem((void*)(index + 1), insertionIndex);
			item->SetSelected(true);
			InvalidateItem(item);
			_InternalSelectionChanged();
		}
	}
}

// Select
void
ColumnListView::Select(int32 start, int32 finish, bool extend)
{
	// check for valid indices first
	int32 count = CountItems();
	if (start >= 0 && start < count && finish >= start && finish < count) {
		if (!extend) {
			fSelectionDepth++;
			DeselectAll();
		}
		int32 insertionIndex = _FindSelectionInsertionIndex(start);
		for (int32 i = start; i <= finish; i++) {
			ColumnListItem* item = ItemAt(i);
			if (!item->IsSelected()) {
				fSelectedItems.AddItem((void*)(i + 1), insertionIndex);
				item->SetSelected(true);
			}
			insertionIndex++;
		}
		Invalidate();
		_InternalSelectionChanged();
	}
}

// SelectionChanged
void
ColumnListView::SelectionChanged()
{
}

// CountSelectedItems
int32
ColumnListView::CountSelectedItems() const
{
	return fSelectedItems.CountItems();
}

// SetInvocationMessage
void
ColumnListView::SetInvocationMessage(BMessage* message)
{
	delete fInvocationMessage;
	fInvocationMessage = message;

}

// InvocationMessage
BMessage*
ColumnListView::InvocationMessage() const
{
	return fInvocationMessage;
}

// InvocationCommand
uint32
ColumnListView::InvocationCommand() const
{
	if (fInvocationMessage)
		return fInvocationMessage->what;
	return 0;
}

// SetSelectionMessage
void
ColumnListView::SetSelectionMessage(BMessage* message)
{
	delete fSelectionMessage;
	fSelectionMessage = message;
}

// SelectionMessage
BMessage*
ColumnListView::SelectionMessage() const
{
	return fSelectionMessage;
}

// SelectionCommand
uint32
ColumnListView::SelectionCommand() const
{
	if (fSelectionMessage)
		return fSelectionMessage->what;
	return 0;
}

// ResizeColumn
void
ColumnListView::ResizeColumn(int32 index, float width)
{
	ResizeColumn(ColumnAt(index), width);
}

// ResizeColumn
void
ColumnListView::ResizeColumn(Column* column, float width)
{
	if (column) {
		if (column->IsVisible())
			ResizeVisibleColumn(_VisibleColumnIndexOf(column), width);
		else
			column->SetWidth(width);
	}
}

// MoveColumn
void
ColumnListView::MoveColumn(int32 index, int32 dest)
{
	MoveColumn(ColumnAt(index), dest);
}

// MoveColumn
void
ColumnListView::MoveColumn(Column* column, int32 dest)
{
	if (column) {
		if (column->IsVisible())
			MoveVisibleColumn(_VisibleColumnIndexOf(column), dest);
//		else
	}
}

// PrimarySortColumn
status_t
ColumnListView::PrimarySortColumn(int32 *outIndex,  bool *outInverse) const
{
	status_t ret = B_BAD_VALUE;
	if (outIndex && outInverse) {
		ret = B_ERROR;
		if (fPrimarySortColumn) {
			*outIndex = ColumnIndexOf(fPrimarySortColumn);
			*outInverse = fPrimarySortColumn->Flags() & COLUMN_SORT_INVERSE;
			ret = B_OK;
		} else {
			*outIndex = -1;
			*outInverse = true;
		}
	}
	return ret;
}

// PrimarySortColumn
status_t
ColumnListView::SecondarySortColumn(int32 *outIndex,  bool *outInverse) const
{
	status_t ret = B_BAD_VALUE;
	if (outIndex && outInverse) {
		ret = B_ERROR;
		if (fSecondarySortColumn) {
			*outIndex = ColumnIndexOf(fSecondarySortColumn);
			*outInverse = fSecondarySortColumn->Flags() & COLUMN_SORT_INVERSE;
			ret = B_OK;
		} else {
			*outIndex = -1;
			*outInverse = true;
		}
	}
	return ret;
}

// SetPrimarySortColumn
void
ColumnListView::SetPrimarySortColumn(int32 index, bool inverse)
{
	_SetPrimarySortOrderedColumn(OrderedColumnIndexOf(index), inverse);
}

// SetSecondarySortColumn
void
ColumnListView::SetSecondarySortColumn(int32 index, bool inverse)
{
	_SetSecondarySortOrderedColumn(OrderedColumnIndexOf(index), inverse);
}

// SetSortCompareFunction
void
ColumnListView::SetSortCompareFunction(column_list_compare_function* function)
{
	fCompareFunction = function;
}

// Sort
void
ColumnListView::Sort()
{
	_Sort();
	Invalidate();
}

// SetColors
void
ColumnListView::SetColors(const column_list_view_colors* colors)
{
	*fColors = *colors;
	fHeaderView->SetColors(&fColors->header_view_colors);
	Invalidate();
}

// Colors
const column_list_view_colors*
ColumnListView::Colors() const
{
	return fColors;
}

// ResizeVisibleColumn
void
ColumnListView::ResizeVisibleColumn(int32 index, float width)
{
	if (Column* column = _VisibleColumnAt(index)) {
		float oldWidth = column->Width();
		column->SetWidth(width);
		// We don't need to do anything if nothing has changed.
		if (column->Width() != oldWidth) {
			_UpdateColumnXOffsets(index + 1, false);
			_InvalidateVisibleColumns(index, -1);
			fHeaderView->ResizeHeader(index, column->Width());
		}
	}
}

// MoveVisibleColumn
//
// Move the column at index /oldIndex/ so that it afterwards is at index
// /newIndex/. Both indices must be valid visible indices, otherwise no column
// is moved at all.
void
ColumnListView::MoveVisibleColumn(int32 oldIndex, int32 newIndex)
{
/*	int32 count = _CountVisibleColumns();
	// Check if the indices are valid and different.
	if (oldIndex >= 0 && oldIndex < count &&
		newIndex >= 0 && newIndex < count &&
		newIndex != oldIndex) {
		Column* column = _VisibleColumnAt(oldIndex);
		// move within the visible list
		fVisibleColumns.RemoveItem(oldIndex);
		fVisibleColumns.AddItem((void*)column, newIndex);
		// move within the column list
		fColumns.RemoveItem((void*)column);
		if (newIndex > 0) {
			fColumns.AddItem((void*)column,
				fColumns.IndexOf(fVisibleColumns.ItemAt(newIndex - 1)) + 1);
		} else
			fColumns.AddItem((void*)column, 0);
		// Update the graphics stuff.
		int32 first = min(oldIndex, newIndex);
//		int32 last = max(oldIndex, newIndex);
		_UpdateColumnXOffsets(first, true);
		// move header
		fHeaderView->MoveHeader(oldIndex, newIndex);
	}
*/
	MoveVisibleColumns(oldIndex, newIndex, 1);
}

// MoveVisibleColumns
//
// Move the /count/ columns at index /index/ so that they afterwards are at
// index /dest/. All indices must be valid visible indices, otherwise no
// column is moved at all.
void
ColumnListView::MoveVisibleColumns(int32 index, int32 dest, int32 count)
{
	int32 columnCount = _CountVisibleColumns();
	if (index >= 0 && count > 0 && index + count <= columnCount &&
		dest >= 0 && dest + count <= columnCount && index != dest) {
		// store columns in a list
		BList columns;
		for (int32 i = index; i < index + count; i++)
			columns.AddItem((void*)_VisibleColumnAt(i));
		// move within the visible list
		fVisibleColumns.RemoveItems(index, count);
		fVisibleColumns.AddList(&columns, dest);
		// move within the column list
		for (int32 i = 0; Column* column = (Column*)columns.ItemAt(i); i++)
			fColumns.RemoveItem((void*)column);
		if (dest > 0) {
			fColumns.AddList(&columns,
				fColumns.IndexOf(fVisibleColumns.ItemAt(dest - 1)) + 1);
		} else
			fColumns.AddList(&columns, 0);
		// Update the graphics stuff.
		int32 first = std::min(index, dest);
//		int32 last = std::max(index, dest) + count - 1;
		_UpdateColumnXOffsets(first, true);
		// move header
		fHeaderView->MoveHeaders(index, dest, count);
	}
}

// DisableScrolling
void
ColumnListView::DisableScrolling()
{
	fScrollingEnabled = false;
}

// EnableScrolling
void
ColumnListView::EnableScrolling()
{
	if (!fScrollingEnabled) {
		fScrollingEnabled = true;
		if (fCurrentScrollOffset != ScrollOffset()) {
//			fCurrentScrollOffset = ScrollOffset();
//			_Layout();
			ScrollOffsetChanged(fCurrentScrollOffset, ScrollOffset());
		}
	}
}

// SetPrimarySortVisibleColumn
void
ColumnListView::SetPrimarySortVisibleColumn(int32 index, bool inverse)
{
	_SetPrimarySortOrderedColumn(
		OrderedColumnIndexOf(_VisibleColumnAt(index)), inverse);
}

// SetSecondarySortVisibleColumn
void
ColumnListView::SetSecondarySortVisibleColumn(int32 index, bool inverse)
{
	_SetSecondarySortOrderedColumn(
		OrderedColumnIndexOf(_VisibleColumnAt(index)), inverse);
}

// _RemoveOrderedColumn
Column*
ColumnListView::_RemoveOrderedColumn(int32 index)
{
	Column* column = NULL;
	int32 count = CountColumns();
	if (index >= 0 && index < count) {
		column = (Column*)fColumns.RemoveItem(index);
		_ReindexColumns(column->Index(), -1);
		if (column->IsVisible())
			_RemoveVisibleColumn(column);
		// check, if this column was one of the sort columns
		if (column == fPrimarySortColumn)
			fPrimarySortColumn = NULL;
		if (column == fSecondarySortColumn)
			fSecondarySortColumn = NULL;
	}
	return column;
}

// _HideOrderedColumn
void
ColumnListView::_HideOrderedColumn(int32 index)
{
	_HideOrderedColumn(_OrderedColumnAt(index));
}

// _HideOrderedColumn
//
// If /column/ != NULL, it must be one of the list views columns.
void
ColumnListView::_HideOrderedColumn(Column* column)
{
	if (column && column->IsVisible()) {
		column->SetVisible(false);
		_RemoveVisibleColumn(column);
	}
}

// _ShowOrderedColumn
void
ColumnListView::_ShowOrderedColumn(int32 index)
{
	_ShowOrderedColumn(_OrderedColumnAt(index));
}

// _ShowOrderedColumn
//
// If /column/ != NULL, it must be one of the list views columns.
void
ColumnListView::_ShowOrderedColumn(Column* column)
{
	if (column && !column->IsVisible()) {
		column->SetVisible(true);
		_AddVisibleColumn(column);
	}
}

// _OrderedColumnAt
Column*
ColumnListView::_OrderedColumnAt(int32 index) const
{
	return (Column*)fColumns.ItemAt(index);
}

// OrderedColumnIndexOf
int32
ColumnListView::OrderedColumnIndexOf(Column* column) const
{
	return fColumns.IndexOf((void*)column);
}

// OrderedColumnIndexOf
int32
ColumnListView::OrderedColumnIndexOf(int32 index) const
{
	for (int32 i = 0; Column* column = _OrderedColumnAt(i); i++) {
		if (column->Index() == index)
			return i;
	}
	return -1;
}

// _SetPrimarySortOrderedColumn
void
ColumnListView::_SetPrimarySortOrderedColumn(int32 index, bool inverse)
{
	if (Column* column = _OrderedColumnAt(index)) {
		bool sort = false;
		bool columnInverse = (column->Flags() & COLUMN_SORT_INVERSE);
		if (column != fPrimarySortColumn) {
			// the column can't be both, primary and secondary sort column
			if (column == fSecondarySortColumn)
				_SetSecondarySortOrderedColumn(-1, false);
			Column* oldColumn = fPrimarySortColumn;
			fPrimarySortColumn = column;
			column->AddFlags(COLUMN_PRIMARY_SORT_KEY);
			if (inverse)
				column->AddFlags(COLUMN_SORT_INVERSE);
			if (oldColumn) {
				oldColumn->ClearFlags(COLUMN_PRIMARY_SORT_KEY
									  | COLUMN_SORT_INVERSE);
			}
			// ...
			// just to see an effect:
			fHeaderView->_InvalidateHeaders(0, -1);
			sort = true;
		} else if (inverse != columnInverse) {
			if (inverse)
				column->AddFlags(COLUMN_SORT_INVERSE);
			else
				column->ClearFlags(COLUMN_SORT_INVERSE);
			sort = true;
		}
		// re-sort
		if (sort) {
			_Sort();
			_InvalidateVisibleColumns(0, -1);
		}
	} else {
		// unset the sort column
		if (fPrimarySortColumn) {
			fPrimarySortColumn->ClearFlags(COLUMN_PRIMARY_SORT_KEY
										   | COLUMN_SORT_INVERSE);
		}
		fPrimarySortColumn = NULL;
	}
}

// _SetSecondarySortOrderedColumn
void
ColumnListView::_SetSecondarySortOrderedColumn(int32 index, bool inverse)
{
	if (Column* column = _OrderedColumnAt(index)) {
		bool sort = false;
		bool columnInverse = (column->Flags() & COLUMN_SORT_INVERSE);
		if (column == fPrimarySortColumn) {
			// the column can't be both, primary and secondary sort column
			// we do nothing
		} else if (column != fSecondarySortColumn) {
			Column* oldColumn = fSecondarySortColumn;
			fSecondarySortColumn = column;
			column->AddFlags(COLUMN_SECONDARY_SORT_KEY);
			if (inverse)
				column->AddFlags(COLUMN_SORT_INVERSE);
			if (oldColumn) {
				oldColumn->ClearFlags(COLUMN_SECONDARY_SORT_KEY
									  | COLUMN_SORT_INVERSE);
			}
			// ...
			// just to see an effect:
			fHeaderView->_InvalidateHeaders(0, -1);
			sort = true;
		} else if (inverse != columnInverse) {
			if (inverse)
				column->AddFlags(COLUMN_SORT_INVERSE);
			else
				column->ClearFlags(COLUMN_SORT_INVERSE);
			sort = true;
		}
		// re-sort
		if (sort) {
			_Sort();
			_InvalidateVisibleColumns(0, -1);
		}
	} else {
		// unset the sort column
		if (fSecondarySortColumn) {
			fSecondarySortColumn->ClearFlags(COLUMN_SECONDARY_SORT_KEY
											 | COLUMN_SORT_INVERSE);
		}
		fSecondarySortColumn = NULL;
	}
}

// _CalculateVisibleColumnIndex
//
// Calculates the position of /column/ in the list of visible columns
// based on the information from the column list. That is, all columns
// before /column/ must have a correct visibility flag, though /column/
// doesn't need to.
// No checking is done!
int32
ColumnListView::_CalculateVisibleColumnIndex(Column* column)
{
	int32 index = 0;
	for (int32 i = 0; Column* col = _OrderedColumnAt(i); i++) {
		if (col == column)
			break;
		if (col->IsVisible())
			index++;
	}
	return index;
}

// _AddVisibleColumn
//
// /column/ must already be in the columns list and will now be inserted
// at the correct position in the list of the visible columns.
// The header view is notified as well.
// No checking is done!
void
ColumnListView::_AddVisibleColumn(Column* column)
{
	// Get the insertion index.
	int32 index = _CalculateVisibleColumnIndex(column);
	// insert in visible list
	fVisibleColumns.AddItem((void*)column, index);
	// insert header in header view
	fHeaderView->AddHeader(column->Header(), index);
	// Update the graphics stuff.
	_UpdateColumnXOffsets(index, false);
	_InvalidateVisibleColumns(index, -1);
}

// _RemoveVisibleColumn
//
// Removes /column/ from the list of visible columns.
// The header view is notified as well.
// No checking is done!
void
ColumnListView::_RemoveVisibleColumn(Column* column)
{
	// Get the columns index and remove it.
	int32 index = _VisibleColumnIndexOf(column);
	fVisibleColumns.RemoveItem(index);
	// remove header
	fHeaderView->RemoveHeader(index);
	// Update the graphics stuff.
	_UpdateColumnXOffsets(index, true);
//	_InvalidateVisibleColumns(index, -1);
}

// _VisibleColumnAt
Column*
ColumnListView::_VisibleColumnAt(int32 index) const
{
	return (Column*)fVisibleColumns.ItemAt(index);
}

// _VisibleColumnIndexOf
int32
ColumnListView::_VisibleColumnIndexOf(Column* column) const
{
	return fVisibleColumns.IndexOf((void*)column);
}

// _VisibleColumnIndexOf
//
// Returns the index of the visible column that contains the supplied
// coordinates (measured in view coordinates).
// Returns -1 in case no such column exists.
int32
ColumnListView::_VisibleColumnIndexOf(BPoint point) const
{
	return _VisibleColumnIndexOf(point.x);
}

// _VisibleColumnIndexOf
//
// Returns the index of the visible column that contains the supplied
// X coordinate (measured in view coordinates).
// Returns -1 in case no such column exists.
int32
ColumnListView::_VisibleColumnIndexOf(float x) const
{
	// search column -- slightly inefficient
	for (int32 i = 0; Column* column = _VisibleColumnAt(i); i++) {
		BRect rect(_VisibleColumnFrame(column));
		if (rect.left <= x && rect.right >= x)
			return i;
	}
	return -1;
}

// _CountVisibleColumns
int32
ColumnListView::_CountVisibleColumns() const
{
	return fVisibleColumns.CountItems();
}

// _VisibleColumnPosition
//
// See _VisibleColumnPosition(Column*).
BPoint
ColumnListView::_VisibleColumnPosition(int32 index) const
{
	return _VisibleColumnPosition(_VisibleColumnAt(index));
}

// _VisibleColumnPosition
//
// Returns the suplied column's position (left top coordinates) measured in
// virtual scrolling coordinates, that is, it is correctly offset for
// directly being used for drawing.
// Returns an undefined position in case of a NULL pointer.
BPoint
ColumnListView::_VisibleColumnPosition(Column* column) const
{
	BPoint position;
	if (column) {
		position = _ItemsRect().LeftTop() - fCurrentScrollOffset;
		position.x += column->XOffset();
	}
	return position;
}

// _VisibleColumnFrame
//
// See _VisibleColumnFrame(Column*).
BRect
ColumnListView::_VisibleColumnFrame(int32 index) const
{
	return _VisibleColumnFrame(_VisibleColumnAt(index));
}

// _VisibleColumnFrame
//
// Returns the suplied column's frame measured in virtual scrolling
// coordinates, that is, its coordinates are correctly offset for directly
// being used for drawing.
// Returns an invalid rectangle in case of a NULL pointer.
BRect
ColumnListView::_VisibleColumnFrame(Column* column) const
{
	BRect frame;
	if (column) {
		frame.SetLeftTop(_VisibleColumnPosition(column));
		frame.right = frame.left + column->Width();
		frame.bottom = frame.top + DataRect().Height();
	}
	return frame;
}

// _ItemPosition
//
// See _ItemPosition(ColumnListItem*).
BPoint
ColumnListView::_ItemPosition(int32 index) const
{
	return _ItemPosition(ItemAt(index));
}

// _ItemPosition
//
// Returns the suplied item's position (left top coordinates) measured in
// virtual scrolling coordinates, that is, it is correctly offset for
// directly being used for drawing.
// Returns an undefined position in case of a NULL pointer.
BPoint
ColumnListView::_ItemPosition(ColumnListItem* item) const
{
	BPoint position;
	if (item) {
		position = _ItemsRect().LeftTop() - fCurrentScrollOffset;
		position.y += item->YOffset();
	}
	return position;
}

// _ItemFrame
//
// See _ItemFrame(ColumnListItem*).
BRect
ColumnListView::_ItemFrame(int32 index) const
{
	return _ItemFrame(ItemAt(index));
}

// _ItemFrame
//
// Returns the suplied item's frame measured in virtual scrolling coordinates,
// that is, its coordinates are correctly offset for directly being used for
// drawing.
// Returns an invalid rectangle in case of a NULL pointer.
BRect
ColumnListView::_ItemFrame(ColumnListItem* item) const
{
	BRect frame;
	if (item) {
		frame.SetLeftTop(_ItemPosition(item));
		frame.right = frame.left + DataRect().Width();
		frame.bottom = frame.top + item->Height();
	}
	return frame;
}

// _FindSelectedItem
//
// The supplied index must be valid and contained in the selection list.
int32
ColumnListView::_FindSelectedItem(int32 index) const
{
	return _FindSelectionInsertionIndex(index);
}

// _FindSelectionInsertionIndex
//
// The supplied index must be valid.
int32
ColumnListView::_FindSelectionInsertionIndex(int32 index) const
{
	// binary search
	int32 lower = 0;
	int32 upper = fSelectedItems.CountItems();
	while (lower < upper) {
		int32 mid = (lower + upper) / 2;
		int32 midIndex = CurrentSelection(mid);
		if (index <= midIndex)
			upper = mid;
		else
			lower = mid + 1;
	}
	return lower;
}

// _InternalSelectionChanged
void
ColumnListView::_InternalSelectionChanged()
{
	if (fSelectionDepth == 0) {
		_InternalInvoke(fSelectionMessage);
		SelectionChanged();
	}
}

// _InternalInvoke
void
ColumnListView::_InternalInvoke(const BMessage* message)
{
	if (message) {
		BMessage clone(*message);
		clone.AddInt64("when", system_time());
		clone.AddPointer("source", (void*)this);
		int32 count = fSelectedItems.CountItems();
		for (int32 i = 0; i < count; i++)
			clone.AddInt32("index", CurrentSelection(i));
		InvokeNotify(&clone);
	}
}

// _ReindexColumns
//
// Adds /offset/ to each column index >= /index/.
void
ColumnListView::_ReindexColumns(int32 index, int32 offset)
{
	for (int32 i = 0; Column* column = _OrderedColumnAt(i); i++) {
		if (column->Index() >= index)
			column->SetIndex(column->Index() + offset);
	}
}

// _ReindexSelectedItems
//
// Adds /offset/ to each selected item index >= /index/.
void
ColumnListView::_ReindexSelectedItems(int32 index, int32 offset)
{
	int32 count = fSelectedItems.CountItems();
	for (int32 i = 0; i < count; i++) {
		int32 item = CurrentSelection(i);
		if (item >= index) {
			fSelectedItems.RemoveItem(i);
			fSelectedItems.AddItem((void*)(item + offset + 1), i);
		}
	}
}

// _RebuildSelectionList
//
// Empties the selection list and rebuilds it from the items list and the
// items selected flag.
void
ColumnListView::_RebuildSelectionList()
{
	fSelectedItems.MakeEmpty();
	for (int32 i = 0; ColumnListItem* item = ItemAt(i); i++) {
		if (item->IsSelected())
			fSelectedItems.AddItem((void*)(i + 1));
	}
}

// _UpdateColumnXOffsets
void
ColumnListView::_UpdateColumnXOffsets(int32 index, bool update)
{
	int32 count = CountColumns();
	if (index < 0)
		index = 0;
	if (index > count)
		index = count;
	int32 firstDifference = -1;
	int32 lastDifference = -1;
	Column* column = _VisibleColumnAt(index - 1);
	float offset = (column) ? column->XOffset() + column->Width() + 1.0 : 0.0;
	for (; Column* column = _VisibleColumnAt(index); index++) {
		if (offset != column->XOffset()) {
			if (firstDifference == - 1)
				firstDifference = index;
			lastDifference = index;
		}
		column->SetXOffset(offset);
		offset += column->Width() + 1.0f;
	}
	// update list view width
	BRect dataRect(DataRect());
	dataRect.right = dataRect.left + offset - 1.0f;
	SetDataRect(dataRect);
//	if (list width != offset) {
//		if (firstDifference == - 1)
//			firstDifference = index;
//		lastDifference = index;
//	}
	// force an update to validate the invalid columns
	if (update && firstDifference != -1) {
		_InvalidateVisibleColumns(firstDifference,
								  lastDifference - firstDifference + 1);
	}
}

// _UpdateItemYOffsets
void
ColumnListView::_UpdateItemYOffsets(int32 index, bool update)
{
	int32 count = CountItems();
	if (index < 0)
		index = 0;
	if (index > count)
		index = count;
	int32 firstDifference = -1;
	int32 lastDifference = -1;
	ColumnListItem* item = ItemAt(index - 1);
	float offset = (item) ? item->YOffset() + item->Height() + 1.0f : 0.0f;
	for (; ColumnListItem* item = ItemAt(index); index++) {
		if (offset != item->YOffset()) {
			if (firstDifference == - 1)
				firstDifference = index;
			lastDifference = index;
		}
		item->SetYOffset(offset);
		offset += item->Height() + 1.0f;
	}
	// update list view height
	BRect dataRect(DataRect());
	dataRect.bottom = dataRect.top + offset - 1.0f;
	SetDataRect(dataRect);
//	if (list height != offset) {
//		if (firstDifference == - 1)
//			firstDifference = index;
//		lastDifference = index;
//	}
	// force an update to validate the invalid columns
	if (update && firstDifference != -1) {
		_InvalidateItems(firstDifference,
						 lastDifference - firstDifference + 1);
	}
}

// _UpdateVisibleColumnList
/*void
ColumnListView::_UpdateVisibleColumnList(bool update)
{
	// Keep a copy of the old list for comparison.
	BList oldBVisibleList(fVisibleList);
	fVisibleList.MakeEmpty();
	// build the new list and find out where it differs first from the old one
	int32 index = 0;
	int32 firstDifference = -1;
	for (int32 i = 0; Column* column = _OrderedColumnAt(i); i++) {
		if (column->IsVisible()) {
			fVisibleList.AddItem((void*)column);
			if (firstDifference == -1 &&
				oldVisibleList.ItemAt(index) == (void*)column) {
				fistDifference = index;
			}
			index++;
		}
	}
	// the old list is longer than the new one
//	if (firstDifference == -1 &&
//		fVisibleList.CountItems() < oldVisibleList.CountItems()) {
//		firstDifference = fVisibleList.CountItems();
//	}
	if (firstDifference != -1)
		_UpdateColumnXOffsets(firstDifference, false);
	// force an update to validate the invalid columns
	if (update && firstDifference != -1) {
		_InvalidateItems(firstDifference,
						 lastDifference - firstDifference + 1);
	}
}*/

// _InvalidateVisibleColumns
void
ColumnListView::_InvalidateVisibleColumns(int32 index, int32 count)
{
	// nothing to do
	if (count == 0)
		return;
	int32 columnCount = _CountVisibleColumns();
	BRect itemsRect(_ItemsRect());
	BRect rect(itemsRect);
	if (index < 0 || index >= columnCount) {
		// invalidate the region on right of the last column
		if (Column* column = _VisibleColumnAt(columnCount - 1))
			rect.left = _VisibleColumnFrame(column).right + 1.0f;
	} else {
		if (Column* column = _VisibleColumnAt(index))
			rect.left = _VisibleColumnPosition(column).x;
		if (count > 0) {
			if (index + count > columnCount)
				count = columnCount - index;
			if (Column* column = _VisibleColumnAt(index + count - 1))
				rect.right = _VisibleColumnFrame(column).right;
		}
	}
	rect = rect & itemsRect;
	if (rect.IsValid())
		Invalidate(rect);
}

// _InvalidateItems
//
// Invalidates /count/ items starting at /index/. If /index/ < 0 or
// >= the item count the region below the last item is invalidated.
// Otherwise if count < 0 the whole region starting at index is invalidated.
void
ColumnListView::_InvalidateItems(int32 index, int32 count)
{
	// nothing to do
	if (count == 0)
		return;
	int32 itemCount = CountItems();
	BRect itemsRect(_ItemsRect());
	BRect rect(itemsRect);
	if (index < 0 || index >= itemCount) {
		// invalidate the region below the last item
		if (ColumnListItem* item = ItemAt(itemCount - 1))
			rect.top = _ItemFrame(item).bottom + 1.0f;
	} else {
		if (ColumnListItem* item = ItemAt(index))
			rect.top = _ItemPosition(item).y;
		if (count > 0) {
			if (index + count > itemCount)
				count = itemCount - index;
			if (ColumnListItem* item = ItemAt(index + count - 1))
				rect.bottom = _ItemFrame(item).bottom;
		}
	}
	rect = rect & itemsRect;
	if (rect.IsValid())
		Invalidate(rect);
}

// _Layout
void
ColumnListView::_Layout()
{
	BRect rect(_HeaderViewRect());
	fHeaderView->MoveTo(rect.LeftTop());
	fHeaderView->ResizeTo(rect.Width(), rect.Height());
}

// _HeaderViewRect
//
// Returns the rectangle that contains the headers, measured in virtual
// scrolling coordinates, that is its coordinates are correctly offset for
// directly being used for drawing the headers.
BRect
ColumnListView::_HeaderViewRect() const
{
	BRect rect;
	rect.left = -fCurrentScrollOffset.x;
	rect.right = rect.left + std::max(DataRect().Width(), Bounds().Width());
	rect.top = 0.0f;
	rect.bottom = rect.top + fHeaderView->Height();
	return rect;
}

// _ItemsRect
//
// Returns the part of the view, that is intended to display the items
// (measured in view coorinates).
BRect
ColumnListView::_ItemsRect() const
{
	BRect rect(Bounds());
	rect.top += fHeaderView->Height() + 1.0f;
	return rect;
}

// _ActualItemsRect
//
// Returns the part of the _ItemsRect() that is actually covered by items.
BRect
ColumnListView::_ActualItemsRect() const
{
	BRect rect(_ItemsRect());
	if (_CountVisibleColumns() == 0 || CountItems() == 0) {
		rect.right = rect.left - 1.0f;
		rect.bottom = rect.top - 1.0f;
	} else {
		Column* lastColumn = _VisibleColumnAt(_CountVisibleColumns() - 1);
		ColumnListItem* lastItem = ItemAt(CountItems() - 1);
		rect.right = std::min(rect.right, _VisibleColumnFrame(lastColumn).right);
		rect.bottom = std::min(rect.bottom, _ItemFrame(lastItem).bottom);
	}
	return rect;
}

// _CompareItems
int
ColumnListView::_CompareItems(ColumnListItem* item1, ColumnListItem* item2)
{
	int result = 0;
	if (fCompareFunction && (fPrimarySortColumn || fSecondarySortColumn)) {
		// make clear what key(s) to compare for
		Column* primary = fPrimarySortColumn;
		Column* secondary = fSecondarySortColumn;
		int inverse1 = 1;
		int inverse2 = 1;
		if (secondary && (secondary->Flags() & COLUMN_SORT_INVERSE))
			inverse2 =  -1;
		if (primary) {
			if ((primary->Flags() & COLUMN_SORT_INVERSE))
				inverse1 = -1;
		} else {
			primary = secondary;
			inverse1 = inverse2;
			secondary = NULL;
		}
		result = item_cmp(fCompareFunction, primary, secondary, inverse1,
						  inverse2, item1, item2);
	}
	return result;
}

// _Sort
void
ColumnListView::_Sort()
{
	int32 count = CountItems();
	if (!fCompareFunction || (!fPrimarySortColumn && !fSecondarySortColumn)
		|| count < 2) {
		return;
	}
	// copy the items to an array
	ColumnListItem** items = new ColumnListItem*[2 * count];
	ColumnListItem** buffer = items + count;
	for (int32 i = 0; i < count; i++)
		items[i] = ItemAt(i);
	ItemGreater greater(fCompareFunction, fPrimarySortColumn,
						fSecondarySortColumn);
	ColumnListItem** sortedItems = merge_sort_iterative(items, buffer, count,
														greater);
	// empty the items list and insert the items
	fItems.MakeEmpty();
	for (int32 i = 0; i < count; i++)
		fItems.AddItem((void*)sortedItems[i], i);
	delete[] items;
	// update the selection list
	_RebuildSelectionList();
	_UpdateItemYOffsets(0, false);
}

// _FindSortedInsertionIndex
//
// Finds the index in the items list, /item/ has to be inserted according
// to the current sort compare function. The item is inserted after all
// equal items.
int32
ColumnListView::_FindSortedInsertionIndex(ColumnListItem* item) const
{
	int32 index = 0;
	if (item && fCompareFunction
		&& (fPrimarySortColumn || fSecondarySortColumn)) {
		ItemGreater greater(fCompareFunction, fPrimarySortColumn,
							fSecondarySortColumn);
		// binary search
		int32 lower = 0;
		int32 upper = CountItems();
		while (lower < upper) {
			int32 mid = (lower + upper) / 2;
			ColumnListItem* midItem = ItemAt(mid);
			if (greater(midItem, item)) {
				upper = mid;
			} else
				lower = mid + 1;
		}
		index = lower;
	}
	return index;
}


// _ChangeState
void
ColumnListView::_ChangeState(State* state)
{
	delete fState;
	fState = state;
}


// item_cmp
//
// A helper function to compare two items.
// /secondary/ may be NULL.
// PRE: /primary/, /cmpFunc/, /item1/, /item2/ valid.
static inline
int
item_cmp(column_list_compare_function* cmpFunc,
		 Column* primary, Column* secondary,
		 int inverse1, int inverse2,
		 ColumnListItem* item1, ColumnListItem* item2)
{
	int cmp = (*cmpFunc)(item1, item2, primary) * inverse1;
	if (cmp == 0) {
		if (secondary)
			return (*cmpFunc)(item1, item2, secondary) * inverse2;
	}
	return cmp;
}


