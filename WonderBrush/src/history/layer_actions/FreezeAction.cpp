// FreezeAction.cpp

#include <stdio.h>

#include <Bitmap.h>

#include "BitmapStroke.h"
#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"
#include "Stroke.h"

#include "bitmap_support.h"

#include "FreezeAction.h"

// constructor
FreezeAction::FreezeAction(Layer* layer)
	: LayerAction(layer),
	  fStrokes(NULL),
	  fCount(0),
	  fItemsRemoved(false),
	  fBitmapStroke(NULL)
{
	if (fLayer) {
		// remember all strokes in the history
		if (History* history = fLayer->GetHistory()) {
			fCount = history->CountModifiers();
			if (fCount > 0) {
				fStrokes = new Stroke*[fCount];
				for (int32 i = 0; i < fCount; i++) {
					fStrokes[i] = history->ModifierAt(i);
				}
			}
		}
		BBitmap* bitmap = cropped_bitmap(fLayer->Bitmap(),
										 fLayer->ActiveBounds(),
										 false, true);
		if (bitmap && bitmap->IsValid())
			fBitmapStroke = new BitmapStroke(bitmap);
		else
			delete bitmap;
	}
}

// destructor
FreezeAction::~FreezeAction()
{
	if (fItemsRemoved) {
		for (int32 i = 0; i < fCount; i++) {
			delete fStrokes[i];
		}
	} else
		delete fBitmapStroke;
	delete[] fStrokes;
}

// InitCheck
status_t
FreezeAction::InitCheck()
{
	return fLayer && fStrokes && fBitmapStroke ? B_OK : B_BAD_VALUE;
}

// Perform
status_t
FreezeAction::Perform(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		// remove modifiers from history
		status = B_ERROR;
		if (History* history = fLayer->GetHistory()) {
			fItemsRemoved = true;
			// remove modifiers from history (without deleting them)
			history->MakeEmpty();
			// put bitmap modifier in their place
			history->AddModifier(fBitmapStroke);
			// remove all items from history list view
			HistoryListView* listView = view->GetHistoryListView();
			if (listView && listView->GetLayer() == fLayer) {
				listView->ClearList();
				listView->AddModifier(fBitmapStroke);
			}
			status = B_OK;
		}
	}
	return status;
}

// Undo
status_t
FreezeAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		if (History* history = fLayer->GetHistory()) {
			fItemsRemoved = false;
			// remove bitmap modifier
			history->RemoveModifier(fBitmapStroke);
			// put modifiers back into history
			for (int32 i = 0; i < fCount; i++) {
				history->AddModifier(fStrokes[i]);
			}
			// update the history list view
			HistoryListView* listView = view->GetHistoryListView();
			if (listView && listView->GetLayer() == fLayer)
				listView->SetLayer(fLayer, true);
		}
	}
	return status;
}

// Redo
status_t
FreezeAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
FreezeAction::GetName(BString& name)
{
	name << _GetString(FREEZE, "Freeze");
}
