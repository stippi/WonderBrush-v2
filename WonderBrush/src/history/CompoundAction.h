// CompoundAction.h

#ifndef COMPOUND_ACTION_H
#define COMPOUND_ACTION_H

#include "Action.h"

class CompoundAction : public Action {
 public:
								CompoundAction(Action** actions,
											   int32 count,
											   const char* name,
											   int32 nameIndex);
	virtual						~CompoundAction();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

 private:
			Action**			fActions;
			int32				fCount;

			BString				fName;
			int32				fNameIndex;
};

#endif // COMPOUND_ACTION_H
