// CanvasView.cpp

#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Bitmap.h>
#include <Cursor.h>
#include <Clipboard.h>
#include <MenuItem.h>
#include <Message.h>
#include <MessageFilter.h>
#include <MessageRunner.h>
#include <PopUpMenu.h>
#include <Region.h>
#include <Screen.h>
#include <ScrollBar.h>
#include <TextControl.h>
#include <Window.h>

#include "bitmap_support.h"
#include "blending.h"
#include "cms_support.h"
#include "cursors.h"
#include "defines.h"
#include "lab_convert.h"
#include "support.h"

#include "BitmapStroke.h"
#include "Brush.h"
#include "BrushView.h"
#include "Canvas.h"
#include "GlobalSettings.h"
#include "Guide.h"
#include "History.h"
#include "Layer.h"
#include "HistoryListView.h"
#include "HistoryManager.h"
#include "InfoView.h"
#include "LanguageManager.h"
#include "LayersListView.h"
#include "NavigatorView.h"
#include "Scrollable.h"
#include "Strings.h"
#include "Stroke.h"
#include "TextStroke.h"

// actions
#include "AddModifierAction.h"
#include "AddModifiersAction.h"
#include "ChangeColorAction.h"
#include "DeleteModifiersAction.h"
#include "MoveModifiersAction.h"

// states
#include "ColorpickState.h"
#include "CropState.h"
#include "EditGradientState.h"
#include "EllipseState.h"
#include "FillState.h"
#include "GuidesState.h"
#include "PickObjectsState.h"
#include "RoundRectState.h"
#include "ShapeState.h"
#include "SimpleSelectState.h"
#include "StrokeState.h"
#include "TextState.h"
#include "TransformState.h"

#include "CanvasView.h"

enum {
	MSG_ENABLE_SCROLLING		= 'ensc',
	MSG_ALLOW_UPDATES			= 'alup',
	MSG_AUTO_SCROLL				= 'scrl',
};

#define AUTO_SCROLL_DELAY		40000 // 40 ms

static const float kExtraCanvasSpacing = 50;

// CanvasFilter class

class CanvasFilter : public BMessageFilter {
 public:
	CanvasFilter(CanvasView* target)
		: BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE),
		  fTarget(target)
		{
		}
	virtual	~CanvasFilter()
		{
		}
	virtual	filter_result	Filter(BMessage* message, BHandler** target)
		{
			filter_result result = B_DISPATCH_MESSAGE;
			switch (message->what) {
				case B_KEY_DOWN: {
					uint32 key;
					uint32 modifiers;
					if (message->FindInt32("raw_char", (int32*)&key) >= B_OK
						&& message->FindInt32("modifiers", (int32*)&modifiers) >= B_OK)
						if (fTarget->HandleKeyDown(key, modifiers))
							result = B_SKIP_MESSAGE;
					break;
				}
				case B_KEY_UP: {
					uint32 key;
					uint32 modifiers;
					if (message->FindInt32("raw_char", (int32*)&key) >= B_OK
						&& message->FindInt32("modifiers", (int32*)&modifiers) >= B_OK)
						if (fTarget->HandleKeyUp(key, modifiers))
							result = B_SKIP_MESSAGE;
					break;
				}
				case B_UNMAPPED_KEY_DOWN:
				case B_UNMAPPED_KEY_UP:
					fTarget->ModifiersChanged();
					break;

				case B_MOUSE_WHEEL_CHANGED: {
					float x;
					float y;
					if (message->FindFloat("be:wheel_delta_x", &x) >= B_OK
						&& message->FindFloat("be:wheel_delta_y", &y) >= B_OK) {
						if (fTarget->MouseWheelChanged(x, y))
							result = B_SKIP_MESSAGE;
					}
					break;
				}
/*				case B_MOUSE_MOVED:
					if (fTarget->CursorShowing()) {
						BPoint where;
						if (message->FindPoint("where", &where) >= B_OK) {
							BView* parent = fTarget;
							while (parent) {
								parent->ConvertFromParent(&where);
								parent = parent->Parent();
							}
							if (!fTarget->Bounds().Contains(where)) {
								fTarget->MouseMoved(where, B_EXITED_VIEW, NULL);
								printf("canvas shows cursor, but it shouldn't\n");
							}
						}
					}
					break;*/
				default:
					break;
			}
			return result;
		}
 private:
 	CanvasView*		fTarget;
};

class CanvasPopupMenu : public BPopUpMenu {
 public:
			CanvasPopupMenu(CanvasView* view)
				: BPopUpMenu("canvas popup", false, false),
				  fCanvasView(view)
			{
			}
	virtual	~CanvasPopupMenu()
			{
				if (fCanvasView->LockLooper()) {
					BPoint where;
					uint32 buttons;
					fCanvasView->GetMouse(&where, &buttons, false);
					fCanvasView->MouseMoved(where, B_ENTERED_VIEW, NULL);
					fCanvasView->_UpdateToolCursor();
					fCanvasView->UnlockLooper();
				}
			}
 private:
	CanvasView*	fCanvasView;
};

// tool_uses_color
bool
tool_uses_color(uint32 mode)
{
	bool color = false;
	switch (mode) {
		case TOOL_PEN:
		case TOOL_BRUSH:
		case TOOL_BUCKET_FILL:
		case TOOL_TEXT:
		case TOOL_SHAPE:
		case TOOL_ELLIPSE:
		case TOOL_ROUND_RECT:
			color = true;
			break;
	}
	return color;
}

// constructor
CanvasView::CanvasView(BRect frame, const char* name, BrushView* brushView, InfoView* infoView,
					   HistoryListView* historyListView, LayersListView* layersListView,
					   NavigatorView* navigatorView)
	: ScrollView(NULL, SCROLL_HORIZONTAL | SCROLL_VERTICAL,
				 frame, name, B_FOLLOW_NONE, B_WILL_DRAW | B_NAVIGABLE_JUMP
				 | B_FULL_UPDATE_ON_RESIZE | B_FRAME_EVENTS | B_PULSE_NEEDED),
	  fCanvas(NULL),
	  fLayer(NULL),
	  fHistory(NULL),

	  fCanvasFrame(0.0, 0.0, -1.0, -1.0),
	  fCanvasBounds(0.0, 0.0, -1.0, -1.0),

	  fCursorPosition(-1.0, -1.0),
	  fCursorShowing(false),
	  fMouseDown(false),
	  fTabletMode(false),
	  fBusy(false),
	  fEventDropped(false),
	  fShowGrid(false),
	  fDemoMode(false),

	  fSlideAction(NULL),

	  fSpaceHeldDown(false),

	  fEraser(false),
	  fColor((rgb_color){ 0, 0, 0, 255 }),
	  fColorSpace(COLOR_SPACE_NONLINEAR_RGB),
	  fMode(TOOL_BRUSH),
	  fLastColorMode(TOOL_BRUSH),

	  // brush parameters
	  fBrushRadius((range){ 0.0, 5.0 }),
	  fBrushHardness((range){ 1.0, 1.0 }),
	  fBrushSpacing((range){ 0.10, 0.10 }),
	  fBrushAlpha((range){ 0.0, 1.0 }),
	  fBrushFlags(0),

	  fZoomLevel(100.0),

	  // bitmaps
	  fDisplayBitmap(NULL),
	  fStrokeBitmap(NULL),
	  fBottomBitmap(NULL),
	  fTopBitmap(NULL),
	  fCurrentBitmap(0),

	  // scrolling
	  fScrollable(new Scrollable()),
	  fCurrentScrollOffset(0.0, 0.0),
	  fScrollTrackingStart(0.0, 0.0),
	  fScrollTracking(false),
	  fScrollingEnabled(true),
	  fNeedsScrolling(false),

	  fAutoScroller(NULL),

	  fDirtyBounds(0.0, 0.0, -1.0, -1.0),

	  fPickMask(PICK_MASK_ALL),

	  // states
	  fCurrentState(NULL),
	  fStrokeState(new StrokeState(this)),
	  fColorpickState(new ColorpickState(this)),
	  fTransformState(new TransformState(this)),
	  fCropState(new CropState(this)),
	  fFillState(new FillState(this)),
	  fTextState(new TextState(this)),
	  fShapeState(new ShapeState(this)),
	  fSelectState(new SimpleSelectState(this)),
	  fEllipseState(new EllipseState(this)),
	  fRoundRectState(new RoundRectState(this)),
	  fGradientState(new EditGradientState(this)),
	  fGuidesState(new GuidesState(this)),
	  fPickObjectsState(new PickObjectsState(this)),

	  fGammaTable(new uint8[256]),

	  // other views that we control
	  fBrushView(brushView),
	  fInfoView(infoView),
	  fHistoryListView(historyListView),
	  fLayersListView(layersListView),
	  fNavigatorView(navigatorView)
{
	fBitmap[0] = NULL;
	fBitmap[1] = NULL;

	SetViewColor(B_TRANSPARENT_32_BIT);

#ifdef TARGET_PLATFORM_ZETA
	SetDoubleBuffering(B_UPDATE_INVALIDATED);
#endif
	// support Haiku with subpixel precise drawing
	system_info info;
	if (get_system_info(&info) == B_OK) {
printf("running on '%s'\n", info.kernel_name);
		BString kernelName(info.kernel_name);
		if (kernelName.IFindFirst("x86") >= 0)
			SetFlags(Flags() | B_SUBPIXEL_PRECISE);
	}

	// scrolling
	SetScrollTarget(fScrollable);
	_SetDataRect(frame);
	// layout scroll bars and scroll corner
	float innerWidth = frame.Width() - B_V_SCROLL_BAR_WIDTH;
	float innerHeight = frame.Height() - B_H_SCROLL_BAR_HEIGHT;
	BRect rect(-1.0, innerHeight + 1.0, innerWidth + 1.0, frame.bottom + 1.0);
	HScrollBar()->MoveTo(rect.LeftTop());
	HScrollBar()->ResizeTo(rect.Width(), rect.Height());

	rect.Set(innerWidth + 1.0, -1.0, frame.right + 1.0, innerHeight + 1.0);
	VScrollBar()->MoveTo(rect.LeftTop());
	VScrollBar()->ResizeTo(rect.Width(), rect.Height());

	HVScrollCorner()->MoveTo(innerWidth + 2.0, innerHeight + 2.0);
	HVScrollCorner()->SetResizingMode(B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);

	// init gamma table
	for (int32 i = 0; i < 256; i++)
		fGammaTable[i] = (uint8)(pow((double)i / 255.0, 0.4) * 255.0);
	// attach to history
	fHistoryListView->SetCanvasView(this);
	// attach to navigator
	fNavigatorView->SetTarget(this);

	// tweak view settings
	SetLowColor(112, 112, 112, 255);
	SetHighColor(104, 104, 104, 255);
}

// destructor
CanvasView::~CanvasView()
{
	// prevent usage of the list view in methods below
	fHistoryListView = NULL;
	fLayer = NULL;
	fHistory = NULL;
	fCanvas = NULL;
	fInfoView = NULL;
	fNavigatorView = NULL;
	_UnInit();
	// cleanup scrolling
	delete fAutoScroller;
	SetScrollTarget(NULL);
	// delete states
	delete fStrokeState;
	delete fColorpickState;
	delete fTransformState;
	delete fCropState;
	delete fFillState;
	delete fTextState;
	delete fShapeState;
	delete fSelectState;
	delete fEllipseState;
	delete fRoundRectState;
	delete fGradientState;
	delete fGuidesState;
	delete fPickObjectsState;

	delete fScrollable;
}

// layoutprefs
minimax
CanvasView::layoutprefs()
{
	mpm.mini.x = 10.0;
	mpm.mini.y = 10.0;
	mpm.maxi.x = 10000.0;
	mpm.maxi.y = 10000.0;
	mpm.weight = 1.0;
	return mpm;
}

// layout
BRect
CanvasView::layout(BRect frame)
{
	MoveTo(frame.LeftTop());
	ResizeTo(frame.Width(), frame.Height());
	frame = Frame();
	FrameResized(frame.Width(), frame.Height());
	return frame;
}

// AttachedToWindow
void
CanvasView::AttachedToWindow()
{
//	MakeFocus(true);
	// scrolling
	ScrollView::AttachedToWindow();
	_SetDataRect(BRect(0.0, 0.0,
					   fCanvasFrame.Width(), fCanvasFrame.Height()));
	// add message filter to window
	Window()->AddCommonFilter(new CanvasFilter(this));
	if (fInfoView)
		fInfoView->SetCanvasView(this);
}

// WindowActivated
void
CanvasView::WindowActivated(bool active)
{
//	if (active && Bounds().Contains(fCursorPosition))
//		_DrawCursor(fCursorPosition);
}

// FrameResized
void
CanvasView::FrameResized(float width, float height)
{
	width -= B_V_SCROLL_BAR_WIDTH;
	height -= B_H_SCROLL_BAR_HEIGHT;
	BPoint offset(0.0, 0.0);
	BPoint scrollOffset = fCurrentScrollOffset;
	if (width > fCanvasFrame.Width()) {
		// we need to center the canvas horizontally
		offset.x = floorf((width - fCanvasFrame.Width()) / 2.0 + 0.5);
		fCanvasFrame.OffsetTo(offset.x, fCanvasFrame.top);
	} else {
		if (fCanvasFrame.left > 0.0) {
			fCanvasFrame.OffsetTo(0.0, fCanvasFrame.top);
			scrollOffset.x = 0.0;
		}
	}
	if (height > fCanvasFrame.Height()) {
		// we need to center the canvas vertically
		offset.y = floorf((height - fCanvasFrame.Height()) / 2.0 + 0.5);
		fCanvasFrame.OffsetTo(fCanvasFrame.left, offset.y);
	} else {
		if (fCanvasFrame.top > 0.0) {
			fCanvasFrame.OffsetTo(fCanvasFrame.left, 0.0);
			scrollOffset.y = 0.0;
		}
	}
	if (scrollOffset != fCurrentScrollOffset) {
		fScrollingEnabled = false;
		fScrollable->SetScrollOffset(scrollOffset);
		fScrollingEnabled = true;
	}
	// update scroll bars
	fScrollable->SetVisibleSize(width, height);
	_UpdateCanvasZoomInfo();
	_UpdateNavigatorView();
}

// MouseDown
void
CanvasView::MouseDown(BPoint where)
{
	if (!fLayer)
		return;
	BMessage* message = Window()->CurrentMessage();
	if (_InnerBounds().Contains(where) && message) {
		uint32 buttons;
		if (message->FindInt32("buttons", (int32*)&buttons) != B_OK)
			buttons = 0;

		// popup menu
		if (buttons & B_SECONDARY_MOUSE_BUTTON) {
			_ShowPopupMenu(where);
			return;
		}
		if (fSpaceHeldDown || buttons & B_TERTIARY_MOUSE_BUTTON) {
			// switch into scrolling mode and update cursor
			fScrollTracking = true;
			fScrollTrackingStart = where;
			_UpdateToolCursor();
		} else if (fCurrentState) {
			// prepare Point on canvas
			Point canvasWhere;
			fTabletMode = _GetTabletInfo(where, canvasWhere, fEraser, message);
			// let the state do the rest
			fCurrentState->MouseDown(where, canvasWhere, fEraser);
		}
		SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
		fMouseDown = true;
	}
}

// MouseUp
void
CanvasView::MouseUp(BPoint where)
{
	BMessage* message = Window()->CurrentMessage();
	if (fScrollTracking) {
		// stop scroll tracking and update cursor
		fScrollTracking = false;
		_UpdateToolCursor();
	}
	if (fLayer && fCurrentState && !fSpaceHeldDown) {
		// prepare Point on canvas
		Point canvasWhere;
		_GetTabletInfo(where, canvasWhere, fEraser, message);
		// let the state do the rest
		fCurrentState->MouseUp(where, canvasWhere);
	}

	fMouseDown = false;
	fTabletMode = false;

	SetAutoScrolling(false);
}

// MouseMoved
void
CanvasView::MouseMoved(BPoint where, uint32 transit, const BMessage* dragMessage)
{
	// keep track of cursor
	fCursorPosition = where;
	fCursorShowing = transit == B_ENTERED_VIEW || transit == B_INSIDE_VIEW;

	if (!dragMessage) {

		if (!fLayer) {
			_UpdateToolCursor();
			return;
		}

		if (fScrollTracking) {
			// scroll by the difference in where and fTrackingStart
			// remember current where in fTrackingStart
			where.x = roundf(where.x);
			where.y = roundf(where.y);
			fScrollable->SetScrollOffset(fScrollable->ScrollOffset()
										 - (where - fScrollTrackingStart));
			fScrollTrackingStart = where;
		} else if (fCurrentState && !fSpaceHeldDown) {
			BMessage* message = Window()->CurrentMessage();
			if (message) {
				uint32 buttons;
				if (message->FindInt32("buttons", (int32*)&buttons) != B_OK)
					buttons = 0;
				if (!buttons && fMouseDown) {
					MouseUp(where);
					return;
				}
			}

// work arround a problem on Dan0 with "fake" mouse messages
// (appearantly generated, not comming from the input device)
if (message && message->HasInt32("be:cursor_needed")) {
	return;
}
			Point canvasWhere;
//			_GetTabletInfo(where, canvasWhere, fEraser, message);
			bool tablet = _GetTabletInfo(where, canvasWhere, fEraser, message);
			// work arround an app_server bug in Zeta which results in
			// messages from a tablet not containing all data, this check
			// prevents processing mixed (tablet/mouse) messages in one
			// continous stroke
//			if (fMouseDown && fTabletMode != tablet) {
			if (fMouseDown && fTabletMode && !tablet) {
//fprintf(stderr, "B_MOUSE_MOVED does not match B_MOUSE_DOWN fields (expected tablet: %d)!\n", fTabletMode);
//if (message)
//	message->PrintToStream();
				return;
			}
			// let the state do the rest
			fCurrentState->MouseMoved(where, canvasWhere, transit, dragMessage);
#if TARGET_PLATFORM_ZETA
			if (fCursorShowing)
				_UpdateToolCursor();
#endif
		}
	} else
		SetViewCursor(B_CURSOR_SYSTEM_DEFAULT, true);
}

// Draw
void
CanvasView::Draw(BRect updateRect)
{
//bigtime_t bg = system_time();
	BBitmap* source = fColorSpace != COLOR_SPACE_NONLINEAR_RGB ?
					  fDisplayBitmap : DisplayBitmap();
//	BBitmap* source = fStrokeBitmap;

	// allow states to tinker with the clipping region, so that pixels that are
	// later redrawn by the state get excluded from our own draw method
	BRegion originalRegion;
	GetClippingRegion(&originalRegion);
	BRegion modifiedRegion = originalRegion;
	if (fCurrentState) {
		fCurrentState->ModifyClippingRegion(this, &modifiedRegion);
	}

	// modify clipping region to exclude the grid lines
/*	if (fShowGrid && fZoomLevel >= 800.0) {
		BRect gridRect = updateRect & fCanvasFrame;
		BPoint lt = gridRect.LeftTop();
		BPoint rb = gridRect.RightBottom();
		ConvertToCanvas(lt);
		ConvertToCanvas(rb);
		lt.x = floorf(lt.x);
		lt.y = floorf(lt.y);
		rb.x = ceilf(rb.x);
		rb.y = ceilf(rb.y);

		int32 width = (int32)(rb.x - lt.x) + 1;
		int32 height = (int32)(rb.y - lt.y) + 1;
		for (int32 y = 0; y < height; y++) {
			BPoint a(lt.x, lt.y + y);
			BPoint b(rb.x, lt.y + y);
			ConvertFromCanvas(a);
			ConvertFromCanvas(b);
			modifiedRegion.Exclude(BRect(a, b));
		}
		for (int32 x = 0; x < width; x++) {
			BPoint a(lt.x + x, lt.y);
			BPoint b(lt.x + x, rb.y);
			ConvertFromCanvas(a);
			ConvertFromCanvas(b);
			modifiedRegion.Exclude(BRect(a, b));
		}
		ConstrainClippingRegion(&modifiedRegion);
	}*/

	if (source) {
		// see if we have any dirty pixels yet
		if (fDirtyBounds.IsValid()) {
			if (fCurrentState)
				fCurrentState->FinishLayer(fDirtyBounds);
			// compose layers
			ComposeLayers(fDirtyBounds);
			if (fColorSpace != COLOR_SPACE_NONLINEAR_RGB)
				_ConvertToDisplay(DisplayBitmap(), fDirtyBounds);
			// invalidate dirty area
			fDirtyBounds.Set(0.0, 0.0, -1.0, -1.0);
			// update navigator view at this time, because now we have a valid canvas
			if (GlobalSettings::CreateDefault()->LiveUpdates())
				_UpdateNavigatorView();
		}
		if (fCanvasFrame.Contains(updateRect)) {
			DrawBitmap(source, source->Bounds(), fCanvasFrame);
		} else if (updateRect.Intersects(fCanvasFrame)) {
			DrawBitmap(source, source->Bounds(), fCanvasFrame);
			// avoid drawing stuff twice
			BRect r(updateRect);
			r.right = fCanvasFrame.left - 1.0;
			if (r.IsValid())
				FillRect(r, kStripes);
			r.right = updateRect.right;
			r.left = fCanvasFrame.right + 1.0;
			if (r.IsValid())
				FillRect(r, kStripes);
			r.top = updateRect.top;
			r.bottom = fCanvasFrame.top - 1.0;
			r.left = updateRect.left > fCanvasFrame.left ? updateRect.left : fCanvasFrame.left;
			r.right = updateRect.right < fCanvasFrame.right ? updateRect.right : fCanvasFrame.right;
			if (r.IsValid())
				FillRect(r, kStripes);
			r.top = fCanvasFrame.bottom + 1.0;
			r.bottom = updateRect.bottom;
			if (r.IsValid())
				FillRect(r, kStripes);
		} else
			FillRect(updateRect, kStripes);
	} else
		FillRect(updateRect, kStripes);

	if (fCanvas) {
		// grid
		if (fShowGrid && fZoomLevel >= 800.0) {
			BRect gridRect = updateRect & fCanvasFrame;
			BPoint lt = gridRect.LeftTop();
			BPoint rb = gridRect.RightBottom();
			ConvertToCanvas(lt);
			ConvertToCanvas(rb);
			lt.x = floorf(lt.x);
			lt.y = floorf(lt.y);
			rb.x = ceilf(rb.x);
			rb.y = ceilf(rb.y);

			int32 width = (int32)(rb.x - lt.x) + 1;
			int32 height = (int32)(rb.y - lt.y) + 1;
			SetHighColor(80, 80, 80, 255);
			for (int32 y = 0; y < height; y++) {
				BPoint a(lt.x, lt.y + y);
				BPoint b(rb.x/* + 1.0*/, lt.y + y);
				ConvertFromCanvas(a);
				ConvertFromCanvas(b);
				StrokeLine(a, b);
			}
			for (int32 x = 0; x < width; x++) {
				BPoint a(lt.x + x, lt.y);
				BPoint b(lt.x + x, rb.y/* + 1.0*/);
				ConvertFromCanvas(a);
				ConvertFromCanvas(b);
				StrokeLine(a, b);
			}
		}
	}
	// restore original clipping region
	ConstrainClippingRegion(&originalRegion);

	if (fCanvas) {
		// draw guides
		if (fCanvas->ShowGuides()) {
			rgb_color color = (rgb_color){ 120, 120, 255, 255 };
			BRect bounds(Bounds());
			for (int32 i = 0; Guide* guide = fCanvas->GuideAt(i); i++)
				guide->Draw(this, color, fCanvasFrame, bounds);
		}

		if (fCurrentState)
			fCurrentState->Draw(this, updateRect);
	}
}

// MessageReceived
void
CanvasView::MessageReceived(BMessage* message)
{
	// let the current state have a look at the message first
	if (fCurrentState && fCurrentState->MessageReceived(message))
		return;

	switch (message->what) {
		case MSG_PICK_MASK:
			uint32 mask;
			if (message->FindInt32("mask", (int32*)&mask) >= B_OK) {
				if (fPickMask & mask) {
					SetPickMask(fPickMask & ~mask);
				} else {
					SetPickMask(fPickMask | mask);
				}
			}
			break;
		case MSG_PICK_MASK_ALL:
			SetPickMask(PICK_MASK_ALL);
			break;
		case MSG_PICK_MASK_NONE:
			SetPickMask(0);
			break;
		case B_SELECT_ALL:
		case MSG_PICK_ALL:
			if (fHistoryListView)
				fHistoryListView->SelectAll();
			break;
		case MSG_PICK_NONE:
			if (fHistoryListView)
				fHistoryListView->DeselectAll();
			break;
		case MSG_AUTO_SCROLL:
			if (fAutoScroller) {
				BPoint scrollOffset(0.0, 0.0);
				BRect bounds(Bounds());
				BPoint mousePos = fCursorPosition;
				mousePos.ConstrainTo(bounds);
				bounds.InsetBy(40.0, 40.0);
				if (!bounds.Contains(mousePos)) {
					// mouse is close to the border
					if (mousePos.x <= bounds.left)
						scrollOffset.x = mousePos.x - bounds.left;
					else if (mousePos.x >= bounds.right)
						scrollOffset.x = mousePos.x - bounds.right;
					if (mousePos.y <= bounds.top)
						scrollOffset.y = mousePos.y - bounds.top;
					else if (mousePos.y >= bounds.bottom)
						scrollOffset.y = mousePos.y - bounds.bottom;

					scrollOffset.x = roundf(scrollOffset.x * 0.8);
					scrollOffset.y = roundf(scrollOffset.y * 0.8);
				}
				if (scrollOffset != B_ORIGIN) {
					BPoint currentScrollOffset = fScrollable->ScrollOffset();
					fScrollable->SetScrollOffset(currentScrollOffset
												 + scrollOffset);
					if (currentScrollOffset != fScrollable->ScrollOffset())
						MouseMoved(fCursorPosition, B_INSIDE_VIEW, NULL);
				}
			}
			break;
		case B_PASTE:
printf("CanvasView::MessageReceived(B_PASTE)\n");
			if (!HandlePaste(message)) {
printf(" - not handled\n");
				BView::MessageReceived(message);
			}
			break;
		case MSG_SET_COLOR: {
			rgb_color color;
			if (restore_color_from_message(message, color) == B_OK) {
				SetColor(color);
				if (!tool_uses_color(fMode) &&
					// check for these tools specifically
					fMode != TOOL_COLOR_PICKER && fMode != TOOL_EDIT_GRADIENT) {

					SetTool(fLastColorMode);
				}
			}
			break;
		}
		case MSG_SET_ALPHA: {
			float min;
			float max;
			if (message->FindFloat("min value", &min) == B_OK
				&& message->FindFloat("max value", &max) == B_OK)
				SetMaxAlpha(min, max);
			break;
		}
		case MSG_SET_RADIUS: {
			float min;
			float max;
			if (message->FindFloat("min value", &min) == B_OK
				&& message->FindFloat("max value", &max) == B_OK)
				SetBrushRadius(min * 100.0, max * 100.0);
			break;
		}
		case MSG_SET_HARDNESS: {
			float min;
			float max;
			if (message->FindFloat("min value", &min) == B_OK
				&& message->FindFloat("max value", &max) == B_OK)
				SetBrushHardness(min, max);
			break;
		}
		case MSG_SET_SPACING: {
			float min;
			float max;
			if (message->FindFloat("min value", &min) == B_OK
				&& message->FindFloat("max value", &max) == B_OK)
				SetBrushSpacing(min, max);
			break;
		}
		case MSG_SUB_PIXEL_PRECISION: {
			int32 value;
			uint32 toolID;
			if (message->FindInt32("be:value", &value) >= B_OK
				&& message->FindInt32("tool", (int32*)&toolID) >= B_OK)
				SetPrecise(value == B_CONTROL_ON, toolID);
			break;
		}
		case MSG_SHOW_GRID:
			// toggle grid showing
			SetShowGrid(!fShowGrid);
			Invalidate();
			break;
		case MSG_SET_COLOR_SPACE: {
			uint32 value;
			if (message->FindInt32("color space", (int32*)&value) == B_OK) {
				if (fColorSpace != value) {
					fColorSpace = value;
					if (fCanvas)
						fCanvas->SetColorSpace(fColorSpace);
					_MakeBottomBitmap();
					RebuildCanvas(fCanvasBounds);
				}
			}
			break;
		}
		case MSG_CONTROL_ALPHA: {
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK) {
				uint32 flags = value == 0 ? fBrushFlags & ~FLAG_PRESSURE_CONTROLS_APHLA
										  : fBrushFlags | FLAG_PRESSURE_CONTROLS_APHLA;
				SetBrushFlags(flags);
			}
			break;
		}
		case MSG_CONTROL_RADIUS: {
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK) {
				uint32 flags = value == 0 ? fBrushFlags & ~FLAG_PRESSURE_CONTROLS_RADIUS
										  : fBrushFlags | FLAG_PRESSURE_CONTROLS_RADIUS;
				SetBrushFlags(flags);
			}
			break;
		}
		case MSG_CONTROL_HARDNESS: {
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK) {
				uint32 flags = value == 0 ? fBrushFlags & ~FLAG_PRESSURE_CONTROLS_HARDNESS
										  : fBrushFlags | FLAG_PRESSURE_CONTROLS_HARDNESS;
				SetBrushFlags(flags);
			}
			break;
		}
		case MSG_SET_SOLID: {
			int32 value;
			if (message->FindInt32("be:value", &value) >= B_OK) {
				uint32 flags = value == 0 ? fBrushFlags & ~FLAG_SOLID
										  : fBrushFlags | FLAG_SOLID;
				SetBrushFlags(flags);
			}
			break;
		}
		case MSG_SET_TILT: {
			int32 value;
			if (message->FindInt32("be:value", &value) >= B_OK) {
				uint32 flags = value == 0 ? fBrushFlags & ~FLAG_TILT_CONTROLS_SHAPE
										  : fBrushFlags | FLAG_TILT_CONTROLS_SHAPE;
				SetBrushFlags(flags);
			}
			break;
		}
		case MSG_SET_TOOL: {
			int32 value;
			if (message->FindInt32("tool", &value) == B_OK
				&& value >= 0/* && value < MODE_LAST*/) {
				SetTool(value);
				Window()->PostMessage(message);
			}
			break;
		}
		case MSG_SET_LAYER: {
			int32 index;
			if (message->FindInt32("index", &index) == B_OK) {
				fLayersListView->Select(index);//SetToLayer(index);
			}
			break;
		}
		case MSG_ZOOM_IN:
			_SetZoom(_NextZoomInLevel(fZoomLevel));
			break;
		case MSG_ZOOM_OUT:
			_SetZoom(_NextZoomOutLevel(fZoomLevel));
			break;
		case MSG_SET_ZOOM: {
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK)
				_SetZoom((float)value);
			break;
		}
		case MSG_CENTER_CANVAS:
			if (fCanvas) {
				BPoint center;
				if (message->FindPoint("center", &center) >= B_OK) {
					fCanvas->SetZoomState(fZoomLevel, center);
					// force zoom update
					_SetZoom(fZoomLevel, true);
				}
			}
			break;
		case MSG_CLEAR_ALL:
			_MakeEmpty();
			break;
		case MSG_ENABLE_SCROLLING:
			fScrollingEnabled = true;
			if (fNeedsScrolling)
				ScrollOffsetChanged(fCurrentScrollOffset, fScrollable->ScrollOffset());
			break;
		case MSG_ALLOW_UPDATES:
			_SetBusy(false);
			break;
		default:
			BView::MessageReceived(message);
			break;
	}
}

// KeyDown
void
CanvasView::KeyDown(const char* bytes, int32 numBytes)
{
	bool handled = false;
	if (BWindow* window = Window()) {
		if (BMessage* message = window->CurrentMessage()) {
			uint32 key;
			uint32 modifiers;
			if (message->FindInt32("raw_char", (int32*)&key) >= B_OK
				&& message->FindInt32("modifiers", (int32*)&modifiers) >= B_OK)
				handled = HandleKeyDown(key, modifiers);
		}
	}
	if (!handled)
		BView::KeyDown(bytes, numBytes);
}

// KeyUp
void
CanvasView::KeyUp(const char* bytes, int32 numBytes)
{
	bool handled = false;
	if (BWindow* window = Window()) {
		if (BMessage* message = window->CurrentMessage()) {
			uint32 key;
			uint32 modifiers;
			if (message->FindInt32("raw_char", (int32*)&key) >= B_OK
				&& message->FindInt32("modifiers", (int32*)&modifiers) >= B_OK)
				handled = HandleKeyUp(key, modifiers);
		}
	}
	if (!handled)
		BView::KeyUp(bytes, numBytes);
}

// Pulse
void
CanvasView::Pulse()
{
	// periodically "unbusy" ourselves, this avoids the problem
	// that we have been so busy, that we missed our message to
	// become unbusy.
	if (fBusy) {
		_SetBusy(false);
	}
}

#ifdef TARGET_PLATFORM_HAIKU

// MinSize
BSize
CanvasView::MinSize()
{
	return ScrollView::MinSize();
}

// PreferredSize
BSize
CanvasView::PreferredSize()
{
	return ScrollView::PreferredSize();
}

// MaxSize
BSize
CanvasView::MaxSize()
{
	return ScrollView::MaxSize();
}

#endif // TARGET_PLATFORM_HAIKU

// #pragma mark -

// ScrollOffsetChanged
void
CanvasView::ScrollOffsetChanged(BPoint oldOffset, BPoint newOffset)
{
	// do the visual scrolling
	if (fScrollingEnabled) {
		ScrollView::ScrollOffsetChanged(oldOffset, newOffset);

		BRect oldCanvasFrame(fCanvasFrame);

		// offset canvas frame
		fCurrentScrollOffset = newOffset;
		fCanvasFrame.OffsetBy(oldOffset - newOffset);

		CopyBits(oldCanvasFrame, fCanvasFrame);

		BRegion areaAroundCanvas(Bounds());
		areaAroundCanvas.Exclude(fCanvasFrame);
		if (areaAroundCanvas.Frame().IsValid())
			Invalidate(&areaAroundCanvas);

		fScrollingEnabled = false;
		Window()->PostMessage(MSG_ENABLE_SCROLLING, this);

		_UpdateCanvasZoomInfo();
		if (fCursorShowing)
			_DrawCursor(fCursorPosition + (oldOffset - newOffset));
		_UpdateNavigatorView();

		fNeedsScrolling = false;
	} else
		fNeedsScrolling = true;
}

// SetTo
void
CanvasView::SetTo(Canvas* canvas)
{
//printf("CanvasView::SetTo(%p)\n", canvas);
	if (fCanvas != canvas) {
		// state might have some cleanup to do
		CleanUpState(fCurrentState);
		_UnInit();
		fCanvas = canvas;
		if (fCanvas) {
			fCanvasFrame = fCanvas->Bounds();
			fCanvasBounds = fCanvasFrame;
			fBottomBitmap = new BBitmap(fCanvasBounds, B_RGBA32);
			fColorSpace = canvas->ColorSpace();
			_MakeBottomBitmap();
			// handle demo mode
			if (fDemoMode) {
				fTopBitmap = new BBitmap(fCanvasBounds, B_RGBA32);
				_MakeTopBitmap();
			}
			_Init();
			ComposeLayers(fCanvasBounds);
			if (fColorSpace != COLOR_SPACE_NONLINEAR_RGB)
				_ConvertToDisplay(DisplayBitmap(), fCanvasBounds);
		} else {
			fHistoryListView->SetLayer(NULL);
			fZoomLevel = 100.0;
			Invalidate();
			_UpdateNavigatorView();
		}
		_UpdateToolCursor();
	}
}

// SetToLayer
void
CanvasView::SetToLayer(int32 index)
{
//printf("CanvasView::SetToLayer(%ld)\n", index);
	if (fCanvas) {
		Layer* layer = fCanvas->LayerAt(index);
		if (layer != fLayer) {
			// prepare state
			CleanUpState(fCurrentState);
			fLayer = layer;
			if (fLayer && fLayer->InitCheck() >= B_OK && fLayer->Bitmap()) {
				// set history to layers history
				fHistory = fLayer->GetHistory();
				fHistoryListView->SetLayer(fLayer);
				// copy layers bitmap into our first buffer
				memcpy(BackBitmap()->Bits(), fLayer->Bitmap()->Bits(), BackBitmap()->BitsLength());
				fCanvas->SetCurrentLayer(index);
				// prepare state
				if (!fCurrentState)
					SwitchState(fStrokeState);
				else
					InitState(fCurrentState);
			} else {
				fLayer = NULL;
				fHistory = NULL;
				fHistoryListView->SetLayer(NULL);
			}
		}
	}
	_UpdateToolCursor();
}

// SetDemoMode
void
CanvasView::SetDemoMode(bool demo)
{
	if (fDemoMode != demo) {
		fDemoMode = demo;
		delete fTopBitmap;
		fTopBitmap = NULL;
		if (fDemoMode && fCanvas) {
			fTopBitmap = new BBitmap(fCanvasBounds, B_RGBA32);
			_MakeTopBitmap();
		}
		if (fCanvas)
			_InvalidateCanvas(fCanvas->Bounds(), true);
	}
}

// HandleDemoMode
void
CanvasView::HandleDemoMode(BBitmap* bitmap, BRect area) const
{
	if (fDemoMode && fTopBitmap) {
		// merge with top bitmap
		BRect r = area & bitmap->Bounds();
		r = r & fTopBitmap->Bounds();
		if (area.IsValid() && r.IsValid()) {

			int32 left, top, right, bottom;
			rect_to_int(r, left, top, right, bottom);

			uint8* src = (uint8*)fTopBitmap->Bits();
			uint8* dst = (uint8*)bitmap->Bits();
			uint32 srcBPR = fTopBitmap->BytesPerRow();
			uint32 dstBPR = bitmap->BytesPerRow();

			src += 4 * (left - (int32)fTopBitmap->Bounds().left)
				   + srcBPR * (top - (int32)fTopBitmap->Bounds().top);
			dst += 4 * (left - (int32)bitmap->Bounds().left)
				   + dstBPR * (top - (int32)bitmap->Bounds().top);

			for (; top <= bottom; top++) {
				uint8* srcHandle = src;
				uint8* dstHandle = dst;
				for (int32 x = left; x <= right; x++) {
					blend_colors_copy(dstHandle, dstHandle, srcHandle);
					srcHandle += 4;
					dstHandle += 4;
				}
				src += srcBPR;
				dst += dstBPR;
			}
		}
	}
}

// CurrentCanvas
Canvas*
CanvasView::CurrentCanvas() const
{
	return fCanvas;
}

// IsTracking
bool
CanvasView::IsTracking() const
{
	if (fCurrentState)
		return fCurrentState->IsTracking();
	return false;
}

// EventDropped
void
CanvasView::EventDropped()
{
//printf("CanvasView::EventDropped()\n");
	fEventDropped = true;
}

// EditModifier
void
CanvasView::EditModifier(int32 index)
{
	if (fHistory) {
		EditModifier(fHistory->ModifierAt(index));
	}
}

// EditModifier
void
CanvasView::EditModifier(Stroke* modifier)
{
	if (modifier && fHistory && fHistory->HasModifier(modifier)) {
		if (modifier->ToolID() > TOOL_UNDEFINED) {
			SetTool(modifier->ToolID());
			if (fCurrentState->EditModifier(modifier)) {
//			fHistoryListView->SetItemFocused(index);
				// TODO ...?
			}
		}
	}
}

// PickObjects
void
CanvasView::PickObjects(BPoint where, bool extend) const
{
	if (fHistory && fHistoryListView) {
		if (!fStrokeBitmap->Bounds().Contains(where)) {
			if (!extend)
				fHistoryListView->DeselectAll();
			return;
		}
		// hit test works with integer pixel only
		where.x = floorf(where.x);
		where.y = floorf(where.y);
		// iterate over objects top to bottom
		uint32 pickMask = fCurrentState == fPickObjectsState ? fPickMask : 0xffffffff;
		int32 count = fHistory->CountModifiers();
		int32 i = count - 1;
		for (; i >= 0; i--) {
			if (Stroke* stroke = fHistory->ModifierAt(i)) {
				if (stroke->ToolID() != TOOL_UNDEFINED &&
					stroke->IsPickable(pickMask) &&
					stroke->HitTest(where, fStrokeBitmap)) {
					break;
				}
			}
		}
		if (i >= 0) {
			// select picked object
			fHistoryListView->MakeFocus(true);
			fHistoryListView->Select(i, extend);
			fHistoryListView->ScrollTo(i);
		} else {
			// deselect all objects
			if (!extend)
				fHistoryListView->DeselectAll();
		}
	}
}

// PickObjects
void
CanvasView::PickObjects(const BRect& area, bool extend) const
{
	if (fHistory && fHistoryListView) {
		if (!fStrokeBitmap->Bounds().Intersects(area)) {
			if (!extend)
				fHistoryListView->DeselectAll();
			return;
		}
		// iterate over objects
		uint32 pickMask = fCurrentState == fPickObjectsState ? fPickMask : 0xffffffff;
		bool found = false;
		for (int32 i = 0; Stroke* stroke = fHistory->ModifierAt(i); i++) {
			if (stroke->ToolID() != TOOL_UNDEFINED &&
				stroke->IsPickable(pickMask) &&
				stroke->HitTest(area, fStrokeBitmap)) {
				if (!found) {
					if (!extend)
						fHistoryListView->DeselectAll();
					// scroll to first picked object
					found = true;
					fHistoryListView->ScrollTo(i);
				}
				// select picked object
				fHistoryListView->Select(i, true);
			}
		}
		if (!found) {
			// deselect all objects
			if (!extend)
				fHistoryListView->DeselectAll();
		} else {
			fHistoryListView->MakeFocus(true);
		}
	}
}

// HandlePaste
bool
CanvasView::HandlePaste(BMessage* message)
{
	bool handled = false;
	if (fHistory && be_clipboard->Lock()) {
		if (BMessage* data = be_clipboard->Data()) {
			BMessage bitmapArchive;
			if (data->FindMessage("image/bitmap", &bitmapArchive) >= B_OK) {
				BBitmap* bitmap = new BBitmap(&bitmapArchive);
				if (bitmap && bitmap->IsValid()
					&& (bitmap->ColorSpace() == B_RGB32
						|| bitmap->ColorSpace() == B_RGBA32)) {
					// for some additional comfort
					SetTool(TOOL_TRANSLATE);
					BMessage toolMessage(MSG_SET_TOOL);
					toolMessage.AddInt32("id", TOOL_TRANSLATE);
					Window()->PostMessage(&toolMessage);

					BitmapStroke* stroke = new BitmapStroke(bitmap);

					BPoint location;
					if (data->FindPoint("be:location", &location) >= B_OK)
						stroke->TranslateBy(location);

					int32 index = fHistory->CountModifiers();
					Stroke** strokes = new Stroke*[1];
					strokes[0] = stroke;
					Perform(new AddModifiersAction(fLayer, strokes, 1, index));

					handled = true;
				} else {
					delete bitmap;
				}
			}
		}
		be_clipboard->Unlock();
	}
	return handled;
}

// ModifiersChanged
void
CanvasView::ModifiersChanged()
{
	if (fCurrentState)
		fCurrentState->ModifiersChanged();
}

// PrepareForObjectPropertyChange
void
CanvasView::PrepareForObjectPropertyChange()
{
	if (fCurrentState)
		fCurrentState->PrepareForObjectPropertyChange();
}

// ObjectPropertyChangeDone
void
CanvasView::ObjectPropertyChangeDone()
{
	if (fCurrentState)
		fCurrentState->ObjectPropertyChangeDone();
}

// HandleKeyDown
bool
CanvasView::HandleKeyDown(uint32 key, uint32 modifiers)
{
//printf("CanvasView::HandleKeyDown()\n");

	if (fMouseDown) {
		// eat up key events when the mouse is already pressed
		return true;
	}

	if (BView* focusView = Window()->CurrentFocus()) {
		// handle cut, copy and paste shortcuts
		// NOTE: if the menu item for "B_PASTE" is
		// disabled, the shortcut is never triggered,
		// even if it would make sense, because for example
		// a BTextView is focused
		if (modifiers & B_COMMAND_KEY) {
			if (key == 'x') {
				Window()->PostMessage(B_CUT);
				return true;
			}
			if (key == 'c') {
				Window()->PostMessage(B_COPY);
				return true;
			}
			if (key == 'v') {
				Window()->PostMessage(B_PASTE);
				return true;
			}
		}

		if (dynamic_cast<BTextView*>(focusView) != NULL) {
//printf("CanvasView::HandleKeyDown() - text view has focus\n");
			return false;
		}
	}

	if (!fLayer)
		return false;

	if (fCurrentState) {
		if (fCurrentState->HandleKeyDown(key, modifiers)) {
//printf("CanvasView::HandleKeyDown() - state handled it\n");
			return true;
		}
	}


	bool handled = true;
	// handle some keys regardless if command key pressed
	switch (key) {
		case B_RETURN:
			Window()->PostMessage(MSG_APPLY_CURRENT_TOOL);
			break;
		case B_SPACE:
			fSpaceHeldDown = true;
			_UpdateToolCursor();
			break;
		case '+': {
			if (modifiers & B_OPTION_KEY || modifiers & B_CONTROL_KEY || modifiers & B_COMMAND_KEY || modifiers & B_MENU_KEY) {
				_SetZoom(_NextZoomInLevel(fZoomLevel));
			} else {
				if (modifiers & B_SHIFT_KEY)
					SetBrushRadius(fBrushRadius.min + 1.0, fBrushRadius.max + 1.0);
				else
					SetBrushRadius(fBrushRadius.min * 1.1, fBrushRadius.max * 1.1);
				BMessage message(MSG_SET_RADIUS);
				message.AddFloat("min value", fBrushRadius.min / 100.0);
				message.AddFloat("max value", fBrushRadius.max / 100.0);
				Window()->PostMessage(&message);
			}
			break;
		}
		case '-': {
			if (modifiers & B_OPTION_KEY || modifiers & B_CONTROL_KEY || modifiers & B_COMMAND_KEY || modifiers & B_MENU_KEY) {
				_SetZoom(_NextZoomOutLevel(fZoomLevel));
			} else {
				if (modifiers & B_SHIFT_KEY)
					SetBrushRadius(max_c(0.1, fBrushRadius.min - 1.0), max_c(0.1, fBrushRadius.max - 1.0));
				else
					SetBrushRadius(fBrushRadius.min / 1.1, fBrushRadius.max / 1.1);
				BMessage message(MSG_SET_RADIUS);
				message.AddFloat("min value", fBrushRadius.min / 100.0);
				message.AddFloat("max value", fBrushRadius.max / 100.0);
				Window()->PostMessage(&message);
			}
			break;
		}
		default:
			handled = false;
			break;
	}
	// handle the rest only of command key not pressed
	if (!handled && !(modifiers & B_COMMAND_KEY)) {
		handled = true;
		switch (key) {
			case 'z':
			case 'y':
				if (modifiers & B_SHIFT_KEY)
					Redo();
				else
					Undo();
				break;
/*			case B_TAB: {
				uint32 mode = fMode + 1;
//				if (mode == MODE_RESTORE)
//					mode++;
//				if (mode >= MODE_LAST)
				if (mode >= TOOL_LAST)
					mode = 0;
				SetTool(mode);
				break;
			}*/
			case 'b':
				SetTool(TOOL_BRUSH);
				break;
			case 'c':
				SetTool(TOOL_CLONE);
				break;
			case 'p':
				SetTool(TOOL_PEN);
				break;
			case 's':
				SetTool(TOOL_SHAPE);
				break;
			case 'e':
				SetTool(TOOL_ERASER);
				break;
			case 'd':
				SetTool(TOOL_COLOR_PICKER);
				break;
			case 'r':
				SetTool(TOOL_BLUR);
				break;
			case 'x':
				SetTool(TOOL_CROP);
				break;
			case 'v':
				SetTool(TOOL_TRANSLATE);
				break;
			case 'f':
				SetTool(TOOL_BUCKET_FILL);
				break;
			case 't':
				SetTool(TOOL_TEXT);
				break;
			case 'g':
				SetTool(TOOL_EDIT_GRADIENT);
				break;
			default:
				handled = false;
				break;
		}
	}
//printf("CanvasView::HandleKeyDown() - returning: %d\n", handled);
	return handled;
}

// HandleKeyUp
bool
CanvasView::HandleKeyUp(uint32 key, uint32 modifiers)
{
	if (!fLayer)
		return false;

	if (fMouseDown) {
		// eat up key events when the mouse is already pressed
		return true;
	}

	if (fCurrentState) {
		if (fCurrentState->HandleKeyUp(key, modifiers))
			return true;
	}

	bool handled = true;
	switch (key) {
		case B_SPACE:
			fSpaceHeldDown = false;
			_UpdateToolCursor();
			break;
		default:
			handled = false;
			break;
	}
	return handled;
}

// ScrollWheelChanged
bool
CanvasView::MouseWheelChanged(float x, float y)
{
	bool handled = false;
	BView* lastMouseMovedView = NULL;
	if (BWindow* window = Window()) {
		lastMouseMovedView = window->LastMouseMovedView();
	}
	if (lastMouseMovedView == this || lastMouseMovedView == fNavigatorView) {
		if (y > 0.0) {
			_SetZoom(_NextZoomOutLevel(fZoomLevel));
			handled = true;
		} else if (y < 0.0) {
			_SetZoom(_NextZoomInLevel(fZoomLevel));
			handled = true;
		}
	}
	return handled;
}

// SetBrushRadius
void
CanvasView::SetBrushRadius(float min, float max)
{
	constrain(min, 0.1, 1000.0);
	constrain(max, 0.1, 1000.0);
	if (min != fBrushRadius.min || max != fBrushRadius.max) {
		bool redrawCursor = fCursorShowing;
		if (fCursorShowing)
			_ClearCursor();
		fBrushRadius.min = min;
		fBrushRadius.max = max;
		if (redrawCursor)
			_DrawCursor(fCursorPosition);
		// update brush preview as well
		Brush brush(fBrushRadius, fBrushHardness);
		fBrushView->Update(&brush, fBrushFlags);
	}
}

// SetBrushHardness
void
CanvasView::SetBrushHardness(float min, float max)
{
	constrain(min, 0.0, 1.0);
	constrain(max, 0.0, 1.0);
	if (min != fBrushHardness.min || max != fBrushHardness.max) {
		fBrushHardness.min = min;
		fBrushHardness.max = max;
		Brush brush(fBrushRadius, fBrushHardness);
		fBrushView->Update(&brush, fBrushFlags);
	}
}

// SetMaxAlpha
void
CanvasView::SetMaxAlpha(float min, float max)
{
	constrain(min, 0.0, 1.0);
	constrain(max, 0.0, 1.0);
	fBrushAlpha.min = min;
	fBrushAlpha.max = max;
}

// SetBrushFlags
void
CanvasView::SetBrushFlags(uint32 flags)
{
	if (fBrushFlags != flags) {
		fBrushFlags = flags;
		fBrushView->Update(NULL, fBrushFlags);
		if (BWindow* window = Window()) {
			BMessage message(MSG_SET_BRUSH_FLAGS);
			message.AddInt32("brush flags", fBrushFlags);
			window->PostMessage(&message);
		}
	}
}

// SetBrushSpacing
void
CanvasView::SetBrushSpacing(float min, float max)
{
	constrain(min, 0.0, 1.0);
	constrain(max, 0.0, 1.0);
	fBrushSpacing.min = min;
	fBrushSpacing.max = max;
}

// SetColor
void
CanvasView::SetColor(rgb_color color)
{
	fColor = color;
}

// SetTolerance
void
CanvasView::SetTolerance(uint8 tolerance)
{
	fFillState->SetTolerance(tolerance);
}

// Tolerance
uint8
CanvasView::Tolerance() const
{
	return fFillState->Tolerance();
}

// SetSoftness
void
CanvasView::SetSoftness(uint8 softness)
{
	fFillState->SetSoftness(softness);
}

// Softness
uint8
CanvasView::Softness() const
{
	return fFillState->Softness();
}

// SetFillContiguous
void
CanvasView::SetFillContiguous(bool contiguous)
{
	fFillState->SetContiguous(contiguous);
}

// FillContiguous
bool
CanvasView::FillContiguous() const
{
	return fFillState->Contiguous();
}

// SetText
void
CanvasView::SetText(const char* text)
{
	fTextState->SetText(text);
}

// Text
const char*
CanvasView::Text() const
{
	return fTextState->Text();
}

// SetFamilyAndStyle
void
CanvasView::SetFamilyAndStyle(const char* family, const char* style)
{
	fTextState->SetFamilyAndStyle(family, style);
}

// FontFamily
const char*
CanvasView::FontFamily() const
{
	return fTextState->FontFamily();
}

// FontStyle
const char*
CanvasView::FontStyle() const
{
	return fTextState->FontStyle();
}

// SetFontSize
void
CanvasView::SetFontSize(float size)
{
	fTextState->SetFontSize(size);
}

// FontSize
float
CanvasView::FontSize() const
{
	return fTextState->FontSize();
}

// SetFontOpacity
void
CanvasView::SetFontOpacity(uint8 opacity)
{
	fTextState->SetOpacity(opacity);
}

// FontOpacity
uint8
CanvasView::FontOpacity() const
{
	return fTextState->Opacity();
}

// SetFontAdvanceScale
void
CanvasView::SetFontAdvanceScale(float scale)
{
	fTextState->SetAdvanceScale(scale);
}

// FontAdvanceScale
float
CanvasView::FontAdvanceScale() const
{
	return fTextState->AdvanceScale();
}

// SetTextAlignment
void
CanvasView::SetTextAlignment(uint32 alignment)
{
	fTextState->SetAlignment(alignment);
}

// TextAlignment
uint32
CanvasView::TextAlignment() const
{
	return fTextState->Alignment();
}

// StoreNonGUIFontSettings
void
CanvasView::StoreNonGUIFontSettings(BMessage* message) const
{
	fTextState->StoreNonGUISettings(message);
}

// RestoreNonGUIFontSettings
void
CanvasView::RestoreNonGUIFontSettings(const BMessage* message)
{
	fTextState->RestoreNonGUISettings(message);
}

// SetOpacity
void
CanvasView::SetOpacity(uint8 opacity, uint32 toolID)
{
	switch (toolID) {
		case TOOL_SHAPE:
			fShapeState->SetAlpha(opacity);
			break;
		case TOOL_ELLIPSE:
			fEllipseState->SetAlpha(opacity);
			break;
		case TOOL_ROUND_RECT:
			fRoundRectState->SetAlpha(opacity);
			break;
		case TOOL_BUCKET_FILL:
			fFillState->SetOpacity(opacity);
			break;
	}
}

// Opacity
uint8
CanvasView::Opacity(uint32 toolID) const
{
	uint8 opacity = 255;
	switch (toolID) {
		case TOOL_SHAPE:
			opacity = fShapeState->Alpha();
			break;
		case TOOL_ELLIPSE:
			opacity = fEllipseState->Alpha();
			break;
		case TOOL_ROUND_RECT:
			opacity = fRoundRectState->Alpha();
			break;
		case TOOL_BUCKET_FILL:
			opacity = fFillState->Opacity();
			break;
	}
	return opacity;
}

// SetOutline
void
CanvasView::SetOutline(bool outline, uint32 toolID)
{
	switch (toolID) {
		case TOOL_SHAPE:
			fShapeState->SetOutline(outline);
			break;
		case TOOL_ELLIPSE:
			fEllipseState->SetOutline(outline);
			break;
		case TOOL_ROUND_RECT:
			fRoundRectState->SetOutline(outline);
			break;
	}
}

// Outline
bool
CanvasView::Outline(uint32 toolID) const
{
	bool outline = false;
	switch (toolID) {
		case TOOL_SHAPE:
			outline = fShapeState->Outline();
			break;
		case TOOL_ELLIPSE:
			outline = fEllipseState->Outline();
			break;
		case TOOL_ROUND_RECT:
			outline = fRoundRectState->Outline();
			break;
	}
	return outline;
}

// SetOutlineWidth
void
CanvasView::SetOutlineWidth(float width, uint32 toolID)
{
	switch (toolID) {
		case TOOL_SHAPE:
			fShapeState->SetOutlineWidth(width);
			break;
		case TOOL_ELLIPSE:
			fEllipseState->SetOutlineWidth(width);
			break;
		case TOOL_ROUND_RECT:
			fRoundRectState->SetOutlineWidth(width);
			break;
	}
}

// OutlineWidth
float
CanvasView::OutlineWidth(uint32 toolID) const
{
	float width = 1.0;
	switch (toolID) {
		case TOOL_SHAPE:
			width = fShapeState->OutlineWidth();
			break;
		case TOOL_ELLIPSE:
			width = fEllipseState->OutlineWidth();
			break;
		case TOOL_ROUND_RECT:
			width = fRoundRectState->OutlineWidth();
			break;
	}
	return width;
}

// StoreNonGUIShapeSettings
void
CanvasView::StoreNonGUIShapeSettings(BMessage* message) const
{
	fShapeState->StoreNonGUISettings(message);
}

// RestoreNonGUIShapeSettings
void
CanvasView::RestoreNonGUIShapeSettings(const BMessage* message)
{
	fShapeState->RestoreNonGUISettings(message);
}

// SetRoundCornerRadius
void
CanvasView::SetRoundCornerRadius(float radius)
{
	fRoundRectState->SetRoundCornerRadius(radius);
}

// RoundCornerRadius
float
CanvasView::RoundCornerRadius() const
{
	return fRoundRectState->RoundCornerRadius();
}

// SetShapeCapMode
void
CanvasView::SetShapeCapMode(uint32 mode)
{
	fShapeState->SetCapMode(mode);
}

// ShapeCapMode
uint32
CanvasView::ShapeCapMode() const
{
	return fShapeState->CapMode();
}

// SetShapeJoinMode
void
CanvasView::SetShapeJoinMode(uint32 mode)
{
	fShapeState->SetJoinMode(mode);
}

// ShapeJoinMode
uint32
CanvasView::ShapeJoinMode() const
{
	return fShapeState->JoinMode();
}

// ShapeControlFlags
uint32
CanvasView::ShapeControlFlags() const
{
	return fShapeState->ControlFlags();
}

// SetDropperTipSize
void
CanvasView::SetDropperTipSize(uint32 size)
{
	fColorpickState->SetTipSize(size);
}

// DropperTipSize
uint32
CanvasView::DropperTipSize() const
{
	return fColorpickState->TipSize();
}

// SetDropperUsesAllLayers
void
CanvasView::SetDropperUsesAllLayers(bool all)
{
	fColorpickState->SetUsesAllLayers(all);
}

// DropperUsesAllLayers
bool
CanvasView::DropperUsesAllLayers() const
{
	return fColorpickState->UsesAllLayers();
}

// SetGradientControl
void
CanvasView::SetGradientControl(GradientControl* control)
{
	fGradientState->SetGradientControl(control);
}

// SetPickMask
void
CanvasView::SetPickMask(uint32 mask)
{
	if (fPickMask != mask) {
		fPickMask = mask;
		if (BWindow* window = Window())
			window->PostMessage(MSG_UPDATE_PICK_UI);
	}
}

// PickMask
uint32
CanvasView::PickMask() const
{
	return fPickMask;
}

// SetPrecise
void
CanvasView::SetPrecise(bool enable, uint32 toolID)
{
	switch (toolID) {
		case TOOL_CROP:
			fCropState->SetPrecise(enable);
			break;
		case TOOL_TRANSLATE:
			fTransformState->SetPrecise(enable);
			break;
		case TOOL_TEXT:
			fTextState->SetPrecise(enable);
			break;
		case TOOL_SHAPE:
			fShapeState->SetPrecise(enable);
			break;
		case TOOL_ELLIPSE:
			fEllipseState->SetPrecise(enable);
			break;
		case TOOL_ROUND_RECT:
			fRoundRectState->SetPrecise(enable);
			break;
		case TOOL_EDIT_GRADIENT:
			fGradientState->SetPrecise(enable);
			break;
		case TOOL_GUIDES:
			fGuidesState->SetPrecise(enable);
			break;
		case TOOL_PICK:
			fPickObjectsState->SetPrecise(enable);
			break;
		case TOOL_BRUSH:
		case TOOL_CLONE:
		case TOOL_BLUR:
		case TOOL_PEN:
		case TOOL_PEN_ERASER:
		case TOOL_ERASER:
			fStrokeState->SetPrecise(enable);
			break;
	}
	// update GUI
	if (BWindow* window = Window()) {
		BMessage message(MSG_SUB_PIXEL_PRECISION);
		message.AddBool("active", enable);
		message.AddInt32("tool", toolID);
		window->PostMessage(&message);
	}
}

// Precise
bool
CanvasView::Precise(uint32 toolID) const
{
	bool precise = true;
	switch (toolID) {
		case TOOL_CROP:
			precise = fCropState->Precise();
			break;
		case TOOL_TRANSLATE:
			precise = fTransformState->Precise();
			break;
		case TOOL_TEXT:
			precise = fTextState->Precise();
			break;
		case TOOL_SHAPE:
			precise = fShapeState->Precise();
			break;
		case TOOL_ELLIPSE:
			precise = fEllipseState->Precise();
			break;
		case TOOL_ROUND_RECT:
			precise = fRoundRectState->Precise();
			break;
		case TOOL_EDIT_GRADIENT:
			precise = fGradientState->Precise();
			break;
		case TOOL_GUIDES:
			precise = fGuidesState->Precise();
			break;
		case TOOL_PICK:
			precise = fPickObjectsState->Precise();
			break;
		case TOOL_BRUSH:
		case TOOL_CLONE:
		case TOOL_BLUR:
		case TOOL_PEN:
		case TOOL_PEN_ERASER:
		case TOOL_ERASER:
			precise = fStrokeState->Precise();
			break;
	}
	return precise;
}

// SetShowGrid
void
CanvasView::SetShowGrid(bool enable)
{
	fShowGrid = enable;
	if (BWindow* window = Window()) {
		BMessage message(MSG_SHOW_GRID);
		message.AddBool("active", fShowGrid);
		window->PostMessage(&message);
	}
}

// SetTool
void
CanvasView::SetTool(uint32 mode)
{
	if (fMode != mode) {
		// remember current tool if it was a color tool
		if (!tool_uses_color(mode) && tool_uses_color(fMode)) {
			fLastColorMode = fMode;
		}
		fMode = mode;
		if (BWindow* window = Window()) {
			BMessage message(MSG_SET_TOOL);
			message.AddInt32("id", fMode);
			Window()->PostMessage(&message);
		}
		// new state code:
		SwitchState(StateForCurrentTool());
		if (fCurrentState)
			fCurrentState->SetTool(mode);
	}
}

// Undo
void
CanvasView::Undo()
{
	if (IsTracking() || !fCanvas)
		return;
	if (!fBusy) {
		_SetBusy(true);
		if (HistoryManager* history = fCanvas->GetHistoryManager()) {
			history->Undo(this);
			UpdateHistoryItems();
		}
	}
}

// Redo
void
CanvasView::Redo()
{
	if (IsTracking() || !fCanvas)
		return;

	if (!fBusy) {
		_SetBusy(true);
		if (HistoryManager* history = fCanvas->GetHistoryManager()) {
			history->Redo(this);
			UpdateHistoryItems();
		}
	}
}

// InvalidateCanvas
void
CanvasView::InvalidateCanvas(BRect area, bool deep)
{
	if (deep)
		ComposeLayers(area);
	if (fColorSpace != COLOR_SPACE_NONLINEAR_RGB)
		_ConvertToDisplay(DisplayBitmap(), area);
	_InvalidateCanvas(area);
	_UpdateNavigatorView();
}

// GetCursorPos
bool
CanvasView::GetCursorPos(BPoint* pos) const
{
	if (pos && fCanvas && fCursorShowing) {
		*pos = fCursorPosition;
		ConvertToCanvas(*pos);
		if (fCurrentState && !fCurrentState->Precise()) {
			pos->x = floorf(pos->x + 0.5);
			pos->y = floorf(pos->y + 0.5);
		}
		return true;
	}
	return false;
}

// RebuildLayer
void
CanvasView::RebuildLayer(Layer* layer, BRect area)
{
	if (!(fCurrentState && fCurrentState->RebuildLayer(layer, area)))
		RebuildBitmap(area, layer);
}

// RebuildBitmap
void
CanvasView::RebuildBitmap(BRect area, Layer* layer)
{
	if (!layer)
		layer = fLayer;
	if (layer) {
		layer->Rebuild(fStrokeBitmap, area, fColorSpace, fInfoView);
		if (layer == fLayer)
			copy_area(fLayer->Bitmap(), BackBitmap(), area);
		// finish up
		clear_area(fStrokeBitmap, area);
		InvalidateCanvas(area, true);
		// InvalidateCanvas will not take care of layers
		// other than the current layer
		if (layer != fLayer && fCanvas && fLayersListView) {
			fLayersListView->InvalidateItem(fCanvas->IndexOf(layer));
		}
	}
}

// RebuildCanvas
void
CanvasView::RebuildCanvas(BRect area)
{
	if (fCanvas) {
		BRect entireArea = area;
		for (int32 i = 0; Layer* layer = fCanvas->LayerAt(i); i++) {
			BRect layerArea = area;
			layer->Rebuild(fStrokeBitmap, layerArea, fColorSpace, fInfoView);
			if (layer == fLayer)
				copy_area(fLayer->Bitmap(), BackBitmap(), layerArea);
			entireArea = entireArea | layerArea;
		}
		// finish up
		clear_area(fStrokeBitmap, entireArea);
		InvalidateCanvas(entireArea, true);
	}
}

// ComposeLayers
void
CanvasView::ComposeLayers(BRect area)
{
	if (fCanvas) {
		BBitmap* into = DisplayBitmap();
		// find out, if we have only layers with "normal" blending mode
		bool normal = true;
		for (int32 i = 0; Layer* layer = fCanvas->LayerAt(i); i++) {
			if (layer->Mode() != MODE_NORMAL && !(layer->Flags() & FLAG_INVISIBLE)) {
				normal = false;
				break;
			}
		}
		if (normal) {
			// compose on top of bottom bitmap
			copy_area(fBottomBitmap, into, area);
			fCanvas->Compose(into, area);
		} else {
			// compose into empty bitmap
			clear_area(into, area, 0, 0, 0, 0);
			fCanvas->Compose(into, area);
			// merge with bottom bitmap
			BRect r = area & into->Bounds();
			if (area.IsValid() && r.IsValid()) {
				uint32 left = (uint32)floorf(r.left);
				uint32 top = (uint32)floorf(r.top);
				uint32 right = (uint32)ceilf(r.right);
				uint32 bottom = (uint32)ceilf(r.bottom);
				uint8* src = (uint8*)fBottomBitmap->Bits();
				uint8* dst = (uint8*)into->Bits();
				uint32 bpr = into->BytesPerRow();
				src += 4 * left + bpr * top;
				dst += 4 * left + bpr * top;
				for (; top <= bottom; top++) {
					uint8* srcHandle = src;
					uint8* dstHandle = dst;
					for (uint32 x = left; x <= right; x++) {
						blend_colors_copy(dstHandle, srcHandle, dstHandle);
						srcHandle += 4;
						dstHandle += 4;
					}
					src += bpr;
					dst += bpr;
				}
			}
		}
		HandleDemoMode(into, area);
	}
}

// ModifierSelectionChanged
void
CanvasView::ModifierSelectionChanged(bool itemsSelected)
{
	if (fCurrentState)
		fCurrentState->ModifierSelectionChanged(itemsSelected);

	// the fTransformState needs some special treatment (it wants to know always)
	if (fCurrentState != fTransformState)
		fTransformState->ModifierSelectionChanged(itemsSelected);

	// the fGradientState needs some special treatment (it wants to know always)
	if (fCurrentState != fGradientState)
		fGradientState->ModifierSelectionChanged(itemsSelected);
}

// AddModifiers
void
CanvasView::AddModifiers(const BList& strokeList, int32 index)
{
	int32 count = strokeList.CountItems();
	if (count > 0) {
		Stroke** strokes = new Stroke*[count];
		for (int32 i = 0; i < count; i++) {
			if (Stroke* stroke = (Stroke*)strokeList.ItemAt(i))
				strokes[i] = stroke->Clone();
			else
				strokes[i] = NULL;
		}
		Perform(new AddModifiersAction(fLayer, strokes, count, index));
	}
}

// MoveHistory
void
CanvasView::MoveHistory(BList& items, int32 index)
{
	int32 count = items.CountItems();
	if (fHistory && count > 0) {
		Stroke** strokes = new Stroke*[count];
		for (int32 i = 0; i < count; i++) {
			int32 index = fHistoryListView->IndexOf((BListItem*)items.ItemAt(i));
			strokes[i] = fHistory->ModifierAt(index);
		}
		Perform(new MoveModifiersAction(fLayer, fLayer, strokes, count, index));
	}
}

// CopyHistory
void
CanvasView::CopyHistory(BList& items, int32 index)
{
	int32 count = items.CountItems();
	if (fHistory && count > 0) {
		Stroke** strokes = new Stroke*[count];
		for (int32 i = 0; i < count; i++) {
			int32 index = fHistoryListView->IndexOf((BListItem*)items.ItemAt(i));
			if (Stroke* stroke = fHistory->ModifierAt(index))
				strokes[i] = stroke->Clone();
			else
				strokes[i] = NULL;
		}
		Perform(new AddModifiersAction(fLayer, strokes, count, index));
	}
}

// RemoveHistory
void
CanvasView::RemoveHistory(BList& items)
{
	int32 count = items.CountItems();
	if (fHistory && count > 0) {
		Stroke** strokes = new Stroke*[count];
		for (int32 i = 0; i < count; i++) {
			int32 index = fHistoryListView->IndexOf((BListItem*)items.ItemAt(i));
			strokes[i] = fHistory->ModifierAt(index);
		}
		Perform(new DeleteModifiersAction(fLayer, strokes, count));
	}
}

// ChangeColors
void
CanvasView::ChangeColors(BList& indices, rgb_color color)
{
	int32 count = indices.CountItems();
	if (fHistory && count > 0) {
		// copy items and make color change action
		Stroke** strokes = new Stroke*[count];
		for (int32 i = 0; i < count; i++) {
			int32 index = (int32)(intptr_t)indices.ItemAt(i);
			strokes[i] = fHistory->ModifierAt(index);
		}
		Perform(new ChangeColorAction(fLayer, strokes, color, count));
	}
}

// Perform
bool
CanvasView::Perform(Action* action)
{
	bool success = action != NULL && fCanvas != NULL;
	if (success) {
/*		HistoryManager* history = fCanvas->GetHistoryManager();
		if (history && history->AddAction(action) >= B_OK) {
			action->Perform(this);
			UpdateHistoryItems();
		} else
			success = false;*/
		if (action->InitCheck() >= B_OK && action->Perform(this) >= B_OK) {
			HistoryManager* history = fCanvas->GetHistoryManager();
			if (history && history->AddAction(action) >= B_OK) {
				success = true;
				UpdateHistoryItems();
			}
		} else {
			success = false;
		}
	}
	if (!success)
		delete action;
	return success;
}

// SwitchState
void
CanvasView::SwitchState(CanvasViewState* newState)
{
	if (fCurrentState != newState) {
		CleanUpState(fCurrentState);
		fCurrentState = newState;
		InitState(fCurrentState);
	}
}

// InitState
void
CanvasView::InitState(CanvasViewState* state)
{
	if (state) {
		state->Init(fCanvas, fLayer, fCursorPosition);
	}
}

// CleanUpState
void
CanvasView::CleanUpState(CanvasViewState* state)
{
	if (state) {
		state->CleanUp();
	}
}

// StateForCurrentTool
CanvasViewState*
CanvasView::StateForCurrentTool() const
{
	CanvasViewState* state = NULL;
	switch (fMode) {
		case TOOL_COLOR_PICKER:
			state = fColorpickState;
			break;
		case TOOL_BUCKET_FILL:
			state = fFillState;
			break;
		case TOOL_TRANSLATE:
			state = fTransformState;
			break;
		case TOOL_CROP:
			state = fCropState;
			break;
		case TOOL_TEXT:
			state = fTextState;
			break;
		case TOOL_SHAPE:
			state = fShapeState;
			break;
		case TOOL_SELECT:
			state = fSelectState;
			break;
		case TOOL_ELLIPSE:
			state = fEllipseState;
			break;
		case TOOL_ROUND_RECT:
			state = fRoundRectState;
			break;
		case TOOL_EDIT_GRADIENT:
			state = fGradientState;
			break;
		case TOOL_GUIDES:
			state = fGuidesState;
			break;
		case TOOL_PICK:
			state = fPickObjectsState;
			break;
		case TOOL_BRUSH:
		case TOOL_CLONE:
		case TOOL_PEN:
		case TOOL_PEN_ERASER:
		case TOOL_ERASER:
		case TOOL_BLUR:
		default:
			state = fStrokeState;
			break;
	}
	return state;
}

// _Init
void
CanvasView::_Init(bool makeEmpty)
{
	fCanvasFrame.OffsetTo(0.0, 0.0);
	fCanvasBounds.OffsetTo(0.0, 0.0);
	color_space format = fLayer ? fLayer->Bitmap()->ColorSpace() : B_RGBA32;
	if (!fBitmap[0])
		fBitmap[0] = new BBitmap(fCanvasBounds, format);
	if (!fBitmap[1])
		fBitmap[1] = new BBitmap(fCanvasBounds, format);
//	fDisplayBitmap = new BBitmap(fCanvasBounds, B_RGBA32);
	fStrokeBitmap = new BBitmap(fCanvasBounds, B_GRAY8);
	if (makeEmpty)
		_MakeEmpty();

	// init zoom level
	if (fCanvas) {
		// set zoom state to that of canvas
		float zoom;
		BPoint center;
		fCanvas->GetZoomState(zoom, center);
		_SetZoom(zoom, true, true);
	} else {
		fZoomLevel = 100.0;

	//	fScrollingEnabled = false;
	//	fScrollable->SetScrollOffset(BPoint(0.0, 0.0));
		_SetDataRect(fCanvasFrame);
	//	fScrollingEnabled = true;
		_UpdateInfoView();
	}

	if (LockLooper()) {
		FrameResized(Bounds().Width(), Bounds().Height());
		Invalidate();
		UnlockLooper();
	}
}

// _UnInit
void
CanvasView::_UnInit()
{
	if (fNavigatorView)
		fNavigatorView->Update(NULL, BRect(0.0, 0.0, -1.0, -1.0));
	if (fInfoView)
		fInfoView->SetInfo("Empty.");
	delete fBitmap[0];
	delete fBitmap[1];
	delete fDisplayBitmap;
	delete fStrokeBitmap;
	delete fBottomBitmap;
	delete fTopBitmap;
	fBitmap[0] = NULL;
	fBitmap[1] = NULL;
	fDisplayBitmap = NULL;
	fStrokeBitmap = NULL;
	fBottomBitmap = NULL;
	fTopBitmap = NULL;
	fLayer = NULL;
	fHistory = NULL;

	_UpdateToolCursor();
}

// _UpdateInfoView
void
CanvasView::_UpdateInfoView() const
{
	BString helper("");
	if (fBitmap[0] && fBitmap[0]->IsValid()) {
		helper << fBitmap[0]->Bounds().IntegerWidth() + 1;
		helper << " x ";
		helper << fBitmap[0]->Bounds().IntegerHeight() + 1;
	} else {
		helper << "Empty.";
	}
	fInfoView->SetInfo(helper.String());
	if (BWindow* window = Window()) {
		BMessage message(MSG_SET_ZOOM);
		message.AddInt32("be:value", (int32)fZoomLevel);
		window->PostMessage(&message);
	}
}

// _UpdateNavigatorView
void
CanvasView::_UpdateNavigatorView() const
{
	BRect data = fScrollable->DataRect();
	// exclude extra spacing
	data.InsetBy(kExtraCanvasSpacing, kExtraCanvasSpacing);
	BRect visible = fScrollable->VisibleRect();
	if (data.Width() >= visible.Width()) {
		float scale = data.Width() / fCanvasBounds.Width();
		visible.left /= scale;
		visible.right /= scale;
	} else {
		visible.left = fCanvasBounds.left;
		visible.right = fCanvasBounds.right;
	}
	if (data.Height() >= visible.Height()) {
		float scale = data.Width() / fCanvasBounds.Width();
		visible.top /= scale;
		visible.bottom /= scale;
	} else {
		visible.top = fCanvasBounds.top;
		visible.bottom = fCanvasBounds.bottom;
	}
	BBitmap* bitmap = fColorSpace != COLOR_SPACE_NONLINEAR_RGB ?
					  fDisplayBitmap : DisplayBitmap();
	fNavigatorView->Update(bitmap, visible);

	if (fLayer && fCanvas && fLayersListView) {
		fLayersListView->InvalidateLayer(fCanvas->IndexOf(fLayer));
	}
}

// _SetBusy
void
CanvasView::_SetBusy(bool busy)
{
	if (fBusy != busy) {
		if (BWindow* window = Window()) {
			fBusy = busy;
			if (fBusy) {
				window->PostMessage(MSG_ALLOW_UPDATES, this);
			} else {
				if (fEventDropped) {
					window->PostMessage(MSG_REDO_MOUSE_MOVED);
					fEventDropped = false;
				}
			}
		}
	}
}

// ConvertToCanvas
void
CanvasView::ConvertToCanvas(BPoint& where) const
{
	where -= fCanvasFrame.LeftTop();
	where.x /= fZoomLevel / 100.0;
	where.y /= fZoomLevel / 100.0;
}

// ConvertFromCanvas
void
CanvasView::ConvertFromCanvas(BPoint& where) const
{
	where.x *= fZoomLevel / 100.0;
	where.y *= fZoomLevel / 100.0;
	where += fCanvasFrame.LeftTop();
}

// ConvertFromCanvas
BRect
CanvasView::ConvertFromCanvas(const BRect& rect) const
{
	float scale = fZoomLevel / 100.0;
	BRect r(rect.left * scale,
			rect.top * scale,
			rect.right * scale,
			rect.bottom * scale);
	r.OffsetBy(fCanvasFrame.left, fCanvasFrame.top);
	return r;
}

// _InvalidateCanvas
void
CanvasView::_InvalidateCanvas(BRect updateRect, bool compose)
{
	if (compose) {
		if (fDirtyBounds.IsValid()) {
			fDirtyBounds = fDirtyBounds | updateRect;
		} else {
			fDirtyBounds = updateRect;
		}
	}
	float scale = fZoomLevel / 100.0;
	updateRect.left = floorf(floorf(updateRect.left) * scale);
	updateRect.top = floorf(floorf(updateRect.top) * scale);
	updateRect.right = ceilf(ceilf(updateRect.right + 1.0) * scale) - 1.0;
	updateRect.bottom = ceilf(ceilf(updateRect.bottom + 1.0) * scale) - 1.0;

	updateRect.OffsetBy(fCanvasFrame.left, fCanvasFrame.top);

	Invalidate(updateRect);
}

// _SetToColorAtPixel
void
CanvasView::_SetToColorAtPixel(BPoint where, bool useAllLayer, uint32 size) const
{
	BWindow* window = Window();
	BBitmap* bitmap = useAllLayer ? DisplayBitmap() : BackBitmap();
	if (bitmap && window) {
		uint8* bits = (uint8*)bitmap->Bits();
		if (bitmap->Bounds().Contains(where)) {
			bits += (int32)(where.y) * bitmap->BytesPerRow() + (int32)(where.x) * 4;
			rgb_color color;
			switch (size) {
				case 1:
					color.red = bits[2];
					color.green = bits[1];
					color.blue = bits[0];
					break;
				case 3: {
					BRect r(where - BPoint(1.0, 1.0), where + BPoint(1.0, 1.0));
					color = average_color(bitmap, r);
					break;
				}
				case 5: {
					BRect r(where - BPoint(2.0, 2.0), where + BPoint(2.0, 2.0));
					color = average_color(bitmap, r);
					break;
				}
			}

			color.alpha = 255;
			BMessage message(MSG_SET_COLOR);
			store_color_in_message(&message, color);
			window->PostMessage(&message);
		}
	}
}

// AddStroke
bool
CanvasView::AddStroke(Stroke* stroke)
{
	bool success = false;
	if (stroke) {
		if (fHistory) {
			success = fHistory->AddModifier(stroke);
			if (success) {
				_AddHistory(stroke);
			}
		}
	}
	return success;
}

// _AddHistory
void
CanvasView::_AddHistory(Stroke* stroke)
{
	fHistoryListView->AddModifier(stroke);
}

// _MakeEmpty
void
CanvasView::_MakeEmpty()
{
	if (fLayer)
		fLayer->MakeEmpty();
//	fCurrentStroke = NULL;
	fCurrentBitmap = 0;
	// clear modifier stack
	if (fHistoryListView)
		fHistoryListView->ClearList();
	// prepare bitmaps
	if (DisplayBitmap()) {
		BRect r(DisplayBitmap()->Bounds());
		if (fLayer) {
			copy_area(fLayer->Bitmap(), BackBitmap(), r);
			ComposeLayers(r);
		}
		memset(fStrokeBitmap->Bits(), 0, fStrokeBitmap->BitsLength());
	}
}

// _GetTabletInfo
bool
CanvasView::_GetTabletInfo(BPoint& where, Point& canvasWhere, bool& eraser,
						   BMessage* message) const
{
	bool tablet = false;
	if (message) {
		// coordinates
		float x, y;
		if (message->FindFloat("be:tablet_x", &x) >= B_OK
			&& message->FindFloat("be:tablet_y", &y) >= B_OK) {
			BScreen screen(Window());
			x *= screen.Frame().Width();
			y *= screen.Frame().Height();
			canvasWhere.point = ConvertFromScreen(BPoint(x, y));
		} else {
			canvasWhere.point = where;
		}

		// convert to canvas space and round if subpixel precision is off
		ConvertToCanvas(canvasWhere.point);
		if (fCurrentState && !fCurrentState->Precise()) {
			canvasWhere.point.x = floorf(canvasWhere.point.x + 0.5);
			canvasWhere.point.y = floorf(canvasWhere.point.y + 0.5);
		}

		// pressure
		if (message->FindFloat("be:tablet_pressure", &canvasWhere.pressure) < B_OK) {
			canvasWhere.pressure = 1.0;
		} else {
			tablet = true;
		}
		constrain(canvasWhere.pressure, 0.0, 1.0);

		// eraser
		int32 eraserMode;
		if (message->FindInt32("be:tablet_eraser", &eraserMode) < B_OK)
			eraserMode = 0;
		eraser = eraserMode > 0;

		// tilt
		if (message->FindFloat("be:tablet_tilt_x", &canvasWhere.tiltX) < B_OK)
			canvasWhere.tiltX = 0.0;
		if (message->FindFloat("be:tablet_tilt_y", &canvasWhere.tiltY) < B_OK)
			canvasWhere.tiltY = 0.0;
	}
	return tablet;
}

// DisplayBitmap()
BBitmap*
CanvasView::DisplayBitmap() const
{
	return fBitmap[fCurrentBitmap];
}

// BackBitmap()
BBitmap*
CanvasView::BackBitmap() const
{
	if (fCurrentBitmap == 1)
		return fBitmap[0];
	else
		return fBitmap[1];
}

// SwitchBuffers
void
CanvasView::SwitchBuffers(BRect areaToCopy)
{
	if (fLayer) {
		copy_area(fLayer->Bitmap(), BackBitmap(), areaToCopy);
	}
	_UpdateNavigatorView();
}

// _ConvertToDisplay
void
CanvasView::_ConvertToDisplay(BBitmap* from, BRect area) const
{
	_ConvertToDisplay(from, fDisplayBitmap, area);
}

// _ConvertToDisplay
void
CanvasView::_ConvertToDisplay(BBitmap* from, BBitmap* into, BRect area) const
{
//bigtime_t now = system_time();
	if (area.IsValid() && area.Intersects(from->Bounds())) {
		// make sure we don't draw out of bounds
		area = from->Bounds() & area;
		uint32 bpr32 = from->BytesPerRow();
		uint8* dest = (uint8*)into->Bits();
		uint8* src = (uint8*)from->Bits();
		// coordinates and offset into bitmaps
		int32 y = (int32)area.top;
		int32 x = (int32)area.left;
		int32 offset32 = y * bpr32 + 4 * x;
		src += offset32;
		dest += offset32;
		int32 bottom = (uint32)area.bottom;
		int32 right = (uint32)area.right;
		switch (fColorSpace) {
			case COLOR_SPACE_NONLINEAR_RGB: {
				// here for completeness, we don't actually convert when in this colorspace
				uint32 bytesToCopy = (right - (int32)area.left) * 4;
				for (; y <= bottom; y++) {
					memcpy(dest, src, bytesToCopy);
					src += bpr32;
					dest += bpr32;
				}
				break;
			}
			case COLOR_SPACE_LINEAR_RGB:
				for (; y <= bottom; y++) {
					uint8* srcHandle = src;
					uint8* destHandle = dest;
					for (x = (int32)area.left; x <= right; x++) {
						destHandle[0] = fGammaTable[srcHandle[0]];
						destHandle[1] = fGammaTable[srcHandle[1]];
						destHandle[2] = fGammaTable[srcHandle[2]];
						srcHandle += 4;
						destHandle += 4;
					}
					src += bpr32;
					dest += bpr32;
				}
				break;
			case COLOR_SPACE_LAB:
// conversion time for a 800x600 bitmap:
// 755065 us
// ... down to ~120000 us with LUTs and removal of pow(x, 3.0)
				for (; y <= bottom; y++) {
					uint8* srcHandle = src;
					uint8* destHandle = dest;
					for (x = (int32)area.left; x <= right; x++) {
						lab2rgb(srcHandle[0], srcHandle[1], srcHandle[2],
								destHandle[2], destHandle[1], destHandle[0]);
						srcHandle += 4;
						destHandle += 4;
					}
					src += bpr32;
					dest += bpr32;
				}
				break;
			case COLOR_SPACE_UNKNOWN:
			default:
				break;
		}
	}
//printf("color space conversion: %lld\n", system_time() - now);
}

// _MakeBottomBitmap
void
CanvasView::_MakeBottomBitmap() const
{
	// make the pattern for transparent background
	uint8* bits = (uint8*)fBottomBitmap->Bits();
	uint32 bpr = fBottomBitmap->BytesPerRow();
	uint32 height = fBottomBitmap->Bounds().IntegerHeight() + 1;
	uint32 width = fBottomBitmap->Bounds().IntegerWidth() + 1;
	int32 whiteX = 0;
	int32 whiteY = 0;

	uint8 c1, c2, c3;
	get_components_for_color(kAlphaLow, fColorSpace, c1, c2, c3);

	uint32 lowColor;
	uint8* handle = (uint8*)&lowColor;
	handle[0] = c1;
	handle[1] = c2;
	handle[2] = c3;
	handle[3] = 255;

	get_components_for_color(kAlphaHigh, fColorSpace, c1, c2, c3);

	uint32 highColor;
	handle = (uint8*)&highColor;
	handle[0] = c1;
	handle[1] = c2;
	handle[2] = c3;
	handle[3] = 255;

	int32 step = 20;
	int32 step2 = 10;

	for (uint32 y = 0; y < height; y++) {
		uint32* bitsHandle = (uint32*)bits;
		if (whiteY < step2)
			whiteX = step2;
		else
			whiteX = 0;
		whiteY++;
		if (whiteY >= step)
			whiteY = 0;
		for (uint32 x = 0; x < width; x++) {
			if (whiteX < step2)
				*bitsHandle = highColor;
			else
				*bitsHandle = lowColor;
			whiteX++;
			if (whiteX >= step)
				whiteX = 0;
			bitsHandle++;
		}
		bits += bpr;
	}
}

// _MakeTopBitmap
void
CanvasView::_MakeTopBitmap() const
{
	if (fTopBitmap && fTopBitmap->IsValid()) {
		// fill top bitmap with demo stuff
		memset(fTopBitmap->Bits(), 0, fTopBitmap->BitsLength());

		BBitmap temp(fTopBitmap->Bounds(), 0, B_GRAY8);
		memset(temp.Bits(), 0, temp.BitsLength());

		uint32 height = fTopBitmap->Bounds().IntegerHeight() + 1;
		uint32 width = fTopBitmap->Bounds().IntegerWidth() + 1;

		rgb_color color = (rgb_color){ 255, 255, 255, 255 };
		TextStroke text(color);
		float size = height / 400.0;
		text.SetSize(size);
		text.RotateBy(BPoint(0.0, 0.0), -10.0);
		text.SetOpacity(30);
		char demoText[20];
		demoText[0] = 'W';
		demoText[1] = 'o';
		demoText[2] = 'n';
		demoText[3] = 'd';
		demoText[4] = 'e';
		demoText[5] = 'r';
		demoText[6] = 'B';
		demoText[7] = 'r';
		demoText[8] = 'u';
		demoText[9] = 's';
		demoText[10] = 'h';
		demoText[11] = ' ';
		demoText[12] = 'D';
		demoText[13] = 'e';
		demoText[14] = 'm';
		demoText[15] = 'o';
		demoText[16] = 0;
		text.SetText(demoText);

		float w = text.Bounds().Width();
		float h = text.Bounds().Height() * .7;

		int32 rows = width / (int32)w + 1;
		int32 lines = height / (int32)h + 2;
		for (int32 i = 0; i < lines; i++) {
			for (int32 j = 0; j < rows; j++) {
				text.Draw(&temp);
				text.TranslateBy(BPoint(w, 0.0));
			}
			text.TranslateBy(BPoint(w * -rows, h));
		}
		overlay_color(fTopBitmap, &temp, fTopBitmap->Bounds(),
					  255, 255, 255);
	}
}

// UpdateHistoryItems
void
CanvasView::UpdateHistoryItems() const
{
	if (BWindow* window = Window())
		window->PostMessage(MSG_UPDATE_HISTORY_ITEMS);
}

// SetAutoScrolling
void
CanvasView::SetAutoScrolling(bool scroll)
{
	if (scroll) {
		if (!fAutoScroller) {
			BMessenger messenger(this, Window());
			BMessage message(MSG_AUTO_SCROLL);
			// this trick avoids the MouseMoved() hook
			// to think that the mouse is not pressed
			// anymore when we call it ourselfs from the
			// autoscrolling code
			message.AddInt32("buttons", 1);
			fAutoScroller = new BMessageRunner(messenger,
											   &message,
											   AUTO_SCROLL_DELAY);
		}
	} else {
		delete fAutoScroller;
		fAutoScroller = NULL;
	}
}

// _UpdateToolCursor
void
CanvasView::_UpdateToolCursor()
{
	if (fCanvas && fLayer) {
		if (fScrollTracking || fSpaceHeldDown) {
			const uchar* cursorData = fScrollTracking ? kGrabCursor : kHandCursor;
			BCursor cursor(cursorData);
			SetViewCursor(&cursor, true);
			_ClearCursor();
		} else if (fCurrentState) {
			fCurrentState->DrawCursor(fCursorPosition, true);
			fCurrentState->UpdateToolCursor();
		}
	} else {
		BCursor cursor(kStopCursor);
		SetViewCursor(&cursor, true);
	}
}

// _DrawCursor
void
CanvasView::_DrawCursor(BPoint where, bool invisible)
{
	if (fCurrentState)
		fCurrentState->DrawCursor(where, invisible || fScrollTracking);
}

// _ClearCursor
void
CanvasView::_ClearCursor(BRect* updateRect)
{
	if (fCurrentState)
		fCurrentState->ClearCursor(updateRect);
}

// _ShowPopupMenu
void
CanvasView::_ShowPopupMenu(BPoint where)
{
	if (fCanvas && fLayer) {
		LanguageManager* manager = LanguageManager::Default();
		BPopUpMenu* menu = new CanvasPopupMenu(this);

		// undo / redo
		const char* undoString = manager->GetString(UNDO, "Undo");
		const char* redoString = manager->GetString(REDO, "Redo");
		BMenuItem* undoItem;
		BMenuItem* redoItem;
		if (HistoryManager* history = fCanvas->GetHistoryManager()) {
			BString helper(undoString);
			bool enable = history->GetUndoName(helper);
			undoItem = new BMenuItem(helper.String(), new BMessage(MSG_UNDO));
			undoItem->SetEnabled(enable);
			helper = redoString;
			enable = history->GetRedoName(helper);
			redoItem = new BMenuItem(helper.String(), new BMessage(MSG_REDO));
			redoItem->SetEnabled(enable);
		} else {
			undoItem = new BMenuItem(undoString, NULL);
			undoItem->SetEnabled(false);
			redoItem = new BMenuItem(redoString, NULL);
			redoItem->SetEnabled(false);
		}
		menu->AddItem(undoItem);
		menu->AddItem(redoItem);

		menu->AddSeparatorItem();

		// add the tools
		// pick objects
		BMessage* message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_PICK);
		BMenuItem* item = new BMenuItem(manager->GetString(PICK_OBJECTS, "Pick Objects"), message);
		item->SetMarked(fMode == TOOL_PICK);
		menu->AddItem(item);
		// clipboard
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_SELECT);
		item = new BMenuItem(manager->GetString(SELECT, "Select"), message);
		item->SetMarked(fMode == TOOL_SELECT);
		menu->AddItem(item);
		// crop
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_CROP);
		item = new BMenuItem(manager->GetString(CROP, "Crop"), message);
		item->SetMarked(fMode == TOOL_CROP);
		menu->AddItem(item);
		// translate
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_TRANSLATE);
		item = new BMenuItem(manager->GetString(TRANSLATE, "Translate"), message);
		item->SetMarked(fMode == TOOL_TRANSLATE);
		menu->AddItem(item);

		menu->AddSeparatorItem();
		// brush
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_BRUSH);
		item = new BMenuItem(manager->GetString(BRUSH, "Brush"), message);
		item->SetMarked(fMode == TOOL_BRUSH);
		menu->AddItem(item);
		// pen
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_PEN);
		item = new BMenuItem(manager->GetString(PEN, "Pen"), message);
		item->SetMarked(fMode == TOOL_PEN);
		menu->AddItem(item);
		// eraser
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_ERASER);
		item = new BMenuItem(manager->GetString(ERASER, "Eraser"), message);
		item->SetMarked(fMode == TOOL_ERASER);
		menu->AddItem(item);
		// pen eraser
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_PEN_ERASER);
		item = new BMenuItem(manager->GetString(PEN_ERASER, "Eraser Pen"), message);
		item->SetMarked(fMode == TOOL_PEN_ERASER);
		menu->AddItem(item);

		menu->AddSeparatorItem();
		// clone
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_CLONE);
		item = new BMenuItem(manager->GetString(CLONE, "Clone"), message);
		item->SetMarked(fMode == TOOL_CLONE);
		menu->AddItem(item);
		// blur
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_BLUR);
		item = new BMenuItem(manager->GetString(BLUR, "Blur"), message);
		item->SetMarked(fMode == TOOL_BLUR);
		menu->AddItem(item);

		menu->AddSeparatorItem();
		// fill
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_BUCKET_FILL);
		item = new BMenuItem(manager->GetString(BUCKET_FILL, "Fill"), message);
		item->SetMarked(fMode == TOOL_BUCKET_FILL);
		menu->AddItem(item);
		// text
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_TEXT);
		item = new BMenuItem(manager->GetString(TEXT, "Text"), message);
		item->SetMarked(fMode == TOOL_TEXT);
		menu->AddItem(item);
		// shape
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_SHAPE);
		item = new BMenuItem(manager->GetString(SHAPE, "Shape"), message);
		item->SetMarked(fMode == TOOL_SHAPE);
		menu->AddItem(item);
		// ellipse
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_ELLIPSE);
		item = new BMenuItem(manager->GetString(ELLIPSE, "Ellipse"), message);
		item->SetMarked(fMode == TOOL_ELLIPSE);
		menu->AddItem(item);
		// rect
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_ROUND_RECT);
		item = new BMenuItem(manager->GetString(ROUND_RECT, "(Round) Rect"), message);
		item->SetMarked(fMode == TOOL_ROUND_RECT);
		menu->AddItem(item);

		menu->AddSeparatorItem();
		// dropper
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_COLOR_PICKER);
		item = new BMenuItem(manager->GetString(DROPPER, "Dropper"), message);
		item->SetMarked(fMode == TOOL_COLOR_PICKER);
		menu->AddItem(item);
		// guides
		message = new BMessage(MSG_SET_TOOL);
		message->AddInt32("tool", TOOL_GUIDES);
		item = new BMenuItem(manager->GetString(GUIDES, "Guides"), message);
		item->SetMarked(fMode == TOOL_GUIDES);
		menu->AddItem(item);

		menu->AddSeparatorItem();

		// layers
		BMenu* layersMenu = new BMenu(manager->GetString(LAYER, "Layer"));
		for (int32 i = 0; Layer* layer = fCanvas->LayerAt(i); i++) {
			message = new BMessage(MSG_SET_LAYER);
			message->AddInt32("index", i);
			item = new BMenuItem(layer->Name(), message);
			if (layer == fLayer)
				item->SetMarked(true);
			layersMenu->AddItem(item);
		}
		layersMenu->SetTargetForItems(this);
		menu->AddItem(layersMenu);

		// finish popup
		menu->SetTargetForItems(this);
		menu->SetAsyncAutoDestruct(true);
		menu->SetFont(be_plain_font);
		where = ConvertToScreen(where);
		BRect mouseRect(where, where);
		mouseRect.InsetBy(-10.0, -10.0);
		where += BPoint(5.0, 5.0);
		menu->Go(where, true, false, mouseRect, true);
	}
}

// _InnerBounds
BRect
CanvasView::_InnerBounds() const
{
	BRect r(Bounds());
	r.right -= B_V_SCROLL_BAR_WIDTH;
	r.bottom -= B_H_SCROLL_BAR_HEIGHT;
	return r;
}

// _SetDataRect
void
CanvasView::_SetDataRect(BRect rect)
{
	rect.InsetBy(-kExtraCanvasSpacing, -kExtraCanvasSpacing);
	fScrollable->SetDataRect(rect);
}

// _SetZoom
void
CanvasView::_SetZoom(float level, bool force, bool ignorBusy)
{
	if ((!fBusy || ignorBusy) && fCanvas) {
		if (level > 3200.0)
			level = 3200.0;
		if (level < 7.25)
			level = 7.25;
		if (level != fZoomLevel || force) {
			// prevent zoom messages from piling up
			_SetBusy(true);
			fZoomLevel = level;
			if (fBitmap[0] && fCanvas) {
				// calculate visible part
				BRect r = fBitmap[0]->Bounds();
				float scale = fZoomLevel / 100.0;
				float width = floorf(r.Width() * scale + 1.0 * (scale - 1.0) + 0.5);
				float height = floorf(r.Height() * scale + 1.0 * (scale - 1.0) + 0.5);

				r = _InnerBounds();

				BPoint center((r.left + r.right) / 2.0, (r.top + r.bottom) / 2.0);
/*				BPoint center;
				if (r.Contains(fCursorPosition)) {
					center = fCursorPosition;
				} else {
					center.x = (r.left + r.right) / 2.0,
					center.y = (r.top + r.bottom) / 2.0;
				}*/

				// scale current offset from center of canvas
				BPoint canvasPoint;
				float dummy;
				fCanvas->GetZoomState(dummy, canvasPoint);

				fCanvasFrame.left = roundf(center.x - canvasPoint.x * scale);
				fCanvasFrame.top = roundf(center.y - canvasPoint.y * scale);

				if (width < r.Width()) {
					// center canvas horizontally
					fCanvasFrame.left = roundf(r.left + (r.Width() - width) / 2.0);
					fCanvasFrame.right = fCanvasFrame.left + width;
				} else {
					fCanvasFrame.right = fCanvasFrame.left + width;
					// see that view is not outside of canvas
					if (fCanvasFrame.left > r.left)
						fCanvasFrame.OffsetBy(-(fCanvasFrame.left - r.left), 0.0);
					else if (fCanvasFrame.right < r.right)
						fCanvasFrame.OffsetBy(r.right - fCanvasFrame.right, 0.0);
				}
				if (height < r.Height()) {
					// center canvas horizontally
					fCanvasFrame.top = roundf(r.top + (r.Height() - height) / 2.0);
					fCanvasFrame.bottom = fCanvasFrame.top + height;
				} else {
					fCanvasFrame.bottom = fCanvasFrame.top + height;
					// see that view is not outside of canvas
					if (fCanvasFrame.top > r.top)
						fCanvasFrame.OffsetBy(0.0, -(fCanvasFrame.top - r.top));
					else if (fCanvasFrame.bottom < r.bottom)
						fCanvasFrame.OffsetBy(0.0, r.bottom - fCanvasFrame.bottom);
				}

				BPoint newScrollOffset;
				if (fCanvasFrame.left > 0.0)
					newScrollOffset.x = 0.0;
				else
					newScrollOffset.x = -fCanvasFrame.left;
				if (fCanvasFrame.top > 0.0)
					newScrollOffset.y = 0.0;
				else
					newScrollOffset.y = -fCanvasFrame.top;
				// update all inherited classes with the new scrolling state
				fScrollingEnabled = false;

				newScrollOffset.x = roundf(newScrollOffset.x);
				newScrollOffset.y = roundf(newScrollOffset.y);

				_SetDataRect(BRect(0.0, 0.0, width, height));
				fScrollable->SetScrollOffset(newScrollOffset);
				ScrollView::ScrollOffsetChanged(fCurrentScrollOffset, newScrollOffset);
				// update canvas
				ConvertToCanvas(center);
				fCanvas->SetZoomState(fZoomLevel, center);
				// update cursor
				if (fCursorShowing)
					_DrawCursor(fCursorPosition);
				fCurrentScrollOffset = newScrollOffset;
				_UpdateNavigatorView();
				fScrollingEnabled = true;

				Invalidate();

				_UpdateInfoView();
			}
		}
	}
}

// 	_UpdateCanvasZoomInfo
void
CanvasView::_UpdateCanvasZoomInfo() const
{
	// update canvas
	if (fCanvas) {
		BRect r = _InnerBounds();
		BPoint center((r.left + r.right) / 2.0, (r.top + r.bottom) / 2.0);
		ConvertToCanvas(center);
		fCanvas->SetZoomState(fZoomLevel, center);
	}
}

// _NextZoomInLevel
float
CanvasView::_NextZoomInLevel(float zoom) const
{
	if (zoom < 12.5)
		return 12.5;
	if (zoom < 25.0)
		return 25.0;
	if (zoom < 33.0)
		return 33.0;
	if (zoom < 50.0)
		return 50.0;
	if (zoom < 66.0)
		return 66.0;
	if (zoom < 100.0)
		return 100.0;
	if (zoom < 150.0)
		return 150.0;
	if (zoom < 200.0)
		return 200.0;
	if (zoom < 300.0)
		return 300.0;
	if (zoom < 400.0)
		return 400.0;
	if (zoom < 600.0)
		return 600.0;
	if (zoom < 800.0)
		return 800.0;
	if (zoom < 1600.0)
		return 1600.0;
	return 3200;
}

// _NextZoomOutLevel
float
CanvasView::_NextZoomOutLevel(float zoom) const
{
	if (zoom > 1600.0)
		return 1600.0;
	if (zoom > 800.0)
		return 800.0;
	if (zoom > 600.0)
		return 600.0;
	if (zoom > 400.0)
		return 400.0;
	if (zoom > 300.0)
		return 300.0;
	if (zoom > 200.0)
		return 200.0;
	if (zoom > 150.0)
		return 150.0;
	if (zoom > 100.0)
		return 100.0;
	if (zoom > 66.0)
		return 66.0;
	if (zoom > 50.0)
		return 50.0;
	if (zoom > 33.0)
		return 33.0;
	if (zoom > 25.0)
		return 25.0;
	if (zoom > 12.5)
		return 12.5;
	return 7.5;
}


