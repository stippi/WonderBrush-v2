// BetterMSplitter.h

#ifndef BETTER_MSPLITTER_H
#define BETTER_MSPLITTER_H

#define OWN_IMPLEMENTATION 1

#if OWN_IMPLEMENTATION

#include <View.h>

#include <layout.h>

class BetterMSplitter : public MView, public BView {

#else

#include <MSplitter.h>

class BetterMSplitter : public MSplitter {

#endif

 public:
								BetterMSplitter(const char* name,
												orientation direction = B_VERTICAL);
#if OWN_IMPLEMENTATION
								BetterMSplitter(const char* name,
												MView* previousSibling,
												MView* nextSibling,
												orientation direction = B_VERTICAL);
#endif
	virtual						~BetterMSplitter();

#if OWN_IMPLEMENTATION
								// BView
	virtual void				AttachedToWindow();
	virtual void				DetachedFromWindow();
#endif

	virtual	void 				Draw(BRect updateRect);

#if OWN_IMPLEMENTATION
	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
										   const BMessage* dragMessage);
#endif
								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);

#if OWN_IMPLEMENTATION
								// BetterMSplitter
			void				SetSiblings(MView* previous,
											MView* next);
#endif

 private:
			orientation			fOrient;
#if OWN_IMPLEMENTATION
			bool				fDragging;

			float				fPreviousMinWeight;
			float				fNextMinWeight;
			float				fMaxWeight;

			float				fPrevDist;
			float				fNextDist;

			MView*				fPreviousSibling;
			MView*				fNextSibling;
#endif
};

#endif // BETTER_MSPLITTER_H
