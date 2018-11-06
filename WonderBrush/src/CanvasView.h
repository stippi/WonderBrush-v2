// CanvasView.h

#ifndef CANVAS_VIEW_H
#define CANVAS_VIEW_H

#include <List.h>
#include <View.h>

#include <layout.h>

#include "defines.h"

#include "ScrollView.h"

class Action;
class BBitmap;
class BMessageRunner;
class BrushView;
class Canvas;
class CanvasViewState;
class ColorpickState;
class CropState;
class EditGradientState;
class EllipseState;
class GuidesState;
class GradientControl;
class History;
class HistoryListView;
class HistoryManager;
class InfoView;
class Layer;
class LayersListView;
class NavigatorView;
class PickObjectsState;
class RoundRectState;
class SlideAction;
class ShapeState;
class SimpleSelectState;
class Stroke;
class StrokeState;
class TransformState;
class TranslateState;
class FillState;
class TextState;

enum {
	MSG_SET_TO_BRUSH					= 'stbr',

	MSG_SET_COLOR						= 'scol',
	MSG_SET_ALPHA						= 'salp',
	MSG_SET_RADIUS						= 'srds',
	MSG_SET_HARDNESS					= 'shrd',
	MSG_SET_SPACING						= 'spcn',
	MSG_SET_SOLID						= 'slid',
	MSG_SET_TILT						= 'tilt',
	MSG_SET_BRUSH_FLAGS					= 'sbfl',

	MSG_CROPPING						= 'crpn',
	MSG_CROP_ENTIRE_CANVAS				= 'crec',

	MSG_SET_FILL_OPACITY				= 'stfo',
	MSG_SET_TOLERANCE					= 'stol',
	MSG_SET_SOFTNESS					= 'ssft',
	MSG_SET_FILL_CONTIGUOUS				= 'sfcn',

	MSG_SELECTING						= 'slct',
	MSG_SELECT_ALL_LAYERS				= 'slal',
	MSG_SELECT_ENTIRE_CANVAS			= 'slec',

	MSG_TRANSFORMATION_CHANGED			= 'trch',
	MSG_TRANSLATE						= 'trns',
	MSG_ROTATE							= 'rota',
	MSG_SCALE							= 'scal',

	MSG_DROPPER_TIP_SIZE				= 'dtsz',
	MSG_USE_ALL_LAYERS					= 'uall',

	MSG_SET_TEXT_SIZE					= 'stts',
	MSG_TEXT_EDITED						= 'txed',
	MSG_SET_TEXT_OPACITY				= 'stto',
	MSG_SET_TEXT_ADVANCE_SCALE			= 'stas',
	MSG_SET_TEXT_ALIGNMENT_MODE			= 'stta',

	MSG_SET_SHAPE_OPACITY				= 'stso',
	MSG_SET_SHAPE_OUTLINE				= 'ssho',
	MSG_SET_SHAPE_OUTLINE_WIDTH			= 'ssow',
	MSG_SHAPE_TRANSFORM					= 'strn',
	MSG_SHAPE_NEW_PATH					= 'shnp',
	MSG_SHAPE_REMOVE_POINTS				= 'shrp',
	MSG_SET_SHAPE_CLOSED				= 'stsc',
	MSG_SET_CAP_MODE					= 'stcm',
	MSG_SET_JOIN_MODE					= 'stjm',
	MSG_UPDATE_SHAPE_UI					= 'shui',
	MSG_REVERSE_PATH					= 'srvp',

	MSG_SET_ELLIPSE_OPACITY				= 'steo',
	MSG_SET_ELLIPSE_OUTLINE				= 'seho',
	MSG_SET_ELLIPSE_OUTLINE_WIDTH		= 'seow',

	MSG_SET_RECT_OPACITY				= 'stro',
	MSG_SET_RECT_OUTLINE				= 'srho',
	MSG_SET_RECT_OUTLINE_WIDTH			= 'srow',
	MSG_SET_RECT_CORNER_RADIUS			= 'srrc',

	MSG_SET_GRADIENT_TYPE				= 'stgt',
	MSG_SET_GRADIENT_INTERPOLATION		= 'stgi',
	MSG_SET_GRADIENT_INHERITS_TRANSFORM	= 'gipt',
	MSG_SET_GRADIENT_STOP_OPACITY		= 'gsto',

	MSG_SHOW_GUIDES						= 'sgde',
	MSG_NEW_GUIDE						= 'ngde',
	MSG_REMOVE_GUIDE					= 'rgde',
	MSG_GUIDE_GOLDEN_PROPORTION			= 'gdgp',
	MSG_GUIDE_SELECTED					= 'gdsl',
	MSG_SET_GUIDE_H_POS					= 'ghps',
	MSG_SET_GUIDE_V_POS					= 'gvps',
	MSG_SET_GUIDE_ANGLE					= 'gdan',
	MSG_SET_GUIDE_UNITS					= 'stgu',

	MSG_PICK_MASK						= 'pmsk',
	MSG_PICK_MASK_ALL					= 'pmal',
	MSG_PICK_MASK_NONE					= 'pmnn',
	MSG_PICK_ALL						= 'pkal',
	MSG_PICK_NONE						= 'pknn',
	MSG_UPDATE_PICK_UI					= 'upcn',

	MSG_APPLY_CURRENT_TOOL				= 'aptl',

	MSG_SET_COLOR_SPACE					= 'stcs',

	MSG_SUB_PIXEL_PRECISION				= 'sbpx',

	MSG_CONTROL_ALPHA					= 'alph',
	MSG_CONTROL_RADIUS					= 'radi',
	MSG_CONTROL_HARDNESS				= 'hard',

	MSG_UNDO							= 'undo',
	MSG_REDO							= 'redo',
	MSG_UPDATE_HISTORY_ITEMS			= 'uphs',

	MSG_SET_TOOL						= 'sttl',
	MSG_SET_LAYER						= 'stly',

	MSG_ZOOM_IN							= 'zmin',
	MSG_ZOOM_OUT						= 'zmot',
	MSG_SET_ZOOM						= 'stzm',

	MSG_SHOW_GRID						= 'shgr',

	MSG_CLEAR_ALL						= 'cler',

	MSG_CONFIRM_TOOL					= 'cftl',
	MSG_CANCEL_TOOL						= 'cntl',

	MSG_REDO_MOUSE_MOVED				= 'rdmm',
};

// TODO: does not belong here...
enum {
	SHAPE_UI_FLAGS_HAS_SELECTION		= 0x0001,
	SHAPE_UI_FLAGS_CAN_CREATE_PATH		= 0x0002,
	SHAPE_UI_FLAGS_IS_CREATING_PATH		= 0x0004,
	SHAPE_UI_FLAGS_CAN_CLOSE_PATH		= 0x0008,
	SHAPE_UI_FLAGS_PATH_IS_CLOSED		= 0x0010,
	SHAPE_UI_FLAGS_IS_TRANSFORMING		= 0x0020,
	SHAPE_UI_FLAGS_IS_OUTLINE			= 0x0040,
	SHAPE_UI_FLAGS_CAN_REVERSE_PATH		= 0x0080,
};

class CanvasView : public MView, public ScrollView {
 public:
								CanvasView(BRect frame, const char* name,
										   BrushView* brushView, InfoView* infoView,
										   HistoryListView* historyListView,
										   LayersListView* layerListView,
										   NavigatorView* navigatorView);
	virtual						~CanvasView();

								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);

								// BView
	virtual	void				AttachedToWindow();
	virtual	void				WindowActivated(bool active);
	virtual	void				FrameResized(float width, float height);
	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
										   const BMessage* dragMessage);
	virtual	void				Draw(BRect updateRect);
	virtual	void				MessageReceived(BMessage* message);
	virtual	void				KeyDown(const char* bytes, int32 numBytes);
	virtual	void				KeyUp(const char* bytes, int32 numBytes);
	virtual	void				Pulse();

#ifdef TARGET_PLATFORM_HAIKU

	virtual	BSize				MinSize();
	virtual	BSize				PreferredSize();
	virtual	BSize				MaxSize();

#endif // TARGET_PLATFORM_HAIKU

								// ScrollView
	virtual	void				ScrollOffsetChanged(BPoint oldOffset,
													BPoint newOffset);
	virtual	void				_Layout(uint32 flags) {}

								// CanvasView
			void				SetTo(Canvas* canvas);
			void				SetToLayer(int32 index);

			void				SetDemoMode(bool demo);
			void				HandleDemoMode(BBitmap* bitmap,
											   BRect area) const;

			Canvas*				CurrentCanvas() const;
			BRect				CanvasFrame() const
									{ return fCanvasFrame; }
			bool				IsTracking() const;
			bool				IsBusy() const
									{ return fBusy; }
			void				EventDropped();

			void				EditModifier(int32 index);
			void				EditModifier(Stroke* modifier);

			void				PickObjects(BPoint where,
											bool extend = false) const;
			void				PickObjects(const BRect& area,
											bool extend = false) const;

			void				PrepareForObjectPropertyChange();
			void				ObjectPropertyChangeDone();

			bool				HandlePaste(BMessage* message);

			void				ModifiersChanged();
			bool				HandleKeyDown(uint32 key, uint32 modifiers);
			bool				HandleKeyUp(uint32 key, uint32 modifiers);
			bool				MouseWheelChanged(float x, float y);

			void				SetBrushRadius(float min, float max);
	inline	range				BrushRadius() const
									{ return fBrushRadius; }
			void				SetBrushHardness(float min, float max);
	inline	range				BrushHardness() const
									{ return fBrushHardness; }
			void				SetMaxAlpha(float min, float max);
	inline	range				BrushAlpha() const
									{ return fBrushAlpha; }
			void				SetBrushFlags(uint32 flags);
	inline	uint32				BrushFlags() const
									{ return fBrushFlags; }
			void				SetBrushSpacing(float min, float max);
	inline	range				BrushSpacing() const
									{ return fBrushSpacing; }
			void				SetColor(rgb_color color);
	inline	rgb_color			Color() const
									{ return fColor; }

			void				SetTolerance(uint8 tolerance);
			uint8				Tolerance() const;
			void				SetSoftness(uint8 softness);
			uint8				Softness() const;
			void				SetFillContiguous(bool contiguous);
			bool				FillContiguous() const;

			void				SetText(const char* text);
			const char*			Text() const;
			void				SetFamilyAndStyle(const char* family,
												  const char* style);
			const char*			FontFamily() const;
			const char*			FontStyle() const;
			void				SetFontSize(float size);
			float				FontSize() const;
			void				SetFontOpacity(uint8 opacity);
			uint8				FontOpacity() const;
			void				SetFontAdvanceScale(float scale);
			float				FontAdvanceScale() const;
			void				SetTextAlignment(uint32 alignment);
			uint32				TextAlignment() const;
			void				StoreNonGUIFontSettings(BMessage* message) const;
			void				RestoreNonGUIFontSettings(const BMessage* message);

			void				SetOpacity(uint8 opacity, uint32 toolID);
			uint8				Opacity(uint32 toolID) const;
			void				SetOutline(bool outline, uint32 toolID);
			bool				Outline(uint32 toolID) const;
			void				SetOutlineWidth(float width, uint32 toolID);
			float				OutlineWidth(uint32 toolID) const;
			void				StoreNonGUIShapeSettings(BMessage* message) const;
			void				RestoreNonGUIShapeSettings(const BMessage* message);

			void				SetRoundCornerRadius(float radius);
			float				RoundCornerRadius() const;

			void				SetShapeCapMode(uint32 mode);
			uint32				ShapeCapMode() const;
			void				SetShapeJoinMode(uint32 mode);
			uint32				ShapeJoinMode() const;
			uint32				ShapeControlFlags() const;

			void				SetDropperTipSize(uint32 size);
			uint32				DropperTipSize() const;
			void				SetDropperUsesAllLayers(bool all);
			bool				DropperUsesAllLayers() const;

			void				SetGradientControl(GradientControl* control);

			void				SetPickMask(uint32 mask);
			uint32				PickMask() const;

			void				SetPrecise(bool enable, uint32 toolID);
			bool				Precise(uint32 toolID) const;
			void				SetShowGrid(bool enable);
			bool				ShowGrid() const
									{ return fShowGrid; }

			void				SetTool(uint32 tool);
	inline	uint32				Tool() const
									{ return fMode; }

			uint32				ColorSpace() const
									{ return fColorSpace; }

			void				Undo();
			void				Redo();

			void				InvalidateCanvas(BRect updateRect,
												 bool deep = false);

			bool				GetCursorPos(BPoint* point) const;
			bool				CursorShowing() const
									{ return fCursorShowing; }

			// this is the function objects which know nothing about
			// redrawing a layer are supposed to call, it will be
			// forwared to the current state
			void				RebuildLayer(Layer* layer, BRect area);
			// these methods actually do the work, only call if you
			// know what you're doing
			void				RebuildBitmap(BRect area, Layer* layer = NULL);
			void				RebuildCanvas(BRect area);
			void				ComposeLayers(BRect area);

			InfoView*			GetInfoView() const
									{ return fInfoView; }
			HistoryListView*	GetHistoryListView() const
									{ return fHistoryListView; }
			LayersListView*		GetLayersListView() const
									{ return fLayersListView; }

	// methods used by history list view
			void				ModifierSelectionChanged(bool itemsSelected);

			void				AddModifiers(const BList& strokes, int32 index);
			void				MoveHistory(BList& items, int32 index);
			void				CopyHistory(BList& items, int32 index);
			void				RemoveHistory(BList& items);
			void				ChangeColors(BList& indices, rgb_color color);

			bool				Perform(Action* action);
			void				UpdateHistoryItems() const;

			void				SetAutoScrolling(bool scroll);

	// added for state support
			void				SwitchState(CanvasViewState* newState);
			void				InitState(CanvasViewState* state);
			void				CleanUpState(CanvasViewState* state);
			CanvasViewState*	StateForCurrentTool() const;


			bool				AddStroke(Stroke* stroke);

			void				ConvertToCanvas(BPoint& where) const;
			void				ConvertFromCanvas(BPoint& where) const;

//			BRect				ConvertToCanvas(const BRect& rect) const;
			BRect				ConvertFromCanvas(const BRect& rect) const;

			BBitmap*			StrokeBitmap() const
									{ return fStrokeBitmap; }
			void				SwitchBuffers(BRect differentArea);
			BBitmap*			DisplayBitmap() const;
			BBitmap*			BackBitmap() const;

			void				_InvalidateCanvas(BRect updateRect,
												  bool compose = false);
			float				ZoomLevel() const
									{ return fZoomLevel / 100.0; }

			void				_UpdateNavigatorView() const;

			void				_SetToColorAtPixel(BPoint canvasWhere,
												   bool useAllLayer = true,
												   uint32 size = 1) const;
			void				_SetBusy(bool busy);

 private:
			void				_Init(bool makeEmpty = true);
			void				_UnInit();

			void				_UpdateInfoView() const;

			bool				_AddStroke(Stroke* stroke);
			void				_AddHistory(Stroke* stroke);
			void				_MakeEmpty();

			bool				_GetTabletInfo(BPoint& where,
											   Point& canvasWhere,
											   bool& eraser,
											   BMessage* message) const;

			void				_ConvertToDisplay(BBitmap* from, BRect area) const;
			void				_ConvertToDisplay(BBitmap* from, BBitmap* into,
												  BRect area) const;
			void				_MakeBottomBitmap() const;
			void				_MakeTopBitmap() const;

 public:
			void				_UpdateToolCursor();
 private:
			void				_DrawCursor(BPoint where, bool invisible = false);
			void				_ClearCursor(BRect* updateRect = NULL);

			void				_ShowPopupMenu(BPoint where);

			BRect				_InnerBounds() const;
			void				_SetDataRect(BRect rect);

			void				_SetZoom(float level, bool force = false,
													  bool ignorBusy = false);
			void				_UpdateCanvasZoomInfo() const;
			float				_NextZoomInLevel(float zoom) const;
			float				_NextZoomOutLevel(float zoom) const;

			// canvas (document) -> current layer -> per layer history
			Canvas*				fCanvas;
			Layer*				fLayer;
			History*			fHistory;

			BRect				fCanvasFrame;
			BRect				fCanvasBounds;

			BPoint				fCursorPosition;
			bool				fCursorShowing;
			bool				fMouseDown;
			bool				fTabletMode;
			bool				fBusy;
			bool				fEventDropped;
			bool				fShowGrid;
			bool				fDemoMode;

			SlideAction*		fSlideAction;

			bool				fSpaceHeldDown;

			bool				fEraser;
			rgb_color			fColor;
			uint32				fColorSpace;
			uint32				fMode;
			uint32				fLastColorMode;

			range				fBrushRadius;
			range				fBrushHardness;
			range				fBrushSpacing;
			range				fBrushAlpha;
			uint32				fBrushFlags;

			float				fZoomLevel;

			// bitmaps
			BBitmap*			fBitmap[2];
			BBitmap*			fDisplayBitmap;
			BBitmap*			fStrokeBitmap;
			BBitmap*			fBottomBitmap;
			BBitmap*			fTopBitmap;
			uint32				fCurrentBitmap;

			// scrolling
			Scrollable*			fScrollable;
			BPoint				fCurrentScrollOffset;
			BPoint				fScrollTrackingStart;
			bool				fScrollTracking;
			bool				fScrollingEnabled;
			bool				fZoomingEnabled;
			bool				fNeedsScrolling;

			BMessageRunner*		fAutoScroller;

			BRect				fDirtyBounds;

			uint32				fPickMask;

			// states
			CanvasViewState*	fCurrentState;
			StrokeState*		fStrokeState;
			ColorpickState*		fColorpickState;
			TransformState*		fTransformState;
			CropState*			fCropState;
			FillState*			fFillState;
			TextState*			fTextState;
			ShapeState*			fShapeState;
			SimpleSelectState*	fSelectState;
			EllipseState*		fEllipseState;
			RoundRectState*		fRoundRectState;
			EditGradientState*	fGradientState;
			GuidesState*		fGuidesState;
			PickObjectsState*	fPickObjectsState;

			uint8*				fGammaTable;

			BrushView*			fBrushView;
			InfoView*			fInfoView;
			HistoryListView*	fHistoryListView;
			LayersListView*		fLayersListView;
			NavigatorView*		fNavigatorView;
};

#endif	// CANVAS_VIEW_H
