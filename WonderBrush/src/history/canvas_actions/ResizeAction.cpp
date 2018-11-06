// ResizeAction.cpp

#include <stdio.h>

#include <Bitmap.h>

#include "Canvas.h"
#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "LayersListView.h"

#include "ResizeAction.h"

// constructor
ResizeAction::ResizeAction(Canvas* canvas,
						   BRect newBounds)
	: CanvasAction(canvas),
	  fNewBounds(newBounds),
	  fOldBounds(0.0, 0.0, -1.0, -1.0)
{
	if (fCanvas) {
		fOldBounds = fCanvas->Bounds();
	}
}

// destructor
ResizeAction::~ResizeAction()
{
}

// InitCheck
status_t
ResizeAction::InitCheck()
{
	status_t status = fOldBounds.IsValid() && fNewBounds.IsValid()
					  && fOldBounds != fNewBounds ? B_OK : B_BAD_VALUE;
	return status;
}

// Perform
status_t
ResizeAction::Perform(CanvasView* view)
{
	return _ChangeSize(view, fNewBounds);
}

// Undo
status_t
ResizeAction::Undo(CanvasView* view)
{
	return _ChangeSize(view, fOldBounds);
}

// Redo
status_t
ResizeAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
ResizeAction::GetName(BString& name)
{
	name << _GetString(RESIZE_CANVAS, "Resize Canvas");
}

// _ChangeSize
status_t
ResizeAction::_ChangeSize(CanvasView* view, BRect newBounds)
{
	status_t status = B_ERROR;
	if (fCanvas) {
		// before we start...
		view->SetTo(NULL);
		// resize each layer
		for (int32 i = 0; Layer* layer = fCanvas->LayerAt(i); i++) {
			status = layer->ResizeTo(newBounds);
			if (status < B_OK) {
				// TODO: should we undo what has been
				// done so far? I guess we should!
				fprintf(stderr, "error changing size of layer '%s': %s\n", layer->Name(), strerror(status));
				return status;
			}
		}
		if (status >= B_OK) {
			BRect bounds(0.0, 0.0,
						 ceilf(newBounds.Width()),
						 ceilf(newBounds.Height()));
			fCanvas->SetBounds(bounds);
			// but we need to rebuild each layer yet
			BBitmap* temp = new BBitmap(bounds, 0, B_GRAY8);
			for (int32 i = 0; Layer* layer = fCanvas->LayerAt(i); i++) {
				BRect rebuild = layer->ActiveBounds();
				layer->Rebuild(temp, rebuild, 0,
							   view->GetInfoView());
			}
			delete temp;
			// now that we have finished...
			view->SetTo(fCanvas);
			view->SetToLayer(fCanvas->CurrentLayer());
		}
	}
	return status;
}
