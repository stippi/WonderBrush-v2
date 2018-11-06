// TransformEditor.h

#ifndef TRANSFORM_EDITOR_H
#define TRANSFORM_EDITOR_H

#include "Transformable.h"

class Action;
class BView;
class CanvasView;

class TransformEditor {
 public:
								TransformEditor(CanvasView* canvasView,
												BRect box);
	virtual						~TransformEditor();

	virtual	BRect				Bounds() = 0;
	virtual	void				Update() = 0;

	virtual	void				Draw(BView* into) = 0;

								// an editing
	virtual	Action*				MouseDown(BPoint where) = 0;
	virtual	void				MouseMoved(BPoint where) = 0;
	virtual	Action*				MouseUp() = 0;

	virtual	Action*				MessageReceived(BMessage* message) = 0;

	virtual	void				ModifiersChanged(uint32 modifiers) = 0;
	virtual	void				UpdateToolCursor() = 0;

 protected:
			void				_StrokeBWLine(BView* into,
											  BPoint from, BPoint to) const;
			void				_StrokeBWPoint(BView* into,
											   BPoint point, double angle) const;

			CanvasView*			fCanvasView;

			BPoint				fMousePos;
			bool				fMouseDown;
			uint32				fModifiers;
};

#endif // TRANSFORM_EDITOR_H
