
/*

	Bubblehelp class Copyright (C) 1998 Marco Nelissen <marcone@xs4all.nl>
	Freely usable in non-commercial applications, as long as proper credit
	is given.

	Usage:
	- Add the file BubbleHelper.cpp to your project
	- #include "BubbleHelper.h" in your files where needed
	- Create a single instance of BubbleHelper (it will serve your entire
	  application). It is safe to create one on the stack or as a global.
	- Call SetHelp(view,text) for each view to which you wish to attach a text.
	- Use SetHelp(view,NULL) to remove text from a view.

	This could be implemented as a BMessageFilter as well, but that means using
	one bubblehelp-instance for each window to which you wish to add help-bubbles.
	Using a single looping thread for everything turned out to be the most practical
	solution.

*/

#include <new>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <Application.h>
#include <Region.h>
#include <Screen.h>
#include <TextView.h>
#include <Window.h>

#include "BubbleHelper.h"

using std::nothrow;

// static variables
long			BubbleHelper::fRunCount = 0;
BubbleHelper*	BubbleHelper::fDefaultHelper = NULL;

struct help_pair
{
	BView*	view;
	char*	text;
};

// free_help_pair
void
free_help_pair(help_pair* pair)
{
	if (pair && pair->text)
		free(pair->text);
	delete pair;
}

// constructor
BubbleHelper::BubbleHelper()
	: fHelperThread(B_ERROR),
	  fHelpList(NULL),
	  fTextWindow(NULL),
	  fTextView(NULL)
{
	// You only need one instance per application.
	if (atomic_add(&fRunCount, 1) == 0) {
		fHelpList = new BList(30);
		fHelperThread = spawn_thread(_helper, "helper", B_NORMAL_PRIORITY, this);
		if (fHelperThread >= 0)
			resume_thread(fHelperThread);
		fEnabled = true;
	} else {
		// Since you shouldn't be creating more than one instance
		// you may want to jump straight into the debugger here.
		debugger("only one BubbleHelper instance allowed/necessary");
	  //  fHelperThread=-1;
	  //  fHelpList=NULL;
	  //  fEnabled=false;
	}
}

// destructor
BubbleHelper::~BubbleHelper()
{
	if (fHelperThread >= 0) {
		// force helper thread into a known state
		bool locked = fTextWindow->Lock();
		// Be rude...
		kill_thread(fHelperThread);
		// dispose of window
		if (locked) {
			fTextWindow->PostMessage(B_QUIT_REQUESTED);
			fTextWindow->Unlock();
		}
	}
	if (fHelpList) {
		help_pair *pair;
		int i = fHelpList->CountItems() - 1;
		while(i >= 0) {
			pair = (help_pair*)fHelpList->RemoveItem(i);
			free_help_pair(pair);
			i--;
		}
		delete fHelpList;
	}
	atomic_add(&fRunCount, -1);
}

// CreateDefault
BubbleHelper*
BubbleHelper::CreateDefault()
{
	return Default();
}

// DeleteDefault
void
BubbleHelper::DeleteDefault()
{
	if (fDefaultHelper) {
		delete fDefaultHelper;
		fDefaultHelper = NULL;
	}
}

// Default
BubbleHelper*
BubbleHelper::Default()
{
	if (!fDefaultHelper)
		fDefaultHelper = new(nothrow) BubbleHelper();
	return fDefaultHelper;
}

// SetHelp
void
BubbleHelper::SetHelp(BView* view, const char* text)
{
	if (this && view) {
		// reset previous help if already contained
		bool found = false;
		for (int i = 0; help_pair *pair=(help_pair*)fHelpList->ItemAt(i); i++) {
			if (pair->view == view) {
				found = true;
				if (text) {
					free(pair->text);
					pair->text = strdup(text);
				} else {
					fHelpList->RemoveItem(pair);
					free_help_pair(pair);
				}
				break;
			}
		}

		// add new text, if any
		if (!found) {
			if (text) {
				help_pair* pair = new help_pair;
				pair->view = view;
				pair->text = strdup(text);
				if (!fHelpList->AddItem(pair))
					free_help_pair(pair);
			}
		}
	}
}

// SetEnabled
void
BubbleHelper::SetEnabled(bool enable)
{
	fEnabled=enable;
}

// _DisplayHelp
void
BubbleHelper::_DisplayHelp(const char* text, BPoint where)
{
	fTextView->SetText(text);

	float height = fTextView->TextHeight(0, 2E6) + 4;
	float width = 0;
	int numlines = fTextView->CountLines();
	float linewidth;
	for (int i = 0; i < numlines; i++)
		if ((linewidth = fTextView->LineWidth(i)) > width)
			width = linewidth;
	fTextWindow->ResizeTo(width + 4, height);
	fTextView->SetTextRect(BRect(2.0, 2.0, width + 2.0, height + 2.0));

	BScreen screen;
	BPoint dest = where + BPoint(0.0, 20.0);
	BRect screenframe = screen.Frame();
	if ((dest.y + height) > (screenframe.bottom - 3.0))
		dest.y = dest.y - (16.0 + height + 8.0);

	if ((dest.x + width + 4.0) > (screenframe.right))
		dest.x = dest.x - ((dest.x + width + 4.0) - screenframe.right);

	_ShowBubble(dest);
}

// _Helper
void
BubbleHelper::_Helper()
{
	// Wait until the BApplication becomes valid, in case
	// someone creates this as a global variable.
	while (!be_app_messenger.IsValid())
		snooze(200000);

	fTextWindow = new BWindow(BRect(-100.0, -100.0, -50.0, -50.0), "",
		B_BORDERED_WINDOW_LOOK, B_FLOATING_ALL_WINDOW_FEEL,
		B_NOT_MOVABLE | B_AVOID_FOCUS);

	fTextView = new BTextView(BRect(0.0, 0.0, 50.0, 50.0), " ",
							  BRect(2.0, 2.0, 48.0, 48.0),
							  B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	fTextView->MakeEditable(false);
	fTextView->MakeSelectable(false);
	fTextView->SetWordWrap(false);
	fTextView->SetLowColor(240, 240, 100);
	fTextView->SetViewColor(240, 240, 100);
	fTextView->SetHighColor(0, 0, 0);
	fTextWindow->AddChild(fTextView);
	fTextWindow->Hide();
	fTextWindow->Show();
	if (fTextWindow->Lock()) {
		fTextWindow->Activate(false);
		rename_thread(fTextWindow->Thread(), "bubble");
		fTextWindow->Unlock();
	}

	ulong delaycounter=0;
	BPoint lastwhere;

	while (be_app_messenger.IsValid()) {
		BPoint where;
		ulong buttons;
		if (fEnabled) {
			if (fTextWindow->Lock()) {
				fTextView->GetMouse(&where,&buttons);
				fTextView->ConvertToScreen(&where);
				if (lastwhere != where || buttons)
					delaycounter=0;
				else {
					// mouse didn't move
					if (delaycounter++ > 5) {
						delaycounter=0;
						// mouse didn't move for a while
						BView* view = _FindView(where);
						char* text = NULL;
						while (view && (text = _GetHelp(view)) == NULL)
							view=view->Parent();
						if (text) {
							_DisplayHelp(text, where);
							// wait until mouse moves out of view, or wait
							// for timeout
							long displaycounter = 0;
							BPoint where2;
							long displaytime = max_c(20, strlen(text));
							do {
								fTextWindow->Unlock();
								snooze(100000);
								if(!fTextWindow->Lock())
									goto end; //window is apparently gone
								fTextView->GetMouse(&where2,&buttons);
								fTextView->ConvertToScreen(&where2);
							} while (!buttons && where2 == where
									 && (displaycounter++ < displaytime));

							_HideBubble();
							do {
								fTextWindow->Unlock();
								snooze(100000);
								if (!fTextWindow->Lock())
									goto end; //window is apparently gone
								fTextView->GetMouse(&where2, &buttons);
								fTextView->ConvertToScreen(&where2);
							} while (where2 == where);
						}
					}
				}
				lastwhere=where;
				fTextWindow->Unlock();
			}
		}
end:
		snooze(100000);
	}
	// (this thread normally gets killed by the destructor before arriving here)
}

// _helper
long
BubbleHelper::_helper(void *arg)
{
	((BubbleHelper*)arg)->_Helper();
	return 0;
}

// UnsetHelp
void
BubbleHelper::UnsetHelp(BWindow* window)
{
	if (this && window) {
		// delete all texts of views in this window
		for (int32 i = 0; help_pair* pair = (help_pair*)fHelpList->ItemAt(i); i++) {
			if (pair->view->Window() == window) {
				fHelpList->RemoveItem(pair);
				free(pair->text);
				delete pair;
			}
		}
	}
}

// _GetHelp
char*
BubbleHelper::_GetHelp(BView *view)
{
	int i = 0;
	// This could be sped up by sorting the list and
	// doing a binary search.
	// Right now this is left as an exercise for the
	// reader, or should I say "third party opportunity"?
	while (help_pair* pair = (help_pair*)fHelpList->ItemAt(i++)) {
		if (pair->view == view)
			return pair->text;
	}
	return NULL;
}

// _FindView
BView*
BubbleHelper::_FindView(BPoint where)
{
	BView* winview=NULL;
	BWindow* win;
	long windex = 0;
	while ((winview == NULL) && ((win = be_app->WindowAt(windex++)) != NULL)) {
		if (win != fTextWindow) {
			// lock with timeout, in case somebody has a non-running window around
			// in their app.
			if (win->LockWithTimeout(1E6) == B_OK) {
				BRect frame=win->Frame();
				if (frame.Contains(where)) {
					BPoint winpoint;
					winpoint = where-frame.LeftTop();
					winview = win->FindView(winpoint);
					if (winview) {
						BRegion region;
						BPoint newpoint = where;
						winview->ConvertFromScreen(&newpoint);
						winview->GetClippingRegion(&region);
						if (!region.Contains(newpoint))
							winview=0;
					}
				}
				win->Unlock();
			}
		}
	}
	return winview;
}

// _HideBubble
void
BubbleHelper::_HideBubble()
{
	fTextWindow->MoveTo(-1000,-1000); // hide it
	if (!fTextWindow->IsHidden())
		fTextWindow->Hide();
}

// _ShowBubble
void
BubbleHelper::_ShowBubble(BPoint dest)
{
	fTextWindow->MoveTo(dest);
	fTextWindow->SetWorkspaces(B_CURRENT_WORKSPACE);
	if(fTextWindow->IsHidden())
		fTextWindow->Show();
}



