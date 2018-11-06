// ObjectSelection.h

#ifndef OBJECT_SELECTION_H
#define OBJECT_SELECTION_H

#include "TransformBox.h"
#include "ObjectObserver.h"

class Action;
class CanvasView;
class Stroke;
class TransformState;

class ObjectSelection : public TransformBox,
						public ObjectObserver {
 public:
								ObjectSelection(CanvasView* view,
												TransformState* state,
												Stroke** objects,
												int32 count);
								ObjectSelection(const ObjectSelection& other);
	virtual						~ObjectSelection();

	virtual	BRect				Bounds() const;
	virtual	void				Update(bool deep = true);

								// ObjectObserver
	virtual	void				ObjectChanged(const Observable* object);

								// convinience function
			Action*				Perform();
			Action*				Cancel();

	virtual	TransformAction*	MakeAction(const char* actionName,
										   uint32 nameIndex) const;

			Stroke**			Objects() const
									{ return fObjects; }
			int32				CountObjects() const
									{ return fCount; }


 private:
			TransformState*		fTransformState;

			Stroke**			fObjects;
			int32				fCount;

			// saves the transformable objects transformation states
			// prior to this transformation
			double*				fOriginals;
};

#endif // OBJECT_SELECTION_H

