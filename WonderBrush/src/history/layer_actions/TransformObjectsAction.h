// TransformObjectsAction.h

#ifndef TRANSFORM_OBJECTS_ACTION_H
#define TRANSFORM_OBJECTS_ACTION_H

#include "AdvancedTransformable.h"
#include "LayerAction.h"

class Layer;
class Stroke;
class TransformState;

class TransformObjectsAction : public LayerAction,
							   public AdvancedTransformable {
 public:
								TransformObjectsAction(TransformState* state,
													   Layer* layer,
													   Stroke** strokes,
													   int32 count);
	virtual						~TransformObjectsAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

	// TransformObjectsAction
			Stroke*				ObjectAt(int32 index) const;
			int32				CountObjects() const
									{ return fCount; }

			void				SetCenterOffset(BPoint offset);
			BPoint				CenterOffset() const
									{ return fCenterOffset; }

	virtual	BRect				Bounds() const
									{ return TransformBounds(fBounds); }

 protected:
			TransformState*		fTransformState;

			Stroke**			fObjects;
			int32				fCount;
			BPoint				fOffset;

			double*				fOriginals;

			BRect				fBounds;

			BPoint				fCenterOffset;
};

#endif // TRANSFORM_OBJECTS_ACTION_H
