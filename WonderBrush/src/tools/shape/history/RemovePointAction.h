// RemovePointAction.h

#ifndef REMOVE_POINT_ACTION_H
#define REMOVE_POINT_ACTION_H

#include "PathAction.h"

class RemovePointAction : public PathAction {
 public:
								// for removing the point clicked on
								// independent of selection
								RemovePointAction(ShapeState* state,
												  ShapeStroke* modifier,
												  int32 index,
												  const int32* selection,
												  int32 count);
								// for removing the selection
								RemovePointAction(ShapeState* state,
												  ShapeStroke* modifier,
												  const int32* selection,
												  int32 count);
	virtual						~RemovePointAction();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			void				_Init(const int32* indices, int32 count,
									  const int32* selection,
									  int32 selectionCount);

			int32*				fIndex;
			BPoint*				fPoint;
			BPoint*				fPointIn;
			BPoint*				fPointOut;
			bool*				fConnected;
			int32				fCount;

			bool				fWasClosed;

			int32*				fOldSelection;
			int32				fOldSelectionCount;
};

#endif // REMOVE_POINT_ACTION_H
