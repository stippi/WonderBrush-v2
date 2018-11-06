// ChangeLayerSettingsAction.h

#ifndef CHANGE_LAYER_SETTINGS_ACTION_H
#define CHANGE_LAYER_SETTINGS_ACTION_H

#include "LayerAction.h"

class Layer;
class Stroke;

class ChangeLayerSettingsAction : public LayerAction {
 public:
								ChangeLayerSettingsAction(Layer* layer,
														  const char* name,
														  float alpha,
														  uint32 mode,
														  uint32 flags);
								ChangeLayerSettingsAction(Layer* layer,
														  float oldAlpha);
	virtual						~ChangeLayerSettingsAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			bool				_NeedsUpdate() const;

			BString				fOldName;
			float				fOldAlpha;
			uint32				fOldMode;
			uint32				fOldFlags;

			BString				fNewName;
			float				fNewAlpha;
			uint32				fNewMode;
			uint32				fNewFlags;
};

#endif // CHANGE_LAYER_SETTINGS_ACTION_H
