// ChangeColorAction.h

#ifndef CHANGE_COLOR_ACTION_H
#define CHANGE_COLOR_ACTION_H

#include "LayerAction.h"

class Layer;
class Stroke;

class ChangeColorAction : public LayerAction {
 public:
								ChangeColorAction(Layer* layer,
												  Stroke** strokes,
												  rgb_color color,
												  int32 count);
	virtual						~ChangeColorAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			Stroke**			fStrokes;
			int32				fCount;
			rgb_color			fColor;
			rgb_color*			fUndoColors;

};

#endif // CHANGE_COLOR_ACTION_H
