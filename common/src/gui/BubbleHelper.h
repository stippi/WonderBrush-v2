
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

#ifndef BUBBLEHELPER_H
#define BUBBLEHELPER_H

#include <OS.h>
#include <Point.h>

class BList;
class BTextView;
class BView;
class BWindow;

class BubbleHelper {
 public:
								BubbleHelper();
	virtual						~BubbleHelper();

	static	BubbleHelper*		CreateDefault();
	static	void				DeleteDefault();
	static	BubbleHelper*		GetDefault()
									{ return fDefaultHelper; }
	static	BubbleHelper*		Default();

			void				SetHelp(BView* view,
										const char* text);

			void				SetEnabled(bool enable = true);
			void				UnsetHelp(BView* view)
									{ SetHelp(view, NULL); }
			void				UnsetHelp(BWindow* window);

	inline	bool				IsEnabled()
									{ return fEnabled; }

 private:
			void				_DisplayHelp(const char* text,
											 BPoint where);
			void				_Helper();
	static	int 				_helper(void* arg);

			char*				_GetHelp(BView* view);
			BView*				_FindView(BPoint where);

	// may be accessed by several threads
	volatile bool				fEnabled;

			void				_HideBubble();
			void				_ShowBubble(BPoint dest);

			thread_id			fHelperThread;
			BList*				fHelpList;
			BWindow*			fTextWindow;
			BTextView*			fTextView;

	static	int32				fRunCount;
	static	BubbleHelper*		fDefaultHelper;
};

#endif
