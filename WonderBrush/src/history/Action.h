// Action.h

#ifndef ACTION_H
#define ACTION_H

#include <SupportDefs.h>
#include <String.h>

#include "Strings.h" // included here for convenience

class BString;
class CanvasView;

class Action {
 public:
								Action();
	virtual						~Action();
	
	virtual	status_t			InitCheck();

	virtual	status_t			Perform(CanvasView* view);
	virtual status_t			Undo(CanvasView* view);
	virtual status_t			Redo(CanvasView* view);

	virtual void				GetName(BString& name);

	virtual	bool				CombineWithNext(const Action* next);
	virtual	bool				CombineWithPrevious(const Action* previous);

 protected:
			const char*			_GetString(uint32 key,
										   const char* defaultString) const;

			bigtime_t			fTimeStamp;
};

#endif // ACTION_H
