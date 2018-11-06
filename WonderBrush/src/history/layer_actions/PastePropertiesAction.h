// PastePropertiesAction.h

#ifndef PASTE_PROPERTIES_ACTION_H
#define PASTE_PROPERTIES_ACTION_H

#include "LayerAction.h"

class Stroke;
class PropertyObject;

class PastePropertiesAction : public LayerAction {
 public:
								PastePropertiesAction(CanvasView* view,
													  Layer* layer,
											 	  	  Stroke** objects,
											 	  	  int32 count,
											 	  	  PropertyObject* properties);
	virtual						~PastePropertiesAction();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
	Stroke**					fObjects;
	Stroke**					fClonedObjects;
	int32						fCount;

	int32						fPropertyCount;
};

#endif // PASTE_PROPERTIES_ACTION_H
