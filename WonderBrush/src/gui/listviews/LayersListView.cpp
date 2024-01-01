// LayersListView.cpp

#include <stdio.h>

#include <Bitmap.h>
#include <Cursor.h>
#include <Entry.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Message.h>
#include <String.h>

#include "cursors.h"
#include "support.h"

#include "Canvas.h"
#include "CanvasView.h"
#include "LanguageManager.h"
#include "Layer.h"
#include "LayerConfigView.h"
#include "NamePanel.h"

#include "AddLayerAction.h"
#include "AddModifiersAction.h"
#include "BitmapStroke.h"
#include "ChangeLayerSettingsAction.h"
#include "CompoundAction.h"
#include "History.h"
#include "HistoryListView.h"
#include "IconButton.h"
#include "MergeLayersAction.h"
#include "MoveLayersAction.h"
#include "MoveModifiersAction.h"
#include "RemoveLayersAction.h"
#include "SolidColor.h"

#include "LayersListView.h"

enum {
	MSG_SET_BLENDING_MODE		= 'stbm',
};

// bits for the eye icon
const int32 kEyeWidth = 15;
const int32 kEyeHeight = 7;

const unsigned char kEyeBits [] = {
	0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,
	0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,
	0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,
	0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,
	0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,
	0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,
	0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0xff,0xff,0xff,0x00,0x77,0x74,0x77,0x00
};

// static members
BBitmap*
LayerItem::fEyeIcon = NULL;

// LayerItem class
LayerItem::LayerItem(LayersListView* parent,
					 const char* name)
	: SimpleItem(name),
	  fParent(parent)
{
	if (!fEyeIcon) {
		fEyeIcon = new BBitmap(BRect(0.0, 0.0, kEyeWidth - 1.0, kEyeHeight - 1.0),
							   B_RGB32);
		if (fEyeIcon->IsValid()) {
			uint32 length = fEyeIcon->BitsLength();
			memcpy(fEyeIcon->Bits(), kEyeBits, length);
			// replace any visible color in eye with the keyboard navigation color
			rgb_color navColor = ui_color(B_KEYBOARD_NAVIGATION_COLOR);
			uint8* bits = (uint8*)fEyeIcon->Bits();
			for (uint32 i = 0; i < length / 4; i++) {
				if (bits[3] > 0) {
					bits[0] = navColor.blue;
					bits[1] = navColor.green;
					bits[2] = navColor.red;
				}
				bits += 4;
			}
		} else {
			delete fEyeIcon;
			fEyeIcon = NULL;
		}
	}
}

// destructor
LayerItem::~LayerItem()
{
}

// Update
void
LayerItem::Update(BView* owner, const BFont* font)
{
	font_height fh;
	font->GetHeight(&fh);
	float fontHeight = 2.0 + ceilf(fh.ascent + fh.descent) + 2.0;

	float minHeight = fontHeight > 26.0 ? fontHeight : 26.0;
	SetHeight(minHeight);
	SetWidth(owner->Bounds().Width());
}

// Draw
void
LayerItem::DrawItem(BView* owner, BRect frame, bool even)
{
	if (Layer* layer = fParent->LayerFor(this)) {
		DrawBackground(owner, frame, even);
		BRect r(frame);
		BRect iconRect(frame);
		GetIconRect(iconRect);
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
		// draw visibility eye
		r.right = iconRect.left - 2.0;
		if (fEyeIcon && !(layer->Flags() & FLAG_INVISIBLE)) {
			owner->DrawBitmap(fEyeIcon, BPoint(r.left + (r.Width() 
														 - kEyeWidth) / 2.0,
											   r.top + (r.Height() 
														 - kEyeHeight) / 2.0));
		}
		// draw layer bitmap in icon fashion
		if (BBitmap* bitmap = layer->Bitmap()) {
			// figure out aspect
			BRect bitmapFrame = bitmap->Bounds();
			float xScale = iconRect.Width() / bitmapFrame.Width();
			float yScale = iconRect.Height() / bitmapFrame.Height();
			float scale = xScale > yScale ? yScale : xScale;
			if (scale > 1.0)
				scale = 1.0;
			bitmapFrame.right = floorf(bitmapFrame.right * scale + 0.5);
			bitmapFrame.bottom = floorf(bitmapFrame.bottom * scale + 0.5);
			BPoint offset(iconRect.left, iconRect.top);
			if (xScale > yScale)
				offset.x = floorf((iconRect.Width() - bitmapFrame.Width()) / 2.0) + iconRect.left;
			else
				offset.y = floorf((iconRect.Height() - bitmapFrame.Height()) / 2.0) + iconRect.top;
			bitmapFrame.OffsetBy(offset);
			// draw bitmap
			owner->DrawBitmap(bitmap, bitmap->Bounds(), bitmapFrame);
			bitmapFrame.InsetBy(-1.0, -1.0);
			owner->SetHighColor(0, 0, 0, 90);
			owner->StrokeRect(bitmapFrame);
		}
		r.left = iconRect.right + 2.0;
		r.right = frame.right;
		owner->SetDrawingMode(B_OP_COPY);
		SimpleItem::DrawItem(owner, r, even);
	} else
		SimpleItem::DrawItem(owner, frame, even);
}

// GetIconRect
void
LayerItem::GetIconRect(BRect& itemFrame) const
{
	itemFrame.left += 3.0 + kEyeWidth + 3.0;
	itemFrame.right = itemFrame.left + itemFrame.Height();
	itemFrame.InsetBy(1.0, 1.0);
}

// LayersListView class
LayersListView::LayersListView(BRect frame,
							   BMessage* selectionChangeMessage)
	: SimpleListView(frame, "layers list view",
					 selectionChangeMessage, B_SINGLE_SELECTION_LIST),
	  fCanvasView(NULL),
	  fConfigView(NULL),
	  fCanvas(NULL),
	  fLayerMenu(NULL),
	  fModeMenu(NULL),
	  fDeleteIB(NULL),
	  fNewLayerOnObjectDrop(false)
{
}

// destructor
LayersListView::~LayersListView()
{
	delete LayerItem::fEyeIcon;
	LayerItem::fEyeIcon = NULL;
}

// MessageReceived
void
LayersListView::MessageReceived(BMessage* message)
{
	// do not apply the destructive actions when the
	// canvas view is currently applying a tool
	if (fCanvasView && fCanvasView->IsTracking()) {
		SimpleListView::MessageReceived(message);
		return;
	}
	switch (message->what) {
		case B_PASTE: {
			rgb_color color;
			if (fCanvas && restore_color_from_message(message, color) >= B_OK) {
				Layer* layer = fCanvas->LayerAt(fDropIndex);
				if (fNewLayerOnObjectDrop || !layer) {
					layer = new Layer(fCanvas->Bounds());
					if (History* history = layer->GetHistory()) {
						SolidColor* object = new SolidColor();
						object->SetColor(color);
						history->AddModifier(object);
						BRect b = fCanvas->Bounds();
						layer->Touch(b);
						fCanvasView->RebuildBitmap(b, layer);
						fCanvasView->Perform(new AddLayerAction(fCanvas, layer, fDropIndex));
					}
				} else {
					if (History* history = layer->GetHistory()) {
						Stroke** objects = new Stroke*[1];
						objects[0] = new SolidColor();
						objects[0]->SetColor(color);
						fCanvasView->Perform(new AddModifiersAction(layer, objects, 1,
																	history->CountModifiers()));
					}
				}
			}
			break;
		}
		case MSG_MODIFIER_DATA: {
			if (fCanvasView && fCanvas) {
				Layer* srcLayer;
				if (message->FindPointer("layer", (void**)&srcLayer) >= B_OK && srcLayer) {
					if (History* srcHistory = srcLayer->GetHistory()) {
						int32 index;
						BList objectList;
						for (int32 i = 0; message->FindInt32("index", i, &index) >= B_OK; i++) {
							if (Stroke* object = srcHistory->ModifierAt(index)) {
								objectList.AddItem((void*)object);
							}
						}
						int32 count = objectList.CountItems();
						if (count > 0) {
							Layer* dstLayer = fCanvas->LayerAt(fDropIndex);
							if (fNewLayerOnObjectDrop || !dstLayer) {
								dstLayer = new Layer(fCanvas->Bounds());
								if (dstLayer->InitCheck() < B_OK) {
									delete dstLayer;
									dstLayer = NULL;
								} else if (History* dstHistory = dstLayer->GetHistory()) {
									if (modifiers() & B_SHIFT_KEY) {
										// prepare NEW LAYER action with copied modifiers
										BRect area = ((Stroke*)objectList.ItemAt(0))->Bounds();
										for (int32 i = 0; Stroke* object = (Stroke*)objectList.ItemAt(i); i++) {
											dstHistory->AddModifier(object->Clone());
											area = area | object->Bounds();
										}
										dstLayer->Touch(area);
										fCanvasView->RebuildBitmap(area, dstLayer);
										fCanvasView->Perform(new AddLayerAction(fCanvas, dstLayer, fDropIndex));
									} else {
										// prepare NEW LAYER action
										Action** actions = new Action*[2];
										actions[0] = new AddLayerAction(fCanvas, dstLayer, fDropIndex);
										// prepare MOVE action
										Stroke** objects = new Stroke*[count];
										memcpy(objects, objectList.Items(), count * sizeof(void*));
										actions[1] = new MoveModifiersAction(srcLayer,
																			 dstLayer,
																			 objects, count,
																			 dstHistory->CountModifiers());
										
										fCanvasView->Perform(new CompoundAction(actions, 2,
																				"Add Layer", ADD_LAYER));
									}
								}
							} else {
								if (History* dstHistory = dstLayer->GetHistory()) {
									Stroke** objects = new Stroke*[count];
									if (modifiers() & B_SHIFT_KEY) {
										// prepare COPY action
										for (int32 i = 0; Stroke* object = (Stroke*)objectList.ItemAt(i); i++)
											objects[i] = object->Clone();
										fCanvasView->Perform(new AddModifiersAction(dstLayer,
																					objects, count,
																					dstHistory->CountModifiers()));
									} else {
										// prepare MOVE action
										memcpy(objects, objectList.Items(), count * sizeof(void*));
										fCanvasView->Perform(new MoveModifiersAction(srcLayer,
																					 dstLayer,
																					 objects, count,
																					 dstHistory->CountModifiers()));
									}
								}
							}
						}
					}
				}
			}
			break;
		}
		case MSG_NEW_LAYER:
			if (fCanvas) {
				// insert new layer on top all layers
				// or on top currently selected layer
				int32 index = CurrentSelection(0);
				if (index < 0)
					index = 0;
				// prepare layer with a name
				Layer* layer = new Layer(fCanvas->Bounds());
				BString name = LanguageManager::Default()->GetString(LAYER, "Layer");
				name << " " << fCanvas->CountLayers();
				layer->SetName(name.String());
				// let the action do the rest
				fCanvasView->Perform(new AddLayerAction(fCanvas, layer, index));
			}
			break;
		case MSG_CLONE_LAYER:
			if (fCanvas) {
				// clone currently selected layer and insert in place
				int32 index = CurrentSelection(0);
				if (Layer* layer = fCanvas->LayerAt(index)) {
					// let the action do the rest
					fCanvasView->Perform(new AddLayerAction(fCanvas,
															layer->Clone(),
															index));
				}
			}
			break;
		case MSG_SET_BLENDING_MODE:
			if (fCanvas) {
				if (Layer* layer = fCanvas->LayerAt(CurrentSelection(0))) {
					uint32 mode;
					if (message->FindInt32("mode", (int32*)&mode) == B_OK) {
						fCanvasView->Perform(new ChangeLayerSettingsAction(layer,
																		   layer->Name(),
																		   layer->Alpha(),
																		   mode,
																		   layer->Flags()));
					}
				}
			}
			break;
		case MSG_MERGE_WITH_BACK_LAYER:
			if (fCanvas && fCanvasView) {
				int32 index = CurrentSelection(0);
				Layer* layer1 = fCanvas->LayerAt(index);
				Layer* layer2 = fCanvas->LayerAt(index + 1);
				if (layer1 && layer2) {
					Layer** layers = new Layer*[2];
					layers[0] = layer1;
					layers[1] = layer2;
					fCanvasView->Perform(new MergeLayersAction(fCanvas, layers, 2));
				}
			}
			break;
		case MSG_MERGE_VISIBLE_LAYERS:
			if (fCanvas && fCanvasView) {
				int32 count = 0;
				for (int32 i = 0; Layer* layer = fCanvas->LayerAt(i); i++)
					if (!(layer->Flags() & FLAG_INVISIBLE))
						count++;
				if (count > 1) {
					Layer** layers = new Layer*[count];
					int32 num = 0;
					for (int32 i = 0; Layer* layer = fCanvas->LayerAt(i); i++)
						if (!(layer->Flags() & FLAG_INVISIBLE))
							layers[num++] = layer;
					fCanvasView->Perform(new MergeLayersAction(fCanvas, layers, count));
				}
			}
			break;
		case MSG_RENAME_LAYER: {
			const char* name;
			Layer* layer = NULL;
			if (message->FindString("name", &name) >= B_OK
				&& message->FindPointer("layer", (void**)&layer) >= B_OK
				&& strlen(name) > 0 && fCanvas && fCanvas->HasLayer(layer)) {
				fCanvasView->Perform(new ChangeLayerSettingsAction(layer,
																   name,
																   layer->Alpha(),
																   layer->Mode(),
																   layer->Flags()));
					
			} else {
				if (fCanvas) {
					int32 index = CurrentSelection(0);
					if ((layer = fCanvas->LayerAt(index))) {
						BMessage* panelMessage = new BMessage(*message);
						panelMessage->AddPointer("layer", layer);
						new NamePanel(LanguageManager::Default()->GetString(NAME, "Name"),
									  layer->Name(), Window(), this, panelMessage);
					}
				}
			}
			break;
		}
/*		case MSG_CLEAR_LAYER:
			if (fCanvas) {
				int32 index = CurrentSelection(0);
				if (Layer* layer = fCanvas->LayerAt(index)) {
					layer->MakeEmpty();
					if (fCanvasView) {
						fCanvasView->SetToLayer(-1);
						fCanvasView->SetToLayer(index);
						fCanvasView->InvalidateCanvas(fCanvas->Bounds(), true);
					}
				}
			}
			break;*/
		case MSG_DELETE_LAYER:
			RemoveSelected();
			break;
		case B_SIMPLE_DATA:
			if (!message->HasPointer("list")) {
				if (fDropIndex >= 0)
					message->AddInt32("append index", fDropIndex);
				Window()->PostMessage(message);
				break;
			} else
				SimpleListView::MessageReceived(message);
			break;
		default:
			SimpleListView::MessageReceived(message);
			break;
	}
}

// SelectionChanged
void
LayersListView::SelectionChanged()
{
	int32 selectedIndex = CurrentSelection(0);
	if (fCanvasView)
		fCanvasView->SetToLayer(selectedIndex);
	if (fCanvas) {
		UpdateLayer(fCanvas->LayerAt(selectedIndex));
		if (Layer* layer = fCanvas->LayerAt(selectedIndex)) {
			if (fLayerMenu) {
				if (selectedIndex == fCanvas->CountLayers() - 1)
					fMergeBackMI->SetEnabled(false);
				else
					fMergeBackMI->SetEnabled(true);
				fRenameMI->SetEnabled(true);
				fCloneMI->SetEnabled(true);
				fDeleteMI->SetEnabled(true);
			}
			if (fDeleteIB)
				fDeleteIB->SetEnabled(true);
		} else {
			if (fLayerMenu) {
				fMergeBackMI->SetEnabled(false);
				fRenameMI->SetEnabled(false);
				fCloneMI->SetEnabled(false);
				fDeleteMI->SetEnabled(false);
			}
			if (fDeleteIB)
				fDeleteIB->SetEnabled(false);
		}
	} else {
		if (fModeMenu)
			fModeMenu->SetEnabled(false);
		if (fLayerMenu)
			fLayerMenu->SetEnabled(false);
		if (fDeleteIB)
			fDeleteIB->SetEnabled(false);
	}
}

// MouseDown
void
LayersListView::MouseDown(BPoint where)
{
	bool handled = false;
	int32 index = IndexOf(where);
	LayerItem* item = (LayerItem*)ItemAt(index);
	if (item) {
		BRect itemFrame(ItemFrame(index));
		itemFrame.right = itemFrame.left + 3.0 + kEyeWidth + 2.0;
		Layer* layer = LayerFor(item);
		if (itemFrame.Contains(where) && layer) {
			// toggle layers visibility
			uint32 flags = layer->Flags();
			if (flags & FLAG_INVISIBLE)
				flags &= ~FLAG_INVISIBLE;
			else
				flags |= FLAG_INVISIBLE;
			fCanvasView->Perform(new ChangeLayerSettingsAction(layer,
															   layer->Name(),
															   layer->Alpha(),
															   layer->Mode(),
															   flags));
			handled = true;
		}
	}
	if (!handled)
		SimpleListView::MouseDown(where);
}

// MoveItems
void
LayersListView::MoveItems(BList& items, int32 index)
{
	if (fCanvas && fCanvasView) {
		int32 count = items.CountItems();
		Layer** layers = new Layer*[count];
		for (int32 i = 0; i < count; i++) {
			layers[i] = fCanvas->LayerAt(IndexOf((BListItem*)items.ItemAt(i)));
		}
		fCanvasView->Perform(new MoveLayersAction(fCanvas, layers, count, index));
	}
}

// CopyItems
void
LayersListView::CopyItems(BList& items, int32 index)
{
/*	if (fCanvas && fCanvasView) {
		int32 count = items.CountItems();
		Layer** layers = new Layer*[count];
		for (int32 i = 0; i < count; i++) {
			Layer* layer = fCanvas->LayerAt(IndexOf((BListItem*)items.ItemAt(i)));
			if (layer)
				layers[i] = layer->Clone();
			else
				layers[i] = NULL;
		}
		fCanvasView->Perform(new AddLayersAction(fCanvas, layers, count, index));
	}*/
	if (fCanvas && fCanvasView) {
		int32 count = items.CountItems();
		if (count == 1) {
			Layer* layer = fCanvas->LayerAt(IndexOf((BListItem*)items.ItemAt(0)));
			if (layer)
				fCanvasView->Perform(new AddLayerAction(fCanvas, layer->Clone(), index));
			else
				fprintf(stderr, "LayersListView::CopyItems(): no layer at index %ld\n", IndexOf((BListItem*)items.ItemAt(0)));
		} else
			fprintf(stderr, "LayersListView::CopyItems(): copying %ld layers not supported yet\n", count);
	}
}

// RemoveItemList
void
LayersListView::RemoveItemList(BList& items)
{
	if (fCanvas && fCanvasView) {
		int32 count = items.CountItems();
		int32* indices = new int32[count];
		for (int32 i = 0; i < count; i++) {
			indices[i] = IndexOf((BListItem*)items.ItemAt(i));
		}
		fCanvasView->Perform(new RemoveLayersAction(fCanvas, indices, count));
	}
}

// DoubleClicked
void
LayersListView::DoubleClicked(int32 index)
{
	Window()->PostMessage(MSG_RENAME_LAYER, this);
}

// AcceptDragMessage
bool
LayersListView::AcceptDragMessage(const BMessage* message) const
{
	if (message->what == MSG_MODIFIER_DATA || message->what == B_PASTE) {
		return true;
	} else {
		return SimpleListView::AcceptDragMessage(message);
	}
}

// DropTargetRect
void
LayersListView::SetDropTargetRect(const BMessage* message, BPoint where)

{
	if (AcceptDragMessage(message)) {
		switch (message->what) {
			case MSG_MODIFIER_DATA:
			case B_PASTE: {
				int32 index = IndexOf(where);
				BRect r;
				if (index >= 0) {
					fNewLayerOnObjectDrop = false;
					r = ItemFrame(index);
					// handle dropping "between" layers
					BRect t = r;
					t.InsetBy(0.0, r.Height() * 0.25);
					if (where.y < t.top) {
						fNewLayerOnObjectDrop = true;
						r.Set(r.left, r.top - 1, r.right, r.top);
					} else if (where.y > t.bottom && index < CountItems() - 1) {
						index ++;
						fNewLayerOnObjectDrop = true;
						r.Set(r.left, r.bottom, r.right, r.bottom + 1);
					}
				} else {
					fNewLayerOnObjectDrop = true;
					index = CountItems();
					r = ItemFrame(index - 1);
					r.OffsetBy(0.0, r.Height() + 1);
				}
				SetDropRect(r);
				fDropIndex = index;

				if (message->what != B_PASTE) {
					const uchar* cursorData = modifiers() & B_SHIFT_KEY ? kCopyCursor : B_HAND_CURSOR;
					BCursor cursor(cursorData);
					SetViewCursor(&cursor, true);
				}
				break;
			}
			default:
				SimpleListView::SetDropTargetRect(message, where);
				break;
		}
	}
}

// SetCanvasView
void
LayersListView::SetCanvasView(CanvasView* canvasView)
{
	fCanvasView = canvasView;
}

// SetConfigView
void
LayersListView::SetConfigView(LayerConfigView* configView)
{
	fConfigView = configView;
}

// SetToCanvas
void
LayersListView::SetToCanvas(Canvas* canvas, bool force)
{
	if (canvas != fCanvas || force) {
		fCanvas = canvas;
		Update();
	}
}

// SetToLayer
void
LayersListView::SetToLayer(int32 index)
{
	Select(index, false);
	ScrollToSelection();
}

// UpdateLayer
void
LayersListView::UpdateLayer(Layer* layer)
{
	if (fCanvas && fCanvas->HasLayer(layer)) {
		int32 index = fCanvas->IndexOf(layer);
		if (index == CurrentSelection(0)) {
			// update config
			if (fConfigView)
				fConfigView->SetLayer(layer);
			// enable the mode items and mark the appropriate one
			if (fModeMenu) {
				if (!fModeMenu->IsEnabled())
					fModeMenu->SetEnabled(true);
				for (int32 i = 0; BMenuItem* item = fModeMenu->ItemAt(i); i++) {
					BMessage* message = item->Message();
					uint32 mode;
					if (message && message->FindInt32("mode", (int32*)&mode) == B_OK
						&& mode == layer->Mode())
						item->SetMarked(true);
					else
						item->SetMarked(false);
				}
			}
			if (fDeleteIB) {
				if (!fDeleteIB->IsEnabled())
					fDeleteIB->SetEnabled(true);
			}
		}
		// update layer item
		if (LayerItem* item = (LayerItem*)ItemAt(index)) {
			item->SetText(layer->Name());
			InvalidateItem(index);
		}
	} else if (!layer) {
		if (fModeMenu)
			fModeMenu->SetEnabled(false);
		if (fDeleteIB)
			fDeleteIB->SetEnabled(false);
		// update config
		if (fConfigView)
			fConfigView->SetLayer(layer);
	}
}

// Update
void
LayersListView::Update()
{
	float scrollOffset = Bounds().top;
	ClearList();
	if (fCanvas) {
		for (int32 i = 0; Layer* layer = fCanvas->LayerAt(i); i++)
			AddLayer(layer->Name(), i, false);
		if (!fLayerMenu->IsEnabled())
			fLayerMenu->SetEnabled(true);
		int32 index = fCanvas->CurrentLayer();
		if (index < 0 || index >= fCanvas->CountLayers())
			index = fCanvas->CountLayers() - 1;
		ScrollTo(BPoint(0.0, scrollOffset));
		SetToLayer(index);
	} else {
		if (fConfigView)
			fConfigView->SetLayer(NULL);
		if (fLayerMenu)
			fLayerMenu->SetEnabled(false);
		if (fDeleteIB)
			fDeleteIB->SetEnabled(false);
	}
	Invalidate();
}

// AddLayer
bool
LayersListView::AddLayer(const char* name, int32 index, bool select)
{
	SimpleItem* item = new LayerItem(this, name);
	if (!AddItem(item, index)) {
		delete item;
		return false;
	}
	if (select)
		SetToLayer(index);
	return true;
}

// ClearList
void
LayersListView::ClearList()
{
	int32 last = CountItems();
	while (BListItem* item = RemoveItem(--last))
		delete item;
}

// LayerFor
Layer*
LayersListView::LayerFor(LayerItem* item) const
{
	Layer* layer = NULL;
	if (fCanvas) {
		layer = fCanvas->LayerAt(IndexOf(item));
	}
	return layer;
}

// InvalidateLayer
void
LayersListView::InvalidateLayer(int32 index)
{
	if (LayerItem* item = (LayerItem*)ItemAt(index)) {
		BRect r(ItemFrame(index));
		item->GetIconRect(r);
		Invalidate(r);
	}
}

// SetLayerMenu
void
LayersListView::SetLayerMenu(BMenu* menu)
{
	fLayerMenu = menu;
	if (fLayerMenu) {

		fNewMI = new BMenuItem("New", new BMessage(MSG_NEW_LAYER), 'N', B_SHIFT_KEY);
		fLayerMenu->AddItem(fNewMI);

		fCloneMI = new BMenuItem("Duplicate", new BMessage(MSG_CLONE_LAYER), 'D');
		fLayerMenu->AddItem(fCloneMI);
	
		fLayerMenu->AddSeparatorItem();
	
		fMergeBackMI = new BMenuItem("Merge Down",
									 new BMessage(MSG_MERGE_WITH_BACK_LAYER));
		fLayerMenu->AddItem(fMergeBackMI);
	
		fMergeVisibleMI = new BMenuItem("Merge Visible",
										new BMessage(MSG_MERGE_VISIBLE_LAYERS));
		fLayerMenu->AddItem(fMergeVisibleMI);
	
		fLayerMenu->AddSeparatorItem();

		fRenameMI = new BMenuItem("Rename"B_UTF8_ELLIPSIS, new BMessage(MSG_RENAME_LAYER), 'E');
		fLayerMenu->AddItem(fRenameMI);
	
		fLayerMenu->AddSeparatorItem();
	
		fDeleteMI = new BMenuItem("Delete", new BMessage(MSG_DELETE_LAYER));
		fLayerMenu->AddItem(fDeleteMI);

		fLayerMenu->SetTargetForItems(this);
	}
}

// SetModeMenu
void
LayersListView::SetModeMenu(BMenu* menu)
{
	fModeMenu = menu;
	if (fModeMenu) {
		BMessage* message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_NORMAL);
		fNormalMI = new BMenuItem("Normal", message);

		fModeMenu->AddItem(fNormalMI);

		fModeMenu->AddSeparatorItem();

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_MULTIPLY);
		fMultiplyMI = new BMenuItem("Multiply", message);

		fModeMenu->AddItem(fMultiplyMI);

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_INVERSE_MULTIPLY);
		fInverseMultiplyMI = new BMenuItem("Inverse Multiply", message);

		fModeMenu->AddItem(fInverseMultiplyMI);

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_DARKEN);
		fDarkenMI = new BMenuItem("Darken", message);

		fModeMenu->AddItem(fDarkenMI);

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_LIGHTEN);
		fLightenMI = new BMenuItem("Lighten", message);

		fModeMenu->AddItem(fLightenMI);

/*		fModeMenu->AddSeparatorItem();

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_HARD_LIGHT);
		fHardLightMI = new BMenuItem("Hard Light", message);

		fModeMenu->AddItem(fHardLightMI);

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_SOFT_LIGHT);
		fSoftLightMI = new BMenuItem("Soft Light", message);

		fModeMenu->AddItem(fSoftLightMI);*/

		fModeMenu->AddSeparatorItem();

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_LUMINANCE);
		fLuminanceMI = new BMenuItem("Luminance", message);

		fModeMenu->AddItem(fLuminanceMI);

		fModeMenu->AddSeparatorItem();

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_MULTIPLY_ALPHA);
		fMultiplyAlphaMI = new BMenuItem("Alpha", message);

		fModeMenu->AddItem(fMultiplyAlphaMI);

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_MULTIPLY_INVERSE_ALPHA);
		fMultiplyInverseAlphaMI = new BMenuItem("Inverse Alpha", message);

		fModeMenu->AddItem(fMultiplyInverseAlphaMI);

		fModeMenu->AddSeparatorItem();

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_REPLACE_RED);
		fReplaceRedMI = new BMenuItem("Replace Red", message);

		fModeMenu->AddItem(fReplaceRedMI);

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_REPLACE_GREEN);
		fReplaceGreenMI = new BMenuItem("Replace Green", message);

		fModeMenu->AddItem(fReplaceGreenMI);

		message = new BMessage(MSG_SET_BLENDING_MODE);
		message->AddInt32("mode", MODE_REPLACE_BLUE);
		fReplaceBlueMI = new BMenuItem("Replace Blue", message);

		fModeMenu->AddItem(fReplaceBlueMI);

		fModeMenu->SetTargetForItems(this);
		fModeMenu->SetRadioMode(true);
	}
}

// SetDeleteButton
void
LayersListView::SetDeleteButton(IconButton* button)
{
	fDeleteIB = button;
	if (fDeleteIB) {
		fDeleteIB->SetMessage(new BMessage(MSG_DELETE_LAYER));
		fDeleteIB->SetTarget(this);
		fDeleteIB->SetEnabled(false);
	}
}

// UpdateStrings
void
LayersListView::UpdateStrings()
{
	LanguageManager* manager = LanguageManager::Default();
	// layer menu
	fNewMI->SetLabel(manager->GetString(NEW_LAYER, "New"));
	fCloneMI->SetLabel(manager->GetString(DUPLICATE, "Duplicate"));
	fMergeBackMI->SetLabel(manager->GetString(MERGE_DOWN, "Merge Down"));
	fMergeVisibleMI->SetLabel(manager->GetString(MERGE_VISIBLE, "Merge Visible"));
	BString helper(manager->GetString(RENAME, "Rename"));
	helper << B_UTF8_ELLIPSIS;
	fRenameMI->SetLabel(helper.String());
	fDeleteMI->SetLabel(manager->GetString(DELETE, "Delete"));
	// mode menu
//	fModeMenu->SuperItem()->SetLabel(MODE, "Mode");
	fNormalMI->SetLabel(manager->GetString(NORMAL, "Normal"));
	fMultiplyMI->SetLabel(manager->GetString(MULTIPLY, "Multiply"));
	fInverseMultiplyMI->SetLabel(manager->GetString(INVERSE_MULTIPLY, "Inverse Multiply"));
	fLuminanceMI->SetLabel(manager->GetString(LUMINANCE, "Luminance"));
	fMultiplyAlphaMI->SetLabel(manager->GetString(MULTIPLY_ALPHA, "Alpha"));
	fMultiplyInverseAlphaMI->SetLabel(manager->GetString(MULTIPLY_INVERSE_ALPHA, "Inverse Alpha"));
	fReplaceRedMI->SetLabel(manager->GetString(REPLACE_RED, "Replace Red"));
	fReplaceGreenMI->SetLabel(manager->GetString(REPLACE_GREEN, "Replace Green"));
	fReplaceBlueMI->SetLabel(manager->GetString(REPLACE_BLUE, "Replace Blue"));
	fDarkenMI->SetLabel(manager->GetString(DARKEN, "Darken"));
	fLightenMI->SetLabel(manager->GetString(LIGHTEN, "Lighten"));

	fConfigView->UpdateStrings();
}

// _CheckVisibleItem
void
LayersListView::_CheckVisibleItem()
{
	if (fCanvas && fLayerMenu) {
		// count visible layers and enable "merge visible" item accordingly
		int32 visibleCount = 0;
		for (int32 i = 0; Layer* layer = fCanvas->LayerAt(i); i++) {
			if (!(layer->Flags() & FLAG_INVISIBLE))
				visibleCount++;
		}
		fMergeVisibleMI->SetEnabled(visibleCount > 1);
	}
}


