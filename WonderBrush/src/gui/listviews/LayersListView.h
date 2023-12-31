// LayersListView.h

#ifndef LAYERS_LIST_VIEW_H
#define LAYERS_LIST_VIEW_H

#include "ListViews.h"

enum {
	MSG_NEW_LAYER					= 'newl',
	MSG_CLONE_LAYER					= 'clon',
	MSG_MERGE_WITH_BACK_LAYER		= 'mrgb',
	MSG_MERGE_VISIBLE_LAYERS		= 'mrgv',
	MSG_CLEAR_LAYER					= 'clrl',
	MSG_RENAME_LAYER				= 'rnml',
	MSG_DELETE_LAYER				= 'dell',
};

class IconButton;
class LayersListView;

class LayerItem : public SimpleItem {
 public:
							LayerItem(LayersListView* parent,
									  const char* name);
		virtual				~LayerItem();

							// BListItem
		virtual	void		Update(BView* owner, const BFont* font);

							// SimpleItem
		virtual	void		DrawItem(BView* owner, BRect frame,
								 bool even = false);

				void		GetIconRect(BRect& itemFrame) const;

		static	BBitmap*	fEyeIcon;

 private:
		LayersListView*		fParent;
};

class BMenu;
class BMenuItem;
class CanvasView;
class LayerConfigView;

class LayersListView : public SimpleListView {
 public:
							LayersListView(BRect frame,
										   BMessage* selectionChangeMessage = NULL);
							~LayersListView();

							// BListView
	virtual void			MessageReceived(BMessage* message);
	virtual void			SelectionChanged();

							// SimpleListView
	virtual	void			MouseDown(BPoint where);
	virtual	void			MoveItems(BList& items, int32 toIndex);
	virtual	void			CopyItems(BList& items, int32 toIndex);
	virtual	void			RemoveItemList(BList& indices);
	virtual	void			DoubleClicked(int32 index);

	virtual	bool			AcceptDragMessage(const BMessage* message) const;
	virtual	void			SetDropTargetRect(const BMessage* message,
											  BPoint where);

							// LayersListView
			void			SetCanvasView(CanvasView* canvasView);
			void			SetConfigView(LayerConfigView* configView);

			void			SetToCanvas(Canvas* canvas, bool force = false);
			void			SetToLayer(int32 index);
			void			UpdateLayer(Layer* layer);
			void			Update();

			bool			AddLayer(const char* name,
									 int32 index, bool select = true);
			void			ClearList();
			Layer*			LayerFor(LayerItem* item) const;
			void			InvalidateLayer(int32 index);

			void			SetLayerMenu(BMenu* menu);
			void			SetModeMenu(BMenu* menu);
			void			SetDeleteButton(IconButton* button);
			void			UpdateStrings();

 private:
			void			_CheckVisibleItem();

	CanvasView*				fCanvasView;
	LayerConfigView*		fConfigView;
	Canvas*					fCanvas;

	BMenu*					fLayerMenu;
	BMenu*					fModeMenu;

	BMenuItem*				fNewMI;
	BMenuItem*				fCloneMI;
	BMenuItem*				fMergeBackMI;
	BMenuItem*				fMergeVisibleMI;
	BMenuItem*				fRenameMI;
	BMenuItem*				fDeleteMI;

	BMenuItem*				fNormalMI;
	BMenuItem*				fMultiplyMI;
	BMenuItem*				fInverseMultiplyMI;
	BMenuItem*				fLuminanceMI;
	BMenuItem*				fMultiplyAlphaMI;
	BMenuItem*				fMultiplyInverseAlphaMI;
	BMenuItem*				fReplaceRedMI;
	BMenuItem*				fReplaceGreenMI;
	BMenuItem*				fReplaceBlueMI;
	BMenuItem*				fDarkenMI;
	BMenuItem*				fLightenMI;
	BMenuItem*				fHardLightMI;
	BMenuItem*				fSoftLightMI;

	IconButton*				fDeleteIB;

	bool					fNewLayerOnObjectDrop;
};

#endif // LAYERS_LIST_VIEW_H
