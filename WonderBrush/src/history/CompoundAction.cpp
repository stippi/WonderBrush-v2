// CompoundAction.cpp

#include <stdio.h>

#include <Bitmap.h>

#include "LayerContainer.h"
#include "Layer.h"
#include "Stroke.h"

#include "CanvasView.h"
#include "LayersListView.h"

#include "CompoundAction.h"

// constructor
CompoundAction::CompoundAction(Action** actions,
							   int32 count,
							   const char* name,
							   int32 nameIndex)
	: Action(),
	  fActions(actions),
	  fCount(count),
	  fName(name),
	  fNameIndex(nameIndex)
{
}

// destructor
CompoundAction::~CompoundAction()
{
	for (int32 i = 0; i < fCount; i++)
		delete fActions[i];
	delete[] fActions;
}

// InitCheck
status_t
CompoundAction::InitCheck()
{
	status_t status = fActions && fCount > 0 ? B_OK : B_BAD_VALUE;
	return status;
}

// Perform
status_t
CompoundAction::Perform(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		int32 i = 0;
		for (; i < fCount; i++) {
			if (fActions[i])
				status = fActions[i]->Perform(view);
			if (status < B_OK)
				break;
		}
/*		if (status < B_OK) {
			// roll back
			i--;
			for (; i >= 0; i--) {
				if (fActions[i])
					fActions[i]->Undo(view);
			}
		}*/
	}
	return status;
}

// Undo
status_t
CompoundAction::Undo(CanvasView* view)
{
	status_t status = InitCheck();
	if (status >= B_OK) {
		int32 i = fCount - 1;
		for (; i >= 0; i--) {
			if (fActions[i])
				status = fActions[i]->Undo(view);
			if (status < B_OK)
				break;
		}
	}
	return status;
}

// Redo
status_t
CompoundAction::Redo(CanvasView* view)
{
	return Perform(view);
}

// GetName
void
CompoundAction::GetName(BString& name)
{
	name << _GetString(fNameIndex, fName.String());
}
