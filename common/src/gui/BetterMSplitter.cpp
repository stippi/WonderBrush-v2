// BetterMSplitter.cpp

#include <stdio.h>

#include <Cursor.h>
#include <Message.h>
#include <Window.h>

#include <MGroup.h>

#include "BetterMSplitter.h"

const unsigned char kHCursorData[] = { 16, 1, 8, 8,
	0x01, 0x80, 0x02, 0x40, 0x04, 0x20, 0x08, 0x10,
	0x0f, 0xf0, 0x00, 0x00, 0xff, 0xff, 0x80, 0x01,
	0x80, 0x01, 0xff, 0xff, 0x00, 0x00, 0x0f, 0xf0,
	0x08, 0x10, 0x04, 0x20, 0x02, 0x40, 0x01, 0x80,

	0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0,
	0x0f, 0xf0, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
	0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x0f, 0xf0,
	0x0f, 0xf0, 0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80
};

const unsigned char kVCursorData[] = { 16, 1, 8, 8,
	0x03, 0xc0, 0x02, 0x40, 0x02, 0x40, 0x02, 0x40,
	0x1a, 0x58, 0x2a, 0x54, 0x4a, 0x52, 0x8a, 0x51,
	0x8a, 0x51, 0x4a, 0x52, 0x2a, 0x54, 0x1a, 0x58,
	0x02, 0x40, 0x02, 0x40, 0x02, 0x40, 0x03, 0xc0,

	0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0,
	0x1b, 0xd8, 0x3b, 0xdc, 0x7b, 0xde, 0xfb, 0xdf,
	0xfb, 0xdf, 0x7b, 0xde, 0x3b, 0xdc, 0x1b, 0xd8,
	0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0, 0x03, 0xc0
};


// constructor
BetterMSplitter::BetterMSplitter(const char* name,
								 orientation direction)
#if OWN_IMPLEMENTATION
	: MView(),
	  BView(BRect(0.0, 0.0, 10.0, 10.0), "name",
	  		B_FOLLOW_NONE, B_WILL_DRAW | B_NAVIGABLE_JUMP | B_FULL_UPDATE_ON_RESIZE),
	  fOrient(direction),
	  fDragging(false),
	  fPreviousMinWeight(-1-0),
	  fNextMinWeight(-1.0),
	  fMaxWeight(-1.0),
	  fPreviousSibling(NULL),
	  fNextSibling(NULL)
#else
	: MSplitter(false),
	  fOrient(direction)
#endif
{
#if !OWN_IMPLEMENTATION
	SetName(name);
#endif
	SetViewColor(B_TRANSPARENT_32_BIT);
}

#if OWN_IMPLEMENTATION
// constructor
BetterMSplitter::BetterMSplitter(const char* name,
								 MView* previousSibling,
								 MView* nextSibling,
								 orientation direction)
	: MView(),
	  BView(BRect(0.0, 0.0, 10.0, 10.0), "name",
	  		B_FOLLOW_NONE, B_WILL_DRAW | B_NAVIGABLE_JUMP),
	  fOrient(direction),
	  fDragging(false),
	  fPreviousMinWeight(-1-0),
	  fNextMinWeight(-1.0),
	  fMaxWeight(-1.0),
	  fPreviousSibling(previousSibling),
	  fNextSibling(nextSibling)
{
	SetViewColor(B_TRANSPARENT_32_BIT);
}
#endif

// destructor
BetterMSplitter::~BetterMSplitter()
{
}

#if OWN_IMPLEMENTATION
// SetSiblings
void
BetterMSplitter::SetSiblings(MView* previous, MView* next)
{
	fPreviousSibling = previous;
	fNextSibling = next;
}

// AttachedToWindow
void
BetterMSplitter::AttachedToWindow()
{
	if (!fPreviousSibling && !fNextSibling) {
		MGroup* parent = dynamic_cast<MGroup*>(Parent());
		if (parent) {
			fPreviousSibling = dynamic_cast<MView*>(PreviousSibling());
			fNextSibling = dynamic_cast<MView*>(NextSibling());
		}
	} else {
		BView* pParent = dynamic_cast<BView*>(fPreviousSibling);
		BView* nParent = dynamic_cast<BView*>(fNextSibling);
		if (pParent && nParent) {
			pParent = pParent->Parent();
			nParent = nParent->Parent();
			if (pParent != nParent) {
				debugger("BetterMSplitter: Siblings need to be attached to the same MGroup!");
			}
		}
	}
}

// DetachedFromWindow
void
BetterMSplitter::DetachedFromWindow()
{
	fPreviousSibling = NULL;
	fNextSibling = NULL;
}
#endif

// Draw
void
BetterMSplitter::Draw(BRect updateRect)
{
	rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color light = tint_color(background, B_LIGHTEN_MAX_TINT);
	rgb_color shadow = tint_color(background, B_DARKEN_2_TINT);
	rgb_color darkShadow = tint_color(background, B_DARKEN_3_TINT);

	BRect r = Bounds();

	// frame
	BeginLineArray(4);
		AddLine(BPoint(r.left, r.bottom),
				BPoint(r.left, r.top), light);
		AddLine(BPoint(r.left + 1.0, r.top),
				BPoint(r.right, r.top), light);
		AddLine(BPoint(r.right, r.top + 1.0),
				BPoint(r.right, r.bottom), shadow);
		AddLine(BPoint(r.right - 1.0, r.bottom),
				BPoint(r.left + 1.0, r.bottom), shadow);
	EndLineArray();

	// background frame
	r.InsetBy(1.0, 1.0);
	BeginLineArray(4);
		AddLine(BPoint(r.left, r.bottom),
				BPoint(r.left, r.top), background);
		AddLine(BPoint(r.left + 1.0, r.top),
				BPoint(r.right, r.top), background);
		AddLine(BPoint(r.right, r.top + 1.0),
				BPoint(r.right, r.bottom), background);
		AddLine(BPoint(r.right - 1.0, r.bottom),
				BPoint(r.left + 1.0, r.bottom), background);
	EndLineArray();

	// dots
	r.InsetBy(1.0, 1.0);
	if (fOrient == B_VERTICAL) {
		BPoint dot = r.LeftTop();
		BPoint stop = r.LeftBottom();
		int32 num = 1;
		while (dot.y <= stop.y) {
			rgb_color col1;
			rgb_color col2;
			switch (num) {
				case 1:
					col1 = darkShadow;
					col2 = background;
					break;
				case 2:
					col1 = background;
					col2 = light;
					break;
				case 3:
					col1 = background;
					col2 = background;
					num = 0;
					break;
			}
			SetHighColor(col1);
			StrokeLine(dot, dot, B_SOLID_HIGH);
			SetHighColor(col2);
			dot.x++;
			StrokeLine(dot, dot, B_SOLID_HIGH);
			dot.x -= 1.0;
			// next pixel
			num++;
			dot.y++;
		}
	} else {
		BPoint dot = r.LeftTop();
		BPoint stop = r.RightTop();
		int32 num = 1;
		while (dot.x <= stop.x) {
			rgb_color col1;
			rgb_color col2;
			switch (num) {
				case 1:
					col1 = darkShadow;
					col2 = background;
					break;
				case 2:
					col1 = background;
					col2 = light;
					break;
				case 3:
					col1 = background;
					col2 = background;
					num = 0;
					break;
			}
			SetHighColor(col1);
			StrokeLine(dot, dot, B_SOLID_HIGH);
			SetHighColor(col2);
			dot.y++;
			StrokeLine(dot, dot, B_SOLID_HIGH);
			dot.y -= 1.0;
			// next pixel
			num++;
			dot.x++;
		}
	}
}

#if OWN_IMPLEMENTATION
// MouseDown
void
BetterMSplitter::MouseDown(BPoint where)
{
	if (fPreviousSibling && fNextSibling) {
		fMaxWeight = fPreviousSibling->ct_mpm.weight + fNextSibling->ct_mpm.weight;

		BView* prev = dynamic_cast<BView*>(fPreviousSibling);
		BView* next = dynamic_cast<BView*>(fNextSibling);

		if (fOrient == B_HORIZONTAL) {
			float totalHeight = next->Frame().Height() + prev->Frame().Height();
//printf("total height: %f\n", totalHeight);

			fPreviousMinWeight = (fPreviousSibling->mpm.mini.y / totalHeight) * fMaxWeight;
			fNextMinWeight = (fNextSibling->mpm.mini.y / totalHeight) * fMaxWeight;
//printf("min weight prev: %f\n", fPreviousMinWeight);
//printf("min weight next: %f\n", fNextMinWeight);

			float center = (Frame().top + Frame().bottom) / 2.0;
//printf("center: %f\n", center);
			fPrevDist = center - prev->Frame().top;
			fNextDist = next->Frame().bottom - center;
//printf("prev dist: %f\n", fPrevDist);
//printf("next dist: %f\n", fNextDist);
		}
	}

	fDragging = true;
	SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
}

// MouseUp
void
BetterMSplitter::MouseUp(BPoint where)
{
	fDragging = false;
}

// MouseMoved
void
BetterMSplitter::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	// TODO: this function should contain a check, if the siblings
	// are still contained in the parent view. Maybe iterating over the
	// children of the parent, before even using the cached sibling pointers...
	if (fDragging && fPreviousSibling && fNextSibling) {
		// check if we should still be dragging
		uint32 buttons;
		// R5 app_server can give us bogus mouse coords in case we
		// have moved meanwhile...
		GetMouse(&where, &buttons, false);
		if (buttons == 0) {
			MouseUp(where);
			return;
		}
		// get our parent and relayout sibling views
		BView* pSibling = dynamic_cast<BView*>(fPreviousSibling);
		BView* nSibling = dynamic_cast<BView*>(fNextSibling);
		if (pSibling && nSibling) {
			MGroup* parent = dynamic_cast<MGroup*>(pSibling->Parent());
			if (parent) {
				float weight = -1.0;
				if (fOrient == B_VERTICAL) {

					float pWidth = pSibling->Bounds().Width();
					float nWidth = nSibling->Bounds().Width();
					float width = pWidth + nWidth;
	
					float diff = where.x - (Bounds().left + Bounds().right) / 2.0;
	
					float pDestWidth = pWidth + diff;
	
					weight = (pDestWidth / width) * fMaxWeight;

				} else {

					float pHeight = pSibling->Bounds().Height();
					float nHeight = nSibling->Bounds().Height();
					float height = pHeight + nHeight;
	
					float diff = where.y - (Bounds().top + Bounds().bottom) / 2.0;
	
					float pDestHeight = pHeight + diff;
	
					weight = (pDestHeight / height) * fMaxWeight;
//					where = ConvertToParent(where);
//					weight = where.y 
				}
				if (weight < fPreviousMinWeight)
					weight = fPreviousMinWeight;
				if (fMaxWeight - weight < fNextMinWeight)
					weight = fMaxWeight - fNextMinWeight;
//				if (weight >= fPreviousMinWeight && fMaxWeight - weight >= fNextMinWeight) {
				if (weight != fPreviousSibling->mpm.weight ||
					fMaxWeight - weight != fNextSibling->mpm.weight) {

//printf("previous weight: %f (%f)\n", fPreviousSibling->mpm.weight, fPreviousMinWeight);
//printf("next weight: %f (%f) (sum: %f)\n", fNextSibling->mpm.weight, fNextMinWeight, fMaxWeight);
//printf("weight: %f (%f)\n", weight, fSiblingWeight);

					fPreviousSibling->mpm.weight = fPreviousSibling->ct_mpm.weight = weight;
					fNextSibling->mpm.weight = fNextSibling->ct_mpm.weight = fMaxWeight - weight;
					parent->layout(Parent()->Frame());
				}
			}
		}
	}
	BCursor cursor(fOrient == B_VERTICAL ? kVCursorData : kHCursorData);
	SetViewCursor(&cursor, true);
}
#endif

// layoutprefs
minimax
BetterMSplitter::layoutprefs()
{
//	mpm = MSplitter::layoutprefs();

	if (fOrient == B_VERTICAL) {
		mpm.mini.x = mpm.maxi.x = 6.0;
		mpm.mini.y = 6.0;
		mpm.maxi.y = 10000.0;
	} else {
		mpm.mini.x = 6.0;
		mpm.maxi.x = 10000.0;
		mpm.mini.y = mpm.maxi.y = 6.0;
	}
	mpm.weight = ct_mpm.weight;
	
	return mpm;
}

// layout
BRect
BetterMSplitter::layout(BRect frame)
{
	MoveTo(frame.LeftTop());
	ResizeTo(frame.Width(), frame.Height());
	return Frame();
}

