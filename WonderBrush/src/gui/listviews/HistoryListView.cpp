// HistoryListView.cpp

#include <new>
#include <stdio.h>

#include <ClassInfo.h>
#include <Clipboard.h>
#include <Menu.h>
#include <MenuItem.h>
#include <String.h>
#include <Window.h>

#include "support.h"

#include "CanvasView.h"
#include "DuplicatePanel.h"
#include "History.h"
#include "IconButton.h"
#include "LanguageManager.h"
#include "Layer.h"
#include "PropertyListView.h"
#include "ShapeStroke.h"
#include "Stroke.h"
#include "TextStroke.h"

#include "FilterFactory.h"

#include "DuplicateAction.h"
#include "FreezeAction.h"
#include "MergeShapesAction.h"
#include "TextToShapeAction.h"

#include "HistoryListView.h"

using std::nothrow;

enum {
	MSG_SELECT_ALL			= B_SELECT_ALL,
	MSG_SELECT_NONE			= 'slnn',
	MSG_INVERT_SELECTION	= 'invs',

	MSG_CONVERT_TO_SHAPE	= 'cnsp',
	MSG_MERGE_SHAPES		= 'mrgs',
};

#define TEXT_OFFSET			5.0

// PaintObjectItem
void
ObjectItemPainter::PaintObjectItem(BView* owner, BRect itemFrame, uint32)
{
	owner->SetHighColor( 0, 0, 0, 255 );
	font_height fh;
	owner->GetFontHeight( &fh );
	const char* text = fName.String();
	BString truncatedString( text );
	owner->TruncateString( &truncatedString, B_TRUNCATE_MIDDLE,
						   itemFrame.Width() - TEXT_OFFSET - 4.0 );
	float height = itemFrame.Height();
	float textHeight = fh.ascent + fh.descent;
	BPoint textPoint;
	textPoint.x = itemFrame.left + TEXT_OFFSET;
	textPoint.y = itemFrame.top
				  + ceilf(height / 2.0 - textHeight / 2.0
				  		  + fh.ascent);
	owner->DrawString(truncatedString.String(), textPoint);
}

// HistoryItem class
HistoryItem::HistoryItem(const char* name)
	: SimpleItem(name),
	  fPainter(NULL)
{
}

// HistoryItem class
HistoryItem::HistoryItem(const char* name, ObjectItemPainter* painter)
	: SimpleItem(name),
	  fPainter(painter)
{
}

// destructor
HistoryItem::~HistoryItem()
{
	delete fPainter;
}

// Update
void
HistoryItem::Update(BView* owner, const BFont* font)
{
	SimpleItem::Update(owner, font);
}

// Draw
void
HistoryItem::DrawItem(BView* owner, BRect frame, bool even)
{
	if (fPainter) {
		SimpleItem::DrawBackground(owner, frame, even);
		// additional grafics to distinguish tools
		fPainter->PaintObjectItem(owner, frame, 0);
	} else {
		SimpleItem::DrawItem(owner, frame, even);
	}
}

// SetPainter
void
HistoryItem::SetPainter(ObjectItemPainter* painter)
{
	delete fPainter;
	fPainter = painter;
}

// HistoryListView class
HistoryListView::HistoryListView(BRect frame,
								 BMessage* selectionChangeMessage)
	: SimpleListView(frame, selectionChangeMessage),
	  fCanvasView(NULL),
	  fHistoryMenu(NULL),
	  fLayer(NULL),
	  fPropertyListView(NULL),
	  fClipboard(new BClipboard("WonderBrush modifiers")),
	  fAllM(NULL),
	  fSelectM(NULL),
	  fFilterM(NULL),
	  fDeleteIB(NULL),
	  fSupressSelectionChanged(false),
	  fAllowColorDrops(true)
{
	SetDragCommand(MSG_MODIFIER_DATA);
}

// destructor
HistoryListView::~HistoryListView()
{
	delete fClipboard;
}

// MessageReceived
void
HistoryListView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_DUPLICATE:
			if (fLayer) {
				if (message->HasPointer("objects")) {
					if (History* history = fLayer->GetHistory()) {
						BList objects;
						Stroke* object;
						int32 insertIndex = 0;
						for (int32 i = 0; message->FindPointer("objects", i, (void**)&object) >= B_OK; i++) {
							int32 index = history->IndexOf(object);
							if (index >= 0) {
								objects.AddItem(object);
								if (insertIndex <= index)
									insertIndex = index + 1;
							}
						}
						int32 count = objects.CountItems();
						if (count > 0) {
							int32 duplicateCount;
							float xOffset, yOffset, rotation, xScale, yScale;
							if (message->FindInt32("count", &duplicateCount) >= B_OK &&
								message->FindFloat("x offset", &xOffset) >= B_OK &&
								message->FindFloat("y offset", &yOffset) >= B_OK &&
								message->FindFloat("rotation", &rotation) >= B_OK &&
								message->FindFloat("x scale", &xScale) >= B_OK &&
								message->FindFloat("y scale", &yScale) >= B_OK) {

								Action* action = new DuplicateAction(fLayer,
																	 (const Stroke**)objects.Items(),
																	 count, insertIndex,
																	 duplicateCount,
																	 xOffset, yOffset,
																	 rotation,
																	 xScale, yScale);
								fCanvasView->Perform(action);
							}
						}
					}
				} else {
					if (History* history = fLayer->GetHistory()) {
						BMessage* m = new BMessage(MSG_DUPLICATE);
						for (int32 i = 0; Stroke* object = history->ModifierAt(CurrentSelection(i)); i++)
							m->AddPointer("objects", (void*)object);
						if (m->HasPointer("objects"))
							new DuplicatePanel(Window(), this, m);
						else
							delete m;
					}
				}
			}
			break;
		case MSG_CONVERT_TO_SHAPE:
			if (fLayer) {
				History* history = fLayer->GetHistory();
				if (history && CurrentSelection() >= 0) {
					BList textObjects(20);
					for (int32 i = 0; Stroke* object = history->ModifierAt(CurrentSelection(i)); i++) {
						TextStroke* textObject = dynamic_cast<TextStroke*>(object);
						if (textObject) {
							textObjects.AddItem((void*)textObject);
						}
					}
					int32 count = textObjects.CountItems();
					if (count > 0) {
						TextStroke** objects = new(nothrow) TextStroke*[count];
						if (objects) {
							memcpy(objects, textObjects.Items(), count * sizeof(void*));
							fCanvasView->Perform(new TextToShapeAction(fLayer,
																	   objects,
																	   count));
						}
					}
				}
			}
			break;
		case MSG_MERGE_SHAPES:
			if (fLayer) {
				History* history = fLayer->GetHistory();
				if (history && CurrentSelection() >= 0) {
					BList shapeObjects(20);
					for (int32 i = 0; Stroke* object = history->ModifierAt(CurrentSelection(i)); i++) {
						ShapeStroke* shapeObject = dynamic_cast<ShapeStroke*>(object);
						if (shapeObject) {
							shapeObjects.AddItem((void*)shapeObject);
						}
					}
					int32 count = shapeObjects.CountItems();
					if (count > 1) {
						ShapeStroke** objects = new(nothrow) ShapeStroke*[count];
						if (objects) {
							memcpy(objects, shapeObjects.Items(), count * sizeof(void*));
							fCanvasView->Perform(new MergeShapesAction(fLayer,
																	   objects,
																	   count));
						}
					}
				}
			}
			break;
		case MSG_ADD_FILTER: {
			if (fCanvasView) {
				int32 id;
				if (message->FindInt32("id", &id) >= B_OK) {
					BList filters(1);
					FilterObject* filter = FilterFactory::Default()->MakeFilter(fCanvasView, id);
					if (filter) {
						filters.AddItem((void*)filter);
						fCanvasView->AddModifiers(filters, CountItems());
					}
				}
			}
			break;
		}
		case B_PASTE:
			if (fCanvasView) {
				rgb_color color;
				if (restore_color_from_message(message, color) >= B_OK &&
					fAllowColorDrops && fFocusedIndex < 0) {
					BList selectedIndices;
					int32 index;
					for (int32 i = 0; (index = CurrentSelection(i)) >= 0; i++)
						selectedIndices.AddItem((void*)index);
					fCanvasView->ChangeColors(selectedIndices, color);
				} else if (fClipboard->Lock()) {
					if (BMessage* data = fClipboard->Data()) {
						BList strokes(20);
						BMessage strokeArchive;
						for (int32 i = 0; data->FindMessage("modifier", i, &strokeArchive) >= B_OK; i++) {
							if (BArchivable* archivable = instantiate_object(&strokeArchive)) {
								// see if this is actually a stroke
								Stroke* stroke = cast_as(archivable, Stroke);
								if (!stroke || !strokes.AddItem((void*)stroke))
									delete archivable;
							}
							strokeArchive.MakeEmpty();
						}
						fCanvasView->AddModifiers(strokes, CountItems());
					}
					fClipboard->Unlock();
				}
			}
			break;
		case B_CUT:
		case B_COPY:
			if (fLayer) {
				History* history = fLayer->GetHistory();
				if (history && CurrentSelection() >= 0) {
					if (fClipboard->Lock()) {
						if (BMessage* data = fClipboard->Data()) {
							fClipboard->Clear();
							for (int32 i = 0; Stroke* stroke = history->ModifierAt(CurrentSelection(i)); i++) {
								BMessage strokeArchive;
								stroke->Archive(&strokeArchive);
								data->AddMessage("modifier", &strokeArchive);
							}
							fClipboard->Commit();
						}
						fClipboard->Unlock();
					}
					if (/*fFocusedIndex < 0 && */message->what == B_CUT)
						RemoveSelected();
					_CheckMenuStatus();
				}
			}
			break;
		case MSG_FREEZE_HISTORY:
			if (/*fFocusedIndex < 0 && */fLayer && fCanvasView) {
				fCanvasView->Perform(new FreezeAction(fLayer));
			}
			break;
		case MSG_REMOVE_ALL:
			if (/*fFocusedIndex < 0 && */fLayer) {
				BList items(CountItems());
				for (int32 i = 0;BListItem* item = ItemAt(i); i++)
					items.AddItem((void*)item);
				RemoveItemList(items);
			}
			break;
		case MSG_REMOVE:
			if (/*fFocusedIndex < 0 && */fLayer) {
				BList items(CountSelectedItems());
				for (int32 i = 0;BListItem* item = ItemAt(CurrentSelection(i)); i++)
					items.AddItem((void*)item);
				RemoveItemList(items);
			}
			break;
		// MSG_SELECT_ALL is handled in base class already
		case MSG_SELECT_NONE:
			DeselectAll();
			break;
		case MSG_INVERT_SELECTION: {
			int32 count = CountItems();
			if (count > 0) {
				fSupressSelectionChanged = true;
				if (BWindow* window = Window())
					window->BeginViewTransaction();
				for (int32 i = 0; i < count; i++) {
					if (IsItemSelected(i))
						Deselect(i);
					else
						Select(i, true);
				}
				if (BWindow* window = Window())
					window->EndViewTransaction();
				fSupressSelectionChanged = false;
				SelectionChanged();
			}
			break;
		}
		default:
			SimpleListView::MessageReceived(message);
			break;
	}
}

// SelectionChanged
void
HistoryListView::SelectionChanged()
{
	if (!fSupressSelectionChanged) {

		_CheckMenuStatus();

		// inform canvas view
		if (fCanvasView)
			fCanvasView->ModifierSelectionChanged(CurrentSelection(0) >= 0);

		// make property object and update property list view
		BList objects(20);
		bool textObjectsSelected = false;
		int32 shapeObjectsSelected = 0;
		if (fLayer) {
			if (History* history = fLayer->GetHistory()) {
				for (int32 i = 0; Stroke* object = history->ModifierAt(CurrentSelection(i)); i++) {
					if (fPropertyListView)
						objects.AddItem((void*)object);
					if (dynamic_cast<TextStroke*>(object))
						textObjectsSelected = true;
					if (dynamic_cast<ShapeStroke*>(object))
						shapeObjectsSelected++;
				}
			}
			if (fPropertyListView)
				fPropertyListView->SetTo(fLayer, (Stroke**)objects.Items(),
										 objects.CountItems());
		}
		fTextToShapeMI->SetEnabled(textObjectsSelected);
		fMergeShapesMI->SetEnabled(shapeObjectsSelected > 1);
	}
}

// MoveItems
void
HistoryListView::MoveItems(BList& items, int32 index)
{
	if (fCanvasView)
		fCanvasView->MoveHistory(items, index);
//	SimpleListView::MoveItems(items, index);
}

// CopyItems
void
HistoryListView::CopyItems(BList& items, int32 index)
{
	if (fCanvasView)
		fCanvasView->CopyHistory(items, index);
//	SimpleListView::CopyItems(items, index);
}

// RemoveItemList
void
HistoryListView::RemoveItemList(BList& items)
{
	if (fCanvasView)
		fCanvasView->RemoveHistory(items);
	_CheckMenuStatus();
}

// DoubleClicked
void
HistoryListView::DoubleClicked(int32 index)
{
	if (fCanvasView) {
		fCanvasView->EditModifier(index);
	}
}

// SetItemFocused
void
HistoryListView::SetItemFocused(int32 index)
{
	SimpleListView::SetItemFocused(index);

	_CheckMenuStatus();
}

// MakeDragMessage
void
HistoryListView::MakeDragMessage(BMessage* message) const
{
	SimpleListView::MakeDragMessage(message);
	if (message) {
		message->AddPointer("layer", (void*)fLayer);
	}
}

// SetCanvasView
void
HistoryListView::SetCanvasView(CanvasView* canvasView)
{
	fCanvasView = canvasView;
}

// SetHistoryMenu
void
HistoryListView::SetHistoryMenu(BMenu* menu)
{
	fHistoryMenu = menu;
	if (fHistoryMenu) {

		fSelectM = new BMenu("Select");
		fSelectAllMI = new BMenuItem("All", new BMessage(MSG_SELECT_ALL));
		fSelectM->AddItem(fSelectAllMI);
		fSelectNoneMI = new BMenuItem("None", new BMessage(MSG_SELECT_NONE));
		fSelectM->AddItem(fSelectNoneMI);
		fInvertSelectionMI = new BMenuItem("Invert Selection", new BMessage(MSG_INVERT_SELECTION));
		fSelectM->AddItem(fInvertSelectionMI);
		fSelectM->SetTargetForItems(this);

		fHistoryMenu->AddItem(fSelectM);

		fHistoryMenu->AddSeparatorItem();

		fCutMI = new BMenuItem("Cut", new BMessage(B_CUT), 'X');
		fHistoryMenu->AddItem(fCutMI);
		fCopyMI = new BMenuItem("Copy", new BMessage(B_COPY), 'C');
		fHistoryMenu->AddItem(fCopyMI);
		fPasteMI = new BMenuItem("Paste", new BMessage(B_PASTE), 'V');
		fHistoryMenu->AddItem(fPasteMI);

		fHistoryMenu->AddSeparatorItem();

		fDuplicateMI = new BMenuItem("Duplicate"B_UTF8_ELLIPSIS, new BMessage(MSG_DUPLICATE));
		fHistoryMenu->AddItem(fDuplicateMI);

		fHistoryMenu->AddSeparatorItem();

		fRemoveMI = new BMenuItem("Remove", new BMessage(MSG_REMOVE));
		fHistoryMenu->AddItem(fRemoveMI);

		fHistoryMenu->AddSeparatorItem();

		fAllM = new BMenu("All");
		fFreezeMI = new BMenuItem("Freeze", new BMessage(MSG_FREEZE_HISTORY));
		fAllM->AddItem(fFreezeMI);
		fRemoveAllMI = new BMenuItem("Remove", new BMessage(MSG_REMOVE_ALL));
		fAllM->AddItem(fRemoveAllMI);
		fAllM->SetTargetForItems(this);

		fHistoryMenu->AddItem(fAllM);

		fSpecialM = new BMenu("Special");
		fTextToShapeMI = new BMenuItem("Text to Shape", new BMessage(MSG_CONVERT_TO_SHAPE));
		fSpecialM->AddItem(fTextToShapeMI);
		fMergeShapesMI = new BMenuItem("Merge Shapes", new BMessage(MSG_MERGE_SHAPES));
		fSpecialM->AddItem(fMergeShapesMI);
		fSpecialM->SetTargetForItems(this);

		fHistoryMenu->AddItem(fSpecialM);

		BMenu* super = fHistoryMenu->Supermenu();
		if (!super) {
			super = fHistoryMenu;
			fHistoryMenu->AddSeparatorItem();
		}

		fFilterM = new BMenu("Filter");
		super->AddItem(fFilterM);
		_RebuildFilterMenu();

		fHistoryMenu->SetTargetForItems(this);
		// disable menus
		_CheckMenuStatus();
	}
}

// SetDeleteButton
void
HistoryListView::SetDeleteButton(IconButton* button)
{
	fDeleteIB = button;
	if (fDeleteIB) {
		fDeleteIB->SetMessage(new BMessage(MSG_REMOVE));
		fDeleteIB->SetTarget(this);
	}
}

// SetLayer
void
HistoryListView::SetLayer(Layer* layer, bool force)
{
	if (layer != fLayer || force) {
		ClearList();
		fLayer = layer;
		if (fLayer) {
			// do a visual copy of the layers history
			History* history = fLayer->GetHistory();
			for (int32 i = 0; Stroke* stroke = history->ModifierAt(i); i++)
				_AddModifier(stroke);
			// enable menu
			if (fHistoryMenu && !fHistoryMenu->IsEnabled())
				fHistoryMenu->SetEnabled(true);
			// enable menu
			if (fFilterM && !fFilterM->IsEnabled())
				fFilterM->SetEnabled(true);
		} else {
			// disable menu
			if (fHistoryMenu && fHistoryMenu->IsEnabled())
				fHistoryMenu->SetEnabled(false);
			// disable menu
			if (fFilterM && fFilterM->IsEnabled())
				fFilterM->SetEnabled(false);
		}
		fPropertyListView->SetTo(fLayer, NULL);
	}
}

// SetPropertyList
void
HistoryListView::SetPropertyList(PropertyListView* listView)
{
	fPropertyListView = listView;
}

// SetPainter
void
HistoryListView::SetPainter(int32 index, ObjectItemPainter* painter)
{
	if (HistoryItem* item = dynamic_cast<HistoryItem*>(ItemAt(index))) {
		item->SetPainter(painter);
		InvalidateItem(index);
	}
}

// AddModifier
bool
HistoryListView::AddModifier(Stroke* stroke)
{
	if (stroke && _AddModifier(stroke)) {
		Select(CountItems() - 1, false);
		ScrollToSelection();
		return true;
	}
	return false;
}

// AddModifier
bool
HistoryListView::AddModifier(Stroke* stroke, int32 index)
{
	if (stroke && _AddModifier(stroke, index)) {
		Select(index, false);
		ScrollToSelection();
		return true;
	}
	return false;
}

// AddModifiers
bool
HistoryListView::AddModifiers(Stroke** strokes, int32* indices, int32 count)
{
	if (strokes && indices && count > 0) {
		DeselectAll();
		for (int32 i = 0; i < count; i++) {
			if (_AddModifier(strokes[i], indices[i]))
				Select(indices[i], true);
		}
		ScrollToSelection();
	}
	return true;
}

// RemoveModifier
void
HistoryListView::RemoveModifier()
{
	if (BListItem* item = RemoveItem(CountItems() - 1))
		delete item;
	_CheckMenuStatus();
}

// ClearList
void
HistoryListView::ClearList()
{
	BList temp(CountItems());
	for (int32 i = 0; BListItem* item = ItemAt(i); i++)
		temp.AddItem((void*)item);
	MakeEmpty();
	for (int32 i = 0; BListItem* item = (BListItem*)temp.ItemAt(i); i++)
		delete item;
	// disable menu items
	_CheckMenuStatus();
/*	int32 last = CountItems();
	while (BListItem* item = RemoveItem(--last))
		delete item;
	// disable menu items
	_CheckMenuStatus();*/
}

// SetAllowColorDrops
void
HistoryListView::SetAllowColorDrops(bool allow)
{
	fAllowColorDrops = allow;
}

// UpdateStrings
void
HistoryListView::UpdateStrings()
{
	LanguageManager* manager = LanguageManager::Default();

	if (fSelectM && fAllM) {
		fSelectM->Superitem()->SetLabel(manager->GetString(OBJECT_SELECTION, "Select"));
		fSelectAllMI->SetLabel(manager->GetString(SELECT_ALL_OBJECTS, "All"));
		fSelectNoneMI->SetLabel(manager->GetString(SELECT_NO_OBJECTS, "None"));
		fInvertSelectionMI->SetLabel(manager->GetString(INVERT_SELECTION, "Invert Selection"));

		fAllM->Superitem()->SetLabel(manager->GetString(ALL_OBJECTS, "All"));
		fFreezeMI->SetLabel(manager->GetString(FREEZE, "Freeze"));
		fRemoveAllMI->SetLabel(manager->GetString(DELETE, "Remove"));

		fCutMI->SetLabel(manager->GetString(CUT, "Cut"));
		fCopyMI->SetLabel(manager->GetString(COPY, "Copy"));
		fPasteMI->SetLabel(manager->GetString(PASTE, "Paste"));
		fRemoveMI->SetLabel(manager->GetString(DELETE, "Remove"));

		BString duplicate = manager->GetString(DUPLICATE, "Duplicate");
		duplicate << B_UTF8_ELLIPSIS;
		fDuplicateMI->SetLabel(duplicate.String());

		fSpecialM->Superitem()->SetLabel(manager->GetString(SPECIAL, "Special"));
		fTextToShapeMI->SetLabel(manager->GetString(TEXT_TO_SHAPE, "Text to Shape"));
		fMergeShapesMI->SetLabel(manager->GetString(MERGE_SHAPE_OBJECTS, "Merge Shapes"));

		fFilterM->Superitem()->SetLabel(manager->GetString(ADD_FILTER, "Filter"));
		_RebuildFilterMenu();
	}

	SetLayer(fLayer, true);
}

// _AddModifier
bool
HistoryListView::_AddModifier(Stroke* stroke, int32 index)
{
	if (index < 0)
		index = CountItems();

	SimpleItem* item = new HistoryItem(stroke->Name(), stroke->Painter());
	if (!AddItem(item, index)) {
		delete item;
		return false;
	}
	_CheckMenuStatus();
	return true;
}

// _CheckMenuStatus
void
HistoryListView::_CheckMenuStatus()
{
	bool gotSelection = CurrentSelection(0) >= 0;
	bool gotItems = CountItems() > 0;

	fCopyMI->SetEnabled(gotSelection);
	fSelectNoneMI->SetEnabled(gotSelection);
	fSelectAllMI->SetEnabled(gotItems);
	fInvertSelectionMI->SetEnabled(gotItems);

	fSelectM->SetEnabled(gotItems);
	fAllM->SetEnabled(gotItems);

	fSpecialM->SetEnabled(gotSelection);

	if (fDeleteIB)
		fDeleteIB->SetEnabled(gotSelection);

/*	if (fFocusedIndex >= 0) {
		fCutMI->SetEnabled(false);
		fPasteMI->SetEnabled(false);
		fRemoveMI->SetEnabled(false);
		fAllM->SetEnabled(false);
		fFilterM->SetEnabled(false);
	} else {*/
		fFilterM->SetEnabled(true);

		fCutMI->SetEnabled(gotSelection);
		fRemoveMI->SetEnabled(gotSelection);
		fDuplicateMI->SetEnabled(gotSelection);

		bool clipboardHasData = false;
		if (fClipboard->Lock()) {
			if (BMessage* data = fClipboard->Data()) {
				clipboardHasData = data->HasMessage("modifier");
			}
			fClipboard->Unlock();
		}

		fPasteMI->SetEnabled(clipboardHasData);
//	}
}

// _RebuildFilterMenu
void
HistoryListView::_RebuildFilterMenu()
{
	while (BMenuItem* item = fFilterM->RemoveItem((int32)0))
		delete item;
	FilterFactory* factory = FilterFactory::Default();
	int32 id;
	const char* name;
	for (int32 i = 0; factory->GetFilterInfoAt(i, &name, &id); i++) {
		BMessage* message = new BMessage(MSG_ADD_FILTER);
		message->AddInt32("id", id);
		fFilterM->AddItem(new BMenuItem(name, message));
	}
	fFilterM->SetTargetForItems(this);
}

