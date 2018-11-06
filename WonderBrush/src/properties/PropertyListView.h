// PropertyListView.h

#ifndef PROPERTY_LIST_VIEW_H
#define PROPERTY_LIST_VIEW_H

#include <List.h>
#include <View.h>

#include "MouseWheelFilter.h"
#include "ObjectObserver.h"
#include "Scrollable.h"

class BClipboard;
class BMenu;
class BMenuItem;
class CanvasView;
class Layer;
class PropertyItemView;
class PropertyObject;
class Stroke;

class PropertyListView : public BView,
						 public Scrollable,
						 private BList,
						 public ObjectObserver,
						 public MouseWheelTarget {
 public:
								PropertyListView(CanvasView* canvasView);
	virtual						~PropertyListView();

								// BView
	virtual	void				AttachedToWindow();
	virtual	void				FrameResized(float width, float height);
	virtual	void				Draw(BRect updateRect);
	virtual	void				MakeFocus(bool focus);

	virtual	void				MouseDown(BPoint where);

	virtual	void				MessageReceived(BMessage* message);

								// ObjectObserver
	virtual	void				ObjectChanged(const Observable* object);

								// Scrollable
	virtual	void				ScrollOffsetChanged(BPoint oldOffset,
													BPoint newOffset);

								// MouseWheelTarget
	virtual	bool				MouseWheelChanged(float x, float y) { return false; }

								// PropertyListView
			bool				TabFocus(bool shift);

			void				SetMenu(BMenu* menu);

			void				SetTo(Layer* layer, Stroke* object);
			void				SetTo(Layer* layer,
									  Stroke** objects, int32 count);

			void				Select(PropertyItemView* item);
			void				DeselectAll();

			void				Clicked(PropertyItemView* item);
			void				DoubleClicked(PropertyItemView* item);

			void				UpdateObject();

			void				UpdateStrings();

 private:
			void				_SetTo(PropertyObject* object);

			bool				_AddItem(PropertyItemView* item);
			PropertyItemView*	_RemoveItem(int32 index);
			PropertyItemView*	_ItemAt(int32 index) const;
			int32				_CountItems() const;

			void				_MakeEmpty();

			BRect				_ItemsRect() const;
			void				_LayoutItems();

			void				_CheckMenuStatus();

			BClipboard*			fClipboard;

			BMenu*				fSelectM;
			BMenuItem*			fSelectAllMI;
			BMenuItem*			fSelectNoneMI;
			BMenuItem*			fInvertSelectionMI;

			BMenu*				fPropertyM;
			BMenuItem*			fCopyMI;
			BMenuItem*			fPasteMI;

			PropertyObject*		fPropertyObject;
			Layer*				fLayer;
			Stroke*				fObject;
			Stroke**			fObjects;
			int32				fObjectCount;

			PropertyItemView*	fLastClickedItem;

			CanvasView*			fCanvasView;

			bool				fSuspendUpdates;
};

#endif // PROPERTY_LIST_VIEW_H
