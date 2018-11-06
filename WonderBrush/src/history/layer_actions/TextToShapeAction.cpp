// TextToShapeAction.cpp

#include <new>
#include <stdio.h>

#include "support.h"

#include "CanvasView.h"
#include "History.h"
#include "HistoryListView.h"
#include "Layer.h"
#include "ShapeStroke.h"
#include "AGGTextRenderer.h"
#include "TextStroke.h"

#include "TextToShapeAction.h"

// constructor
TextToShapeAction::TextToShapeAction(Layer* layer,
									 TextStroke** textObjects,
									 int32 count)
	: LayerAction(layer),
	  fTextObjects(textObjects),
	  fShapeObjects(NULL),
	  fCount(count),
	  fPerformed(false)
{
	if (fTextObjects && fCount > 0) {
		fShapeObjects = new(nothrow) ShapeStroke*[fCount];
		// convert the text objects
		for (int32 i = 0; i < fCount; i++) {
			TextStroke* textObject = fTextObjects[i];
			TextRenderer* renderer = NULL;
			if (textObject) {
				renderer = textObject->GetTextRenderer();
			}
			if (renderer) {
				fShapeObjects[i] = new ShapeStroke(textObject->Color());
				renderer->GetPaths(fShapeObjects[i]);
				fShapeObjects[i]->SetTo(textObject);

				// take care of rounding translation to integer
				// when hinting is on (TextObject does that during rendering)
				Transformable transform(*textObject);
				if (renderer->Hinting() && textObject->IsTranslationOnly()) {
					BPoint p(0.0, 0.0);
					textObject->Transform(&p);
					p.x = roundf(p.x) - p.x;
					p.y = roundf(p.y) - p.y;
					fShapeObjects[i]->TranslateBy(p);
				}

				fShapeObjects[i]->SetFillingRule(FILL_MODE_NON_ZERO);
			} else {
				fTextObjects[i] = NULL;
				fShapeObjects[i] = NULL;
			}
		}
	} else {
		delete[] fTextObjects;
		fTextObjects = NULL;
		fCount = 0;
	}
}

// destructor
TextToShapeAction::~TextToShapeAction()
{
	if (fPerformed) {
//printf("~TextToShapeAction() - deleting text objects\n");
		for (int32 i = 0; i < fCount; i++)
			delete fTextObjects[i];
		delete[] fTextObjects;
	} else {
//printf("~TextToShapeAction() - deleting shape objects\n");
		for (int32 i = 0; i < fCount; i++)
			delete fShapeObjects[i];
		delete[] fShapeObjects;
	}
}

// InitCheck
status_t
TextToShapeAction::InitCheck()
{
	return fShapeObjects ? LayerAction::InitCheck() : B_NO_INIT;
}

// Perform
status_t
TextToShapeAction::Perform(CanvasView* view)
{
	status_t ret = InitCheck();
	if (ret >= B_OK)
		ret = _SwapObjects(view, true);
	return ret;
}

// Undo
status_t
TextToShapeAction::Undo(CanvasView* view)
{
	status_t ret = InitCheck();
	if (ret >= B_OK)
		ret = _SwapObjects(view, false);
	return ret;
}

// Redo
status_t
TextToShapeAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
TextToShapeAction::GetName(BString& name)
{
	if (fCount > 1)
		name << _GetString(TEXTS_TO_SHAPES, "Texts to Shapes");
	else
		name << _GetString(TEXT_TO_SHAPE, "Text to Shape");
}

// _SwapObjects
status_t
TextToShapeAction::_SwapObjects(CanvasView* view, bool textToShape)
{
	status_t ret = B_ERROR;
	// replace the text objects by the shape objects
	if (History* history = fLayer->GetHistory()) {
		ret = B_OK;
		fPerformed = textToShape;

		BRect area(0.0, 0.0, -1.0, -1.0);
		HistoryListView* listView = view->GetHistoryListView();
		bool isListViewValid = listView && listView->GetLayer() == fLayer;

		for (int32 i = 0; i < fCount && ret >= B_OK; i++) {

			Stroke* srcObject = textToShape ? fTextObjects[i] : fShapeObjects[i];
			Stroke* dstObject = textToShape ? fShapeObjects[i] : fTextObjects[i];

			if (srcObject && dstObject) {
				int32 index = history->IndexOf(srcObject);
				// track the dirty region
				if (i == 0) {
					area = srcObject->Bounds();
					area = area | dstObject->Bounds();
				} else {
					area = area | srcObject->Bounds();
					area = area | dstObject->Bounds();
				}
				// remove text object
				if (history->RemoveModifier(srcObject) &&
					history->AddModifier(dstObject, index)) {
					// update GUI
					if (isListViewValid)
						listView->SetPainter(index, dstObject->Painter());
				} else {
					fprintf(stderr, "TextToShapeAction::_SwapObjects() - weird error: no text and shape at index %ld\n", i);
//					ret = B_ERROR;
				}
			}
		}
		// rebuild canvas
		if (area.IsValid()) {
			fLayer->Touch(area);
			view->RebuildBitmap(area, fLayer);
		}
		// somehow tell the states that something changed
		if (isListViewValid)
			listView->SelectionChanged();
	}
	return ret;
}

