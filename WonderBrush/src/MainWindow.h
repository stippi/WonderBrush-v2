// MainWindow.h

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <List.h>
#include <TranslationDefs.h>
#include <TranslatorFormats.h>

#include <MWindow.h>

#include "selected_color_mode.h"

#define CANVAS_LISTVIEW 1

class BetterSpinButton;
class BMenu;
class BMenuItem;
class BMessenger;
class BrushPanel;
class BrushView;
class Canvas;
#if CANVAS_LISTVIEW
class CanvasListView;
#else
class CanvasTabView;
#endif
class CanvasView;
class ColorField;
class ColorPickerPanel;
class ColorSlider;
class DualSlider;
class Exporter;
class ExportPanel;
class FontPopup;
class GradientControl;
class HistoryListView;
class IconButton;
class IconOptionsControl;
class InfoView;
class LabelPopup;
class Layer;
class LayeredGroup;
class LayerConfigView;
class LayersListView;
class MButton;
class MCheckBox;
class MenuBar;
class MPopup;
class MStringView;
class MTextView;
class NavigatorView;
class NumericalTextControl;
class PopupSlider;
class PropertyListView;
class ReportingTextView;
class SettingsPanel;
class StatusBar;
class SwatchView;
class ToggleFullscreenIB;
class VGroup;

enum {
	MSG_NEW						= 'newc',
	MSG_OPEN					= 'open',
	MSG_PRINT					= 'prnt',
	MSG_SETUP_PAGE				= 'supg',
	MSG_SET_SWATCHES			= 'stsw',
	MSG_SAVE_SWATCHES			= 'svsw',
	MSG_DELETE_SWATCHES			= 'dlsw',
	MSG_RENAME_SWATCHES			= 'rnsw',
	MSG_BRUSH_PANEL				= 'brpn',
	MSG_TOOL_TIPS				= 'tips',
	MSG_SET_PAINT_MODE			= 'stpm',
	MSG_SET_SELECTION_MODE		= 'stsm',
	MSG_ACTIVATE_LAYER			= 'actl',
	MSG_DUMP_MISSING_STRINGS	= 'dmms',
};

class MainWindow : public MWindow {
 public:
								MainWindow(BRect frame);
	virtual						~MainWindow();

								// MWindow
	virtual	bool				QuitRequested();
	virtual	void				MessageReceived(BMessage* message);
	virtual	void				Zoom(BPoint origin, float width, float height);

								// MainWindow
			Canvas*				NewDocument(BRect bounds,
											Layer* layer = NULL);

			status_t			Save(Canvas* canvas,
									 const entry_ref* ref,
									 Exporter* exporter = NULL);

			bool				AddDocument(Canvas* canvas);
			bool				RemoveDocument(Canvas* canvas);
			int32				CountDocuments() const;
			void				SetToDocument(Canvas* canvas,
											  bool force = false);
			void				RenameCanvas(Canvas* canvas,
											 const char* name);

			bool				CloseDocument(Canvas* canvas,
											  bool setToNext = true);
			bool				QuitDocuments();

			status_t			SetupPage(Canvas* canvas) const;
			status_t			Print(Canvas* canvas);// const;

			void				UpdateStrings();

			void				SetStatus(const char* message,
										  uint32 warningLevel,
										  bool permanent = false);
			void				SetBusy(bool busy,
										const char* message = NULL);

			void				SetConfirmationEnabled(bool enable) const;

 private:
			MView*				_CreateMenuBar();
			MView*				_CanvasIconView(float width);
			MView*				_ToolsView();
			MView*				_PalettesView();
			MView*				_ToolSetupView();
			MView*				_ConfirmGroup();

			void				_LoadSettings();
			void				_SaveSettings();
			void				_SetTool(uint32 tool);

			void				_BuildSwatchMenu();
			void				_LoadSwatches(const BMessage* message);
			void				_SaveSwatches(BMessage* message,
											  const entry_ref* ref);

			void				_SetBrushFlags(uint32 flags);
			void				_CanvasNameChanged(Canvas* canvas);
			void				_UpdatePermanentStatus();
			void				_RememberPath(const entry_ref* ref, bool save);
			void				_UpdateShapeControls(bool enable);
			void				_UpdatePickControls(bool enable);

	static	int32				_wait_for_file(void* cookie);

	LayeredGroup*				fToolSetupLG;

	CanvasView*					fCanvasView;
	BrushView*					fBrushView;
	InfoView*					fInfoView;
	NavigatorView*				fNavigatorView;
#if CANVAS_LISTVIEW
	CanvasListView*				fCanvasListView;
#else
	CanvasTabView*				fCanvasTabView;
#endif

	MStringView*				fTranslationSV;
	MStringView*				fRotationSV;
	MStringView*				fScaleSV;
	NumericalTextControl*		fXOffsetTC;
	NumericalTextControl*		fYOffsetTC;
	NumericalTextControl*		fRotationTC;
	NumericalTextControl*		fScaleXTC;
	NumericalTextControl*		fScaleYTC;
	MCheckBox*					fMoveSubPixelCB;

	NumericalTextControl*		fCropLeftTC;
	NumericalTextControl*		fCropTopTC;
	NumericalTextControl*		fCropWidthTC;
	NumericalTextControl*		fCropHeightTC;
	MCheckBox*					fCropSubPixelCB;
	MButton*					fCropAllB;

	DualSlider*					fRadiusDS;
	DualSlider*					fAlphaDS;
	DualSlider*					fHardnessDS;
	DualSlider*					fSpacingDS;
	MCheckBox*					fBrushSubPixelCB;
	MCheckBox*					fSolidCB;
	MCheckBox*					fBrushTiltCB;

	DualSlider*					fFillOpacityDS;
	DualSlider*					fToleranceDS;
	DualSlider*					fSoftnessDS;
	MCheckBox*					fFillContiguousCB;

	LabelPopup*					fDropperSizeLP;
	BMenuItem*					fDropperSize1x1MI;
	BMenuItem*					fDropperSize3x3MI;
	BMenuItem*					fDropperSize5x5MI;
	MCheckBox*					fUseAllLayersCB;

	FontPopup*					fFontLP;
	DualSlider*					fFontSizeSB;
	DualSlider*					fFontOpacityDS;
	DualSlider*					fFontAdvanceScaleDS;
	ReportingTextView*			fTextTV;
	IconOptionsControl*			fFontAlignModeIOC;
	IconButton*					fFontAlignLeftIB;
	IconButton*					fFontAlignCenterIB;
	IconButton*					fFontAlignRightIB;
	IconButton*					fFontAlignJustifyIB;

	DualSlider*					fShapeOpacityDS;
	MCheckBox*					fShapeOutlineCB;
	DualSlider*					fShapeOutlineWidthDS;
	MCheckBox*					fShapeClosedCB;
	MCheckBox*					fShapeSubpixelsCB;

	IconOptionsControl*			fShapeCapModeIOC;
	IconButton*					fCapModeButtIB;
	IconButton*					fCapModeSquareIB;
	IconButton*					fCapModeRoundIB;
	IconOptionsControl*			fShapeJoinModeIOC;
	IconButton*					fJoinModeMiterIB;
	IconButton*					fJoinModeRoundIB;
	IconButton*					fJoinModeBevelIB;

	IconButton*					fShapeTransformIB;
	IconButton*					fShapeNewPathIB;
	IconButton*					fShapeRemovePointsIB;
	IconButton*					fShapeReversePathIB;

	DualSlider*					fEllipseOpacityDS;
	MCheckBox*					fEllipseOutlineCB;
	DualSlider*					fEllipseOutlineWidthDS;
	MCheckBox*					fEllipseSubpixelsCB;

	DualSlider*					fRectOpacityDS;
	MCheckBox*					fRectOutlineCB;
	DualSlider*					fRectOutlineWidthDS;
	DualSlider*					fRectCornerRadiusDS;
	MCheckBox*					fRectSubpixelsCB;

	NumericalTextControl*		fSelectLeftTC;
	NumericalTextControl*		fSelectTopTC;
	NumericalTextControl*		fSelectWidthTC;
	NumericalTextControl*		fSelectHeightTC;
	MCheckBox*					fSelectAllLayersCB;
	MButton*					fSelectAllB;

	GradientControl*			fGradientControl;
	MCheckBox*					fGradientInheritsTransformCB;
/*	LabelPopup*					fGradientTypeLP;
	BMenuItem*					fGradientTypeLinearMI;
	BMenuItem*					fGradientTypeCircularMI;
	BMenuItem*					fGradientTypeDiamontMI;
	BMenuItem*					fGradientTypeConicMI;
	BMenuItem*					fGradientTypeXYMI;
	BMenuItem*					fGradientTypeSqrtXYMI;*/
	IconOptionsControl*			fGradientTypeIOC;
	IconButton*					fGradientTypeLinearIB;
	IconButton*					fGradientTypeCircularIB;
	IconButton*					fGradientTypeDiamontIB;
	IconButton*					fGradientTypeConicIB;
	IconButton*					fGradientTypeXYIB;
	IconButton*					fGradientTypeSqrtXYIB;
	DualSlider*					fGradientStopAlphaDS;

	LabelPopup*					fGradientInterpolationLP;
	BMenuItem*					fGradientInterpolationLinearMI;
	BMenuItem*					fGradientInterpolationSmoothMI;

	MCheckBox*					fGuidesShowCB;
	IconButton*					fGuideCreateIB;
	IconButton*					fGuideRemoveIB;
	IconButton*					fGuideProportionIB;
	NumericalTextControl*		fGuideHPosTC;
	NumericalTextControl*		fGuideVPosTC;
	MPopup*						fGuideUnitsLP;
	BMenuItem*					fGuideUnitsPercentMI;
	BMenuItem*					fGuideUnitsPixelsMI;
	NumericalTextControl*		fGuideAngleTC;

	MStringView*				fSelectableSV;
	IconButton*					fPickMaskBrushIB;
	IconButton*					fPickMaskEraserIB;
	IconButton*					fPickMaskPenIB;
	IconButton*					fPickMaskPenEraserIB;
	IconButton*					fPickMaskCloneIB;
	IconButton*					fPickMaskBlurIB;
	IconButton*					fPickMaskFillIB;
	IconButton*					fPickMaskTextIB;
	IconButton*					fPickMaskShapeIB;
	IconButton*					fPickMaskBitmapIB;
	MButton*					fPickMaskAllB;
	MButton*					fPickMaskNoneB;
	MStringView*				fSelectSV;
	MButton*					fPickAllB;
	MButton*					fPickNoneB;

	IconButton*					fNewIB;
	IconButton*					fOpenIB;
	IconButton*					fSaveImageIB;
	IconButton*					fSaveProjectIB;
	IconButton*					fCloseIB;

	IconButton*					fUndoIB;
	IconButton*					fRedoIB;

	IconButton*					fPaintModeIB;
	IconButton*					fSelectModeIB;

	IconButton*					fConfirmIB;
	IconButton*					fCancelIB;

	IconButton*					fBrushIB;
	IconButton*					fCloneBrushIB;
	IconButton*					fPenIB;
	IconButton*					fPenEraserIB;
	IconButton*					fEraserIB;
	IconButton*					fColorPickerIB;
	IconButton*					fTextIB;
	IconButton*					fBlurIB;
	IconButton*					fTranslateIB;
	IconButton*					fCropIB;
	IconButton*					fFillIB;
	IconButton*					fShapeIB;
	IconButton*					fCopyIB;
	IconButton*					fEllipseIB;
	IconButton*					fRoundRectIB;
	IconButton*					fGradientIB;
	IconButton*					fGuidesIB;
	IconButton*					fPickIB;

	IconButton*					fZoomInIB;
	IconButton*					fZoomOutIB;

	IconButton*					fDeleteLayersIB;
	IconButton*					fDeleteObjectsIB;

	ToggleFullscreenIB*			fWindowIB;

	SwatchView*					fCurrentColorSV;
	SwatchView*					fSwatchViews[20];
	ColorField*					fColorField;
	ColorSlider*				fColorSlider;

	VGroup*						fLayerVG;
	VGroup*						fHistoryVG;
	VGroup*						fPropertyVG;

	BMenu*						fFileM;
	BMenu*						fOpenM;
	BMenuItem*					fExportProjectMI;
	BMenuItem*					fExportProjecAstMI;
	BMenuItem*					fSaveProjectMI;
	BMenuItem*					fSaveProjectAsMI;
	BMenuItem*					fSetupPageMI;
	BMenuItem*					fPrintMI;
	BMenuItem*					fAboutMI;
	BMenuItem*					fQuitMI;

	BMenu*						fEditM;
	BMenuItem*					fUndoMI;
	BMenuItem*					fRedoMI;
	BMenuItem*					fPasteMI;

	BMenu*						fSettingsM;
	BMenuItem*					fFullscreenMI;
	BMenuItem*					fShowGridMI;
	BMenuItem*					fToolTipsMI;

	BMenuItem*					fAppSettingsMI;

	BMenu*						fCanvasM;
	BMenuItem*					fNewCanvasMI;
	BMenuItem*					fResizeCanvasMI;
	BMenuItem*					fClearCanvasMI;
	BMenuItem*					fCloseCanvasMI;

	BMenu*						fRotateCanvasM;
	BMenuItem*					fRotateCanvas90MI;
	BMenuItem*					fRotateCanvas180MI;
	BMenuItem*					fRotateCanvas270MI;

	BMenu*						fFormatM;
	BMenu*						fLayerM;
	BMenu*						fModeM;
	BMenu*						fHistoryM;

	BMenu*						fSwatchesM;
	BMenu*						fLoadSwatchesM;
	BMenu*						fSaveSwatchesM;
	BMenuItem*					fSaveSwatchesAsMI;
	BMenu*						fDeleteSwatchesM;

	PopupSlider*				fZoomPS;
	LayersListView*				fLayersListView;
	HistoryListView*			fHistoryListView;
	PropertyListView*			fPropertyListView;
	LayerConfigView*			fLayerConfigView;
	StatusBar*					fStatusBar;

	BMessage*					fSettings;

	bool						fZoomToFullscreen;
	BRect						fNonFullscreenFrame;

	BRect						fColorPickerFrame;
	BRect						fNewCanvasFrame;
	BRect						fResizeCanvasFrame;
	selected_color_mode			fColorPickerMode;
	ColorPickerPanel*			fColorPickerPanel;
	BrushPanel*					fBrushPanel;
	uint32						fRememberedTool;

	BRect						fSettingsPanelFrame;
	SettingsPanel*				fSettingsPanel;

	BList						fDocuments;
	ExportPanel*				fSavePanel;
	entry_ref*					fLastSavePath;
	entry_ref*					fLastExportPath;
	thread_id					fLoadingThread;
};

#endif // MAIN_WINDOW_H
