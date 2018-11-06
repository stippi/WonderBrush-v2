// RotateAction.cpp

#include <stdio.h>

#include <Bitmap.h>

#include "Canvas.h"
#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "LayersListView.h"
#include "Transformable.h"

#include "RotateAction.h"

// constructor
RotateAction::RotateAction(Canvas* canvas,
						   double angle)
	: CanvasAction(canvas),
	  fAngle(angle)
{
}

// destructor
RotateAction::~RotateAction()
{
}

// InitCheck
status_t
RotateAction::InitCheck()
{
	status_t status = fAngle != 0.0 ? CanvasAction::InitCheck()
									: B_BAD_VALUE;
	return status;
}

// Perform
status_t
RotateAction::Perform(CanvasView* view)
{
	return _Rotate(view, fAngle);
}

// Undo
status_t
RotateAction::Undo(CanvasView* view)
{
	return _Rotate(view, -fAngle);
}

// Redo
status_t
RotateAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
RotateAction::GetName(BString& name)
{
	name << _GetString(ROTATE_CANVAS, "Rotate Canvas");
}

// _Rotate
status_t
RotateAction::_Rotate(CanvasView* view, double angle)
{
	status_t status = B_ERROR;
	if (fCanvas) {
		// before we start...
		view->SetTo(NULL);
		// resize each layer
		for (int32 i = 0; Layer* layer = fCanvas->LayerAt(i); i++) {
			status = layer->RotateBy(angle);
			if (status < B_OK) {
				// TODO: should we undo what has been
				// done so far? I guess we should!
				fprintf(stderr, "error rotating layer '%s': %s\n", layer->Name(), strerror(status));
				return status;
			}
		}
		if (status >= B_OK) {

			BRect bounds(fCanvas->Bounds());
			BPoint center((bounds.left + bounds.right + 1) / 2.0,
						  (bounds.top + bounds.bottom + 1) / 2.0);

			Transformable transform;
			transform.RotateBy(center, angle);

			bounds = transform.TransformBounds(bounds);
			bounds.OffsetTo(BPoint(0.0, 0.0));
			fCanvas->SetBounds(bounds);

/*			float zoom;
			BPoint zoomCenter;
			fCanvas->GetZoomState(zoom, zoomCenter);
zoomCenter.PrintToStream();
			transform.Transform(&zoomCenter);
zoomCenter.PrintToStream();
			fCanvas->SetZoomState(zoom, zoomCenter);*/

			// but we need to rebuild each layer yet
			BBitmap* temp = new BBitmap(bounds, 0, B_GRAY8);
			BRect compose;
			for (int32 i = 0; Layer* layer = fCanvas->LayerAt(i); i++) {
				BRect rebuild = layer->ActiveBounds();
				if (i == 0)
					compose = rebuild;
				else
					compose = compose | rebuild;
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
