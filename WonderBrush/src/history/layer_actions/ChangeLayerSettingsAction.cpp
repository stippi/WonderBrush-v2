// ChangeLayerSettingsAction.cpp

#include <stdio.h>

#include "CanvasView.h"
#include "Layer.h"
#include "LayersListView.h"

#include "ChangeLayerSettingsAction.h"

// constructor
ChangeLayerSettingsAction::ChangeLayerSettingsAction(Layer* layer,
													 const char* name,
													 float alpha,
													 uint32 mode,
													 uint32 flags)
	: LayerAction(layer),
	  fNewName(name),
	  fNewAlpha(alpha),
	  fNewMode(mode),
	  fNewFlags(flags)
{
	if (fLayer) {
		fOldName = fLayer->Name();
		fOldAlpha = fLayer->Alpha();
		fOldMode = fLayer->Mode();
		fOldFlags = fLayer->Flags();
	}
}

// constructor
ChangeLayerSettingsAction::ChangeLayerSettingsAction(Layer* layer,
													 float oldAlpha)
	: LayerAction(layer),
	  fOldAlpha(oldAlpha)
{
	if (fLayer) {
		fOldName = fNewName = fLayer->Name();
		fNewAlpha = -1.0;
		fOldMode = fNewMode = fLayer->Mode();
		fOldFlags = fNewFlags = fLayer->Flags();
	}
}

// destructor
ChangeLayerSettingsAction::~ChangeLayerSettingsAction()
{
}

// InitCheck
status_t
ChangeLayerSettingsAction::InitCheck()
{
	status_t status = LayerAction::InitCheck();
	if (status >= B_OK) {
		if (fNewName == fOldName
			&& fLayer->Alpha() == fOldAlpha
			&& fNewFlags == fOldFlags
			&& fNewMode == fOldMode)
			status = B_BAD_VALUE;
	}
	return status;
}

// Perform
status_t
ChangeLayerSettingsAction::Perform(CanvasView* view)
{
	if (fLayer) {
		BRect area(fLayer->ActiveBounds());

		fLayer->SetName(fNewName.String());
		// in case of interactive alpha change,
		// the layer will already have the new alpha
		// the second form of the constructor will have been used
		// and fNewAlpha == -1.0
		// _NeedsUpdate() is also aware of this
		if (fNewAlpha >= 0.0)
			fLayer->SetAlpha(fNewAlpha);
		fLayer->SetMode(fNewMode);
		fLayer->SetFlags(fNewFlags);

		area = area | fLayer->ActiveBounds();
		if (area.IsValid() && _NeedsUpdate())
			view->InvalidateCanvas(area, true);

		// update config gui
		if (LayersListView* listView = view->GetLayersListView())
			listView->UpdateLayer(fLayer);

		//  validate fNewAlpha at this point
		fNewAlpha = fLayer->Alpha();
		return B_OK;
	}
	return B_ERROR;
}

// Undo
status_t
ChangeLayerSettingsAction::Undo(CanvasView* view)
{
	if (fLayer) {
		BRect area(fLayer->ActiveBounds());

		fLayer->SetName(fOldName.String());
		fLayer->SetAlpha(fOldAlpha);
		fLayer->SetMode(fOldMode);
		fLayer->SetFlags(fOldFlags);

		area = area | fLayer->ActiveBounds();
		if (area.IsValid() && _NeedsUpdate())
			view->InvalidateCanvas(area, true);

		// update config gui
		if (LayersListView* listView = view->GetLayersListView())
			listView->UpdateLayer(fLayer);

		return B_OK;
	}
	return B_ERROR;
}

// Redo
status_t
ChangeLayerSettingsAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
ChangeLayerSettingsAction::GetName(BString& name)
{
	name << _GetString(CHANGE_LAYER_SETTINGS, "Change Layer Settings");

}

// _NeedsUpdate
bool
ChangeLayerSettingsAction::_NeedsUpdate() const
{
	return (fNewAlpha >= 0.0 && fNewAlpha != fOldAlpha) || fNewMode != fOldMode || fNewFlags != fOldFlags;
}

