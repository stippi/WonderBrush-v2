// HistoryListView.h

#ifndef HISTORY_LIST_VIEW_H
#define HISTORY_LIST_VIEW_H

#include <AppDefs.h>
#include <String.h>

#include "ListViews.h"

enum {
	MSG_MODIFIER_DATA		= 'mddt',

	MSG_FREEZE_HISTORY		= 'frhs',
	MSG_REMOVE_ALL			= 'rmal',
	MSG_REMOVE				= 'rmov',

	MSG_DUPLICATE			= 'dupl',

	MSG_ADD_FILTER			= 'adfl',
};

class BClipboard;
class BView;
class IconButton;
class SwatchView;

class ObjectItemPainter {
 public:
							ObjectItemPainter(const char* name)
								: fName(name)
								{}
	virtual					~ObjectItemPainter()
								{}

	virtual	void			PaintObjectItem(BView* owner,
											BRect itemFrame,
											uint32 flags);
 private:
	BString					fName;
};

class HistoryItem : public SimpleItem {
 public:
							HistoryItem(const char* name);
							HistoryItem(const char* name,
										ObjectItemPainter* painter);
		virtual				~HistoryItem();

		virtual	void		Update(BView* owner, const BFont* font);


							// SimpleItem
		virtual	void		DrawItem(BView* owner, BRect frame,
								 bool even = false);

							// HistoryItem
				void		SetPainter(ObjectItemPainter* painter);

 private:
		ObjectItemPainter*	fPainter;
};

class BClipboard;
class BMenu;
class BMenuItem;
class CanvasView;
class History;
class Layer;
class PropertyListView;
class Stroke;

class HistoryListView : public SimpleListView {
 public:
							HistoryListView(BRect frame,
											BMessage* selectionChangeMessage = NULL);
							~HistoryListView();

							// BListView
	virtual void			MessageReceived(BMessage* message);
	virtual void			SelectionChanged();

							// SimpleListView
	virtual	void			MoveItems(BList& items, int32 toIndex);
	virtual	void			CopyItems(BList& items, int32 toIndex);
	virtual	void			RemoveItemList(BList& items);

	virtual	void			DoubleClicked(int32 index);
	virtual	void			SetItemFocused(int32 index);

	virtual	void			MakeDragMessage(BMessage* message) const;

							// HistoryListView
			void			SetCanvasView(CanvasView* canvasView);
			void			SetHistoryMenu(BMenu* menu);
			void			SetDeleteButton(IconButton* button);
			void			SetLayer(Layer* layer, bool force = false);
			const Layer*	GetLayer() const
								{ return fLayer; }
			void			SetPropertyList(PropertyListView* listView);

			void			SetPainter(int32 index,
									   ObjectItemPainter* painter);

			bool			AddModifier(Stroke* stroke);
			bool			AddModifier(Stroke* stroke, int32 index);
			bool			AddModifiers(Stroke** strokes, int32* indices,
										 int32 count);
			void			RemoveModifier();
			void			ClearList();

			void			SetAllowColorDrops(bool allow);

			void			UpdateStrings();

 private:
			bool			_AddModifier(Stroke* stroke, int32 index = -1);
			void			_CheckMenuStatus();
			void			_RebuildFilterMenu();

			CanvasView*		fCanvasView;
			BMenu*			fHistoryMenu;
			Layer*			fLayer;
			PropertyListView*	fPropertyListView;

			BClipboard*		fClipboard;

			BMenu*			fAllM;
			BMenuItem*		fFreezeMI;
			BMenuItem*		fRemoveAllMI;

			BMenu*			fSelectM;
			BMenuItem*		fSelectAllMI;
			BMenuItem*		fSelectNoneMI;
			BMenuItem*		fInvertSelectionMI;

			BMenuItem*		fCutMI;
			BMenuItem*		fCopyMI;
			BMenuItem*		fPasteMI;
			BMenuItem*		fRemoveMI;

			BMenuItem*		fDuplicateMI;

			BMenu*			fSpecialM;
			BMenuItem*		fTextToShapeMI;
			BMenuItem*		fMergeShapesMI;

			BMenu*			fFilterM;

			IconButton*		fDeleteIB;

			bool			fSupressSelectionChanged;
			bool			fAllowColorDrops;
};

#endif // HISTORY_LIST_VIEW_H
