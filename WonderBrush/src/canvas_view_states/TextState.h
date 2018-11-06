// TextState.h

#ifndef TEXT_STATE_H
#define TEXT_STATE_H

#include <String.h>

#include "defines.h"

#include "CanvasView.h"
#include "CanvasViewState.h"
#include "ObjectObserver.h"

class EditModifierAction;
class HistoryManager;
class Stroke;
class TextAction;
class TextStroke;

class TextState : public CanvasViewState,
				  public ObjectObserver {
 public:
								TextState(CanvasView* parent);
	virtual						~TextState();

								// CanvasViewState
	virtual	void				Init(Canvas* canvas,
									 Layer* layer,
									 BPoint currentMousePos);
	virtual	void				CleanUp();

	virtual	bool				EditModifier(Stroke* modifier);

	virtual	void				MouseDown(BPoint where, Point canvasWhere,
										  bool eraser = false);
	virtual	void				MouseUp(BPoint where, Point canvasWhere);
	virtual	void				MouseMoved(BPoint where, Point canvasWhere,
										   uint32 transit,
										   const BMessage* dragMessage);

	virtual	void				ModifierSelectionChanged(bool itemsSelected);

	virtual	void				Draw(BView* into, BRect updateRect);
	virtual	bool				MessageReceived(BMessage* message);

//	virtual	void				ModifiersChanged();
	virtual	bool				HandleKeyDown(uint32 key, uint32 modifiers);
//	virtual	bool				HandleKeyUp(uint32 key, uint32 modifiers);

	virtual	void				UpdateToolCursor();

	virtual	bool				RebuildLayer(Layer* layer, BRect area) const;

	virtual	void				PrepareForObjectPropertyChange();

								// ObjectObserver
	virtual	void				ObjectChanged(const Observable* object);

								// TextState
			void				StoreNonGUISettings(BMessage* message) const;
			void				RestoreNonGUISettings(const BMessage* message);

			void				SetText(const char* text, bool action = false,
														  bool notify = true);
			const char*			Text() const
									{ return fText.String(); }

			void				SetFamilyAndStyle(const char* family,
												  const char* style,
												  bool action = false,
												  bool notify = true);
			const char*			FontFamily() const
									{ return fFontFamily; }
			const char*			FontStyle() const
									{ return fFontStyle; }


			template<class V, class F>
			void				SetSomething(V value, F func,
											 bool ignorBusy = false);


			void				SetFontSize(float size, bool action = false,
														bool notify = true);
			float				FontSize() const
									{ return fFontSize; }

			void				SetOpacity(uint8 opacity, bool action = false,
														  bool notify = true);
			uint8				Opacity() const
									{ return fOpacity; }

			void				SetAdvanceScale(float scale, bool action = false,
															 bool notify = true);
			float				AdvanceScale() const
									{ return fAdvanceScale / 2.0; }

			void				SetLineSpacingScale(float scale, bool action = false,
																 bool notify = true);
			float				LineSpacingScale() const
									{ return fLineSpacingScale; }

			void				SetTextWidth(float width, bool action = false,
														  bool notify = true);
			float				TextWidth() const
									{ return fTextWidth; }

			void				SetParagraphInset(float inset, bool action = false,
															   bool notify = true);
			float				ParagraphInset() const
									{ return fParagraphInset; }

			void				SetColor(rgb_color color, bool action = false,
														  bool notify = true);

			void				SetAlignment(uint32 aligment, bool action = false,
															  bool notify = true);
			uint32				Alignment() const
									{ return fAlignment; }


 private:
			friend	class		TextAction;

			void				_SetModifier(TextStroke* modifier, bool edit = false);

			void				_SetDragMode(uint32 mode);
			uint32				_DragModeFor(BPoint canvasWhere) const;

			BRect				_LayerBounds() const;

			void				_RedrawStroke(BRect oldStrokeBounds,
											  BRect oldRedrawBounds,
											  bool forceStrokeDrawing = false,
											  bool ignorBusy = false) const;

			void				_Perform();
			void				_Cancel();
			void				_SetConfirmationEnabled(bool enable) const;

			void				_GetTextWidthLine(BPoint* start,
												  BPoint* inset,
												  BPoint* end) const;
			BRect				_InvalidationRect() const;

			void				_Nudge(BPoint point);
//			void				_NudgeRotation(float diff);
			void				_SetFont(int32 indexOffset);

			HistoryManager*		fPrivateHistory;
			Canvas*				fCanvas;

			TextStroke*			fTextStroke;
			EditModifierAction*	fEditAction;
			bool				fCanceling;

			Layer*				fLayer;

			BString				fText;

			font_family			fFontFamily;
			font_style			fFontStyle;

			float				fFontSize;
			uint8				fOpacity;
			float				fAdvanceScale;
			float				fLineSpacingScale;
			rgb_color			fColor;
			float				fTextWidth;
			uint32				fAlignment;
			float				fParagraphInset;
			float				fParagraphSpacing;
			uint32				fFontFlags;

			uint32				fDragMode;

			BPoint				fTrackingStart;
			BPoint				fLastCanvasPos;
			BPoint				fStartOffset;

			bigtime_t			fLastTextEditTime;
			bigtime_t			fLastNudgeTime;
			bigtime_t			fLastNudgeRotationTime;
			bigtime_t			fLastNudgeSizeTime;
};

#endif	// TEXT_STATE_H
