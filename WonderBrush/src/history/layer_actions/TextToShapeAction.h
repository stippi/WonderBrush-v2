// TextToShapeAction.h

#ifndef TEXT_TO_SHAPE_ACTION_H
#define TEXT_TO_SHAPE_ACTION_H

#include "LayerAction.h"

class ShapeStroke;
class TextStroke;

class TextToShapeAction : public LayerAction {
 public:
								// action takes over ownership of "textObjects"
								TextToShapeAction(Layer* layer,
											   	  TextStroke** textObjects,
											   	  int32 count);
	virtual						~TextToShapeAction();

	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			status_t			_SwapObjects(CanvasView* view,
											 bool textToShape);

	TextStroke**				fTextObjects;
	ShapeStroke**				fShapeObjects;
	int32						fCount;
	bool						fPerformed;
};

#endif // TEXT_TO_SHAPE_ACTION_H
