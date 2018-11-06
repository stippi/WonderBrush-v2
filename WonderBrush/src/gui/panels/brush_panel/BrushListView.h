// BrushListView.h

#ifndef BRUSH_LIST_VIEW_H
#define BRUSH_LIST_VIEW_H

#include "ListViews.h"

enum {
	MSG_NEW_BRUSH					= 'nwbr',
	MSG_RENAME_BRUSH				= 'rnbr',
	MSG_DELETE_BRUSH				= 'dlbr',
	MSG_DELETE_ALL					= 'dlal',
};

class Bitmap;
class BrushStroke;

class BrushItem : public SimpleItem {
 public:
							BrushItem(BrushStroke* stroke,
									  const char* name);
	virtual					~BrushItem();

							// BListItem
	virtual	void			Update(BView* owner, const BFont* font);

							// SimpleItem
	virtual	void			Draw(BView* owner, BRect frame,
								 uint32 flags);

			void			GetIconRect(BRect& itemFrame) const;

			BrushStroke*	GetBrushStroke() const
								{ return fStroke; }
			float			BrushScale() const
								{ return fBrushScale; }

 private:
			void			_FillBitmap(BBitmap* strokeBitmap,
										BBitmap* bitmap,
										BrushStroke* stroke) const;

			BrushStroke*	fStroke;
			BBitmap*		fStrokeBitmap;
			BBitmap*		fBitmap;
			float			fBrushScale;

			bool			fDirty;
			rgb_color		fBackgroundColor;
};

class BMenu;
class BMenuItem;
class CanvasView;

class BrushListView : public SimpleListView {
 public:
							BrushListView(CanvasView* view,
										  BRect frame,
										  BMessage* selectionChangeMessage = NULL);
							~BrushListView();

							// BListView
	virtual	void			FrameResized(float width, float height);
	virtual void			MessageReceived(BMessage* message);
	virtual void			SelectionChanged();

							// SimpleListView
	virtual	void			DoubleClicked(int32 index);
	virtual	BListItem*		CloneItem(int32 atIndex) const;

							// BrushListView
			void			SetCanvasView(CanvasView* canvasView);
			void			SetBrushMenu(BMenu* menu);
			void			UpdateStrings();

			void			ClearList();

 private:

			CanvasView*		fCanvasView;

			BMenu*			fBrushM;
			BMenuItem*		fNewMI;
			BMenuItem*		fRenameMI;
			BMenuItem*		fRemoveMI;
			BMenuItem*		fRemoveAllMI;
};

#endif // BRUSH_LIST_VIEW_H
