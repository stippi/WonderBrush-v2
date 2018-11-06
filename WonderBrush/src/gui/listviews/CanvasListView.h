// CanvasListView.h

#ifndef CANVAS_TAB_VIEW_H
#define CANVAS_TAB_VIEW_H

#define USE_SIMPLE_LISTVIEW 1

#if USE_SIMPLE_LISTVIEW
# include "ListViews.h"
#else
# include <MListView.h>
#endif

#include "BetterMScrollView.h"

class Canvas;
class CanvasListItem;

#if USE_SIMPLE_LISTVIEW
class CanvasListView : public SimpleListView {
#else
class CanvasListView : public MListView {
#endif
 public:
								CanvasListView(const char* name,
											   BMessage* selectionChangeMessage = NULL,
											   BHandler* target = NULL);
	virtual						~CanvasListView();

								// BListView
	virtual	void				SelectionChanged();

#if USE_SIMPLE_LISTVIEW
	virtual	void				MessageReceived(BMessage* message);

	virtual	void				MakeDragMessage(BMessage* message) const;

	virtual	bool				AcceptDragMessage(const BMessage* message) const;
	virtual	void				SetDropTargetRect(const BMessage* message,
												  BPoint where);

	virtual	void				MoveItems(BList& items, int32 toIndex);
	virtual	void				CopyItems(BList& items, int32 toIndex);
	virtual	void				RemoveItemList(BList& indices);

	virtual	BListItem*			CloneItem(int32 atIndex) const;
#endif
								// CanvasListView
			bool				AddCanvas(Canvas* canvas);
			bool				RemoveCanvas(Canvas* canvas);


			void				SetCanvas(Canvas* canvas);
			void				UpdateCanvasName(Canvas* canvas);

 private:
			CanvasListItem*		_ItemForCanvas(Canvas* canvas) const;

			BMessage*			fMessage;
			Canvas*				fCurrentCanvas;
			bool				fAutoSelect;
};

class CanvasListScrollView : public BetterMScrollView {
 public:
								CanvasListScrollView(CanvasListView* listView);

	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);

 private:
			CanvasListView*		fListView;
};


#endif // CANVAS_TAB_VIEW_H
