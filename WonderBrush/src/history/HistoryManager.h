// HistoryManager.h

#ifndef HISTORY_MANAGER_H
#define HISTORY_MANAGER_H

#include <stack.h>
#include <Locker.h>

class Action;
class CanvasView;

class HistoryManager : public BLocker {
 public:
								HistoryManager();
	virtual						~HistoryManager();

			status_t			AddAction(Action* action);

			status_t			Undo(CanvasView* view);
			status_t			Redo(CanvasView* view);

			bool				GetUndoName(BString& name);
			bool				GetRedoName(BString& name);

			void				Clear();
			void				Save();
			bool				IsSaved();

 private:

	typedef stack<Action*> ActionStack;

			ActionStack			fUndoHistory;
			ActionStack			fRedoHistory;
			Action*				fSavedAction;
};

#endif // HISTORY_MANAGER_H
