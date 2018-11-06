// HistoryManager.cpp

#include <stdio.h>
#include <string.h>

#include <Locker.h>
#include <String.h>

#include "Action.h"

#include "HistoryManager.h"

// constructor
HistoryManager::HistoryManager()
	: BLocker("history"),
	  fSavedAction(NULL)
{
}

// destructor
HistoryManager::~HistoryManager()
{
	Clear();
}

// AddAction
status_t
HistoryManager::AddAction(Action* action)
{
	status_t status = B_ERROR;
	if (Lock()) {
		if (action && (status = action->InitCheck()) == B_OK) {
			// try to collapse actions to a single action
			bool add = true;
			if (!fUndoHistory.empty()) {
				if (Action* top = fUndoHistory.top()) {
					if (top->CombineWithNext(action)) {
						add = false;
						delete action;
					} else if (action->CombineWithPrevious(top)) {
						fUndoHistory.pop();
						delete top;
					}
				}
			}
			if (add)
				fUndoHistory.push(action);

			// the redo stack needs to be empty
			// as soon as an action was added (also in case of collapsing)
			while (!fRedoHistory.empty()) {
				delete fRedoHistory.top();
				fRedoHistory.pop();
			}
		}
		Unlock();
	}
	return status;
}

// Undo
status_t
HistoryManager::Undo(CanvasView* view)
{
	status_t status = B_ERROR;
	if (Lock()) {
		if (!fUndoHistory.empty()) {
			Action* action = fUndoHistory.top();
			fUndoHistory.pop();
			status = action->Undo(view);
			if (status == B_OK)
				fRedoHistory.push(action);
			else
				fUndoHistory.push(action);
		}
		Unlock();
	}
	return status;
}

// Redo
status_t
HistoryManager::Redo(CanvasView* view)
{
	status_t status = B_ERROR;
	if (Lock()) {
		if (!fRedoHistory.empty()) {
			Action* action = fRedoHistory.top();
			fRedoHistory.pop();
			status = action->Redo(view);
			if (status == B_OK)
				fUndoHistory.push(action);
			else
				fRedoHistory.push(action);
		}
		Unlock();
	}
	return status;
}

// UndoName
bool
HistoryManager::GetUndoName(BString& name)
{
	bool success = false;
	if (Lock()) {
		if (!fUndoHistory.empty()) {
			name << " ";
			fUndoHistory.top()->GetName(name);
			success = true;
		}
		Unlock();
	}
	return success;
}

// RedoName
bool
HistoryManager::GetRedoName(BString& name)
{
	bool success = false;
	if (Lock()) {
		if (!fRedoHistory.empty()) {
			name << " ";
			fRedoHistory.top()->GetName(name);
			success = true;
		}
		Unlock();
	}
	return success;
}

// Clear
void
HistoryManager::Clear()
{
	if (Lock()) {
		while (!fUndoHistory.empty()) {
			delete fUndoHistory.top();
			fUndoHistory.pop();
		}
		while (!fRedoHistory.empty()) {
			delete fRedoHistory.top();
			fRedoHistory.pop();
		}
		Unlock();
	}
}

// Save
void
HistoryManager::Save()
{
	if (Lock()) {
		if (!fUndoHistory.empty())
			fSavedAction = fUndoHistory.top();
		Unlock();
	}
}

// IsSaved
bool
HistoryManager::IsSaved()
{
	bool saved = false;
	if (Lock()) {
		saved = fUndoHistory.empty();
		if (fSavedAction && !saved) {
			if (fSavedAction == fUndoHistory.top())
				saved = true;
		}
		Unlock();
	}
	return saved;
}



