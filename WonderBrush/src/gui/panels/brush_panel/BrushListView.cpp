// BrushListView.cpp

#include <math.h>
#include <stdio.h>

#include <Bitmap.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Message.h>
#include <String.h>

#include "bitmap_support.h"

#include "Brush.h"
#include "BrushStroke.h"
#include "CanvasView.h"
#include "LanguageManager.h"
#include "NamePanel.h"

#include "BrushListView.h"

#define MIN_ITEM_HEIGHT		50.0
#define TRACK_POINT_COUNT	50.0

// BrushItem class
BrushItem::BrushItem(BrushStroke* stroke,
					 const char* name)
	: SimpleItem(name),
	  fStroke(stroke),
	  fStrokeBitmap(NULL),
	  fBitmap(NULL),
	  fBrushScale(1.0),
	  fDirty(false)
{
}

// destructor
BrushItem::~BrushItem()
{
	delete fStroke;
	delete fStrokeBitmap;
	delete fBitmap;
}

// Update
void
BrushItem::Update(BView* owner, const BFont* font)
{
	font_height fh;
	font->GetHeight(&fh);
	float fontHeight = 2.0 + ceilf(fh.ascent + fh.descent) + 2.0;

	float minHeight = fontHeight > MIN_ITEM_HEIGHT ? fontHeight : MIN_ITEM_HEIGHT;
	SetHeight(minHeight);

	// see if we need to scale down brush
	if (fStroke) {
		if (Brush* brush = fStroke->GetBrush()) {
			range radius = brush->Radius();
			// prevent Update() being called multiple times messing things up
			// -> reset radius and scale to original values to have a clean start
			radius.min *= fBrushScale;
			radius.max *= fBrushScale;
			float maxRadius = radius.min > radius.max ? radius.min : radius.max;
			fBrushScale = 1.0;
			if (maxRadius > minHeight * 0.35) {
				float r = maxRadius;
				while (r > minHeight * 0.35) {
					// scale down brush
					fBrushScale += 0.5;
					r = maxRadius / fBrushScale;
				}
				radius.min /= fBrushScale;
				radius.max /= fBrushScale;
				brush->SetRadius(radius);
			}
		}
	}

	float width = owner->Bounds().Width();
	SetWidth(width);
	// delete and rebuild bitmap
	delete fStrokeBitmap;
	delete fBitmap;
	float textWidth = width * 0.3;
	float maxTextWidth = font->StringWidth("Radius: 00.00 - 000.00");
	if (textWidth > maxTextWidth)
		textWidth = maxTextWidth;
	BRect r(0.0, 0.0, width - textWidth, minHeight);
	fStrokeBitmap = new BBitmap(r, B_GRAY8);
	fBitmap = new BBitmap(r, B_RGB32);

	if (fStrokeBitmap->IsValid() && fBitmap->IsValid()) {
		_FillBitmap(fStrokeBitmap, fBitmap, fStroke);
		fDirty = true;
	} else {
		delete fStrokeBitmap;
		fStrokeBitmap = NULL;
		delete fBitmap;
		fBitmap = NULL;
	}
}

// Draw
void
BrushItem::Draw(BView* owner, BRect frame, uint32 flags)
{
	if (fBitmap && fBitmap->IsValid()) {
		DrawBackground(owner, BRect(0.0, 0.0, -1.0, -1.0), flags);
		// see if we need to update our bitmap contents
		rgb_color color = owner->LowColor();
		if (fDirty
			|| fBackgroundColor.red != color.red
			|| fBackgroundColor.green != color.green
			|| fBackgroundColor.blue != color.blue) {
			// translate alpha bitmap into display bitmap
			uint8* src = (uint8*)fStrokeBitmap->Bits();
			uint8* dst = (uint8*)fBitmap->Bits();
			uint32 bpr8 = fStrokeBitmap->BytesPerRow();
			uint32 bpr32 = fBitmap->BytesPerRow();
			BRect r = fBitmap->Bounds();
			uint32 width = r.IntegerWidth() + 1;
			uint32 height = r.IntegerHeight() + 1;
			for (uint32 y = 0; y < height; y++) {
				uint8* srcHandle = src;
				uint8* dstHandle = dst;
				for (uint32 x = 0; x < width; x++) {
					dstHandle[0] = color.blue - (color.blue * *srcHandle) / 255;
					dstHandle[1] = color.green - (color.green * *srcHandle) / 255; 
					dstHandle[2] = color.red - (color.red * *srcHandle) / 255;
					// next pixel
					srcHandle += 1;
					dstHandle += 4;
				}
				src += bpr8;
				dst += bpr32;
			}
			fDirty = false;
			fBackgroundColor = color;
		}
		owner->DrawBitmap(fBitmap, frame.LeftTop());
		if (fBrushScale > 1.0) {
			// inform the user that the brush is scaled down
			BPoint top = fBitmap->Bounds().LeftTop() + BPoint(6.0, 2.0 + frame.top);
			BPoint bottom = fBitmap->Bounds().LeftBottom() + BPoint(6.0, -2.0 + frame.top);
			float scaleHeight = bottom.y - (bottom.y - top.y) / fBrushScale;
			rgb_color rulerColor = tint_color(color, B_DARKEN_2_TINT);
			rgb_color fillColor = (rgb_color){ 255, 0, 0, 255 };
			owner->BeginLineArray(5);
				owner->AddLine(BPoint(top.x - 1.0, top.y),
							   BPoint(top.x + 1.0, top.y), rulerColor);
				owner->AddLine(BPoint(top.x, top.y + 1.0),
							   BPoint(top.x, scaleHeight - 1.0), rulerColor);
				owner->AddLine(BPoint(top.x - 1.0, scaleHeight),
							   BPoint(top.x + 1.0, scaleHeight), fillColor);
				owner->AddLine(BPoint(top.x, scaleHeight + 1.0),
							   BPoint(bottom.x, bottom.y - 1.0), fillColor);
				owner->AddLine(BPoint(bottom.x - 1.0, bottom.y),
							   BPoint(bottom.x + 1.0, bottom.y), fillColor);
			owner->EndLineArray();
			
/*			BString helper("x");
			helper << fBrushScale;
			font_height fh;
			owner->GetFontHeight(&fh);
			BPoint textPoint;
			textPoint.x = fBitmap->Bounds().right - owner->StringWidth(helper.String()) - 1.0;
			textPoint.y = frame.bottom - fh.descent;
			owner->SetHighColor(255, 0, 0, 255);
			owner->SetDrawingMode(B_OP_ALPHA);
			owner->DrawString(helper.String(), textPoint);*/
		}
		// let the base class finish off
		frame.left = fBitmap->Bounds().right + 1.0;
		owner->SetDrawingMode(B_OP_COPY);
		SimpleItem::Draw(owner, frame, flags);
	} else
		SimpleItem::Draw(owner, frame, flags);
}

// GetIconRect
void
BrushItem::GetIconRect(BRect& itemFrame) const
{
	if (fBitmap && fBitmap->IsValid())
		itemFrame.right = itemFrame.left + fBitmap->Bounds().Width();
	else
		// render frame invalid
		itemFrame.right = itemFrame.left - 1.0;
}

// _FillBitmap
void
BrushItem::_FillBitmap(BBitmap* strokeBitmap,
					   BBitmap* bitmap,
					   BrushStroke* stroke) const
{
	if (stroke
		&& strokeBitmap && strokeBitmap->IsValid()
		&& strokeBitmap->ColorSpace() == B_GRAY8
		&& bitmap && bitmap->IsValid()
		&& bitmap->ColorSpace() == B_RGB32) {
		// cache some values
		float width = bitmap->Bounds().Width();
		float height = bitmap->Bounds().Height();
		if (width > 0.0 && height > 0.0) {
			stroke->MakeEmpty();
			// generate sinus stroke line
			BPoint point;
			float height2 = height / 2.0;
			float strokeWidth = width - height;
			for (int32 i = 0; i < TRACK_POINT_COUNT; i++) {
				// calculate new position
				point.x = height2 + (strokeWidth / TRACK_POINT_COUNT) * i;
				point.y = height2 + (height2 / 2.0)
					* (sinf(2.0 * M_PI * ((point.x - height2) / strokeWidth)));
				
				float pressure = (float)i / ((float)TRACK_POINT_COUNT / 2.0) - 1.0;
				// pressure is now in range [-1.0,1.0] -> apply gaussian
				pressure = powf(2.0, (-2.0 * ((pressure * 2.0) * (pressure * 2.0))));
				stroke->AddPoint(Point(point, pressure));
			}
			// draw stroke into bitmap
			memset(strokeBitmap->Bits(), 0, strokeBitmap->BitsLength());
			stroke->Draw(strokeBitmap, strokeBitmap->Bounds());
		}
	}
}



// BrushListView class
BrushListView::BrushListView(CanvasView* view,
							 BRect frame,
							 BMessage* selectionChangeMessage)
	: SimpleListView(frame, "layers list view",
					 selectionChangeMessage, B_SINGLE_SELECTION_LIST),
	  fCanvasView(view)//,
//	  fConfigView(NULL)
{
}

// destructor
BrushListView::~BrushListView()
{
}

// FrameResized
void
BrushListView::FrameResized(float width, float height)
{
	BFont font;
	GetFont(&font);
	for (int32 i = 0; BrushItem* item = (BrushItem*)ItemAt(i); i++) {
		item->Update(this, &font);
	}
	SimpleListView::FrameResized(width, height);
}

// MessageReceived
void
BrushListView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_NEW_BRUSH:
			if (fCanvasView && fCanvasView->LockLooper()) {
				// TODO: maybe we should check
				// if current tool is actually a brush tool
				range radius = fCanvasView->BrushRadius();
				BrushStroke* stroke = new BrushStroke(radius,
													  fCanvasView->BrushHardness(),
													  fCanvasView->BrushAlpha(),
													  fCanvasView->BrushSpacing(),
													  fCanvasView->BrushFlags(),
													  fCanvasView->Color(),
													  MODE_BRUSH);
				fCanvasView->UnlockLooper();
				BString name("Radius: ");
				name << radius.min << " - " << radius.max;
				AddItem(new BrushItem(stroke, name.String()));
			}
			break;
		case MSG_RENAME_BRUSH: {
			const char* name;
			BrushItem* item;
			if (message->FindString("name", &name) >= B_OK
				&& strlen(name) > 0
				&& message->FindPointer("item", (void**)&item) >= B_OK
				&& HasItem(item)) {
				// set the item name and invalidate it
				item->SetText(name);
				InvalidateItem(IndexOf(item));
			} else {
				// launch rename panel
				BrushItem* item = (BrushItem*)ItemAt(CurrentSelection(0));
				if (item) {
					BMessage* panelMessage = new BMessage(*message);
					panelMessage->AddPointer("item", item);
					new NamePanel(LanguageManager::Default()->GetString(BRUSH_NAME, "Brush Name"),
								  item->Text(), Window(), this, panelMessage);
				}
			}
			break;
		}
		case MSG_DELETE_BRUSH:
			RemoveSelected();
			break;
		case MSG_DELETE_ALL:
			ClearList();
			break;
		default:
			SimpleListView::MessageReceived(message);
			break;
	}
}

// SelectionChanged
void
BrushListView::SelectionChanged()
{
	BrushItem* item = (BrushItem*)ItemAt(CurrentSelection(0));
	if (item) {
		if (BrushStroke* stroke = item->GetBrushStroke()) {
			BMessage message(MSG_SET_TO_BRUSH);
			stroke->Archive(&message);
			message.AddFloat("brush scale", item->BrushScale());
			if (BLooper* looper = fCanvasView->Looper()) {
				looper->PostMessage(&message);
			}
		}
	}
	fRenameMI->SetEnabled(item != NULL);
	fRemoveMI->SetEnabled(item != NULL);
}

// DoubleClicked
void
BrushListView::DoubleClicked(int32 index)
{
	Window()->PostMessage(MSG_RENAME_BRUSH, this);
}

// CloneItem
BListItem*
BrushListView::CloneItem(int32 atIndex) const
{
	BListItem* clone = NULL;
	if (BrushItem* item = (BrushItem*)ItemAt(atIndex)) {
		if (BrushStroke* stroke = item->GetBrushStroke()) {
			BrushStroke* clonedStroke = dynamic_cast<BrushStroke*>(stroke->Clone());
			// scale brush if scale was not 1.0
			if (Brush* brush = clonedStroke->GetBrush()) {
				range radius = brush->Radius();
				radius.min *= item->BrushScale();
				radius.max *= item->BrushScale();
				brush->SetRadius(radius);
			}
			clone = new BrushItem(clonedStroke, item->Text());
		}
	}
	return clone;
}

// SetBrushMenu
void
BrushListView::SetBrushMenu(BMenu* menu)
{
	fBrushM = menu;
	if (fBrushM) {
		fNewMI = new BMenuItem("New", new BMessage(MSG_NEW_BRUSH), 'N');
		fBrushM->AddItem(fNewMI);
	
		fBrushM->AddSeparatorItem();
	
		fRenameMI = new BMenuItem("Rename", new BMessage(MSG_RENAME_BRUSH), 'E');
		fRenameMI->SetEnabled(false);
		fBrushM->AddItem(fRenameMI);
	
		fBrushM->AddSeparatorItem();
	
		fRemoveMI = new BMenuItem("Delete", new BMessage(MSG_DELETE_BRUSH));
		fRemoveMI->SetEnabled(false);
		fBrushM->AddItem(fRemoveMI);

		fRemoveAllMI = new BMenuItem("Delete All", new BMessage(MSG_DELETE_ALL));
//		fRemoveAllMI->SetEnabled(false);
		fBrushM->AddItem(fRemoveAllMI);

		fBrushM->SetTargetForItems(this);
	}
}

// UpdateStrings
void
BrushListView::UpdateStrings()
{
	LanguageManager* manager = LanguageManager::Default();
	// layer menu
	fNewMI->SetLabel(manager->GetString(NEW, "New"));
	fRenameMI->SetLabel(manager->GetString(RENAME, "Rename"));
	fRemoveMI->SetLabel(manager->GetString(DELETE, "Delete"));
	fRemoveAllMI->SetLabel(manager->GetString(DELETE_ALL, "Delete All"));
}

// ClearList
void
BrushListView::ClearList()
{
	int32 last = CountItems();
	while (BListItem* item = RemoveItem(--last))
		delete item;
}

