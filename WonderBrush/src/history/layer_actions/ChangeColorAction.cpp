// ChangeColorAction.cpp

#include <stdio.h>

#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"

#include "ChangeColorAction.h"

// constructor
ChangeColorAction::ChangeColorAction(Layer* layer,
									 Stroke** strokes,
									 rgb_color color,
									 int32 count)
	: LayerAction(layer),
	  fStrokes(strokes),
	  fCount(count),
	  fColor(color),
	  fUndoColors(new rgb_color[count])
{
	for (int32 i = 0; i < count; i++) {
		if (fStrokes[i])
			fUndoColors[i] = fStrokes[i]->Color();
	}
}

// destructor
ChangeColorAction::~ChangeColorAction()
{
	delete[] fStrokes;
	delete[] fUndoColors;
}

// InitCheck
status_t
ChangeColorAction::InitCheck()
{
	status_t status = LayerAction::InitCheck();
	if (status >= B_OK)
		status = fStrokes && fUndoColors ? B_OK : B_NO_INIT;
	return status;
}

// Perform
status_t
ChangeColorAction::Perform(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		BRect area(0.0, 0.0, -1.0, -1.0);
	
		HistoryListView* listView = view->GetHistoryListView();
		if (listView && listView->GetLayer() != fLayer)
			listView = NULL;
	
		for (int32 i = 0; i < fCount; i++) {
			if (Stroke* stroke = fStrokes[i]) {
				stroke->SetColor(fColor);
				if (i == 0)
					area = stroke->Bounds();
				else
					area = area | stroke->Bounds();
				if (listView) {
					int32 index = fLayer->GetHistory()->IndexOf(stroke);
					listView->SetPainter(index, stroke->Painter());
				}
			}
		}
		if (area.IsValid())
			view->RebuildBitmap(area, fLayer);
	}

	return status;
}

// Undo
status_t
ChangeColorAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		BRect area(0.0, 0.0, -1.0, -1.0);
		HistoryListView* listView = view->GetHistoryListView();
		if (listView && listView->GetLayer() != fLayer)
			listView = NULL;
		for (int32 i = 0; i < fCount; i++) {
			if (Stroke* stroke = fStrokes[i]) {
				stroke->SetColor(fUndoColors[i]);
				if (i == 0)
					area = stroke->Bounds();
				else
					area = area | stroke->Bounds();
				if (listView) {
					int32 index = fLayer->GetHistory()->IndexOf(stroke);
					listView->SetPainter(index, stroke->Painter());
				}
			}
		}
		if (area.IsValid())
			view->RebuildBitmap(area, fLayer);
	}
	return status;
}

// Redo
status_t
ChangeColorAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
ChangeColorAction::GetName(BString& name)
{
	if (fCount > 1)
		name << _GetString(CHANGE_COLORS, "Change Colors");
	else
		name << _GetString(CHANGE_COLOR, "Change Color");
}
