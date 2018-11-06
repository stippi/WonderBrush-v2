// PropertyListView.cpp

#include <stdio.h>
#include <string.h>

#include <ClassInfo.h>
#include <Clipboard.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Message.h>
#include <Window.h>

#include "CanvasView.h"
#include "EditModifierAction.h"
#include "History.h"
#include "HistoryListView.h"
#include "LanguageManager.h"
#include "Layer.h"
#include "Scrollable.h"
#include "ScrollView.h"
#include "Stroke.h"
#include "PastePropertiesAction.h"
#include "Property.h"
#include "PropertyItemView.h"
#include "PropertyObject.h"

#include "PropertyListView.h"

enum {
	MSG_COPY_PROPERTIES		= 'cppr',
	MSG_PASTE_PROPERTIES	= 'pspr',

	MSG_SELECT_ALL			= B_SELECT_ALL,
	MSG_SELECT_NONE			= 'slnn',
	MSG_INVERT_SELECTION	= 'invs',
};

// TabFilter class

class TabFilter : public BMessageFilter {
 public:
	TabFilter(PropertyListView* target)
		: BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE),
		  fTarget(target)
		{
		}
	virtual	~TabFilter()
		{
		}
	virtual	filter_result	Filter(BMessage* message, BHandler** target)
		{
			filter_result result = B_DISPATCH_MESSAGE;
			switch (message->what) {
				case B_UNMAPPED_KEY_DOWN:
				case B_KEY_DOWN: {
					uint32 key;
					uint32 modifiers;
					if (message->FindInt32("raw_char", (int32*)&key) >= B_OK
						&& message->FindInt32("modifiers", (int32*)&modifiers) >= B_OK)
						if (key == B_TAB && fTarget->TabFocus(modifiers & B_SHIFT_KEY))
							result = B_SKIP_MESSAGE;
					break;
				}
				default:
					break;
			}
			return result;
		}
 private:
 	PropertyListView*		fTarget;
};


// constructor
PropertyListView::PropertyListView(CanvasView* canvasView)
	: BView(BRect(0.0, 0.0, 100.0, 100.0), NULL, B_FOLLOW_NONE,
			B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE),
	  Scrollable(),
	  BList(20),
	  fClipboard(new BClipboard("WonderBrush properties")),
	  fPropertyObject(NULL),
	  fLayer(NULL),
	  fObject(NULL),
	  fObjects(NULL),
	  fObjectCount(0),
	  fLastClickedItem(NULL),
	  fCanvasView(canvasView),
	  fSuspendUpdates(false)
{
	SetLowColor(255, 255, 255, 255);
	SetHighColor(0, 0, 0, 255);
	SetViewColor(B_TRANSPARENT_32_BIT);
}

// destructor
PropertyListView::~PropertyListView()
{
	delete fClipboard;
	delete fPropertyObject;
	delete[] fObjects;
}

// AttachedToWindow
void
PropertyListView::AttachedToWindow()
{
	Window()->AddCommonFilter(new MouseWheelFilter(this));
	Window()->AddCommonFilter(new TabFilter(this));
}

// FrameResized
void
PropertyListView::FrameResized(float width, float height)
{
	SetVisibleSize(width, height);
}

// Draw
void
PropertyListView::Draw(BRect updateRect)
{
	if (!fSuspendUpdates)
		FillRect(updateRect, B_SOLID_LOW);
}

// MakeFocus
void
PropertyListView::MakeFocus(bool focus)
{
	if (focus != IsFocus()) {
		BView::MakeFocus(focus);
		if (ScrollView* scrollView = dynamic_cast<ScrollView*>(Parent()))
			scrollView->ChildFocusChanged(focus);
	}
}

// MouseDown
void
PropertyListView::MouseDown(BPoint where)
{
	if (!(modifiers() & B_SHIFT_KEY)) {
		DeselectAll();
	}
	MakeFocus(true);
}

// MessageReceived
void
PropertyListView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_PASTE_PROPERTIES:
			if (fCanvasView && (fObject || fObjectCount > 0) && fClipboard->Lock()) {
				if (BMessage* data = fClipboard->Data()) {
					PropertyObject propertyObject;
					BMessage propertyArchive;
					for (int32 i = 0; data->FindMessage("property", i, &propertyArchive) >= B_OK; i++) {
						if (BArchivable* archivable = instantiate_object(&propertyArchive)) {
							// see if this is actually a stroke
							Property* property = cast_as(archivable, Property);
							if (!property || !propertyObject.AddProperty(property))
								delete archivable;
						}
					}
					if (propertyObject.CountProperties() > 0) {
						PastePropertiesAction* action = NULL;
						if (fObject) {
							action = new PastePropertiesAction(fCanvasView,
															   fLayer,
															   &fObject, 1,
															   &propertyObject);
						} else {
							action = new PastePropertiesAction(fCanvasView,
															   fLayer,
															   fObjects, fObjectCount,
															   &propertyObject);
						}
						fCanvasView->Perform(action);
					}
				}
				fClipboard->Unlock();
			}
			break;
		case MSG_COPY_PROPERTIES:
			if (fPropertyObject) {
				if (fClipboard->Lock()) {
					if (BMessage* data = fClipboard->Data()) {
						fClipboard->Clear();
						for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
							if (item->IsSelected()) {
								const Property* property = item->GetProperty();
								if (property) {
									BMessage propertyArchive;
									if (property->Archive(&propertyArchive) >= B_OK) {
										data->AddMessage("property", &propertyArchive);
									}
								}
							}
						}
						fClipboard->Commit();
					}
					fClipboard->Unlock();
				}
				_CheckMenuStatus();
			}
			break;
		case MSG_SELECT_ALL:
			for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
				item->SetSelected(true);
			}
			_CheckMenuStatus();
			break;
		case MSG_SELECT_NONE:
			for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
				item->SetSelected(false);
			}
			_CheckMenuStatus();
			break;
		case MSG_INVERT_SELECTION:
			for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
				item->SetSelected(!item->IsSelected());
			}
			_CheckMenuStatus();
			break;
		default:
			BView::MessageReceived(message);
	}
}

// ObjectChanged
void
PropertyListView::ObjectChanged(const Observable* observable)
{
	const Stroke* object = dynamic_cast<const Stroke*>(observable);
	if (object) {
		if (object == fObject) {
			if (PropertyObject* propertyObject = object->MakePropertyObject()) {
				_SetTo(propertyObject);
			}
		}
	}
}

// HandleKeyDown
bool
PropertyListView::TabFocus(bool shift)
{
	bool result = false;
	PropertyItemView* item = NULL;
	if (IsFocus() && !shift) {
		item = _ItemAt(0);
	} else {
		int32 focussedIndex = -1;
		for (int32 i = 0; PropertyItemView* oldItem = _ItemAt(i); i++) {
			if (oldItem->IsFocused()) {
				focussedIndex = shift ? i - 1 : i + 1;
				break;
			}
		}
		item = _ItemAt(focussedIndex);
	}
	if (item) {
		item->MakeFocus(true);
		result = true;
	}
	return result;
}

// SetMenu
void
PropertyListView::SetMenu(BMenu* menu)
{
	fPropertyM = menu;
	if (fPropertyM) {

		fSelectM = new BMenu("Select");
		fSelectAllMI = new BMenuItem("All", new BMessage(MSG_SELECT_ALL));
		fSelectM->AddItem(fSelectAllMI);
		fSelectNoneMI = new BMenuItem("None", new BMessage(MSG_SELECT_NONE));
		fSelectM->AddItem(fSelectNoneMI);
		fInvertSelectionMI = new BMenuItem("Invert Selection", new BMessage(MSG_INVERT_SELECTION));
		fSelectM->AddItem(fInvertSelectionMI);
		fSelectM->SetTargetForItems(this);

		fPropertyM->AddItem(fSelectM);

		fPropertyM->AddSeparatorItem();

		fCopyMI = new BMenuItem("Copy", new BMessage(MSG_COPY_PROPERTIES));
		fPropertyM->AddItem(fCopyMI);
		fPasteMI = new BMenuItem("Paste", new BMessage(MSG_PASTE_PROPERTIES));
		fPropertyM->AddItem(fPasteMI);

		fPropertyM->SetTargetForItems(this);
		// disable menus
		_CheckMenuStatus();
	}
}

// SetTo
void
PropertyListView::SetTo(Layer* layer, Stroke* object)
{
	fLayer = layer;
	if (fObject != object) {
		if (fObject)
			fObject->RemoveObserver(this);

		fObject = object;
	
		PropertyObject* propertyObject = NULL;
		if (fObject) {
			propertyObject = fObject->MakePropertyObject();
			fObject->AddObserver(this);
		}
		_SetTo(propertyObject);
	}

	delete[] fObjects;
	fObjects = NULL;
	fObjectCount = 0;

	_CheckMenuStatus();
}

// SetTo
void
PropertyListView::SetTo(Layer* layer, Stroke** objects, int32 count)
{
	if (count == 0) {
		SetTo(layer, NULL);
	} else if (count == 1) {
		SetTo(layer, objects[0]);
	} else if (objects && count > 1) {
		SetTo(layer, NULL);
		fObjects = new Stroke*[count];
		fObjectCount = count;
		memcpy(fObjects, objects, sizeof(Stroke*) * count);
		_CheckMenuStatus();
	}
}

// ScrollOffsetChanged
void
PropertyListView::ScrollOffsetChanged(BPoint oldOffset, BPoint newOffset)
{
	ScrollBy(newOffset.x - oldOffset.x,
			 newOffset.y - oldOffset.y);
}

// Select
void
PropertyListView::Select(PropertyItemView* item)
{
	if (item) {
		if (modifiers() & B_SHIFT_KEY) {
			item->SetSelected(!item->IsSelected());
		} else if (modifiers() & B_OPTION_KEY) {
			item->SetSelected(true);
			int32 firstSelected = _CountItems();
			int32 lastSelected = -1;
			for (int32 i = 0; PropertyItemView* otherItem = _ItemAt(i); i++) {
				if (otherItem->IsSelected()) {
					 if (i < firstSelected)
					 	firstSelected = i;
					 if (i > lastSelected)
					 	lastSelected = i;
				}
			}
			if (lastSelected > firstSelected) {
				for (int32 i = firstSelected; PropertyItemView* otherItem = _ItemAt(i); i++) {
					if (i > lastSelected)
						break;
					otherItem->SetSelected(true);
				}
			}
		} else {
			for (int32 i = 0; PropertyItemView* otherItem = _ItemAt(i); i++) {
				otherItem->SetSelected(otherItem == item);
			}
		}
	}
	_CheckMenuStatus();
}

// DeselectAll
void
PropertyListView::DeselectAll()
{
	for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
		item->SetSelected(false);
	}
	_CheckMenuStatus();
}

// Clicked
void
PropertyListView::Clicked(PropertyItemView* item)
{
	fLastClickedItem = item;
}

// DoubleClicked
void
PropertyListView::DoubleClicked(PropertyItemView* item)
{
	if (fLastClickedItem == item)
		fCanvasView->EditModifier(fObject);
	fLastClickedItem = NULL;
}

// UpdateObject
void
PropertyListView::UpdateObject()
{
	if (fObject && fCanvasView) {

		fCanvasView->PrepareForObjectPropertyChange();

		EditModifierAction* action = new EditModifierAction(fLayer, fObject);
		BRect bounds = fObject->Bounds();
		if (fObject->SetToPropertyObject(fPropertyObject)) {
			// rebuild layer
			if (!fCanvasView->IsBusy()) {
				bounds = bounds | fObject->Bounds();
				fLayer->Touch(bounds);
				fCanvasView->RebuildLayer(fLayer, bounds);
			}
			// add to undo stack
			fCanvasView->Perform(action);
			// update object list item
			if (HistoryListView* listView = fCanvasView->GetHistoryListView()) {
				int32 index = fLayer->GetHistory()->IndexOf(fObject);
				listView->SetPainter(index, fObject->Painter());
			}
		} else
			delete action;

		fCanvasView->ObjectPropertyChangeDone();
	}
}

// UpdateStrings
void
PropertyListView::UpdateStrings()
{
	ObjectChanged(fObject);

	if (fSelectM) {
		LanguageManager* m = LanguageManager::Default();
	
		fSelectM->Superitem()->SetLabel(m->GetString(PROPERTY_SELECTION, "Select"));
		fSelectAllMI->SetLabel(m->GetString(SELECT_ALL_PROPERTIES, "All"));
		fSelectNoneMI->SetLabel(m->GetString(SELECT_NO_PROPERTIES, "None"));
		fInvertSelectionMI->SetLabel(m->GetString(INVERT_SELECTION, "Invert Selection"));
	
		fPropertyM->Superitem()->SetLabel(m->GetString(PROPERTY, "Property"));
		fCopyMI->SetLabel(m->GetString(COPY, "Copy"));
		if (fObjectCount > 0)
			fPasteMI->SetLabel(m->GetString(MULTI_PASTE, "Multi Paste"));
		else
			fPasteMI->SetLabel(m->GetString(PASTE, "Paste"));
	}
}

// _SetTo
void
PropertyListView::_SetTo(PropertyObject* object)
{
	if (fPropertyObject != object) {
		// try to do without rebuilding the list
		// it should in fact be pretty unlikely that this does not
		// work, but we keep being defensive
		if (fPropertyObject && object &&
			fPropertyObject->ContainsSameProperties(*object)) {
			// iterate over view items and update their value views
			for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
				const Property* property = object->PropertyAt(i);
				if (!item->SetProperty(property)) {
					fprintf(stderr, "property mismatch at %ld\n", i);
					break;
				}
			}
			// we didn't take on ownership, but kept our old object
			delete object;
		} else {
			// remember scroll pos, selection and focused item
			BPoint scrollOffset = ScrollOffset();
/*			BList selection(20);
			int32 focused = -1;
			for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
				if (item->IsSelected())
					selection.AddItem((void*)i);
				if (item->IsFocused())
					focused = i;
			}*/
			if (Window())
				Window()->BeginViewTransaction();
			fSuspendUpdates = true;

			// rebuild list
			_MakeEmpty();
	
			fPropertyObject = object;
			if (fPropertyObject) {
				// fill with content
				for (int32 i = 0; Property* property = fPropertyObject->PropertyAt(i); i++)
					_AddItem(new PropertyItemView(property));
				_LayoutItems();

				// restore scroll pos, selection and focus
				SetScrollOffset(scrollOffset);
/*				for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
					if (selection.HasItem((void*)i))
						item->SetSelected(true);
					if (i == focused)
						item->MakeFocus(true);
				}*/
				// enable menu items etc.
			} else {
				// disable menu items etc.
			}

			if (Window())
				Window()->EndViewTransaction();
			fSuspendUpdates = false;

			SetDataRect(_ItemsRect());
		}

		_CheckMenuStatus();
	}
}

// _AddItem
bool
PropertyListView::_AddItem(PropertyItemView* item)
{
	if (item && BList::AddItem((void*)item)) {
//		AddChild(item);
		item->SetListView(this);
		return true;
	}
	return false;
}

// _RemoveItem
PropertyItemView*
PropertyListView::_RemoveItem(int32 index)
{
	PropertyItemView* item = (PropertyItemView*)BList::RemoveItem(index);
	if (item) {
		item->SetListView(NULL);
		if (!RemoveChild(item))
			fprintf(stderr, "failed to remove view in PropertyListView::_RemoveItem()\n");
	}
	return item;
}

// _ItemAt
PropertyItemView*
PropertyListView::_ItemAt(int32 index) const
{
	return (PropertyItemView*)BList::ItemAt(index);
}

// _CountItems
int32
PropertyListView::_CountItems() const
{
	return BList::CountItems();
}

// _MakeEmpty
void
PropertyListView::_MakeEmpty()
{
	int32 count = _CountItems();
	while (PropertyItemView* item = _RemoveItem(count - 1)) {
		delete item;
		count--;
	}
	delete fPropertyObject;
	fPropertyObject = NULL;

	SetScrollOffset(BPoint(0.0, 0.0));
}

// _ItemsRect
BRect
PropertyListView::_ItemsRect() const
{
	float width = Bounds().Width();
	float height = -1.0;
	for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
		height += item->PreferredHeight() + 1.0;
	}
	if (height < 0.0)
		height = 0.0;
	return BRect(0.0, 0.0, width, height);
}

// _LayoutItems
void
PropertyListView::_LayoutItems()
{
	// figure out maximum label width
	float labelWidth = Bounds().Width() * 0.5;
	for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
		if (item->PreferredLabelWidth() > labelWidth)
			labelWidth = item->PreferredLabelWidth();
	}
	labelWidth = ceilf(labelWidth);
	// layout items
	float top = 0.0;
	float width = Bounds().Width();
	for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
		item->MoveTo(BPoint(0.0, top));
		float height = item->PreferredHeight();
		item->SetLabelWidth(labelWidth);
		item->ResizeTo(width, height);
		item->FrameResized(item->Bounds().Width(),
						   item->Bounds().Height());
		top += height + 1.0;

		AddChild(item);
	}
}

// _CheckMenuStatus
void
PropertyListView::_CheckMenuStatus()
{
	if (!fSuspendUpdates) {
		bool gotSelection = false;
		for (int32 i = 0; PropertyItemView* item = _ItemAt(i); i++) {
			if (item->IsSelected()) {
				gotSelection = true;
				break;
			}
		}
		fCopyMI->SetEnabled(gotSelection);
	
		bool clipboardHasData = false;
		if (fClipboard->Lock()) {
			if (BMessage* data = fClipboard->Data()) {
				clipboardHasData = data->HasMessage("property");
			}
			fClipboard->Unlock();
		}
	
		fPasteMI->SetEnabled(clipboardHasData);
		LanguageManager* m = LanguageManager::Default();
		if (fObjectCount > 0)
			fPasteMI->SetLabel(m->GetString(MULTI_PASTE, "Multi Paste"));
		else
			fPasteMI->SetLabel(m->GetString(PASTE, "Paste"));

		bool enableMenu = fObject || fObjectCount > 0;
		if (fPropertyM->IsEnabled() != enableMenu)
			fPropertyM->SetEnabled(enableMenu);

		bool gotItems = _CountItems() > 0;
		if (fSelectM->IsEnabled() != gotItems)
			fSelectM->SetEnabled(gotItems);
	}
}


