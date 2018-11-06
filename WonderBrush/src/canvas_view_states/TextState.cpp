// TextState.cpp

//#include <math.h>
#include <stdio.h>

#include <Bitmap.h>
#include <Font.h>
#include <Cursor.h>
#include <Message.h>
#include <TextView.h>
#include <TextControl.h>
#include <Window.h>

#include "bitmap_support.h"
#include "cursors.h"
#include "defines.h"
#include "support.h"

#include "Canvas.h"
#include "CanvasView.h"
#include "FontManager.h"
#include "History.h"
#include "HistoryListView.h"
#include "HistoryManager.h"
#include "Layer.h"
#include "MainWindow.h"
#include "TextRenderer.h"
#include "TextStroke.h"
#include "ReportingTextView.h"

#include "AddModifierAction.h"
#include "EditModifierAction.h"

#include "ChangeFontAdvanceScaleAction.h"
#include "ChangeFontSizeAction.h"
#include "ChangeTextAlignmentAction.h"
#include "ChangeTextInsetAction.h"
#include "ChangeTextOpacityAction.h"
#include "ChangeTextWidthAction.h"
#include "MoveTextAction.h"
#include "SetColorAction.h"
#include "SetFontAction.h"
#include "SetTextAction.h"

#include "TextState.h"


const char* kDefaultText = "Enter text here.";

#define ACTION_TRIGGER_TIMEOUT 500000
#define CONTROL_POINT_SIZE 3.0

enum {
	DRAG_TEXT = 0,
	DRAG_TEXT_WIDTH,
	DRAG_PARAGRAPH_INSET,
};

// constructor
TextState::TextState(CanvasView* parent)
	: CanvasViewState(parent),
	  fPrivateHistory(new HistoryManager()),
	  fTextStroke(NULL),
	  fEditAction(NULL),
	  fCanceling(false),
	  fLayer(NULL),

	  fText(kDefaultText),
	  fFontSize(12.0 / 16.0),
	  fOpacity(255),
	  fAdvanceScale(1.0),
	  fLineSpacingScale(1.2),
	  fTextWidth(200.0),
	  fAlignment(ALIGN_LEFT),
	  fParagraphInset(0.0),
	  fParagraphSpacing(1.0),
	  fFontFlags(FONT_FLAG_KERNING | FONT_FLAG_HINTING),

	  fDragMode(DRAG_TEXT),

	  fLastTextEditTime(system_time()),
	  fLastNudgeTime(fLastTextEditTime),
	  fLastNudgeRotationTime(fLastTextEditTime),
	  fLastNudgeSizeTime(fLastTextEditTime)
{
	be_plain_font->GetFamilyAndStyle(&fFontFamily, &fFontStyle);
}

// destructor
TextState::~TextState()
{
	delete fPrivateHistory;
}

// Init
void
TextState::Init(Canvas* canvas, Layer* layer, BPoint lastMousePosition)
{
	CanvasViewState::Init(canvas, layer, lastMousePosition);
	fLayer = layer;
	fCanvas = canvas;
	_SetDragMode(DRAG_TEXT);
}

// CleanUp
void
TextState::CleanUp()
{
	_Perform();
	CanvasViewState::CleanUp();
	fLayer = NULL;
	fCanvas = NULL;
}

// EditModifier
bool
TextState::EditModifier(Stroke* modifier)
{
	TextStroke* textModifier = dynamic_cast<TextStroke*>(modifier);
	if (textModifier && textModifier != fTextStroke) {
		_SetModifier(textModifier, true);
		return true;
	}
	return false;
}

// MouseDown
void
TextState::MouseDown(BPoint where, Point canvasWhere, bool eraser)
{
	CanvasViewState::MouseDown(where, canvasWhere, eraser);

	if (fLayer) {
		// start new stroke
		if (!fTextStroke) {
			TextStroke* textStroke = new TextStroke(fCanvasView->Color());
			if (!fCanvasView->AddStroke(textStroke)) {
				delete textStroke;
			} else {
				textStroke->TranslateBy(canvasWhere.point);
				_SetModifier(textStroke);
			}
		}
		if (fTextStroke) {
			fTrackingStart = canvasWhere.point;
			fLastCanvasPos = canvasWhere.point;
			switch (fDragMode) {
				case DRAG_TEXT:
					// move text on layer
					fStartOffset = BPoint(0.0, 0.0);
					fTextStroke->Transform(&fStartOffset);
					break;
				case DRAG_TEXT_WIDTH:
				case DRAG_PARAGRAPH_INSET:
					break;
			}
			fCanvasView->SetAutoScrolling(true);
		}
	}
}

// MouseUp
void
TextState::MouseUp(BPoint where, Point canvasWhere)
{
	CanvasViewState::MouseUp(where, canvasWhere);
	if (fTextStroke) {
		if (fDragMode == DRAG_TEXT) {
			BPoint p(0.0, 0.0);
			fTextStroke->Transform(&p);
			if (fStartOffset != p)
				fCanvasView->Perform(new MoveTextAction(this, fTextStroke, p - fStartOffset));
		}
	}
}

// MouseMoved
void
TextState::MouseMoved(BPoint where, Point canvasWhere,
						uint32 transit, const BMessage* dragMessage)
{
	CanvasViewState::MouseMoved(where, canvasWhere, transit, dragMessage);

	// since the tablet is generating mouse moved messages
	// even if only the pressure changes (and not the actual mouse position)
	// we insert this additional check to prevent too much calculation
	if (fLastCanvasPos != canvasWhere.point) {
		fLastCanvasPos = canvasWhere.point;
		if (fMouseDown && fTextStroke) {
			if (!fCanvasView->IsBusy()) {
				BRect ir(_InvalidationRect());
				switch (fDragMode) {
					case DRAG_TEXT: {
						BRect r(fTextStroke->Bounds());
						fTextStroke->TranslateBy(canvasWhere.point - fTrackingStart);
						fTrackingStart = canvasWhere.point;
						_RedrawStroke(r, ir, true);
						break;
					}
					case DRAG_TEXT_WIDTH: {
						BPoint local = canvasWhere.point;
						fTextStroke->InverseTransform(&local);
						SetTextWidth(max_c(0.0, local.x), true);

						_InvalidateCanvasRect(ir | _InvalidationRect(), false);
						break;
					}
					case DRAG_PARAGRAPH_INSET: {
						BPoint local = canvasWhere.point;
						fTextStroke->InverseTransform(&local);
						SetParagraphInset(local.x, true);

						_InvalidateCanvasRect(ir | _InvalidationRect(), false);
						break;
					}
				}
			} else {
				fCanvasView->EventDropped();
			}
		} else {
			_SetDragMode(_DragModeFor(canvasWhere.point));
		}
	}
}

// ModifierSelectionChanged
void
TextState::ModifierSelectionChanged(bool itemsSelected)
{
	_Perform();
}

// Draw
void
TextState::Draw(BView* into, BRect updateRect)
{
	if (fTextStroke) {
		BPoint start;
		BPoint inset;
		BPoint end;
		_GetTextWidthLine(&start, &inset, &end);

		fCanvasView->ConvertFromCanvas(start);
		fCanvasView->ConvertFromCanvas(inset);
		fCanvasView->ConvertFromCanvas(end);

		into->SetDrawingMode(B_OP_INVERT);
		into->StrokeLine(start, end);

		into->SetDrawingMode(B_OP_COPY);
		BRect r(inset, inset);

		if (inset != end) {
			r.InsetBy(-(CONTROL_POINT_SIZE - 1), -(CONTROL_POINT_SIZE - 1));
			into->SetHighColor(kWhite);
			into->FillEllipse(r);
			r.InsetBy(-1, -1);
			into->SetHighColor(kBlack);
			into->StrokeEllipse(r);
		}

		r = BRect(end, end);
		r.InsetBy(-(CONTROL_POINT_SIZE - 1), -(CONTROL_POINT_SIZE - 1));
		into->SetHighColor(kWhite);
		into->FillRect(r);
		r.InsetBy(-1, -1);
		into->SetHighColor(kBlack);
		into->StrokeRect(r);
	}
}

// MessageReceived
bool
TextState::MessageReceived(BMessage* message)
{
	bool result = true;
	bool sliderAction = message->HasBool("begin");
	switch (message->what) {
		case MSG_SET_FONT: {
			const char* family;
			const char* style;
			if (message->FindString("font family", &family) >= B_OK
				&& message->FindString("font style", &style) >= B_OK) {
				SetFamilyAndStyle(family, style, true, false);
			}
			break;
		}
		case MSG_SET_TEXT_SIZE: {
			float value;
			if (message->FindFloat("max value", &value) >= B_OK) {
				if (!fCanvasView->IsBusy() || sliderAction)
					SetFontSize((value * value) * 50.0, sliderAction, false);
				else
					fCanvasView->EventDropped();
			}
			break;
		}
		case MSG_SET_COLOR: {
			rgb_color color;
			if (restore_color_from_message(message, color) == B_OK) {
				if (!fCanvasView->IsBusy() || sliderAction)
					SetColor(color, sliderAction, false);
				else
					fCanvasView->EventDropped();
			}
			result = false;
			break;
		}
		case MSG_TEXT_EDITED: {
			BView* source;
			if (message->FindPointer("source", (void**)&source) >= B_OK) {
//				if (BTextControl* textView = dynamic_cast<BTextControl*>(source)) {
				if (ReportingTextView* textView = dynamic_cast<ReportingTextView*>(source)) {
					bigtime_t now = system_time();
					bool action = now - fLastTextEditTime > ACTION_TRIGGER_TIMEOUT;
					SetText(textView->Text(), action, false);
					fLastTextEditTime = now;
				}
			}
			break;
		}
		case MSG_SET_TEXT_OPACITY: {
			float value;
			if (message->FindFloat("max value", &value) >= B_OK) {
				if (!fCanvasView->IsBusy() || sliderAction)
					SetOpacity((uint8)floorf(value * 255.0 + 0.5), sliderAction, false);
				else
					fCanvasView->EventDropped();
			}
			break;
		}
		case MSG_SET_TEXT_ADVANCE_SCALE: {
			float value;
			if (message->FindFloat("max value", &value) >= B_OK) {
				if (!fCanvasView->IsBusy() || sliderAction)
					SetAdvanceScale(value, sliderAction, false);
				else
					fCanvasView->EventDropped();
			}
			break;
		}
		case MSG_SET_TEXT_ALIGNMENT_MODE: {
			uint32 alignment;
			if (message->FindInt32("mode", (int32*)&alignment) >= B_OK) {
				SetAlignment(alignment, true, false);
			}
			break;
		}
		case MSG_CONFIRM_TOOL:
			_Perform();
			break;
		case MSG_CANCEL_TOOL:
			_Cancel();
			break;
		default:
			result = false;
			break;
	}
	return result;
}

/*
// ModifiersChanged
void
TextState::ModifiersChanged()
{
	if (fMode == MODE_CLONE) {
		if (modifiers() & B_COMMAND_KEY) {
			fCloneStep = CLONE_NEEDS_OFFSET;
			DrawCursor(fCursor.position);
		}
	}
}
*/
// HandleKeyDown
bool
TextState::HandleKeyDown(uint32 key, uint32 modifiers)
{
	bool result = true;

	bigtime_t now = system_time();
	bool sizeAction = now - fLastNudgeSizeTime > ACTION_TRIGGER_TIMEOUT;

	switch (key) {
		// commit
		case B_RETURN:
			_Perform();
			break;
		// cancel
		case B_ESCAPE:
			_Cancel();
			break;
		// nudging position and rotation
		case B_UP_ARROW:
			if (modifiers & B_COMMAND_KEY) {
				// select previous font in line
				_SetFont(-1);
			} else {
				_Nudge(BPoint(0.0, -1.0));
			}
			break;
		case B_DOWN_ARROW:
			if (modifiers & B_COMMAND_KEY) {
				// select next font in line
				_SetFont(1);
			} else {
				_Nudge(BPoint(0.0, 1.0));
			}
			break;
		case B_LEFT_ARROW:
			if (modifiers & B_COMMAND_KEY) {
/*				// rotation
				if (modifiers & B_SHIFT_KEY)
					_NudgeRotation(0.1);
				else
					_NudgeRotation(5.0);*/
			} else
				_Nudge(BPoint(-1.0, 0.0));
			break;
		case B_RIGHT_ARROW:
			if (modifiers & B_COMMAND_KEY) {
/*				// rotation
				if (modifiers & B_SHIFT_KEY)
					_NudgeRotation(-0.1);
				else
					_NudgeRotation(-5.0);*/
			} else
				_Nudge(BPoint(1.0, 0.0));
			break;
		// keyboard control for text size
		case '+': {
			if (modifiers & B_SHIFT_KEY) {
				SetFontSize(min_c(78.125, fFontSize + 1.0 / 64), sizeAction);
			} else {
				SetFontSize(min_c(78.125, fFontSize * 1.1), sizeAction);
			}
			fLastNudgeSizeTime = now;
			break;
		}
		case '-': {
			if (modifiers & B_SHIFT_KEY) {
				SetFontSize(max_c(0.0, fFontSize - 1.0 / 64), sizeAction);
			} else {
				SetFontSize(max_c(0.0, fFontSize / 1.1), sizeAction);
			}
			fLastNudgeSizeTime = now;
			break;
		}

		default:
			result = false;
	}
	return result;
}
/*
// HandleKeyUp
bool
TextState::HandleKeyUp(uint32 key, uint32 modifiers)
{
	return false;
}
*/
// UpdateToolCursor
void
TextState::UpdateToolCursor()
{
	const unsigned char* cursorData = B_I_BEAM_CURSOR;
	if (fTextStroke) {
		switch (fDragMode) {
			case DRAG_TEXT:
				cursorData = kMoveCursor;
				break;
			case DRAG_TEXT_WIDTH:
			case DRAG_PARAGRAPH_INSET: {
				float rotation = fmod(360.0 - fTextStroke->rotation() * 180.0
					/ M_PI + 22.5, 180.0);
				if (rotation < 45.0) {
					cursorData = kLeftRightCursor;
				} else if (rotation < 90.0) {
					cursorData = kLeftBottomRightTopCursor;
				} else if (rotation < 135.0) {
					cursorData = kUpDownCursor;
				} else {
					cursorData = kLeftTopRightBottomCursor;
				}
				break;
			}
		}
	}

	BCursor cursor(cursorData);
	fCanvasView->SetViewCursor(&cursor, true);
	fCanvasView->Sync();
}

// RebuildLayer
bool
TextState::RebuildLayer(Layer* layer, BRect area) const
{
	if (fLayer && fTextStroke && layer == fLayer) {
//printf("TextState::RebuildLayer()\n");
		_RedrawStroke(area, area, true, true);
		return true;
	}
	return false;
}

// PrepareForObjectPropertyChange
void
TextState::PrepareForObjectPropertyChange()
{
	// just in case the text width changes
	if (fTextStroke)
		_InvalidateCanvasRect(_InvalidationRect(), false);
}

// ObjectChanged
void
TextState::ObjectChanged(const Observable* object)
{
	const TextStroke* textObject = dynamic_cast<const TextStroke*>(object);
	if (textObject && textObject == fTextStroke) {

		// update all controls

		// family & style
		const char* family = fTextStroke->GetTextRenderer()->Family();
		const char* style = fTextStroke->GetTextRenderer()->Style();
		if (family && style) {
			if (strcmp(family, fFontFamily) != 0 || strcmp(style, fFontStyle) != 0) {
				sprintf(fFontFamily, "%s", family);
				sprintf(fFontStyle, "%s", style);
//printf("family & style changed: %s, %s\n", fFontFamily, fFontStyle);

				if (fCanvasView->Window()) {
					BMessage fm(MSG_SET_FONT);
					fm.AddString("family", fFontFamily);
					fm.AddString("style", fFontStyle);
					fCanvasView->Window()->PostMessage(&fm);
				}
			}
		}

		// size
		if (fTextStroke->GetTextRenderer()->PointSize() != fFontSize) {

			fFontSize = fTextStroke->GetTextRenderer()->PointSize();
//printf("size changed: %f\n", fFontSize);

			if (fCanvasView->Window()) {
				BMessage tsm(MSG_SET_TEXT_SIZE);
				tsm.AddFloat("value", sqrtf(fFontSize / 50.0));
				fCanvasView->Window()->PostMessage(&tsm);
			}
		}

		// text
		const char* text = fTextStroke->GetTextRenderer()->Text();
		if (text && strcmp(fText.String(), text) != 0) {
			fText.SetTo(text);
//printf("text changed: %s\n", fText.String());

			if (fCanvasView->Window()) {
				BMessage tm(MSG_TEXT_EDITED);
				tm.AddString("text", fText.String());
				fCanvasView->Window()->PostMessage(&tm);
			}
		}

		// opacity
		if (fTextStroke->GetTextRenderer()->Opacity() != fOpacity) {

			fOpacity = fTextStroke->GetTextRenderer()->Opacity();
//printf("opacity changed: %d\n", fOpacity);

			if (fCanvasView->Window()) {
				BMessage om(MSG_SET_TEXT_OPACITY);
				om.AddFloat("value", fOpacity / 255.0);
				fCanvasView->Window()->PostMessage(&om);
			}
		}

		// advance scale
		if (fTextStroke->AdvanceScale() != fAdvanceScale) {

			fAdvanceScale = fTextStroke->AdvanceScale();
//printf("advance scale changed: %f\n", fAdvanceScale);

			if (fCanvasView->Window()) {
				BMessage am(MSG_SET_TEXT_ADVANCE_SCALE);
				am.AddFloat("value", fAdvanceScale / 2.0);
				fCanvasView->Window()->PostMessage(&am);
			}
		}

		// line spacing scale
		if (fTextStroke->LineSpacingScale() != fLineSpacingScale) {

			fLineSpacingScale = fTextStroke->LineSpacingScale();

/*			if (fCanvasView->Window()) {
				BMessage am(MSG_SET_TEXT_ADVANCE_SCALE);
				am.AddFloat("value", fAdvanceScale / 2.0);
				fCanvasView->Window()->PostMessage(&am);
			}*/
		}

		// color
		if (fColor.red != fTextStroke->Color().red ||
			fColor.green != fTextStroke->Color().green ||
			fColor.blue != fTextStroke->Color().blue) {

			fColor = fTextStroke->Color();

			if (fCanvasView->Window()) {
				BMessage cm(MSG_SET_COLOR);
				store_color_in_message(&cm, fTextStroke->Color());
				fCanvasView->Window()->PostMessage(&cm);
			}
			// update list view
			if (fLayer && fTextStroke) {
				History* history = fLayer->GetHistory();
				HistoryListView* listView = fCanvasView->GetHistoryListView();
				if (history && listView) {
					int32 index = history->IndexOf(fTextStroke);
					listView->SetPainter(index, fTextStroke->Painter());
				}
			}
		}
		// text width
		if (fTextWidth != fTextStroke->TextWidth()) {
			fTextWidth = fTextStroke->TextWidth();
			_InvalidateCanvasRect(_InvalidationRect(), false);
		}
		// text alignment
		if (fAlignment != fTextStroke->Alignment()) {
			fAlignment = fTextStroke->Alignment();

			if (fCanvasView->Window()) {
				BMessage am(MSG_SET_TEXT_ALIGNMENT_MODE);
				am.AddInt32("mode", fAlignment);
				fCanvasView->Window()->PostMessage(&am);
			}
		}
		// paragraph inset
		if (fParagraphInset != fTextStroke->ParagraphInset()) {
			fParagraphInset = fTextStroke->ParagraphInset();
			_InvalidateCanvasRect(_InvalidationRect(), false);
		}
		// paragraph spacing
//		if (fParagraphSpacing != fTextStroke->ParagraphSpacing()) {
			fParagraphSpacing = fTextStroke->ParagraphSpacing();
//		}
		// font flags
//		if (fFontFlags != fTextStroke->Flags()) {
			fFontFlags = fTextStroke->Flags();
//		}
	}
}

// StoreNonGUISettings
void
TextState::StoreNonGUISettings(BMessage* message) const
{
	if (message) {
		if (message->ReplaceFloat("text line spacing", fLineSpacingScale) < B_OK)
			message->AddFloat("text line spacing", fLineSpacingScale);
		if (message->ReplaceFloat("text width", fTextWidth) < B_OK)
			message->AddFloat("text width", fTextWidth);
		if (message->ReplaceFloat("text paragraph inset", fParagraphInset) < B_OK)
			message->AddFloat("text paragraph inset", fParagraphInset);
		if (message->ReplaceFloat("text paragraph spacing", fParagraphSpacing) < B_OK)
			message->AddFloat("text paragraph spacing", fParagraphSpacing);
		if (message->ReplaceInt32("text font flags", fFontFlags) < B_OK)
			message->AddInt32("text font flags", fFontFlags);
	}
}

// RestoreNonGUISettings
void
TextState::RestoreNonGUISettings(const BMessage* message)
{
	if (message) {
		if (message->FindFloat("text line spacing", &fLineSpacingScale) < B_OK)
			fLineSpacingScale = 1.2;
		if (message->FindFloat("text width", &fTextWidth) < B_OK)
			fTextWidth = 200.0;
		if (message->FindFloat("text paragraph inset", &fParagraphInset) < B_OK)
			fParagraphInset = 0.0;
		if (message->FindFloat("text paragraph spacing", &fParagraphSpacing) < B_OK)
			fParagraphSpacing = 1.0;
		if (message->FindInt32("text font flags", (int32*)&fFontFlags) < B_OK)
			fFontFlags = FONT_FLAG_KERNING | FONT_FLAG_HINTING;
	}
}

// SetText
void
TextState::SetText(const char* text, bool action, bool notify)
{
	if (text) {
		if (strcmp(fText.String(), text) != 0) {
			if (!fTextStroke || !notify) {
				fText.SetTo(text);
			}
			if (fTextStroke) {
				if (action)
					fCanvasView->Perform(new SetTextAction(this, fTextStroke));
	
				// handle invalidation
				BRect r(fTextStroke->Bounds());
				BRect ir(_InvalidationRect());
				fTextStroke->SetText(text);
				_RedrawStroke(r, ir, true, true);
			}
		}
	}
}

// SetFamilyAndStyle
void
TextState::SetFamilyAndStyle(const char* family, const char* style,
							 bool action, bool notify)
{
	if (family && style) {
		if (strcmp(fFontFamily, family) != 0 ||
			strcmp(fFontStyle, style) != 0) {
			if (!fTextStroke || !notify) {
				sprintf(fFontFamily, "%s", family);
				sprintf(fFontStyle, "%s", style);
				// the font can be keyboard controlled, so
				// we need to sync the GUI
				if (notify) {
					if (fCanvasView->Window()) {
						BMessage fm(MSG_SET_FONT);
						fm.AddString("family", fFontFamily);
						fm.AddString("style", fFontStyle);
						fCanvasView->Window()->PostMessage(&fm);
					}
				}
			}
			if (fTextStroke) {
				if (action)
					fCanvasView->Perform(new SetFontAction(this, fTextStroke));
	
				// handle invalidation
				BRect r(fTextStroke->Bounds());
				BRect ir(_InvalidationRect());
				fTextStroke->SetFamilyAndStyle(family, style);
				_RedrawStroke(r, ir, true, true);
			}
		}
	}
}

// SetSomething
template <class V, class F>
void
TextState::SetSomething(V value, F func, bool ignorBusy)
{
	// handle invalidation
	if (fTextStroke) {
		BRect r(fTextStroke->Bounds());
		BRect ir(_InvalidationRect());
		(fTextStroke->*func)(value);
		_RedrawStroke(r, ir, true, ignorBusy);
	}
}

// SetFontSize
void
TextState::SetFontSize(float size, bool action, bool notify)
{
	if (fFontSize != size) {
		if (!fTextStroke || !notify) {
			fFontSize = size;
			if (notify) {
				// the size can be keyboard controlled, so
				// we need to sync the GUI
				if (fCanvasView->Window()) {
					BMessage tsm(MSG_SET_TEXT_SIZE);
					tsm.AddFloat("value", sqrtf(fFontSize / 50.0));
					fCanvasView->Window()->PostMessage(&tsm);
				}
			}
		}
		if (fTextStroke) {
			if (action)
				fCanvasView->Perform(new ChangeFontSizeAction(this, fTextStroke));
	
			SetSomething(size, &TextStroke::SetSize, notify);
		}
	}
}

// SetOpacity
void
TextState::SetOpacity(uint8 opacity, bool action, bool notify)
{
	if (fOpacity != opacity) {
		if (!fTextStroke || !notify) {
			fOpacity = opacity;
		}
		if (fTextStroke) {
			if (action)
				fCanvasView->Perform(new ChangeTextOpacityAction(this, fTextStroke));
	
			SetSomething(opacity, &TextStroke::SetOpacity, notify);
		}
	}
}

// SetAdvanceScale
void
TextState::SetAdvanceScale(float scale, bool action, bool notify)
{
	scale *= 2.0;
	if (fAdvanceScale != scale) {
		if (!fTextStroke || !notify) {
			fAdvanceScale = scale;
		}
		if (fTextStroke) {
			if (action)
				fCanvasView->Perform(new ChangeFontAdvanceScaleAction(this, fTextStroke));
	
			SetSomething(scale, &TextStroke::SetAdvanceScale, notify);
		}
	}
}

// SetLineSpacingScale
void
TextState::SetLineSpacingScale(float scale, bool action, bool notify)
{
	if (fLineSpacingScale != scale) {
		if (!fTextStroke || !notify) {
			fLineSpacingScale = scale;
		}
		if (fTextStroke) {
//			if (action)
//				fCanvasView->Perform(new ChangeFontAdvanceScaleAction(this, fTextStroke));
	
			SetSomething(scale, &TextStroke::SetLineSpacingScale, notify);
		}
	}
}

// SetParagraphInset
void
TextState::SetParagraphInset(float inset, bool action, bool notify)
{
	if (fParagraphInset != inset) {
		if (!fTextStroke || !notify) {
			fParagraphInset = inset;
		}
		if (fTextStroke) {
			if (action)
				fCanvasView->Perform(new ChangeTextInsetAction(this, fTextStroke));
	
			SetSomething(inset, &TextStroke::SetParagraphInset, notify);
		}
	}
}

// SetTextWidth
void
TextState::SetTextWidth(float width, bool action, bool notify)
{
	if (fTextWidth != width) {
		if (!fTextStroke || !notify) {
			fTextWidth = width;
		}
		if (fTextStroke) {
			if (action)
				fCanvasView->Perform(new ChangeTextWidthAction(this, fTextStroke));
	
			SetSomething(width, &TextStroke::SetTextWidth, notify);
		}
	}
}

// SetColor
void
TextState::SetColor(rgb_color color, bool action, bool notify)
{
	if (fColor.red != color.red ||
		fColor.green != color.green ||
		fColor.blue != color.blue) {

		if (!fTextStroke || !notify) {
			fColor = color;
		}

		if (fTextStroke) {
			if (action)
				fCanvasView->Perform(new SetColorAction(this, fTextStroke));

			// handle invalidation
			BRect r(fTextStroke->Bounds());
			BRect ir(_InvalidationRect());
			fTextStroke->SetColor(color);
			_RedrawStroke(r, ir, true, notify);

			// update list view
			if (fLayer && fTextStroke) {
				History* history = fLayer->GetHistory();
				HistoryListView* listView = fCanvasView->GetHistoryListView();
				if (history && listView) {
					int32 index = history->IndexOf(fTextStroke);
					listView->SetPainter(index, fTextStroke->Painter());
				}
			}
		}
	}
}

// SetAlignment
void
TextState::SetAlignment(uint32 alignment, bool action, bool notify)
{
	if (fAlignment != alignment) {
		if (!fTextStroke || !notify) {
			fAlignment = alignment;
		}
		if (fTextStroke) {
			if (action)
				fCanvasView->Perform(new ChangeTextAlignmentAction(this, fTextStroke));
	
			SetSomething(alignment, &TextStroke::SetAlignment, notify);
		}
	}
}

// _SetModifier
void
TextState::_SetModifier(TextStroke* modifier, bool edit)
{
	if (fTextStroke)
		_Perform();
	if (modifier) {
		fTextStroke = modifier;
		// set the shape stroke focused in the history list
		HistoryListView* listView = fCanvasView->GetHistoryListView();
		History* history = fLayer->GetHistory();
		if (history && listView) {
			listView->SetItemFocused(history->IndexOf(fTextStroke));
		}
		if (fCanvas) {
			fCanvas->SetTemporaryHistory(fPrivateHistory);
			fCanvasView->UpdateHistoryItems();
		}

		if (edit)
			fEditAction = new EditModifierAction(fLayer, fTextStroke);
		if (fEditAction) {
			// customize ourself to the text stroke
			// and trigger the notifications
			ObjectChanged(fTextStroke);

			_InvalidateCanvasRect(_InvalidationRect(), false);
		} else {
			// customize the text stroke
			fTextStroke->SetSize(fFontSize);
			fTextStroke->SetText(fText.String());
			fTextStroke->SetFamilyAndStyle(fFontFamily, fFontStyle);
			fTextStroke->SetOpacity(fOpacity);
			fTextStroke->SetAdvanceScale(fAdvanceScale);
			fTextStroke->SetLineSpacingScale(fLineSpacingScale);
			fTextStroke->SetTextWidth(fTextWidth);
			fTextStroke->SetAlignment(fAlignment);
			fTextStroke->SetParagraphInset(fParagraphInset);
			fTextStroke->SetParagraphSpacing(fParagraphSpacing);
			fTextStroke->SetFlags(fFontFlags);

			_RedrawStroke(fTextStroke->Bounds(),
						  _InvalidationRect(), true);
		}
		fTextStroke->AddObserver(this);

		_SetDragMode(_DragModeFor(fLastCanvasPos));
		_SetConfirmationEnabled(true);
	}
	UpdateToolCursor();
}

// _SetDragMode
void
TextState::_SetDragMode(uint32 mode)
{
	if (fDragMode != mode) {
		fDragMode = mode;
		UpdateToolCursor();
	}
}

// _DragModeFor
uint32
TextState::_DragModeFor(BPoint canvasWhere) const
{
	uint32 mode = DRAG_TEXT;
	if (fTextStroke) {
		BPoint start;
		BPoint inset;
		BPoint end;
		_GetTextWidthLine(&start, &inset, &end);

		// test for text width handle
		fCanvasView->ConvertFromCanvas(canvasWhere);
		fCanvasView->ConvertFromCanvas(end);
		fCanvasView->ConvertFromCanvas(inset);

		if (dist(canvasWhere, end) <= 7.0)
			mode = DRAG_TEXT_WIDTH;
		else if (dist(canvasWhere, inset) <= 7.0)
			mode = DRAG_PARAGRAPH_INSET;
	}
	return mode;
}

// _LayerBounds
BRect
TextState::_LayerBounds() const
{
	BRect r(0.0, 0.0, -1.0, -1.0);
	if (fLayer) {
		if (BBitmap* bitmap = fLayer->Bitmap())
			r = bitmap->Bounds();
	}
	return r;
}

// _RedrawStroke
void
TextState::_RedrawStroke(BRect oldStrokeBounds, BRect oldRedrawBounds,
						 bool forceStrokeDrawing, bool ignorBusy) const
{
	if (fTextStroke && fLayer) {
		if (!fCanvasView->IsBusy() || ignorBusy) {

			fCanvasView->_SetBusy(true);

			if (fEditAction) {
				BRect r(fTextStroke->Bounds());
				fLayer->Touch(r);

				r = r | oldStrokeBounds;

				fCanvasView->RebuildBitmap(r, fLayer);
				_InvalidateCanvasRect(oldRedrawBounds | _InvalidationRect(), false);
			} else {
				BRect r(fTextStroke->Bounds());
				fLayer->Touch(r);
	
				clear_area(fCanvasView->StrokeBitmap(), oldStrokeBounds);
				fTextStroke->Draw(fCanvasView->StrokeBitmap(), r);
	
				r = r | oldStrokeBounds;
		
				if (forceStrokeDrawing)
					_ApplyStroke(fCanvasView->BackBitmap(),
								 fLayer->Bitmap(), r, fTextStroke);
	
				fCanvasView->_InvalidateCanvas(r, true);
				_InvalidateCanvasRect(oldRedrawBounds | _InvalidationRect(), false);
			}
		} else {
			fCanvasView->EventDropped();
		}
	}
}

// _Perform
void
TextState::_Perform()
{
	if (!fCanceling) {
		if (fTextStroke) {
			fTextStroke->RemoveObserver(this);
			_InvalidateCanvasRect(_InvalidationRect(), false);
			// remove private history from canvas
			if (fCanvas) {
				fCanvas->SetTemporaryHistory(NULL);
				fCanvasView->UpdateHistoryItems();
			}
			if (fEditAction) {
				if (fPrivateHistory->IsSaved()) {
					// this means the stroke has not been edited at all
					// we don't need to perform the edit action in this case
					delete fEditAction;
				} else {
					// we will want to be able to undo the changes
					fCanvasView->Perform(fEditAction);
				}
				fEditAction = NULL;
			} else {
				// perform and cause the canvas view to switch buffers
				if (!fCanvasView->Perform(new AddModifierAction(fLayer, fTextStroke))) {
					delete fTextStroke;
				} else {
					BRect r(fTextStroke->Bounds());
					clear_area(fCanvasView->StrokeBitmap(), r);
					fCanvasView->SwitchBuffers(r);
					fCanvasView->_InvalidateCanvas(r);
				}
			}
			// history needs to be empty
			fPrivateHistory->Clear();
			// unfocus the text stroke item (we're now done editing it)
			HistoryListView* listView = fCanvasView->GetHistoryListView();
			if (listView)
				listView->SetItemFocused(-1);
			// this is not ours anymore
			fTextStroke = NULL;
			_SetConfirmationEnabled(false);
		}
		UpdateToolCursor();
	}
}

// _Cancel
void
TextState::_Cancel()
{
	fCanceling = true;
	// clean up
	if (fTextStroke) {
		fTextStroke->RemoveObserver(this);
		_InvalidateCanvasRect(_InvalidationRect(), false);
		// unfocus the shape modifier item, remove it
		// remove modifier from layers history
		HistoryListView* listView = fCanvasView->GetHistoryListView();
		History* history = fLayer->GetHistory();
		if (history && listView) {
			listView->SetItemFocused(-1);
			if (fEditAction) {
				// we have only edited this modifier
				// undo all the changes
				fEditAction->Undo(fCanvasView);
				delete fEditAction;
				fEditAction = NULL;
			} else {
				// we have added this modifier and need to get rid of it
				delete listView->RemoveItem(history->IndexOf(fTextStroke));
				history->RemoveItem(fTextStroke);

				// clear the stroke bitmap area, restore the layer bitmap
				BRect r(fTextStroke->Bounds());
				clear_area(fCanvasView->StrokeBitmap(), r);
				copy_area(fCanvasView->BackBitmap(),
						  fLayer->Bitmap(), r);
				fCanvasView->_InvalidateCanvas(r, true);

				delete fTextStroke;
			}
		}
		fTextStroke = NULL;
		if (fCanvas) {
			fCanvas->SetTemporaryHistory(NULL);
			fCanvasView->UpdateHistoryItems();
		}
		fPrivateHistory->Clear();
	}
	_SetConfirmationEnabled(false);
	UpdateToolCursor();
	fCanceling = false;
}

// _SetConfirmationEnabled
void
TextState::_SetConfirmationEnabled(bool enable) const
{
	if (MainWindow* window = dynamic_cast<MainWindow*>(fCanvasView->Window()))
		window->SetConfirmationEnabled(enable);
}

// _GetTextWidthLine
void
TextState::_GetTextWidthLine(BPoint* start, BPoint* inset, BPoint* end) const
{
	if (fTextStroke) {
		float top = -fTextStroke->GetTextRenderer()->PointSize() * 16.0;
//		top *= fTextStroke->LineSpacingScale();
		start->x = 0.0;
		start->y = top;
		inset->x = fTextStroke->ParagraphInset();
		inset->y = top;
		end->x = fTextStroke->TextWidth();
		end->y = top;
		fTextStroke->Transform(start);
		fTextStroke->Transform(inset);
		fTextStroke->Transform(end);
	}
}

// _InvalidationRect
BRect
TextState::_InvalidationRect() const
{
	BRect r(0.0, 0.0, -1.0, -1.0);
	if (fTextStroke) {
		BPoint lt;
		BPoint pi;
		BPoint rt;
		_GetTextWidthLine(&lt, &pi, &rt);
		r.Set(floorf(min_c(pi.x, min_c(lt.x, rt.x))),
			  floorf(min_c(pi.y, min_c(lt.y, rt.y))),
			  ceilf(max_c(pi.x, max_c(lt.x, rt.x))),
			  ceilf(max_c(pi.y, max_c(lt.y, rt.y))));
		float inset = roundf(min_c(-CONTROL_POINT_SIZE, -CONTROL_POINT_SIZE / fCanvasView->ZoomLevel()));
		r.InsetBy(inset, inset);
	}
	return r;
}

// _Nudge
void
TextState::_Nudge(BPoint offset)
{
	if (fTextStroke && !fCanvasView->IsBusy()) {
		bigtime_t now = system_time();
		bool action = now - fLastNudgeTime > ACTION_TRIGGER_TIMEOUT;

		SetSomething(offset, &TextStroke::TranslateBy, true);

		if (action)
			fCanvasView->Perform(new MoveTextAction(this, fTextStroke,
													BPoint(-offset.x, -offset.y)));

		fLastNudgeTime = now;
	}
}
/*
// _NudgeRotation
void
TextState::_NudgeRotation(float diff)
{
	float rotation = fAdvanceScale + diff;

	if (rotation < 0.0)
		rotation = rotation + 360.0;
	if (rotation > 360.0)
		rotation = rotation - 360.0;

	bigtime_t now = system_time();
	bool action = now - fLastNudgeRotationTime > ACTION_TRIGGER_TIMEOUT;

	SetRotation(rotation, action);

	fLastNudgeRotationTime = now;
}
*/

// _SetFont
void
TextState::_SetFont(int32 indexOffset)
{
	FontManager* manager = FontManager::Default();
	int32 index = manager->IndexFor(fFontFamily, fFontStyle);
	index += indexOffset;
	// wrap around
	int32 count = manager->CountFontFiles();
	if (index >= count)
		index = 0;
	if (index < 0)
		index = count - 1;
	if (const entry_ref* ref = manager->FontFileAt(index)) {
		const char* family = manager->FamilyFor(ref);
		const char* style = manager->StyleFor(ref);
		SetFamilyAndStyle(family, style, true);
	}
}


