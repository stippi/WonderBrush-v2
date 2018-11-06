// CanvasListView.cpp

#include <stdio.h>

#include <ListItem.h>
#include <Message.h>
#include <Mime.h>
#include <Window.h>

#include "Canvas.h"

#include "CanvasListView.h"

#if USE_SIMPLE_LISTVIEW
class CanvasListItem : public SimpleItem {
 public:
				CanvasListItem(Canvas* c)
					: SimpleItem(c->Name()),
#else
class CanvasListItem : public BStringItem {
 public:
				CanvasListItem(Canvas* c)
					: BStringItem(c->Name()),
#endif
					  canvas(c)
				{
				}
	Canvas* 	canvas;
};

enum {
	MSG_DRAG_CANVAS		= 'drgc',
};

// constructor
CanvasListView::CanvasListView(const char* name,
							   BMessage* message, BHandler* target)
#if USE_SIMPLE_LISTVIEW
	: SimpleListView(BRect(0.0, 0.0, 20.0, 20.0), name,
					 NULL, B_SINGLE_SELECTION_LIST),
#else
	: MListView(B_SINGLE_SELECTION_LIST),
#endif
	  fMessage(message),
	  fCurrentCanvas(NULL),
	  fAutoSelect(true)
{
#if USE_SIMPLE_LISTVIEW
	SetDragCommand(MSG_DRAG_CANVAS);
#endif
	SetTarget(target);
}

// destructor
CanvasListView::~CanvasListView()
{
	delete fMessage;
}

// SelectionChanged
void
CanvasListView::SelectionChanged()
{
	CanvasListItem* item = (CanvasListItem*)ItemAt(CurrentSelection(0));
	if (item) {
		if (fMessage && item->canvas != fCurrentCanvas) {
			BMessage message(*fMessage);
			message.AddPointer("canvas", (void*)item->canvas);
			Invoke(&message);
		}
	} else {
		if (fAutoSelect && fCurrentCanvas)
			Select(IndexOf(_ItemForCanvas(fCurrentCanvas)));
	}
}

#if USE_SIMPLE_LISTVIEW

// MessageReceived
void
CanvasListView::MessageReceived(BMessage* message)
{
	if (message->what == B_SIMPLE_DATA) {
		Window()->PostMessage(message);
	} else {
		SimpleListView::MessageReceived(message);
	}
}

// MakeDragMessage
void
CanvasListView::MakeDragMessage(BMessage* message) const
{
	SimpleListView::MakeDragMessage(message);
	if (CanvasListItem* item = dynamic_cast<CanvasListItem*>(ItemAt(CurrentSelection(0)))) {
		message->AddInt32("be:actions", B_COPY_TARGET);
		message->AddInt32("be:actions", B_TRASH_TARGET);

		message->AddString("be:types", B_FILE_MIME_TYPE);
		message->AddString("be:filetypes", "image/x-WonderBrush");
		message->AddString("be:type_descriptions", "WonderBrush Image");

		message->AddString("be:clip_name", item->canvas->Name());

		message->AddString("be:originator", "WonderBrush");
		message->AddPointer("be:originator_data", (void*)item->canvas);
	}
}

// AcceptDragMessage
bool
CanvasListView::AcceptDragMessage(const BMessage* message) const
{
	if (message->what == B_SIMPLE_DATA)
		return true;
	return SimpleListView::AcceptDragMessage(message);
}

// SetDropTargetRect
void
CanvasListView::SetDropTargetRect(const BMessage* message, BPoint where)
{
	if (message->what == B_SIMPLE_DATA) {
		_SetDropAnticipationRect(Bounds());
	} else {
		SimpleListView::SetDropTargetRect(message, where);
	}
}

// MoveItems
void
CanvasListView::MoveItems(BList& items, int32 toIndex)
{
	fAutoSelect = false;
	SimpleListView::MoveItems(items, toIndex);
	fAutoSelect = true;
}

// CopyItems
void
CanvasListView::CopyItems(BList& items, int32 toIndex)
{
	MoveItems(items, toIndex);
}

// RemoveItemList
void
CanvasListView::RemoveItemList(BList& indices)
{
	// not allowed
}

// CloneItem
BListItem*
CanvasListView::CloneItem(int32 index) const
{
	if (CanvasListItem* item = dynamic_cast<CanvasListItem*>(ItemAt(index))) {
		return new CanvasListItem(item->canvas);
	}
	return NULL;
}

#endif // USE_SIMPLE_LISTVIEW

// AddCanvas
bool
CanvasListView::AddCanvas(Canvas* canvas)
{
	bool success = false;
	if (canvas)
		 success = AddItem(new CanvasListItem(canvas));
	return success;
}

// RemoveCanvas
bool
CanvasListView::RemoveCanvas(Canvas* canvas)
{
	fAutoSelect = false;
	CanvasListItem* item = _ItemForCanvas(canvas);
	bool success = RemoveItem(item);
	if (success) {
		delete item;
	}
	fAutoSelect = true;
	return success;
}

// SetCanvas
void
CanvasListView::SetCanvas(Canvas* canvas)
{
	fAutoSelect = false;
	fCurrentCanvas = canvas;
	if (CanvasListItem* item = _ItemForCanvas(canvas)) {
		Select(IndexOf(item));
	} else {
		DeselectAll();
	}
	fAutoSelect = true;
}

// UpdateCanvasName
void
CanvasListView::UpdateCanvasName(Canvas* canvas)
{
	if (canvas) {
		if (CanvasListItem* item = _ItemForCanvas(canvas)) {
			item->SetText(canvas->Name());
			Invalidate(ItemFrame(IndexOf(item)));
		}
	}
}

// _ItemForCanvas
CanvasListItem*
CanvasListView::_ItemForCanvas(Canvas* canvas) const
{
	for (int32 i = 0; CanvasListItem* item = dynamic_cast<CanvasListItem*>(ItemAt(i)); i++) {
		if (item->canvas == canvas)
			return item;
	}
	return NULL;
}

// -----CanvasListScrollView------------------------------------------------

// constructor
CanvasListScrollView::CanvasListScrollView(CanvasListView* listView)
	: BetterMScrollView(listView, false, true),
	  fListView(listView)
{
}

// layoutprefs
minimax
CanvasListScrollView::layoutprefs()
{
	mpm = BetterMScrollView::layoutprefs();

	if (fListView && fListView->CountItems() <= 1)
		mpm.mini.y = mpm.maxi.y = 0.0;

	return mpm;
}

// layout
BRect
CanvasListScrollView::layout(BRect frame)
{
	BScrollBar* scrollBar = ScrollBar(B_VERTICAL);

	if (frame.Height() < 10.0) {
		if (scrollBar && !scrollBar->IsHidden())
			scrollBar->Hide();
		if (fListView && !fListView->IsHidden())
			fListView->Hide();
	} else {
		if (scrollBar && scrollBar->IsHidden())
			scrollBar->Show();
		if (fListView && fListView->IsHidden())
			fListView->Show();
	}

	return BetterMScrollView::layout(frame);
}


