// ColumnListView.h

#ifndef COLUMN_LIST_VIEW_H
#define COLUMN_LIST_VIEW_H

#include <Invoker.h>
#include <List.h>
#include <View.h>

#include "ColumnListViewStates.h"
#include "Scrollable.h"

class Column;
class ColumnHeaderView;
class ColumnListItem;
struct column_list_view_colors;

// item compare function type
typedef int column_list_compare_function(const ColumnListItem*,
										 const ColumnListItem*,
										 const Column*);

enum selection_mode {
	CLV_SINGLE_SELECTION,
	CLV_MULTIPLE_SELECTION,
};

class ColumnListView : public BView, public BInvoker, public Scrollable {
 public:
								ColumnListView();
	virtual						~ColumnListView();

	virtual	void				AttachedToWindow();
	virtual	void				Draw(BRect updateRect);
	virtual	void				Draw(BView* view, BRect updateRect);
	virtual	void				FrameResized(float width, float height);
	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				MakeFocus(bool focusState = true);
	virtual	void				MouseDown(BPoint point);
	virtual	void				MouseMoved(BPoint point, uint32 transit,
										   const BMessage* message);
	virtual	void				MouseUp(BPoint point);

//	virtual	status_t			Invoke(BMessage* message = NULL);

	virtual	void				ScrollOffsetChanged(BPoint oldOffset,
													BPoint newOffset);

	// column management
			void				AddColumn(Column* column, int32 index = -1);
			Column*				RemoveColumn(int32 index);
			bool				RemoveColumn(Column* column);
//			bool				RemoveColumns(int32 index, int32 count);
			Column*				FindColumn(const char* name) const;
			Column*				ColumnAt(int32 index) const;
			int32				ColumnIndexOf(Column* column) const;
//			int32				ColumnIndexOf(BPoint point) const;
			int32				OrderedColumnIndexOf(Column* column) const;
			int32				OrderedColumnIndexOf(int32 index) const;
			int32				CountColumns() const;
			void				HideColumn(int32 index);
			void				HideColumn(Column* column);
			void				ShowColumn(int32 index);
			void				ShowColumn(Column* column);

			void				InvalidateHeader(int32 index);

	// item management
	virtual	void				AddItem(ColumnListItem* item,
										int32 index = -1);
	virtual	ColumnListItem*		RemoveItem(int32 index);
	virtual	bool				RemoveItem(ColumnListItem* item);
	virtual	bool				RemoveItems(int32 index, int32 count);
			bool				HasItem(ColumnListItem* item) const;
			ColumnListItem*		ItemAt(int32 index) const;
			int32				IndexOf(ColumnListItem* item) const;
			int32				IndexOf(BPoint) const;
			int32				IndexOf(float y) const;
			int32				CountItems() const;
			bool				IsEmpty() const;
			BRect				ItemFrame(int32 index) const
										{ return _ItemFrame(index); }
	virtual	void				MakeEmpty();
			void				InvalidateItem(int32 index);
			void				InvalidateItem(ColumnListItem* item);
			void				ItemChanged(int32 index);
			void				ScrollToItem(int32 index);
	virtual	bool				InitiateDrag(BPoint point, int32 index,
											 bool wasSelected);
	virtual	void				ItemDoubleClicked(int32 index);

	// selection
			void				SetSelectionMode(selection_mode mode);
	inline	selection_mode		SelectionMode() const
										{ return fSelectionMode; }
			int32				CurrentSelection(int32 index = 0) const;
			void				Deselect(int32 index);
			void				DeselectAll();
			void				DeselectExcept(int32 start, int32 finish);
			bool				IsItemSelected(int32 index) const;
			void				ScrollToSelection();
			void				Select(int32 index, bool extend = false);
			void				Select(int32 start, int32 finish,
									   bool extend = false);
	virtual	void				SelectionChanged();
			int32				CountSelectedItems() const;

	virtual void				SetInvocationMessage(BMessage* message);
			BMessage*			InvocationMessage() const;
			uint32				InvocationCommand() const;
	virtual	void				SetSelectionMessage(BMessage* message);
			BMessage*			SelectionMessage() const;
			uint32				SelectionCommand() const;

			void				ResizeColumn(int32 index, float width);
			void				ResizeColumn(Column* column, float width);
			void				MoveColumn(int32 index, int32 dest);
			void				MoveColumn(Column* column, int32 dest);

			status_t			PrimarySortColumn(int32 *outIndex,
												  bool *outInverse) const;
			status_t			SecondarySortColumn(int32 *outIndex,
													bool *outInverse) const;

			void				SetPrimarySortColumn(int32 index,
													 bool inverse = false);
			void				SetSecondarySortColumn(int32 index,
													   bool inverse = false);
			void				SetSortCompareFunction(
									column_list_compare_function* function);
			void				Sort();

	// colors
			void				SetColors(
									const column_list_view_colors* colors);
			const column_list_view_colors*	Colors() const;

	// private in principle, but called by the header view
			void				ResizeVisibleColumn(int32 index, float width);
			void				MoveVisibleColumn(int32 oldIndex,
												  int32 newIndex);
			void				MoveVisibleColumns(int32 index, int32 dest,
												   int32 count);

			void				DisableScrolling();
			void				EnableScrolling();

			void				SetPrimarySortVisibleColumn(int32 index,
															bool inverse);
			void				SetSecondarySortVisibleColumn(int32 index,
															  bool inverse);

 private:
			BList				fColumns;
			BList				fVisibleColumns;
			BList				fItems;
			BList				fSelectedItems;
			ColumnHeaderView*	fHeaderView;
			BPoint				fCurrentScrollOffset;
			bool				fScrollingEnabled;
			Column*				fPrimarySortColumn;
			Column*				fSecondarySortColumn;
			BMessage*			fInvocationMessage;
			BMessage*			fSelectionMessage;
			int32				fSelectionDepth;	// to avoid multiple msgs
			ColumnListViewStates::State*	fState;
			column_list_view_colors*	fColors;
			column_list_compare_function*	fCompareFunction;
			selection_mode		fSelectionMode;
			int32				fLastClickedIndex;
			bigtime_t			fLastClickTime;
			bigtime_t			fDoubleClickTime;
			bool				fMouseInside;

			Column*				_RemoveOrderedColumn(int32 index);
			void				_HideOrderedColumn(int32 index);
			void				_HideOrderedColumn(Column* column);
			void				_ShowOrderedColumn(int32 index);
			void				_ShowOrderedColumn(Column* column);
			Column*				_OrderedColumnAt(int32 index) const;

			void				_SetPrimarySortOrderedColumn(int32 index,
															 bool inverse);
			void				_SetSecondarySortOrderedColumn(int32 index,
															   bool inverse);

			int32				_CalculateVisibleColumnIndex(Column* column);
			void				_AddVisibleColumn(Column* column);
			void				_RemoveVisibleColumn(Column* column);

			Column*				_VisibleColumnAt(int32 index) const;
			int32				_VisibleColumnIndexOf(Column* column) const;
			int32				_VisibleColumnIndexOf(BPoint point) const;
			int32				_VisibleColumnIndexOf(float x) const;
			int32				_CountVisibleColumns() const;

			BPoint				_VisibleColumnPosition(int32 index) const;
			BPoint				_VisibleColumnPosition(Column* column) const;
			BRect				_VisibleColumnFrame(int32 index) const;
			BRect				_VisibleColumnFrame(Column* column) const;

			BPoint				_ItemPosition(int32 index) const;
			BPoint				_ItemPosition(ColumnListItem* item) const;
			BRect				_ItemFrame(int32 index) const;
			BRect				_ItemFrame(ColumnListItem* item) const;

			int32				_FindSelectedItem(int32 index) const;
			int32				_FindSelectionInsertionIndex(
										int32 index) const;

			void				_InternalSelectionChanged();
			void				_InternalInvoke(const BMessage* message);

			void				_ReindexColumns(int32 index, int32 offset);
			void				_ReindexSelectedItems(int32 index,
													  int32 offset);
			void				_RebuildSelectionList();

			void				_UpdateColumnXOffsets(int32 index,
													  bool update);
			void				_UpdateItemYOffsets(int32 index, bool update);
//			void				_UpdateVisibleColumnList(bool update);

			void				_InvalidateVisibleColumns(int32 index,
														  int32 count);
			void				_InvalidateItems(int32 index, int32 count);

			void				_Layout();

			BRect				_HeaderViewRect() const;
			BRect				_ItemsRect() const;
			BRect				_ActualItemsRect() const;

	// sorting
			int					_CompareItems(ColumnListItem* item1,
											  ColumnListItem* item2);
			void				_Sort();
			int32				_FindSortedInsertionIndex(
										ColumnListItem* item) const;

	// state stuff
			void				_ChangeState(ColumnListViewStates::State*
											 state);

	friend class ColumnListViewStates::State;
//	friend class ColumnListViewStates::DraggingState;
	friend class ColumnListViewStates::IgnoreState;
	friend class ColumnListViewStates::InsideState;
	friend class ColumnListViewStates::OutsideState;
	friend class ColumnListViewStates::PressedState;
//	friend class ColumnListViewStates::ResizingState;

};

#endif	// COLUMN_LIST_VIEW_H

