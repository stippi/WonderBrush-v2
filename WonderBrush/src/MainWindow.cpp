// MainWindow.cpp

#include <stdio.h>

#include <fs_attr.h>

#include <Alert.h>
#include <Application.h>
#include <Bitmap.h>
#include <BitmapStream.h>
#include <Clipboard.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Menu.h>
#include <MenuItem.h>
#include <NodeInfo.h>
#include <NodeMonitor.h>
#include <Path.h>
#include <PrintJob.h>
#include <PopUpMenu.h>
#include <RecentItems.h>
#include <Roster.h>
#include <Screen.h>
#include <ScrollView.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>
#include <Volume.h>

#include <LayeredGroup.h>
#include <MBorder.h>
#include <MButton.h>
#include <MBViewWrapper.h>
#include <MMenuBar.h>
#include <MPopup.h>
#include <MStringView.h>
#include <HGroup.h>
#include <Space.h>
#include <VGroup.h>

#include "bitmap_support.h"
#include "load.h"
#include "rgb_hsv.h"
#include "support.h"

#include "AddLayerAction.h"
#include "BBP.h"
#include "BetterMCheckBox.h"
#include "BetterMScrollView.h"
#include "BetterMSplitter.h"
#include "BetterSpinButton.h"
#include "BitmapStroke.h"
#include "Brush.h"
#include "BrushConfigView.h"
#include "BrushPanel.h"
#include "BrushView.h"
#include "BubbleHelper.h"
#include "Canvas.h"
#if CANVAS_LISTVIEW
#include "CanvasTabView.h"
#else
#include "CanvasListView.h"
#endif
#include "CanvasView.h"
#include "ColorField.h"
#include "ColorPickerPanel.h"
#include "ColorSlider.h"
#include "DualSlider.h"
#include "DualSliderInputGroup.h"
#include "Exporter.h"
#include "FontManager.h"
#include "FontPopup.h"
#include "GlobalFunctions.h"
#include "GlobalSettings.h"
#include "Gradient.h"
#include "GradientControl.h"
#include "History.h"
#include "HistoryListView.h"
#include "HistoryManager.h"
#include "FramedIconButton.h"
#include "IconButton.h"
#include "IconOptionsControl.h"
#include "Icons.h"
#include "InfoView.h"
#include "Keyfile.h"
#include "LabelPopup.h"
#include "LanguageManager.h"
#include "Layer.h"
#include "LayerConfigView.h"
#include "LayersListView.h"
#include "LicenceeInfo.h"
#include "ListViews.h"
#include "MenuBar.h"
#include "MTextView.h"
#include "NamePanel.h"
#include "NavigatorView.h"
#include "NewCanvasPanel.h"
#include "NumericalTextControl.h"
#include "PopupSlider.h"
#include "PropertyListView.h"
#include "ReportingTextView.h"
#include "ResizeAction.h"
#include "RotateAction.h"
#include "NewSizePanel.h"
#include "Seperator.h"
#include "SettingsPanel.h"
#include "StatusBar.h"
#include "SwatchView.h"
#include "ToggleFullscreenIB.h"
#include "ExportPanel.h"
#include "YBMScrollView.h"

#include "MainWindow.h"

enum {
	MSG_EXPORT					= 'xprt',
	MSG_EXPORT_AS				= 'xpas',

	MSG_SAVE_DOCUMENT			= 'svdc',
	MSG_SAVE_DOCUMENT_AS		= 'sadc',

	MSG_GO_FULL_SCREEN			= 'fscr',

	MSG_GET_COLOR				= 'gtcl',

	MSG_SET_CANVAS				= 'stcv',
	MSG_CLOSE_CANVAS			= 'clcv',
	MSG_RESIZE_CANVAS			= 'rscv',
	MSG_ROTATE_CANVAS			= 'rtcv',

	MSG_GLOBAL_SETTINGS			= 'gbls',
};

#define FONT_SUB_MENUS true

// WonderBrush public rsa keyfile (flattened)
const unsigned char kPublicKey[] = {
	0x00, 0x00, 0x01, 0xff, 0x00, 0x00, 0x01, 0xfd, 0xba, 0x23,
	0x23, 0x06, 0x6d, 0x90, 0x1c, 0x20, 0x3a, 0x6f, 0x2e, 0xec,
	0xbd, 0x5b, 0xbf, 0x42, 0x17, 0x61, 0x4a, 0x1a, 0x3f, 0x8d,
	0xcb, 0xee, 0xd3, 0xe7, 0xaa, 0x3a, 0x3f, 0x83, 0x77, 0x19,
	0x42, 0x97, 0xe8, 0x3f, 0xbf, 0x98, 0x62, 0x89, 0x6f, 0x38,
	0x4f, 0x6c, 0xe5, 0xa2, 0xc1, 0xe4, 0x98, 0x40, 0x23, 0x32,
	0xc6, 0x19, 0x6f, 0x47, 0xa5, 0x77, 0x9a, 0x3f, 0xef, 0x82,
	0x58, 0x27, 0xdf, 0xf7, 0x4c, 0x47, 0x7e, 0x64, 0x17, 0x17,
	0xce, 0xcd, 0x84, 0xce, 0xa0, 0x8f, 0x45, 0x87, 0x11, 0xb2,
	0x3a, 0x22, 0xec, 0xa6, 0x16, 0x55, 0xb4, 0x5e, 0x19, 0x43,
	0xf0, 0xe9, 0xc1, 0xd0, 0xdd, 0x0e, 0x18, 0x5f, 0x72, 0x2f,
	0x73, 0x3c, 0xf8, 0xf4, 0x0f, 0x99, 0x87, 0x51, 0x1c, 0x95,
	0x07, 0x5a, 0xb7, 0xf4, 0x00, 0xce, 0x49, 0xb5, 0x2c, 0x66,
	0xf5, 0x21, 0x4c, 0xb6, 0xf0, 0xd0
};

// used by the "wait for file" thread
struct wait_info {
	entry_ref	ref;
	BWindow*	window;
};

class ZoomSlider : public PopupSlider {
 public:
					ZoomSlider(const char* name = NULL,
							   const char* label = NULL,
							   BMessage* model = NULL,
							   BHandler* target = NULL,
							   int32 min = 0,
							   int32 max = 100,
							   int32 value = 0,
							   const char* formatString = "%ld")
					: PopupSlider(name, label, model, target, min, max, value, formatString)
					{
					}
	virtual	float	Scale(float ratio) const
					{
						return ratio * ratio;
					}
	virtual	float	DeScale(float ratio) const
					{
						return sqrtf(ratio);
					}
};

class PrintView : public BView {
 public:
					PrintView(BBitmap* bitmap, BRect printableRect)
					: BView(printableRect, "print view", B_FOLLOW_NONE, B_WILL_DRAW),
					  fBitmap(bitmap),
					  fPrintableRect(printableRect)
					{
						SetViewColor(B_TRANSPARENT_32_BIT);
					}

	virtual	void	Draw(BRect updateRect)
					{
						if (IsPrinting() && fBitmap && fBitmap->IsValid()) {
							//BRect r(fPrintableRect);
							BRect r(updateRect);
							BRect bitmapFrame(fBitmap->Bounds());
							float xScale = r.Width() / bitmapFrame.Width();
							float yScale = r.Height() / bitmapFrame.Height();

							float scale = xScale > yScale ? yScale : xScale;
							bitmapFrame.right = bitmapFrame.right * scale;
							bitmapFrame.bottom = bitmapFrame.bottom * scale;
							BPoint offset(0.0, 0.0);
							if (xScale > yScale)
								offset.x = (r.Width() - bitmapFrame.Width()) / 2.0;
							else
								offset.y = (r.Height() - bitmapFrame.Height()) / 2.0;
							bitmapFrame.OffsetBy(offset);

//							printf("PrintView::Draw()\n");
//							updateRect.PrintToStream();
//							fPrintableRect.PrintToStream();
// TODO: make bitmapFrame the part of the bitmap bounds that needs to be put into updateRect
//bitmapFrame
							DrawBitmap(fBitmap, fBitmap->Bounds(), bitmapFrame);
						}
					}

 private:
	BBitmap*		fBitmap;
	BRect			fPrintableRect;
};

class UnitPopup : public MPopup {
public:
			UnitPopup(BMessage* message, BHandler* handler)
				: MPopup(NULL, message, handler, NULL)
			{
			}

	virtual	~UnitPopup() {}

	virtual	minimax	layoutprefs()
			{
				mpm = MPopup::layoutprefs();
				if (BMenu* menu = Menu()) {
					float width, height;
					menu->GetPreferredSize(&width, &height);
					mpm.mini.x = mpm.maxi.x = width + 2.0;
				}
				return mpm;
			}
};

// constructor
MainWindow::MainWindow(BRect frame)
	: MWindow(frame, "WonderBrush",
			  B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
			  B_ASYNCHRONOUS_CONTROLS),
	  fSettings(new BMessage('sett')),
	  fZoomToFullscreen(true),
	  fColorPickerFrame(100.0, 100.0, 200.0, 200.0),
	  fNewCanvasFrame(100.0, 100.0, 200.0, 200.0),
	  fResizeCanvasFrame(100.0, 100.0, 200.0, 200.0),
	  fColorPickerMode(H_SELECTED),
	  fColorPickerPanel(NULL),
	  fBrushPanel(NULL),
	  fRememberedTool(TOOL_BRUSH),
	  fSettingsPanelFrame(100.0, 100.0, 200.0, 200.0),
	  fSettingsPanel(NULL),
	  fDocuments(8),
	  fSavePanel(new ExportPanel("save panel")),
	  fLastSavePath(NULL),
	  fLastExportPath(NULL),
	  fKeyfile(NULL),
	  fLoadingThread(B_ERROR)
{
	LanguageManager* manager = LanguageManager::Default();
	AddHandler(manager);
	manager->AttachedToLooper(this);

	fSavePanel->SetTarget(BMessenger(this, this));

	fBrushView = new BrushView("brush preview", new BMessage(MSG_BRUSH_PANEL), this);
	fInfoView = new InfoView("info view");
	fHistoryListView = new HistoryListView(BRect(0.0, 0.0, 30.0, 50.0));
	fLayersListView = new LayersListView(BRect(0.0, 0.0, 30.0, 50.0));

	fNavigatorView = new NavigatorView("navigator view");

#if CANVAS_LISTVIEW
	fCanvasListView = new CanvasListView("canvas list view", new BMessage(MSG_SET_CANVAS), this);
#else
	fCanvasTabView = new CanvasTabView("canvas tab view", new BMessage(MSG_SET_CANVAS), this);
#endif

	fCanvasView = new CanvasView(BRect (0.0, 0.0, 799.0, 599.0), "canvas view",
								 fBrushView, fInfoView, fHistoryListView,
								 fLayersListView, fNavigatorView);

	fPropertyListView = new PropertyListView(fCanvasView);

	fLayersListView->SetCanvasView(fCanvasView);
	fLayerConfigView = new LayerConfigView(fCanvasView);
	fLayersListView->SetConfigView(fLayerConfigView);

	fAlphaDS = new DualSlider("max alpha", "Opacity",
							  new BMessage(MSG_SET_ALPHA),
							  new BMessage(MSG_CONTROL_ALPHA),
							  fCanvasView, 0.0, 1.0);
	fAlphaDS->SetMinEnabled(false);
	fRadiusDS = new DualSlider("max radius", "Radius",
							   new BMessage(MSG_SET_RADIUS),
							   new BMessage(MSG_CONTROL_RADIUS),
							   fCanvasView, 0.0, 0.1);
	fRadiusDS->SetMinEnabled(false);
	fHardnessDS = new DualSlider("hardness", "Hardness",
								 new BMessage(MSG_SET_HARDNESS),
								 new BMessage(MSG_CONTROL_HARDNESS),
								 fCanvasView, 1.0, 1.0);
	fHardnessDS->SetMinEnabled(false);
	fSpacingDS = new DualSlider("spacing", "Spacing",
								new BMessage(MSG_SET_SPACING),
								NULL,//new BMessage(MSG_CONTROL_SPACING),
								fCanvasView, 0.1, 0.1);
fSpacingDS->SetMinEnabled(false);

	// fill controls
	fFillOpacityDS = new DualSlider("fill opacity", "Opacity",
								new BMessage(MSG_SET_FILL_OPACITY),
								NULL, fCanvasView, 0.0, 1.0);
	fFillOpacityDS->SetMinEnabled(false);
	fToleranceDS = new DualSlider("tolerance", "Tolerance",
								new BMessage(MSG_SET_TOLERANCE),
								NULL, fCanvasView, 0.0, 1.0);
	fToleranceDS->SetMinEnabled(false);
	fSoftnessDS = new DualSlider("softness", "Softness",
								new BMessage(MSG_SET_SOFTNESS),
								NULL, fCanvasView, 0.0, 1.0);
	fSoftnessDS->SetMinEnabled(false);

	fFillContiguousCB = new BetterMCheckBox("Contiguous Area", new BMessage(MSG_SET_FILL_CONTIGUOUS),
											fCanvasView, true);

	BMessage* message = new BMessage(MSG_SUB_PIXEL_PRECISION);
	message->AddInt32("tool", TOOL_BRUSH);
	fBrushSubPixelCB = new BetterMCheckBox("Subpixels", message, fCanvasView, true);
	fSolidCB = new BetterMCheckBox("Solid", new BMessage(MSG_SET_SOLID), fCanvasView, false);
	fBrushTiltCB = new BetterMCheckBox("Tilt", new BMessage(MSG_SET_TILT), fCanvasView, false);

	// transform controls
	fTranslationSV = new MStringView("Translate");
	fRotationSV = new MStringView("Rotate");
	fScaleSV = new MStringView("Scale");
	fXOffsetTC = new NumericalTextControl("X", NULL, new BMessage(MSG_TRANSLATE));
	fYOffsetTC = new NumericalTextControl("Y", NULL, new BMessage(MSG_TRANSLATE));
	fRotationTC = new NumericalTextControl("Angle", NULL, new BMessage(MSG_ROTATE));
	fScaleXTC = new NumericalTextControl("X", NULL, new BMessage(MSG_SCALE));
	fScaleYTC = new NumericalTextControl("Y", NULL, new BMessage(MSG_SCALE));
	fXOffsetTC->SetEnabled(false);
	fYOffsetTC->SetEnabled(false);
	fRotationTC->SetEnabled(false);
	fScaleXTC->SetEnabled(false);
	fScaleYTC->SetEnabled(false);

	message = new BMessage(MSG_SUB_PIXEL_PRECISION);
	message->AddInt32("tool", TOOL_TRANSLATE);
	fMoveSubPixelCB = new BetterMCheckBox("Subpixels", message, fCanvasView, true);
	fMoveSubPixelCB->ct_mpm.weight = 0.5;

	// crop controls
	fCropLeftTC = new NumericalTextControl("Left", NULL, new BMessage(MSG_CROPPING));
	fCropTopTC = new NumericalTextControl("Top", NULL, new BMessage(MSG_CROPPING));
	fCropWidthTC = new NumericalTextControl("Width", NULL, new BMessage(MSG_CROPPING));
	fCropHeightTC = new NumericalTextControl("Height", NULL, new BMessage(MSG_CROPPING));

	message = new BMessage(MSG_SUB_PIXEL_PRECISION);
	message->AddInt32("tool", TOOL_CROP);
	fCropSubPixelCB = new BetterMCheckBox("Subpixels", message, fCanvasView, true);

	fCropAllB = new MButton("All",
							new BMessage(MSG_CROP_ENTIRE_CANVAS),
							fCanvasView);

	// dropper controls
	fDropperSizeLP = new LabelPopup("Tip Size");

	message = new BMessage(MSG_DROPPER_TIP_SIZE);
	message->AddInt32("size", 1);
	fDropperSizeLP->Menu()->AddItem(fDropperSize1x1MI = new BMenuItem("1x1 pixels", message));
	fDropperSize1x1MI->SetMarked(true);

	message = new BMessage(MSG_DROPPER_TIP_SIZE);
	message->AddInt32("size", 3);
	fDropperSizeLP->Menu()->AddItem(fDropperSize3x3MI = new BMenuItem("3x3 pixels", message));

	message = new BMessage(MSG_DROPPER_TIP_SIZE);
	message->AddInt32("size", 5);
	fDropperSizeLP->Menu()->AddItem(fDropperSize5x5MI = new BMenuItem("5x5 pixels", message));

	fUseAllLayersCB = new BetterMCheckBox("Include All Layers", new BMessage(MSG_USE_ALL_LAYERS),
										  fCanvasView, true);


	// text tool controls
	fFontLP = new FontPopup("Font", FONT_SUB_MENUS);

//	fFontSizeSB = new BetterSpinButton("Size", SPIN_FLOAT, fCanvasView);
//	fFontSizeSB->SetValue(12.0);
//	fFontSizeSB->SetMessage(new BMessage(MSG_SET_TEXT_SIZE));
	fFontSizeSB = new DualSlider("text size", "Size",
								new BMessage(MSG_SET_TEXT_SIZE),
								NULL, fCanvasView, 0.0, 0.2);
	fFontSizeSB->SetMinEnabled(false);
fFontSizeSB->SetMaxValue(sqrtf((12.0 / 64.0) / 50.0));

	fFontOpacityDS = new DualSlider("text opacity", "Opacity",
								new BMessage(MSG_SET_TEXT_OPACITY),
								NULL, fCanvasView, 0.0, 1.0);
	fFontOpacityDS->SetMinEnabled(false);

	fFontAdvanceScaleDS = new DualSlider("text spacing", "Spacing",
								new BMessage(MSG_SET_TEXT_ADVANCE_SCALE),
								NULL, fCanvasView, 0.0, 0.5);
	fFontAdvanceScaleDS->SetMinEnabled(false);

	fTextTV = new ReportingTextView();
	fTextTV->SetText("Enter text here.");

	fFontAlignModeIOC = new IconOptionsControl(NULL, "Alignment", NULL, fCanvasView);

	message = new BMessage(MSG_SET_TEXT_ALIGNMENT_MODE);
	message->AddInt32("mode", ALIGN_LEFT);
	fFontAlignLeftIB = new IconButton("left align", 0, NULL, message);
	fFontAlignLeftIB->SetIcon(kAlignLeftIconBits, kAlignIconWidth, kAlignIconHeight, kIconColorSpace);
	fFontAlignModeIOC->AddOption(fFontAlignLeftIB);

	message = new BMessage(MSG_SET_TEXT_ALIGNMENT_MODE);
	message->AddInt32("mode", ALIGN_CENTER);
	fFontAlignCenterIB = new IconButton("center align", 0, NULL, message);
	fFontAlignCenterIB->SetIcon(kAlignCenterIconBits, kAlignIconWidth, kAlignIconHeight, kIconColorSpace);
	fFontAlignModeIOC->AddOption(fFontAlignCenterIB);

	message = new BMessage(MSG_SET_TEXT_ALIGNMENT_MODE);
	message->AddInt32("mode", ALIGN_RIGHT);
	fFontAlignRightIB = new IconButton("right align", 0, NULL, message);
	fFontAlignRightIB->SetIcon(kAlignRightIconBits, kAlignIconWidth, kAlignIconHeight, kIconColorSpace);
	fFontAlignModeIOC->AddOption(fFontAlignRightIB);

	message = new BMessage(MSG_SET_TEXT_ALIGNMENT_MODE);
	message->AddInt32("mode", ALIGN_JUSTIFY);
	fFontAlignJustifyIB = new IconButton("justify align", 0, NULL, message);
	fFontAlignJustifyIB->SetIcon(kAlignJustifyIconBits, kAlignIconWidth, kAlignIconHeight, kIconColorSpace);
	fFontAlignModeIOC->AddOption(fFontAlignJustifyIB);

//	DivideSame(fFontLP, fFontAlignModeIOC, NULL);

	// shape tool controls
	fShapeOpacityDS = new DualSlider("shape opacity", "Opacity",
								new BMessage(MSG_SET_SHAPE_OPACITY),
								NULL, fCanvasView, 0.0, 1.0);
	fShapeOpacityDS->SetMinEnabled(false);

	fShapeOutlineCB = new BetterMCheckBox("Outline", new BMessage(MSG_SET_SHAPE_OUTLINE),
										  this, false);

	fShapeOutlineWidthDS = new DualSlider("shape outline width", "Width",
								new BMessage(MSG_SET_SHAPE_OUTLINE_WIDTH),
								NULL, fCanvasView, 0.0, 1.0);
	fShapeOutlineWidthDS->SetMinEnabled(false);
	fShapeOutlineWidthDS->SetMaxValue(sqrtf(1.0 / 100.0));
	fShapeOutlineWidthDS->SetEnabled(false);
	// transform points
	fShapeTransformIB = new IconButton("transform", 0, NULL,
									   new BMessage(MSG_SHAPE_TRANSFORM),
									   fCanvasView);
	fShapeTransformIB->SetIcon(kMoveIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fShapeTransformIB->SetEnabled(false);
	// new subpath
	fShapeNewPathIB = new IconButton("transform", 0, NULL,
									 new BMessage(MSG_SHAPE_NEW_PATH),
									 fCanvasView);
	fShapeNewPathIB->SetIcon(kNewPathIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fShapeNewPathIB->SetEnabled(false);
	// remove points
	fShapeRemovePointsIB = new IconButton("transform", 0, NULL,
										  new BMessage(MSG_SHAPE_REMOVE_POINTS),
										  fCanvasView);
	fShapeRemovePointsIB->SetIcon(kTrashIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fShapeRemovePointsIB->SetEnabled(false);

	fShapeClosedCB = new MCheckBox("Closed", new BMessage(MSG_SET_SHAPE_CLOSED),
								   fCanvasView, false);

	// reverse path
	fShapeReversePathIB = new IconButton("reverse", 0, NULL,
										  new BMessage(MSG_REVERSE_PATH),
										  fCanvasView);
	fShapeReversePathIB->SetIcon(kReversePathIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fShapeReversePathIB->SetEnabled(false);


	message = new BMessage(MSG_SUB_PIXEL_PRECISION);
	message->AddInt32("tool", TOOL_SHAPE);
	fShapeSubpixelsCB = new MCheckBox("Subpixels", message, fCanvasView, true);


	// setup line join and cap mode GUI
	fShapeCapModeIOC= new IconOptionsControl(NULL, "Caps", NULL, fCanvasView);

	message = new BMessage(MSG_SET_CAP_MODE);
	message->AddInt32("mode", CAP_MODE_BUTT);
	fCapModeButtIB = new IconButton("butt caps", 0, NULL, message);
	fCapModeButtIB->SetIcon(kButtCapIconBits, kCapJoinIconWidth, kCapJoinIconHeight, kIconColorSpace);
	fShapeCapModeIOC->AddOption(fCapModeButtIB);

	message = new BMessage(MSG_SET_CAP_MODE);
	message->AddInt32("mode", CAP_MODE_SQUARE);
	fCapModeSquareIB = new IconButton("square caps", 0, NULL, message);
	fCapModeSquareIB->SetIcon(kSquareCapIconBits, kCapJoinIconWidth, kCapJoinIconHeight, kIconColorSpace);
	fShapeCapModeIOC->AddOption(fCapModeSquareIB);

	message = new BMessage(MSG_SET_CAP_MODE);
	message->AddInt32("mode", CAP_MODE_ROUND);
	fCapModeRoundIB = new IconButton("round caps", 0, NULL, message);
	fCapModeRoundIB->SetIcon(kRoundCapIconBits, kCapJoinIconWidth, kCapJoinIconHeight, kIconColorSpace);
	fShapeCapModeIOC->AddOption(fCapModeRoundIB);

	fShapeJoinModeIOC = new IconOptionsControl(NULL, "Joints", NULL, fCanvasView);

	message = new BMessage(MSG_SET_JOIN_MODE);
	message->AddInt32("mode", JOIN_MODE_MITER);
	fJoinModeMiterIB = new IconButton("miter joints", 0, NULL, message);
	fJoinModeMiterIB->SetIcon(kJoinMiterIconBits, kCapJoinIconWidth, kCapJoinIconHeight, kIconColorSpace);
	fShapeJoinModeIOC->AddOption(fJoinModeMiterIB);

	message = new BMessage(MSG_SET_JOIN_MODE);
	message->AddInt32("mode", JOIN_MODE_ROUND);
	fJoinModeRoundIB = new IconButton("round joints", 0, NULL, message);
	fJoinModeRoundIB->SetIcon(kJoinRoundIconBits, kCapJoinIconWidth, kCapJoinIconHeight, kIconColorSpace);
	fShapeJoinModeIOC->AddOption(fJoinModeRoundIB);

	message = new BMessage(MSG_SET_JOIN_MODE);
	message->AddInt32("mode", JOIN_MODE_BEVEL);
	fJoinModeBevelIB = new IconButton("bevel joints", 0, NULL, message);
	fJoinModeBevelIB->SetIcon(kJoinBevelIconBits, kCapJoinIconWidth, kCapJoinIconHeight, kIconColorSpace);
	fShapeJoinModeIOC->AddOption(fJoinModeBevelIB);

	DivideSame(fShapeCapModeIOC, fShapeJoinModeIOC, NULL);


	// ellipse tool controls
	fEllipseOpacityDS = new DualSlider("ellipse opacity", "Opacity",
										new BMessage(MSG_SET_ELLIPSE_OPACITY),
										NULL, fCanvasView, 0.0, 1.0);
	fEllipseOpacityDS->SetMinEnabled(false);

	fEllipseOutlineCB = new BetterMCheckBox("Outline", new BMessage(MSG_SET_ELLIPSE_OUTLINE),
											this, false);

	fEllipseOutlineWidthDS = new DualSlider("ellipse outline width", "Width",
											new BMessage(MSG_SET_ELLIPSE_OUTLINE_WIDTH),
											NULL, fCanvasView, 0.0, 1.0);
	fEllipseOutlineWidthDS->SetMinEnabled(false);
	fEllipseOutlineWidthDS->SetMaxValue(sqrtf(1.0 / 100.0));
	fEllipseOutlineWidthDS->SetEnabled(false);

	message = new BMessage(MSG_SUB_PIXEL_PRECISION);
	message->AddInt32("tool", TOOL_ELLIPSE);
	fEllipseSubpixelsCB = new BetterMCheckBox("Subpixels", message, fCanvasView, true);

	// round rect tool controls
	fRectOpacityDS = new DualSlider("rect opacity", "Opacity",
									new BMessage(MSG_SET_RECT_OPACITY),
									NULL, fCanvasView, 0.0, 1.0);
	fRectOpacityDS->SetMinEnabled(false);

	fRectOutlineCB = new BetterMCheckBox("Outline", new BMessage(MSG_SET_RECT_OUTLINE),
										 this, false);

	fRectOutlineWidthDS = new DualSlider("rect outline width", "Width",
										 new BMessage(MSG_SET_RECT_OUTLINE_WIDTH),
										 NULL, fCanvasView, 0.0, 1.0);
	fRectOutlineWidthDS->SetMinEnabled(false);
	fRectOutlineWidthDS->SetMaxValue(sqrtf(1.0 / 100.0));
	fRectOutlineWidthDS->SetEnabled(false);

	fRectCornerRadiusDS = new DualSlider("rect corner radius", "Round Corner Radius",
										 new BMessage(MSG_SET_RECT_CORNER_RADIUS),
										 NULL, fCanvasView, 0.0, 1.0);
	fRectCornerRadiusDS->SetMinEnabled(false);
	fRectCornerRadiusDS->SetMaxValue(0.0);

	message = new BMessage(MSG_SUB_PIXEL_PRECISION);
	message->AddInt32("tool", TOOL_ROUND_RECT);
	fRectSubpixelsCB = new BetterMCheckBox("Subpixels", message, fCanvasView, true);

	// crop controls
	fSelectLeftTC = new NumericalTextControl("Left", NULL, new BMessage(MSG_SELECTING));
	fSelectTopTC = new NumericalTextControl("Top", NULL, new BMessage(MSG_SELECTING));
	fSelectWidthTC = new NumericalTextControl("Width", NULL, new BMessage(MSG_SELECTING));
	fSelectHeightTC = new NumericalTextControl("Height", NULL, new BMessage(MSG_SELECTING));

	message = new BMessage(MSG_SELECT_ALL_LAYERS);
	fSelectAllLayersCB = new BetterMCheckBox("Include All Layers", message, fCanvasView, false);

	fSelectAllB = new MButton("All",
							  new BMessage(MSG_SELECT_ENTIRE_CANVAS),
							  fCanvasView);

	// edit gradient controls
	fGradientControl = new GradientControl();
	fCanvasView->SetGradientControl(fGradientControl);

	// setup line join and cap mode GUI
	fGradientTypeIOC= new IconOptionsControl(NULL, "Type", NULL, fCanvasView);

	message = new BMessage(MSG_SET_GRADIENT_TYPE);
	message->AddInt32("type", GRADIENT_LINEAR);
	fGradientTypeLinearIB = new IconButton("linear", 0, NULL, message);
	fGradientTypeLinearIB->SetIcon(kGradientLinearIconBits, kGradientTypeIconWidth, kGradientTypeIconHeight, kGradientTypeIconFormat);
	fGradientTypeIOC->AddOption(fGradientTypeLinearIB);

	message = new BMessage(MSG_SET_GRADIENT_TYPE);
	message->AddInt32("type", GRADIENT_CIRCULAR);
	fGradientTypeCircularIB = new IconButton("radial", 0, NULL, message);
	fGradientTypeCircularIB->SetIcon(kGradientRadialIconBits, kGradientTypeIconWidth, kGradientTypeIconHeight, kGradientTypeIconFormat);
	fGradientTypeIOC->AddOption(fGradientTypeCircularIB);

	message = new BMessage(MSG_SET_GRADIENT_TYPE);
	message->AddInt32("type", GRADIENT_DIAMONT);
	fGradientTypeDiamontIB = new IconButton("diamont", 0, NULL, message);
	fGradientTypeDiamontIB->SetIcon(kGradientDiamontIconBits, kGradientTypeIconWidth, kGradientTypeIconHeight, kGradientTypeIconFormat);
	fGradientTypeIOC->AddOption(fGradientTypeDiamontIB);

	message = new BMessage(MSG_SET_GRADIENT_TYPE);
	message->AddInt32("type", GRADIENT_CONIC);
	fGradientTypeConicIB = new IconButton("conic", 0, NULL, message);
	fGradientTypeConicIB->SetIcon(kGradientConicIconBits, kGradientTypeIconWidth, kGradientTypeIconHeight, kGradientTypeIconFormat);
	fGradientTypeIOC->AddOption(fGradientTypeConicIB);

	message = new BMessage(MSG_SET_GRADIENT_TYPE);
	message->AddInt32("type", GRADIENT_XY);
	fGradientTypeXYIB = new IconButton("xy", 0, NULL, message);
	fGradientTypeXYIB->SetIcon(kGradientXYIconBits, kGradientTypeIconWidth, kGradientTypeIconHeight, kGradientTypeIconFormat);
	fGradientTypeIOC->AddOption(fGradientTypeXYIB);

	message = new BMessage(MSG_SET_GRADIENT_TYPE);
	message->AddInt32("type", GRADIENT_SQRT_XY);
	fGradientTypeSqrtXYIB = new IconButton("sqrt-xy", 0, NULL, message);
	fGradientTypeSqrtXYIB->SetIcon(kGradientSqrtXYIconBits, kGradientTypeIconWidth, kGradientTypeIconHeight, kGradientTypeIconFormat);
	fGradientTypeIOC->AddOption(fGradientTypeSqrtXYIB);


	fGradientInterpolationLP = new LabelPopup("Interpolation");
//	fGradientInterpolationLP->SetAlignment(B_ALIGN_RIGHT);

	message = new BMessage(MSG_SET_GRADIENT_INTERPOLATION);
	message->AddInt32("type", INTERPOLATION_LINEAR);
	fGradientInterpolationLinearMI = new BMenuItem("Linear", message);
	fGradientInterpolationLP->Menu()->AddItem(fGradientInterpolationLinearMI);

	message = new BMessage(MSG_SET_GRADIENT_INTERPOLATION);
	message->AddInt32("type", INTERPOLATION_SMOOTH);
	fGradientInterpolationSmoothMI = new BMenuItem("Smooth", message);
	fGradientInterpolationLP->Menu()->AddItem(fGradientInterpolationSmoothMI);
	fGradientInterpolationSmoothMI->SetMarked(true);

	fGradientInheritsTransformCB = new BetterMCheckBox("Inherit Transformation", new BMessage(MSG_SET_GRADIENT_INHERITS_TRANSFORM),
													   fCanvasView, true);

	fGradientStopAlphaDS = new DualSlider("gradient stop alpha", "Opacity",
										  new BMessage(MSG_SET_GRADIENT_STOP_OPACITY),
										  NULL, fCanvasView, 0.0, 1.0);
	fGradientStopAlphaDS->SetMinEnabled(false);

	// guides tool controls
	fGuidesShowCB = new BetterMCheckBox("Show Guides", new BMessage(MSG_SHOW_GUIDES),
										fCanvasView, true);

	fGuideCreateIB = new IconButton("new guide", 0, NULL, new BMessage(MSG_NEW_GUIDE), fCanvasView);
	fGuideCreateIB->SetIcon(kNewGuideIconBits, kIconWidth, kIconHeight, kIconColorSpace);

	fGuideRemoveIB = new IconButton("remove guide", 0, NULL,
									new BMessage(MSG_REMOVE_GUIDE), fCanvasView);
	fGuideRemoveIB->SetIcon(kTrashIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fGuideRemoveIB->SetEnabled(false);

	fGuideProportionIB = new IconButton("golden proportion", 0, NULL,
									new BMessage(MSG_GUIDE_GOLDEN_PROPORTION), fCanvasView);
	fGuideProportionIB->SetIcon(kGoldenerSchnittIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fGuideProportionIB->SetEnabled(false);

	fGuideHPosTC = new NumericalTextControl("Horizontal", "50.00",
										   new BMessage(MSG_SET_GUIDE_H_POS));

	fGuideVPosTC = new NumericalTextControl("Vertical", "50.00",
										   new BMessage(MSG_SET_GUIDE_V_POS));

	fGuideAngleTC = new NumericalTextControl("Angle", "0.00",
											new BMessage(MSG_SET_GUIDE_ANGLE));

	fGuideUnitsLP = new UnitPopup(NULL, NULL);

	message = new BMessage(MSG_SET_GUIDE_UNITS);
	message->AddInt32("units", GUIDE_UNITS_PERCENT);
	fGuideUnitsPercentMI = new BMenuItem("%", message);
	fGuideUnitsLP->Menu()->AddItem(fGuideUnitsPercentMI);
	fGuideUnitsPercentMI->SetMarked(true);

	message = new BMessage(MSG_SET_GUIDE_UNITS);
	message->AddInt32("units", GUIDE_UNITS_PIXELS);
	fGuideUnitsPixelsMI = new BMenuItem("px", message);
	fGuideUnitsLP->Menu()->AddItem(fGuideUnitsPixelsMI);

	// pick controls
	fSelectableSV = new MStringView("Selectable", B_ALIGN_LEFT, minimax(10, 10, 65536, 65536, 0.1));

	message = new BMessage(MSG_PICK_MASK);
	message->AddInt32("mask", PICK_MASK_BRUSH);
	fPickMaskBrushIB = new IconButton("pick brush icon", 0, NULL, message, fCanvasView);
	fPickMaskBrushIB->SetIcon(kBrushIconBits, kIconWidth, kIconHeight,
							  kIconColorSpace, true);

	message = new BMessage(MSG_PICK_MASK);
	message->AddInt32("mask", PICK_MASK_ERASER);
	fPickMaskEraserIB = new IconButton("pick eraser icon", 0, NULL, message, fCanvasView);
	fPickMaskEraserIB->SetIcon(kEraserIconBits, kIconWidth, kIconHeight,
							   kIconColorSpace, true);

	message = new BMessage(MSG_PICK_MASK);
	message->AddInt32("mask", PICK_MASK_PEN);
	fPickMaskPenIB = new IconButton("pick pen icon", 0, NULL, message, fCanvasView);
	fPickMaskPenIB->SetIcon(kPenIconBits, kIconWidth, kIconHeight,
							kIconColorSpace, true);

	message = new BMessage(MSG_PICK_MASK);
	message->AddInt32("mask", PICK_MASK_PEN_ERASER);
	fPickMaskPenEraserIB = new IconButton("pick pen eraser icon", 0, NULL, message, fCanvasView);
	fPickMaskPenEraserIB->SetIcon(kPenEraserIconBits, kIconWidth, kIconHeight,
								  kIconColorSpace, true);

	message = new BMessage(MSG_PICK_MASK);
	message->AddInt32("mask", PICK_MASK_CLONE_BRUSH);
	fPickMaskCloneIB = new IconButton("pick clone icon", 0, NULL, message, fCanvasView);
	fPickMaskCloneIB->SetIcon(kCloneIconBits, kIconWidth, kIconHeight,
							  kIconColorSpace, true);

	message = new BMessage(MSG_PICK_MASK);
	message->AddInt32("mask", PICK_MASK_BLUR_BRUSH);
	fPickMaskBlurIB = new IconButton("pick Blur icon", 0, NULL, message, fCanvasView);
	fPickMaskBlurIB->SetIcon(kBlurIconBits, kIconWidth, kIconHeight,
							 kIconColorSpace, true);

	message = new BMessage(MSG_PICK_MASK);
	message->AddInt32("mask", PICK_MASK_FILL);
	fPickMaskFillIB = new IconButton("pick fill icon", 0, NULL, message, fCanvasView);
	fPickMaskFillIB->SetIcon(kFillIconBits, kIconWidth, kIconHeight,
							 kIconColorSpace, true);

	message = new BMessage(MSG_PICK_MASK);
	message->AddInt32("mask", PICK_MASK_TEXT);
	fPickMaskTextIB = new IconButton("pick text icon", 0, NULL, message, fCanvasView);
	fPickMaskTextIB->SetIcon(kTextIconBits, kIconWidth, kIconHeight,
							 kIconColorSpace, true);

	message = new BMessage(MSG_PICK_MASK);
	message->AddInt32("mask", PICK_MASK_SHAPE);
	fPickMaskShapeIB = new IconButton("pick shape icon", 0, NULL, message, fCanvasView);
	fPickMaskShapeIB->SetIcon(kShapeIconBits, kIconWidth, kIconHeight,
							  kIconColorSpace, true);

	message = new BMessage(MSG_PICK_MASK);
	message->AddInt32("mask", PICK_MASK_BITMAP);
	fPickMaskBitmapIB = new IconButton("pick bitmap icon", 0, NULL, message, fCanvasView);
	fPickMaskBitmapIB->SetIcon(kBitmapIconBits, kIconWidth, kIconHeight,
							   kIconColorSpace, true);

	fPickMaskAllB = new MButton("All", new BMessage(MSG_PICK_MASK_ALL), fCanvasView);
	fPickMaskNoneB = new MButton("None", new BMessage(MSG_PICK_MASK_NONE), fCanvasView);

	fSelectSV = new MStringView("Select", B_ALIGN_LEFT, minimax(10, 10, 65536, 65536, 0.1));
	fPickAllB = new MButton("All", new BMessage(MSG_PICK_ALL), fCanvasView);
	fPickNoneB = new MButton("None", new BMessage(MSG_PICK_NONE), fCanvasView);

	// canvas icons
	fNewIB = new IconButton("new icon", 0, NULL, new BMessage(MSG_NEW), this);
	fNewIB->SetIcon(kNewIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fOpenIB = new IconButton("open icon", 0, NULL, new BMessage(MSG_OPEN), this);
	fOpenIB->SetIcon(kOpenIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fSaveImageIB = new IconButton("save image icon", 0, NULL, new BMessage(MSG_EXPORT), this);
	fSaveImageIB->SetIcon(kDiskIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fSaveProjectIB = new IconButton("save project icon", 0, NULL, new BMessage(MSG_SAVE_DOCUMENT), this);
	fSaveProjectIB->SetIcon(kDiskProjectIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fCloseIB = new IconButton("close icon", 0, NULL, new BMessage(MSG_CLOSE_CANVAS), this);
	fCloseIB->SetIcon(kCloseIconBits, kIconWidth, kIconHeight, kIconColorSpace);

	fUndoIB = new IconButton("undo icon", 0, NULL, new BMessage(MSG_UNDO), this);
	fUndoIB->SetIcon(kUndoIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fRedoIB = new IconButton("redo icon", 0, NULL, new BMessage(MSG_REDO), this);
	fRedoIB->SetIcon(kRedoIconBits, kIconWidth, kIconHeight, kIconColorSpace);

	// tool icons
	// paint mode
	BMessage* msg = new BMessage(MSG_SET_PAINT_MODE);
	fPaintModeIB = new IconButton("paint mode icon", 0, NULL, msg, this);
//	fPaintModeIB->SetIcon(kBrushIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fPaintModeIB->SetPressed(true);
	// selection mode
	msg = new BMessage(MSG_SET_SELECTION_MODE);
	fSelectModeIB = new IconButton("selection mode icon", 0, NULL, msg, this);
//	fSelectModeIB->SetIcon(kBrushIconBits, kIconWidth, kIconHeight, kIconColorSpace);

	// confirm
	msg = new BMessage(MSG_CONFIRM_TOOL);
	fConfirmIB = new IconButton("confirm icon", 0, NULL, msg, fCanvasView);
	fConfirmIB->SetIcon(kConfirmIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// cancel
	msg = new BMessage(MSG_CANCEL_TOOL);
	fCancelIB = new IconButton("cancel icon", 0, NULL, msg, fCanvasView);
	fCancelIB->SetIcon(kCancelIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// nothing to confirm yet
	SetConfirmationEnabled(false);

	// select
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_SELECT);
	fCopyIB = new IconButton("select icon", TOOL_SELECT, NULL, msg, fCanvasView);
	fCopyIB->SetIcon(kCopyIconBits, kIconWidth, kIconHeight, kIconColorSpace);

	// brush
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_BRUSH);
	fBrushIB = new IconButton("brush icon", TOOL_BRUSH, NULL, msg, fCanvasView);
	fBrushIB->SetIcon(kBrushIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fBrushIB->SetPressed(true);
	// brush
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_CLONE);
	fCloneBrushIB = new IconButton("clone icon", TOOL_CLONE, NULL, msg, fCanvasView);
	fCloneBrushIB->SetIcon(kCloneIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// pen
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_PEN);
	fPenIB = new IconButton("pen icon", TOOL_PEN, NULL, msg, fCanvasView);
	fPenIB->SetIcon(kPenIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// pen eraser
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_PEN_ERASER);
	fPenEraserIB = new IconButton("pen eraser icon", TOOL_PEN_ERASER, NULL, msg, fCanvasView);
	fPenEraserIB->SetIcon(kPenEraserIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// eraser
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_ERASER);
	fEraserIB = new IconButton("eraser icon", TOOL_ERASER, NULL, msg, fCanvasView);
	fEraserIB->SetIcon(kEraserIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// color picker
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_COLOR_PICKER);
	fColorPickerIB = new IconButton("color picker icon", TOOL_COLOR_PICKER, NULL, msg, fCanvasView);
	fColorPickerIB->SetIcon(kDropperIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// blur
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_BLUR);
	fBlurIB = new IconButton("blur icon", TOOL_BLUR, NULL, msg, fCanvasView);
	fBlurIB->SetIcon(kBlurIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// text
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_TEXT);
	fTextIB = new IconButton("text icon", TOOL_TEXT, NULL, msg, fCanvasView);
	fTextIB->SetIcon(kTextIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// translate
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_TRANSLATE);
	fTranslateIB = new IconButton("translate icon", TOOL_TRANSLATE, NULL, msg, fCanvasView);
	fTranslateIB->SetIcon(kMoveIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// crop
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_CROP);
	fCropIB = new IconButton("crop icon", TOOL_CROP, NULL, msg, fCanvasView);
	fCropIB->SetIcon(kCropIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// fill
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_BUCKET_FILL);
	fFillIB = new IconButton("fill icon", TOOL_BUCKET_FILL, NULL, msg, fCanvasView);
	fFillIB->SetIcon(kFillIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// shape
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_SHAPE);
	fShapeIB = new IconButton("shape icon", TOOL_SHAPE, NULL, msg, fCanvasView);
	fShapeIB->SetIcon(kShapeIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// ellipse
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_ELLIPSE);
	fEllipseIB = new IconButton("ellipse icon", TOOL_ELLIPSE, NULL, msg, fCanvasView);
	fEllipseIB->SetIcon(kEllipseIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// round rect
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_ROUND_RECT);
	fRoundRectIB = new IconButton("round rect icon", TOOL_ROUND_RECT, NULL, msg, fCanvasView);
	fRoundRectIB->SetIcon(kRoundRectIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// edit gradient
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_EDIT_GRADIENT);
	fGradientIB = new IconButton("edit gradient icon", TOOL_EDIT_GRADIENT, NULL, msg, fCanvasView);
	fGradientIB->SetIcon(kGradientIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// guides
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_GUIDES);
	fGuidesIB = new IconButton("guides icon", TOOL_GUIDES, NULL, msg, fCanvasView);
	fGuidesIB->SetIcon(kGuidesIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// pick
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_PICK);
	fPickIB = new IconButton("pick icon", TOOL_PICK, NULL, msg, fCanvasView);
	fPickIB->SetIcon(kPickIconBits, kIconWidth, kIconHeight, kIconColorSpace);

	// zoom in
	fZoomInIB = new IconButton("zoom in icon", 0, NULL,
							   new BMessage(MSG_ZOOM_IN), fCanvasView);
	fZoomInIB->SetIcon(kZoomInIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	// zoom out
	fZoomOutIB = new IconButton("zoom out icon", 0, NULL,
								new BMessage(MSG_ZOOM_OUT), fCanvasView);
	fZoomOutIB->SetIcon(kZoomOutIconBits, kIconWidth, kIconHeight, kIconColorSpace);

	// trash icons
	fDeleteLayersIB = new FramedIconButton("delete layers icon", 0);
	fDeleteLayersIB->SetIcon(kSmallTrashIconBits, kSmallTrashIconWidth, kSmallTrashIconHeight, kIconColorSpace);

	fDeleteObjectsIB = new FramedIconButton("delete objects icon", 0);
	fDeleteObjectsIB->SetIcon(kSmallTrashIconBits, kSmallTrashIconWidth, kSmallTrashIconHeight, kIconColorSpace);

	// swatch views
	float h = 0;
	float s = 1.0;
	float v = 1.0;
	rgb_color color;
	color.alpha = 255;
	float r, g, b;
	for (int32 i = 0; i < 20; i++) {
		if (i < 10) {
			h = ((float)i / 9.0) * 6.0;
		} else {
			h = ((float)(i - 9) / 10.0) * 6.0;
			v = 0.5;
		}
		HSV_to_RGB(h, s, v, r, g, b);
		color.red = (uint8)(255.0 * r);
		color.green = (uint8)(255.0 * g);
		color.blue = (uint8)(255.0 * b);
		fSwatchViews[i] = new SwatchView("swatch", new BMessage(MSG_SET_COLOR),
										 this, color, 17.0, 14.0);
	}
	color.red = 0;
	color.green = 0;
	color.blue = 0;

	fColorField = new ColorField(BPoint(0.0, 0.0), H_SELECTED, 1.0, B_HORIZONTAL);
	fColorSlider = new ColorSlider(BPoint(0.0, 0.0), H_SELECTED, 1.0, 1.0, B_HORIZONTAL);
	fColorField->SetMarkerToColor(color);
	fColorSlider->SetMarkerToColor(color);

	// Canvas menu
	MenuBar* canvasMenu = new MenuBar("canvas");

	fCanvasM = new BMenu("Canvas");

	fNewCanvasMI = new BMenuItem("New", new BMessage(MSG_NEW), 'N');
	fCanvasM->AddItem(fNewCanvasMI);

	fCanvasM->AddSeparatorItem();

	fResizeCanvasMI = new BMenuItem("Resize"B_UTF8_ELLIPSIS, new BMessage(MSG_RESIZE_CANVAS), 'R');
	fCanvasM->AddItem(fResizeCanvasMI);

	// rotate menu
	fRotateCanvasM = new BMenu("Rotate");
	fCanvasM->AddItem(fRotateCanvasM);
	msg = new BMessage(MSG_ROTATE_CANVAS);
	msg->AddDouble("angle", -90.0);
	fRotateCanvas270MI = new BMenuItem("-90°", msg);
	fRotateCanvasM->AddItem(fRotateCanvas270MI);
	msg = new BMessage(MSG_ROTATE_CANVAS);
	msg->AddDouble("angle", 90.0);
	fRotateCanvas90MI = new BMenuItem("90°", msg);
	fRotateCanvasM->AddItem(fRotateCanvas90MI);
	msg = new BMessage(MSG_ROTATE_CANVAS);
	msg->AddDouble("angle", 180.0);
	fRotateCanvas180MI = new BMenuItem("180°", msg);
	fRotateCanvasM->AddItem(fRotateCanvas180MI);

	fCanvasM->AddSeparatorItem();

	fClearCanvasMI = new BMenuItem("Clear", new BMessage(MSG_CLEAR_ALL));
//	fCanvasM->AddItem(fClearCanvasMI);

	fCloseCanvasMI = new BMenuItem("Close", new BMessage(MSG_CLOSE_CANVAS), 'W', B_SHIFT_KEY);
	fCanvasM->AddItem(fCloseCanvasMI);

	fCanvasM->AddSeparatorItem();

	canvasMenu->AddItem(fCanvasM);

	fFormatM = new BMenu("Format");

	msg = new BMessage(MSG_SET_COLOR_SPACE);
	msg->AddInt32("color space", COLOR_SPACE_NONLINEAR_RGB);
	BMenuItem* item = new BMenuItem("R'G'B'", msg);
	item->SetMarked(true);
	fFormatM->AddItem(item);

	msg = new BMessage(MSG_SET_COLOR_SPACE);
	msg->AddInt32("color space", COLOR_SPACE_LINEAR_RGB);
	item = new BMenuItem("RGB", msg);
	fFormatM->AddItem(item);

	msg = new BMessage(MSG_SET_COLOR_SPACE);
	msg->AddInt32("color space", COLOR_SPACE_LAB);
	item = new BMenuItem("L*a*b*", msg);
	fFormatM->AddItem(item);

#if USE_COLORSPACES
	canvasMenu->AddItem(fFormatM);
#endif

	// History menu
	MenuBar* historyMenu = new MenuBar("history");
	fHistoryM = new BMenu("Object");
	historyMenu->AddItem(fHistoryM);

	MenuBar* propertyMenu = new MenuBar("property");
	BMenu* propertyM = new BMenu("Property");
	propertyMenu->AddItem(propertyM);

	// Layer menu
	MenuBar* layersMenu = new MenuBar("layers");
	fLayerM = new BMenu("Layer");
	layersMenu->AddItem(fLayerM);

	fModeM = new BMenu("Mode");

	layersMenu->AddItem(fModeM);

	HGroup* zoomGroup = new HGroup
	(
		fZoomPS = new ZoomSlider("zoom", "Zoom",
								 new BMessage(MSG_SET_ZOOM),
								 fCanvasView,
								 10, 3200, 100, "%ld%%"),
		horizontal_space(),
		fZoomOutIB,
		fZoomInIB,
		0
	);

	VGroup* sideGroup = new VGroup
	(
		minimax(0.0, 0.0, 10000.0, 10000.0, 0.0),
		new VGroup
		(
			canvasMenu,
			// zoom controls
			new MBorder
			(
				M_RAISED_BORDER, 4, "zoom controls",
				new VGroup
				(
#if CANVAS_LISTVIEW
					new CanvasListScrollView(fCanvasListView),
					vertical_space(),
#else
					fCanvasTabView,
#endif
					fNavigatorView,
					vertical_space(),
					zoomGroup,
					0
				)
			),
			0
		),
		fLayerVG = new VGroup
		(
			new HGroup
			(
				layersMenu,
				fDeleteLayersIB,
				0
			),
			new MBorder
			(
				M_RAISED_BORDER, 4, "layer group",
				new VGroup
				(
					new BetterMScrollView(fLayersListView, false, true),
					vertical_space(),
					fLayerConfigView,
					0
				)
			),
			0
		),
		new BetterMSplitter("list splitter", B_HORIZONTAL),
		fHistoryVG = new VGroup
		(
			new HGroup
			(
				historyMenu,
				fDeleteObjectsIB,
				0
			),
			new MBorder
			(
				M_RAISED_BORDER, 4, "object group",
				new BetterMScrollView(fHistoryListView, false, true)
			),
			0
		),
		new BetterMSplitter("list splitter", B_HORIZONTAL),
		fPropertyVG = new VGroup
		(
			propertyMenu,
			new MBorder
			(
				M_RAISED_BORDER, 4, "property group",
				new YBMScrollView(fPropertyListView,
								  SCROLL_VERTICAL,
								  "property scroll view")
			),
			0
		),
		// status/info view
		fInfoView,
		0
	);
	minimax sidePrefs = sideGroup->layoutprefs();
	// interface layout
#if SLIM_GUI
	BView* topView = new VGroup
	(
		new HGroup
		(
			new VGroup
			(
				_CreateMenuBar(),
				_CanvasIconView(sidePrefs.mini.x),
				0
			),
			new HGroup
			(
				_ToolsView(),
				_ToolSetupView(),
				_ConfirmGroup(),
				_PalettesView(),
				0
			),
			0
		),
		new HGroup
		(
			sideGroup,
			fCanvasView,
			0
		),
		0
	);
	fStatusBar = NULL;
#else
	VGroup* leftGroup;
	VGroup* rightGroup;
	VGroup* topGroup;
//	BetterMSplitter* mainSplitter;

	BView* topView = new HGroup
	(
		leftGroup = new VGroup
		(
			_CreateMenuBar(),
			_CanvasIconView(sidePrefs.mini.x),
			sideGroup,
			0
		),
		rightGroup = new VGroup
		(
			new HGroup
			(
//				mainSplitter = new BetterMSplitter("main splitter", B_VERTICAL),
				topGroup = new VGroup
				(
minimax(0.0, 0.0, 10000.0, 10000.0, 0.01),
					_ToolSetupView(),
					new HGroup
					(
						_ToolsView(),
//						fStatusBar = new StatusBar("Ready.", true, B_PLAIN_BORDER, false),
//new MBorder(M_RAISED_BORDER, 4, "spacer", new Space()),
						_ConfirmGroup(),
						0
					),
fStatusBar = new StatusBar("Ready.", true, B_PLAIN_BORDER, true),
					0
				),
				_PalettesView(),
				0
			),
//fStatusBar = new StatusBar("Ready.", true, B_PLAIN_BORDER, true),
			fCanvasView,
			0
		),
		0
	);
//	mainSplitter->SetSiblings(leftGroup, rightGroup);
rgb_color statusColor = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), 1.7);
fStatusBar->SetTextColor(statusColor);
topGroup->flags &= ~M_USE_FULL_SIZE;
#endif
	AddChild(topView);

	minimax zoomPrefs = zoomGroup->layoutprefs();
	fNavigatorView->mpm.mini.x = fNavigatorView->mpm.maxi.x = zoomPrefs.mini.x;
	fNavigatorView->mpm.mini.y = fNavigatorView->mpm.maxi.y = ceilf((zoomPrefs.mini.x * 3.0) / 4.0 + 1.0);

	fFormatM->SetRadioMode(true);
	fFormatM->SetTargetForItems(fCanvasView);
	fLayersListView->SetModeMenu(fModeM);
	fLayersListView->SetLayerMenu(fLayerM);
	fLayersListView->SetDeleteButton(fDeleteLayersIB);
	fLayerM->SetEnabled(false);
	fModeM->SetEnabled(false);
	fHistoryListView->SetHistoryMenu(fHistoryM);
	fHistoryListView->SetDeleteButton(fDeleteObjectsIB);
	fHistoryListView->SetPropertyList(fPropertyListView);
	fPropertyListView->SetMenu(propertyM);
	fShowGridMI->SetTarget(fCanvasView);
	fDropperSizeLP->Menu()->SetTargetForItems(fCanvasView);
	fTextTV->SetModificationMessage(new BMessage(MSG_TEXT_EDITED));
	fTextTV->SetTarget(fCanvasView);
//	fGradientTypeLP->Menu()->SetTargetForItems(fCanvasView);
	fGradientInterpolationLP->Menu()->SetTargetForItems(fCanvasView);
	fGuideHPosTC->SetTarget(fCanvasView);
	fGuideVPosTC->SetTarget(fCanvasView);
	fGuideAngleTC->SetTarget(fCanvasView);
	fGuideUnitsLP->Menu()->SetTargetForItems(fCanvasView);

	fCanvasView->SetBrushRadius(0.0, 15.0);

	fColorField->SetTarget(this);
	fColorSlider->SetTarget(this);

	AddShortcut('m', B_SHIFT_KEY, new BMessage(MSG_DUMP_MISSING_STRINGS));

	_BuildSwatchMenu();

	_LoadSettings();

	BMessenger windowMessenger(this, this);

#ifndef USE_SERIAL_NUMBERS
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) >= B_OK
		&& path.Append("WonderBrush") >= B_OK) {
		// watch this folder
		BEntry entry(path.Path());
		node_ref node;
		if (entry.GetNodeRef(&node) >= B_OK) {
			status_t err = watch_node(&node, B_WATCH_DIRECTORY, windowMessenger);
			if (err < B_OK)
				fprintf(stderr, "error trying to watch settings folder: %s\n", strerror(err));
		}
		// try to read keyfile now
		if (path.Append("keyfile") >= B_OK) {
			char appName[12];
			appName[0] = 'W';
			appName[1] = 'o';
			appName[2] = 'n';
			appName[3] = 'd';
			appName[4] = 'e';
			appName[5] = 'r';
			appName[6] = 'B';
			appName[7] = 'r';
			appName[8] = 'u';
			appName[9] = 's';
			appName[10] = 'h';
			appName[11] = 0;
			fKeyfile = new Keyfile(path.Path(), kPublicKey,
								   sizeof(kPublicKey), appName,
								   &windowMessenger);
		} else {
#ifndef TARGET_PLATFORM_HAIKU
			fCanvasView->SetDemoMode(true);
#endif
		}
	} else {
		fprintf(stderr, "no 'WonderBrush' settings folder\n");
	}
#endif // USE_SERIAL_NUMBERS

	be_clipboard->StartWatching(windowMessenger);
	PostMessage(B_CLIPBOARD_CHANGED);

	UpdateStrings();

	SetToDocument(NULL, true);
}

// destructor
MainWindow::~MainWindow()
{
	delete fSettings;
	delete fLastSavePath;
	delete fLastExportPath;
	delete fKeyfile;
	be_clipboard->StopWatching(BMessenger(this, this));
}

// QuitRequested
bool
MainWindow::QuitRequested()
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return false;
}

// MessageReceived
void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case B_OBSERVER_NOTICE_CHANGE: {
			uint32 oldWhat;
			if (message->FindInt32(B_OBSERVE_ORIGINAL_WHAT, (int32*)&oldWhat) >= B_OK) {
				switch (oldWhat) {
					case MSG_FONTS_CHANGED: {
						bool subMenus = FONT_SUB_MENUS;
						const char* family = fCanvasView->FontFamily();
						const char* style = fCanvasView->FontStyle();
						FontManager::Default()->PopulateMenu(fFontLP->Menu(), subMenus,
															 family, style);
						if (subMenus) {
							for (int32 i = 0; BMenuItem* item = fFontLP->Menu()->ItemAt(i); i++) {
								if (BMenu* subMenu = item->Submenu())
									subMenu->SetTargetForItems(fCanvasView);
							}
						} else {
							fFontLP->Menu()->SetTargetForItems(fCanvasView);
						}
						fFontLP->SetFamilyAndStyle(family, style);
						break;
					}
					default:
						break;
				}
			}
			break;
		}
		case B_NODE_MONITOR: {
			// we're watching the WonderBrush settings folder
			// maybe the user registered during WonderBrush runtime...
			// let's be fair!
			if (!fKeyfile || !fKeyfile->GetLicenceeInfo()) {
				BPath path;
				// maybe Tracker is still copying the rest of the file
				// give it some time
				snooze(2000000);
				if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) >= B_OK
					&& path.Append("WonderBrush/keyfile") >= B_OK) {
					delete fKeyfile;
					BEntry entry(path.Path());
					if (entry.Exists()) {
						BMessenger windowMessenger(this, this);
						char appName[12];
						appName[0] = 'W';
						appName[1] = 'o';
						appName[2] = 'n';
						appName[3] = 'd';
						appName[4] = 'e';
						appName[5] = 'r';
						appName[6] = 'B';
						appName[7] = 'r';
						appName[8] = 'u';
						appName[9] = 's';
						appName[10] = 'h';
						appName[11] = 0;
						fKeyfile = new Keyfile(path.Path(), kPublicKey,
											   sizeof(kPublicKey), appName,
											   &windowMessenger);
					} else
						fKeyfile = NULL;
				}
			}
			break;
		}
		case MSG_KEYFILE_VALIDATED: {
			bool demo = true;
			if (fKeyfile) {
				if (const LicenceeInfo* info = fKeyfile->GetLicenceeInfo()) {
					if (info->GetApplicationVersion() >= 150)
						demo = false;
				}
			}
#ifndef TARGET_PLATFORM_HAIKU
			fCanvasView->SetDemoMode(demo);
#endif
			break;
		}
		case MSG_DUMP_MISSING_STRINGS:
			LanguageManager::Default()->DumpMissingStrings();
			break;
		case MSG_ACTIVATE_LAYER: {
			int32 value;
			if (message->FindInt32("value", &value) >= B_OK)
				fToolSetupLG->ActivateLayer(value);
			break;
		}
		case MSG_GO_FULL_SCREEN:
			MWindow::Zoom();
			break;
		case MSG_LANGUAGES_CHANGED:
			UpdateStrings();
			break;
		case MSG_SET_CANVAS: {
			void* pointer;
			if (message->FindPointer("canvas", &pointer) == B_OK)
				SetToDocument((Canvas*)pointer);
			break;
		}
		case B_TRASH_TARGET: {
			if (message->IsReply()) {
				if (const BMessage* previous = message->Previous()) {
					Canvas* canvas;
					if (previous->FindPointer("be:originator_data", (void**)&canvas) >= B_OK) {
						CloseDocument(canvas);
					} else {
						fprintf(stderr, "B_TRASH_TARGET - no Canvas found in be:originator_data\n");
					}
				}
			}
			break;
		}
		case MSG_CLOSE_CANVAS:
			if (!fCanvasView->IsTracking())
				CloseDocument(fCanvasView->CurrentCanvas());
			break;
		case MSG_SETUP_PAGE:
			SetupPage(fCanvasView->CurrentCanvas());
			break;
		case MSG_PRINT:
			Print(fCanvasView->CurrentCanvas());
			break;
		case MSG_SET_ZOOM: {
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK)
				fZoomPS->SetValue(value);
			break;
		}
		case MSG_SET_BRUSH_FLAGS:
		case MSG_SET_TO_BRUSH: {
			uint32 flags;
			// restore control flags
			if (message->FindInt32("brush flags", (int32*)&flags) >= B_OK)
				_SetBrushFlags(flags);
			// restore alpha range
			float min, max;
			if (message->FindFloat("min alpha", &min) >= B_OK
				&& message->FindFloat("max alpha", &max) >= B_OK) {
				fCanvasView->SetMaxAlpha(min, max);
				fAlphaDS->SetValues(min, max);
			}
			// restore spacing range
			if (message->FindFloat("min spacing", &min) >= B_OK
				&& message->FindFloat("max spacing", &max) >= B_OK) {
				fCanvasView->SetBrushSpacing(min, max);
				fSpacingDS->SetValues(min, max);
			}
			// restore radius range
			if (message->FindFloat("min radius", &min) >= B_OK
				&& message->FindFloat("max radius", &max) >= B_OK) {
				float scale;
				if (message->FindFloat("brush scale", &scale) < B_OK)
					scale = 1.0;
				min *= scale;
				max *= scale;
				fCanvasView->SetBrushRadius(min, max);
				fRadiusDS->SetValues(min / 100.0, max / 100.0);
			}
			// restore hardness range
			if (message->FindFloat("min hardness", &min) >= B_OK
				&& message->FindFloat("max hardness", &max) >= B_OK) {
				fCanvasView->SetBrushHardness(min, max);
				fHardnessDS->SetValues(min, max);
			}
			break;
		}
		// text tool messages
		case MSG_SET_FONT: {
			const char* family;
			const char* style;
			if (message->FindString("family", &family) >= B_OK
				&& message->FindString("style", &style) >= B_OK)
				fFontLP->SetFamilyAndStyle(family, style);
			break;
		}
		case MSG_TEXT_EDITED: {
			const char* text;
			if (message->FindString("text", &text) >= B_OK) {
				// supress text changed messages from text view
				BMessage* textMessage = NULL;
				if (fTextTV->ModificationMessage())
					textMessage = new BMessage(*fTextTV->ModificationMessage());
				fTextTV->SetModificationMessage(NULL);
				// set text
				fTextTV->SetText(text);
				// reenable text changed messages
				fTextTV->SetModificationMessage(textMessage);
			}
			break;
		}
		case MSG_SET_TEXT_SIZE: {
			float size;
			if (message->FindFloat("value", &size) == B_OK)
				fFontSizeSB->SetMaxValue(size);
			break;
		}
		case MSG_SET_TEXT_ADVANCE_SCALE: {
			float scale;
			if (message->FindFloat("value", &scale) == B_OK)
				fFontAdvanceScaleDS->SetMaxValue(scale);
			break;
		}
		case MSG_SET_TEXT_OPACITY: {
			float opacity;
			if (message->FindFloat("value", &opacity) == B_OK)
				fFontOpacityDS->SetMaxValue(opacity);
			break;
		}
		case MSG_SET_TEXT_ALIGNMENT_MODE: {
			int32 mode;
			if (message->FindInt32("mode", &mode) >= B_OK)
				fFontAlignModeIOC->SetValue(mode);
			break;
		}
		// shape tool messages
		case MSG_SET_SHAPE_OPACITY: {
			float opacity;
			if (message->FindFloat("value", &opacity) == B_OK)
				fShapeOpacityDS->SetMaxValue(opacity);
			break;
		}
		case MSG_SET_SHAPE_OUTLINE: {
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK) {
				// message comes from checkbox
				fShapeOutlineWidthDS->SetEnabled(value == B_CONTROL_ON);
				fShapeCapModeIOC->SetEnabled(value == B_CONTROL_ON);
				fShapeJoinModeIOC->SetEnabled(value == B_CONTROL_ON);
				PostMessage(message, fCanvasView);
			} else {
				// message comes from state
				bool outline;
				if (message->FindBool("outline", &outline) == B_OK) {
					fShapeCapModeIOC->SetEnabled(outline);
					fShapeJoinModeIOC->SetEnabled(outline);
					fShapeOutlineWidthDS->SetEnabled(outline);
					fShapeOutlineCB->SetValue(outline);
				}
			}
			break;
   		}
//		case MSG_SET_SHAPE_CLOSED: {
//			bool closed;
//			if (message->FindBool("closed", &closed) == B_OK)
//				fShapeClosedCB->SetValue(closed);
//			break;
//		}
		case MSG_SET_SHAPE_OUTLINE_WIDTH: {
			float width;
			if (message->FindFloat("value", &width) == B_OK)
				fShapeOutlineWidthDS->SetMaxValue(width);
			break;
		}
//		case MSG_SHAPE_TRANSFORM:
//			fShapeTransformB->SetEnabled(fCanvasView->ShapeTransformEnabled());
//			break;
		case MSG_UPDATE_SHAPE_UI:
			_UpdateShapeControls(fCanvasView->CurrentCanvas() != NULL);
			break;
		case MSG_SET_CAP_MODE: {
			int32 mode;
			if (message->FindInt32("mode", &mode) >= B_OK)
				fShapeCapModeIOC->SetValue(mode);
			break;
		}
		case MSG_SET_JOIN_MODE: {
			int32 mode;
			if (message->FindInt32("mode", &mode) >= B_OK)
				fShapeJoinModeIOC->SetValue(mode);
			break;
		}
		// ellipse tool messages
		case MSG_SET_ELLIPSE_OPACITY: {
			float opacity;
			if (message->FindFloat("value", &opacity) == B_OK)
				fEllipseOpacityDS->SetMaxValue(opacity);
			break;
		}
		case MSG_SET_ELLIPSE_OUTLINE: {
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK) {
				// message comes from checkbox
				fEllipseOutlineWidthDS->SetEnabled(value == B_CONTROL_ON);
				PostMessage(message, fCanvasView);
			} else {
				// message comes from state
				bool outline;
				if (message->FindBool("outline", &outline) == B_OK) {
					fEllipseOutlineWidthDS->SetEnabled(outline);
					fEllipseOutlineCB->SetValue(outline);
				}
			}
			break;
   		}
		case MSG_SET_ELLIPSE_OUTLINE_WIDTH: {
			float width;
			if (message->FindFloat("value", &width) == B_OK)
				fEllipseOutlineWidthDS->SetMaxValue(width);
			break;
		}
		// round rect tool messages
		case MSG_SET_RECT_OPACITY: {
			float opacity;
			if (message->FindFloat("value", &opacity) == B_OK)
				fRectOpacityDS->SetMaxValue(opacity);
			break;
		}
		case MSG_SET_RECT_OUTLINE: {
			int32 value;
			if (message->FindInt32("be:value", &value) == B_OK) {
				// message comes from checkbox
				fRectOutlineWidthDS->SetEnabled(value == B_CONTROL_ON);
				PostMessage(message, fCanvasView);
			} else {
				// message comes from state
				bool outline;
				if (message->FindBool("outline", &outline) == B_OK) {
					fRectOutlineWidthDS->SetEnabled(outline);
					fRectOutlineCB->SetValue(outline);
				}
			}
			break;
   		}
		case MSG_SET_RECT_OUTLINE_WIDTH: {
			float width;
			if (message->FindFloat("value", &width) == B_OK)
				fRectOutlineWidthDS->SetMaxValue(width);
			break;
		}
		case MSG_SET_RECT_CORNER_RADIUS: {
			float radius;
			if (message->FindFloat("value", &radius) == B_OK)
				fRectCornerRadiusDS->SetMaxValue(radius);
			break;
		}
		// edit gradient tool message
		case MSG_SET_GRADIENT_TYPE: {
			int32 type;
			if (message->FindInt32("type", &type) >= B_OK) {
				fGradientTypeIOC->SetValue(type);
			}
			break;
		}
		case MSG_SET_GRADIENT_INTERPOLATION: {
			// both messages do the same, but to a different control:
			// mark the menu item that corresponds to the type in the message
//			LabelPopup* lp = fGradientTypeLP;
//			if (message->what == MSG_SET_GRADIENT_INTERPOLATION)
//				lp = fGradientInterpolationLP;
			LabelPopup* lp = fGradientInterpolationLP;

			int32 type;
			if (message->FindInt32("type", &type) >= B_OK) {
				for (int32 i = 0; BMenuItem* item = lp->Menu()->ItemAt(i); i++) {
					int32 itemType;
					if (item->Message() && item->Message()->FindInt32("type", &itemType) >= B_OK
						&& itemType == type) {
						item->SetMarked(true);
						break;
					}
				}
			}
			break;
		}
		case MSG_SET_GRADIENT_INHERITS_TRANSFORM: {
			bool inherits;
			if (message->FindBool("inherits", &inherits) >= B_OK) {
				fGradientInheritsTransformCB->SetValue(inherits);
			}
			break;
		}
		case MSG_GRADIENT_CONTROL_FOCUS_CHANGED:
			fGradientStopAlphaDS->SetEnabled(fGradientControl->IsFocus());
			break;
		// guide tool messages
		case MSG_GUIDE_SELECTED:
			fGuideRemoveIB->SetEnabled(message->HasBool("selected"));
			fGuideProportionIB->SetEnabled(message->HasBool("selected"));
			break;
		case MSG_SET_GUIDE_H_POS:
		case MSG_SET_GUIDE_V_POS:
		case MSG_SET_GUIDE_ANGLE: {
			float value;
			if (message->FindFloat("value", &value) >= B_OK) {
				switch (message->what) {
					case MSG_SET_GUIDE_H_POS:
						fGuideHPosTC->SetValue(value);
						break;
					case MSG_SET_GUIDE_V_POS:
						fGuideVPosTC->SetValue(value);
						break;
					case MSG_SET_GUIDE_ANGLE:
						fGuideAngleTC->SetValue(value);
						break;
				}
			}
			break;
		}
		case MSG_SHOW_GUIDES: {
			bool show;
			if (message->FindBool("show guides", &show) >= B_OK)
				fGuidesShowCB->SetValue(show);
			break;
		}
		// pick objects tool messages
		case MSG_UPDATE_PICK_UI:
			_UpdatePickControls(fCanvasView->CurrentCanvas() != NULL);
			break;
		// brush tool messages
		case MSG_SET_RADIUS: {
			float min;
			float max;
			if (message->FindFloat("min value", &min) == B_OK
				&& message->FindFloat("max value", &max) == B_OK)
				fRadiusDS->SetValues(min, max);
			break;
		}
		case MSG_SUB_PIXEL_PRECISION: {
			bool active;
			uint32 toolID;
			if (message->FindBool("active", &active) >= B_OK
				&& message->FindInt32("tool", (int32*)&toolID) >= B_OK) {
				switch (toolID) {
					case TOOL_CROP:
						fCropSubPixelCB->SetValue(active);
						break;
					case TOOL_TRANSLATE:
						fMoveSubPixelCB->SetValue(active);
						break;
					case TOOL_SHAPE:
						fShapeSubpixelsCB->SetValue(active);
						break;
					case TOOL_ELLIPSE:
						fEllipseSubpixelsCB->SetValue(active);
						break;
					case TOOL_ROUND_RECT:
						fRectSubpixelsCB->SetValue(active);
						break;
					case TOOL_BRUSH:
					case TOOL_CLONE:
					case TOOL_BLUR:
					case TOOL_PEN:
					case TOOL_PEN_ERASER:
					case TOOL_ERASER:
						fBrushSubPixelCB->SetValue(active);
						break;
				}
			}
			break;
		}
		case MSG_SHOW_GRID: {
			bool active;
			if (message->FindBool("active", &active) >= B_OK)
				fShowGridMI->SetMarked(active);
			break;
		}
		// messages from Transform tool controls
		case MSG_TRANSLATE:
		case MSG_ROTATE:
		case MSG_SCALE: {
			message->AddPoint("translation", BPoint(fXOffsetTC->FloatValue(),
													fYOffsetTC->FloatValue()));
			message->AddDouble("rotation", fRotationTC->FloatValue());
			message->AddDouble("x scale", fScaleXTC->FloatValue());
			message->AddDouble("y scale", fScaleYTC->FloatValue());
			PostMessage(message, fCanvasView);
			break;
		}
		case MSG_TRANSFORMATION_CHANGED: {
			// message comes from the TransformState
			BPoint translation;
			double rotation;
			double xScale;
			double yScale;
			bool enable = false;
			if (message->FindPoint("translation", &translation) >= B_OK
				&& message->FindDouble("rotation", &rotation) >= B_OK
				&& message->FindDouble("x scale", &xScale) >= B_OK
				&& message->FindDouble("y scale", &yScale) >= B_OK) {
				// change values in the controls
				fXOffsetTC->SetValue(translation.x);
				fYOffsetTC->SetValue(translation.y);
				fRotationTC->SetValue(rotation);
				fScaleXTC->SetValue(xScale);
				fScaleYTC->SetValue(yScale);
				enable = true;
			} else {
				// disable controls
				fXOffsetTC->SetText("");
				fYOffsetTC->SetText("");
				fRotationTC->SetText("");
				fScaleXTC->SetText("");
				fScaleYTC->SetText("");
			}
			fXOffsetTC->SetEnabled(enable);
			fYOffsetTC->SetEnabled(enable);
			fRotationTC->SetEnabled(enable);
			fScaleXTC->SetEnabled(enable);
			fScaleYTC->SetEnabled(enable);
			break;
		}
		case MSG_CROPPING: {
			BRect rect;
			if (message->FindRect("rect", &rect) >= B_OK) {
				// message comes from the CropState
				bool valid = rect.IsValid();
				if (valid) {
					fCropLeftTC->SetValue(rect.left);
					fCropTopTC->SetValue(rect.top);
					fCropWidthTC->SetValue(rect.Width() + 1.0);
					fCropHeightTC->SetValue(rect.Height() + 1.0);
				} else {
					fCropLeftTC->SetText("");
					fCropTopTC->SetText("");
					fCropWidthTC->SetText("");
					fCropHeightTC->SetText("");
				}
				fCropLeftTC->SetEnabled(valid);
				fCropTopTC->SetEnabled(valid);
				fCropWidthTC->SetEnabled(valid);
				fCropHeightTC->SetEnabled(valid);
			} else if (message->HasPointer("source")) {
				// message comes from on of the text controls
				rect.left = fCropLeftTC->FloatValue();
				rect.top = fCropTopTC->FloatValue();
				rect.right = rect.left + fCropWidthTC->FloatValue() - 1.0;
				rect.bottom = rect.top + fCropHeightTC->FloatValue() - 1.0;
				message->AddRect("rect", rect);
				PostMessage(message, fCanvasView);
			}
			break;
		}
		case MSG_SELECTING: {
			BRect rect;
			if (message->FindRect("rect", &rect) >= B_OK) {
				// message comes from the CropState
				bool valid = rect.IsValid();
				if (valid) {
					fSelectLeftTC->SetValue(rect.left);
					fSelectTopTC->SetValue(rect.top);
					fSelectWidthTC->SetValue(rect.Width() + 1.0);
					fSelectHeightTC->SetValue(rect.Height() + 1.0);
				} else {
					fSelectLeftTC->SetText("");
					fSelectTopTC->SetText("");
					fSelectWidthTC->SetText("");
					fSelectHeightTC->SetText("");
				}
				fSelectLeftTC->SetEnabled(valid);
				fSelectTopTC->SetEnabled(valid);
				fSelectWidthTC->SetEnabled(valid);
				fSelectHeightTC->SetEnabled(valid);
			} else if (message->HasPointer("source")) {
				// message comes from on of the text controls
				rect.left = fSelectLeftTC->FloatValue();
				rect.top = fSelectTopTC->FloatValue();
				rect.right = rect.left + fSelectWidthTC->FloatValue() - 1.0;
				rect.bottom = rect.top + fSelectHeightTC->FloatValue() - 1.0;
				message->AddRect("rect", rect);
				PostMessage(message, fCanvasView);
			}
			break;
		}
		case MSG_UNDO:
			fCanvasView->Undo();
			break;
		case MSG_REDO:
			fCanvasView->Redo();
			break;
		case MSG_SET_TOOL: {
			uint32 id;
			// toggle tool icons pressed state
			if (message->FindInt32("id", (int32*)&id) == B_OK)
				_SetTool(id);
			break;
		}
		case MSG_CLEAR_ALL:
/*			if (!fCanvasView->IsTracking()) {
				if (Canvas* canvas = fCanvasView->CurrentCanvas()) {
					canvas->MakeEmpty();
					Layer* layer = new Layer(canvas->Bounds());
					if (layer->InitCheck() >= B_OK && canvas->AddLayer(layer, 0)) {
						fCanvasView->SetToLayer(0);
						fLayersListView->ClearList();
						fLayersListView->AddLayer(layer->Name(), 0);
						fCanvasView->InvalidateCanvas(canvas->Bounds(), true);
					} else
						delete layer;
				}
			}*/
			break;
		case MSG_SET_COLOR: {
//printf("MainWindow::MessageReceived(MSG_SET_COLOR)\n");
			rgb_color color;
			if (restore_color_from_message(message, color) == B_OK) {
				// propagate the color to either the color picker or canvas view
				if (fColorPickerPanel && !message->HasRect("panel frame")) {
					if (fColorPickerPanel->Lock()) {
						fColorPickerPanel->SetColor(color);
						fColorPickerPanel->Unlock();
					}
				} else {
					fCurrentColorSV->SetColor(color);
					PostMessage(message, fCanvasView);
					// update colorfield and color slider
					float h, s, v;
					RGB_to_HSV(color.red / 255.0, color.green / 255.0, color.blue / 255.0, h, s, v);
					// if color is grey then h can not be restored
					float hue;
					if (message->FindFloat("hue", &hue) >= B_OK)
						h = hue;
					if (!fColorField->IsTracking()) {
						fColorField->SetFixedValue(h);
						fColorField->SetMarkerToColor(color);
					}
					if (!fColorSlider->IsTracking()) {
						fColorSlider->SetOtherValues(s, v);
						fColorSlider->SetValue(255 - (int32)((h / 6.0) * 255.0 + 0.5));
					}
					if (fGradientControl->IsFocus()) {
						// very high chance of this message comming
						// from the gradient control
						float value = (float)color.alpha / 255.0;
						fGradientStopAlphaDS->SetMaxValue(value);
					}
				}
			}
			// if message contains these fields,
			// then it comes from the color picker panel.
			// it also means the panel has died.
			BRect frame;
			uint32 mode;
			if (message->FindRect("panel frame", &frame) == B_OK
				&& message->FindInt32("panel mode", (int32*)&mode) == B_OK) {
				// message came from the color picker panel
				// we remember the settings of the panel for later
				fColorPickerFrame = frame;
				fColorPickerMode = (selected_color_mode)mode;
				// color picker panel has quit
				fColorPickerPanel = NULL;
				if (fRememberedTool < TOOL_LAST) {
					fCanvasView->SetTool(fRememberedTool);
					_SetTool(fRememberedTool);
				}
			}
			break;
		}
		case MSG_GET_COLOR: {
			rgb_color color;
			if (restore_color_from_message(message, color) == B_OK) {
				if (!fColorPickerPanel) {
					fColorPickerPanel = new ColorPickerPanel(fColorPickerFrame,
															 color, fColorPickerMode,
															 new BMessage(MSG_SET_COLOR),
															 this);
					fColorPickerPanel->Show();
					// TODO: COLOR_PICKER needs to be a special tool
					if (fCanvasView->Tool() != TOOL_TEXT
						&& fCanvasView->Tool() != TOOL_SHAPE) {
						fRememberedTool = fCanvasView->Tool();
						fCanvasView->SetTool(TOOL_COLOR_PICKER);
						_SetTool(TOOL_COLOR_PICKER);
					} else
						fRememberedTool = TOOL_LAST;
				} else {
					if (fColorPickerPanel->Lock()) {
						fColorPickerPanel->Activate();
						fColorPickerPanel->Unlock();
					}
				}
			}
			break;
		}
		case MSG_COLOR_FIELD: {
			// get h from color slider
			float h = ((255 - fColorSlider->Value()) / 255.0) * 6.0;
			float s, v;
			// s and v are comming from the message
			if (message->FindFloat("value", &s) >= B_OK
				&& message->FindFloat("value", 1, &v) >= B_OK) {

				float r, g, b;
				HSV_to_RGB(h, s, v, r, g, b);
				rgb_color color;
				color.red = (uint8)(r * 255.0);
				color.green = (uint8)(g * 255.0);
				color.blue = (uint8)(b * 255.0);
				color.alpha = 255;

				BMessage msg(*message);
				msg.what = MSG_SET_COLOR;
				store_color_in_message(&msg, color);
				msg.AddFloat("hue", h);
				PostMessage(&msg);
			}
			break;
		}
		case MSG_COLOR_SLIDER: {
			float h;
			float s, v;
			fColorSlider->GetOtherValues(&s, &v);
			// h is comming from the message
			if (message->FindFloat("value", &h) >= B_OK) {

				float r, g, b;
				HSV_to_RGB(h, s, v, r, g, b);
				rgb_color color;
				color.red = (uint8)(r * 255.0);
				color.green = (uint8)(g * 255.0);
				color.blue = (uint8)(b * 255.0);
				color.alpha = 255;

				BMessage msg(*message);
				msg.what = MSG_SET_COLOR;
				store_color_in_message(&msg, color);
				msg.AddFloat("hue", h);
				PostMessage(&msg);
			}
			break;
		}
		// settings panel
		case MSG_GLOBAL_SETTINGS: {
			// if message contains these fields,
			// then it comes from the color picker panel.
			// it also means the panel has died.
			BRect frame;
			if (message->FindRect("panel frame", &frame) >= B_OK) {
				// message came from the settings panel
				// we remember the settings of the panel for later
				fSettingsPanelFrame = frame;
				// color picker panel has quit
				fSettingsPanel = NULL;
			} else {
				if (fSettingsPanel) {
					// activate the panel
					if (fSettingsPanel->Lock()) {
						fSettingsPanel->Activate();
						fSettingsPanel->Unlock();
					}
				} else {
					// create and show the panel
					fSettingsPanel = new SettingsPanel(fSettingsPanelFrame,
													   new BMessage(MSG_GLOBAL_SETTINGS),
													   this);
					fSettingsPanel->Show();
				}
			}
			break;
		}
		case B_CUT:
		case B_COPY:
			if (!message->HasBool("ignore")) {
				if (BView* focus = CurrentFocus()) {
					message->AddBool("ignore", true);
					PostMessage(message, focus);
				}
			}
			break;
		// clipboard related messages
		case B_PASTE:
			if (message->HasBool("new canvas") || !fCanvasView->CurrentCanvas()) {
				// handle the message ourselves by creating a
				// new canvas with the clipboard contents
				if (be_clipboard->Lock()) {
					if (BMessage* data = be_clipboard->Data()) {
						BMessage bitmapArchive;
						if (data->FindMessage("image/bitmap", &bitmapArchive) >= B_OK) {
							BBitmap* bitmap = new BBitmap(&bitmapArchive);
							if (bitmap && bitmap->IsValid()
								&& (bitmap->ColorSpace() == B_RGB32
									|| bitmap->ColorSpace() == B_RGBA32)) {

								Layer* layer = new Layer(bitmap);
								if (layer->InitCheck() >= B_OK) {
									layer->SetName("Bitmap clipping");
									Canvas* canvas = NewDocument(bitmap->Bounds(), layer);
									if (canvas) {
										canvas->SetName("Bitmap clipping");
										AddDocument(canvas);
										SetToDocument(canvas);
									}
								} else {
									delete layer;
								}
							} else {
								delete bitmap;
							}
						}
					}
					be_clipboard->Unlock();
				}
			} else {
				if (!fCanvasView->HandlePaste(message) && !message->HasBool("ignore")) {
					if (BView* focus = CurrentFocus()) {
						message->AddBool("ignore", true);
						PostMessage(message, focus);
					}
				}
			}
			break;
		case B_CLIPBOARD_CHANGED:
			if (be_clipboard->Lock()) {
				BMessage* data = be_clipboard->Data();
				if (data && data->HasMessage("image/bitmap"))
					fPasteMI->SetEnabled(true);
				else
					fPasteMI->SetEnabled(false);
				be_clipboard->Unlock();
			}
			break;
		case MSG_SET_SWATCHES:
			_LoadSwatches(message);
			break;
		case MSG_SAVE_SWATCHES: {
			entry_ref ref;
			const char* name;
			if (message->FindRef("ref", &ref) >= B_OK) {
				_SaveSwatches(message, &ref);
			} else if (message->FindString("name", &name) >= B_OK) {
				// message contains name
				BPath path;
				if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) >= B_OK
					&& path.Append("WonderBrush/swatches") >= B_OK
					&& path.Append(name) >= B_OK) {
					BEntry entry(path.Path());
					if (entry.GetRef(&ref) >= B_OK)
						_SaveSwatches(message, &ref);
				}
			} else {
				// ask name from user
				LanguageManager* manager = LanguageManager::Default();
				new NamePanel(manager->GetString(NAME, "Name"), NULL, this, this,
												 new BMessage(*message));
			}
			break;
		}
		case MSG_DELETE_SWATCHES: {
			entry_ref ref;
			if (message->FindRef("ref", &ref) >= B_OK) {
				BEntry entry(&ref);
				if (entry.Remove() >= B_OK)
					_BuildSwatchMenu();
			}
			break;
		}
		case MSG_NEW: {
			int32 width, height;
			if (message->FindInt32("width", &width) >= B_OK
				&& message->FindInt32("height", &height) >= B_OK) {
				BRect canvasFrame(0.0, 0.0, width - 1.0, height - 1.0);
				if (Canvas* canvas = NewDocument(canvasFrame)) {
					const char* name;
					if (message->FindString("name", &name) >= B_OK)
						canvas->SetName(name);
					AddDocument(canvas);
					SetToDocument(canvas);
				}
			}
			BRect frame;
			if (!message->HasBool("no panel")) {
				if (message->FindRect("panel frame", &frame) == B_OK)
					fNewCanvasFrame = frame;
				else {
				 	NewCanvasPanel* panel = new NewCanvasPanel(fNewCanvasFrame,
															   new BMessage(MSG_NEW),
															   this);
					panel->Show();
				}
			}
			break;
		}
		case MSG_RESIZE_CANVAS: {
			int32 width, height;
			Canvas* canvas;
			if (message->FindInt32("width", &width) >= B_OK
				&& message->FindInt32("height", &height) >= B_OK
				&& message->FindPointer("canvas", (void**)&canvas) >= B_OK
				&& fDocuments.HasItem((void*)canvas)) {
				BRect canvasFrame(0.0, 0.0, width - 1.0, height - 1.0);
				fCanvasView->Perform(new ResizeAction(canvas, canvasFrame));
			}
			BRect frame;
			if (message->FindRect("panel frame", &frame) == B_OK)
				fResizeCanvasFrame = frame;
			else {
				if (Canvas* canvas = fCanvasView->CurrentCanvas()) {
					BRect bounds = canvas->Bounds();
					BMessage* resizeMessage = new BMessage(MSG_RESIZE_CANVAS);
					resizeMessage->AddPointer("canvas", (void*)canvas);
				 	NewSizePanel* panel = new NewSizePanel(fResizeCanvasFrame,
				 										   bounds,
														   resizeMessage,
														   this);
					panel->Show();
				}
			}
			break;
		}
		case MSG_ROTATE_CANVAS: {
			double angle;
			if (message->FindDouble("angle", &angle) >= B_OK) {
				fCanvasView->Perform(new RotateAction(fCanvasView->CurrentCanvas(), angle));
			}
			break;
		}
		case MSG_OPEN:
			if (fLastSavePath)
				message->AddRef("last path", fLastSavePath);
			else if (fLastExportPath)
				message->AddRef("last path", fLastExportPath);
			be_app->PostMessage(message);
			break;
		case B_MIME_DATA:
//printf("B_MIME_DATA\n");
//message->PrintToStream();
			break;
		case BBP_OPEN_BBITMAP: {
			entry_ref ref;
			for (int32 i = 0; message->FindRef("ref", i, &ref) >= B_OK; i++)
				message->AddRef("refs", &ref);
			// fall through
		}
		case B_SIMPLE_DATA:
			if (message->HasInt32("be:actions")) {
				uint32 action;
				bool canCopy = false;
				for (int32 i = 0; message->FindInt32("be:actions", i, (int32*)&action) >= B_OK; i++) {
					if (action == B_COPY_TARGET) {
						canCopy = true;
						break;
					}
				}
				if (canCopy) {
					if (message->HasString("be:filetypes")) {
						const char* format = NULL;
						const char* imageFormat = NULL;
						for (int32 i = 0; message->FindString("be:filetypes", i, &format) >= B_OK; i++) {
							BString test(format);
							if (test.FindFirst("image") == 0) {
								imageFormat = format;
								// look no further if we have found PNG
								if (test == "image/png") {
									break;
								}
							}
						}
						if (imageFormat) {
							BMessage reply(B_COPY_TARGET);
							reply.AddString("be:types", B_FILE_MIME_TYPE);
							reply.AddString("be:filetypes", imageFormat);
							entry_ref tmpRef;
							if (get_ref_for_path("/tmp", &tmpRef) >= B_OK)
								reply.AddRef("directory", &tmpRef);
							else
								fprintf(stderr, "failed to get entry_ref for /tmp!\n");
							const char* clipName;
							if (message->FindString("be:clip_name", &clipName) < B_OK)
								clipName = LanguageManager::Default()->GetString(BITMAP, "Bitmap");
							reply.AddString("name", clipName);

							BPath path("/tmp");
							if (path.InitCheck() >= B_OK &&
								path.Append(clipName) >= B_OK) {
								wait_info* info = new wait_info;
								BEntry entry(path.Path());
								if (entry.Exists())
									entry.Remove();

								message->SendReply(&reply);

								entry.GetRef(&tmpRef);
								info->ref = tmpRef;
								info->window = this;
								// launch a thread which wait's on this file
								thread_id thread = spawn_thread(_wait_for_file, "waiter", B_LOW_PRIORITY, info);
								if (thread >= B_OK)
									resume_thread(thread);
							}
						}
					}
				}
				// stop here (don't fall through for negotiated drag&drop)
				break;
			} // else fall through
		case B_REFS_RECEIVED: {

			bool append = modifiers() & B_SHIFT_KEY;
			int32 appendIndex;
			if (message->FindInt32("append index", &appendIndex) < B_OK)
				appendIndex = -1;
			else
				append = true;

			load_info* info = new load_info;
			info->window = this;
			info->canvas_view = append ? fCanvasView : NULL;
			info->message = new BMessage(*message);
			info->append_index = appendIndex;

			fLoadingThread = spawn_thread(load, "loader", B_LOW_PRIORITY, info);
			if (fLoadingThread >= B_OK)
				resume_thread(fLoadingThread);

			break;
		}
		case MSG_EXPORT:
		case MSG_SAVE_DOCUMENT: {
			Canvas* canvas = NULL;
			if (message->FindPointer("canvas", (void**)&canvas) < B_OK
				|| !fDocuments.HasItem((void*)canvas))
				canvas = fCanvasView->CurrentCanvas();
			if (canvas) {

				const entry_ref* ref = NULL;
				Exporter* exporter = NULL;

				if (message->what == MSG_EXPORT) {
					exporter = canvas->GetExporter();
					ref = canvas->ExportRef();
				} else {
					ref = canvas->DocumentRef();
				}

				if (!ref || (message->what == MSG_EXPORT && !exporter)) {
					// fall back to SAVE/EXPORT AS
					BMessage newMessage(*message);
					newMessage.what = message->what == MSG_EXPORT ? MSG_EXPORT_AS
																  : MSG_SAVE_DOCUMENT_AS;
					if (!newMessage.HasPointer("canvas"))
						newMessage.AddPointer("canvas", (void*)canvas);
					PostMessage(&newMessage);
				} else {
					Save(canvas, ref, exporter);
					// remember this path
					_RememberPath(ref, message->what == MSG_SAVE_DOCUMENT);
					// maybe we are supposed to close the document or quit even
					// (picking up where we left, if we saved the document in
					// the middle of shutdown)
					if (message->HasBool("quit"))
						PostMessage(B_QUIT_REQUESTED);
					else if (message->HasBool("close"))
						CloseDocument(canvas);
				}
			}
			break;
		}
		case B_COPY_TARGET: {
			bool success = false;
			if (message->IsReply()) {
				if (const BMessage* previous = message->Previous()) {
					Canvas* canvas;
					if (previous->FindPointer("be:originator_data", (void**)&canvas) >= B_OK) {
						message->AddPointer("canvas", (void*)canvas);
						success = true;
					} else {
						fprintf(stderr, "B_COPY_TARGET - no Canvas found in be:originator_data\n");
					}
				}
			}
			if (!success)
				break;
			// else fall through to next case
		}
		case MSG_EXPORT_AS:
		case MSG_SAVE_DOCUMENT_AS: {
			entry_ref ref;
			const char* name;
			if (message->FindRef("directory", &ref) == B_OK
				&& message->FindString("name", &name) == B_OK) {
				// this message comes from the file panel
				BDirectory dir(&ref);
				BEntry entry;
				status_t status = dir.InitCheck();
				if (status >= B_OK) {
					status = entry.SetTo(&dir, name, true);
					if (status >= B_OK) {
						entry.GetRef(&ref);

						// remember save path
						_RememberPath(&ref, message->what == MSG_SAVE_DOCUMENT_AS);

						Canvas* canvas = NULL;
						if (message->FindPointer("canvas", (void**)&canvas) < B_OK
							|| !fDocuments.HasItem((void*)canvas))
							canvas = fCanvasView->CurrentCanvas();

						Exporter* exporter = NULL;
						int32 exportMode;
						if (message->what == MSG_EXPORT_AS &&
							message->FindInt32("export mode", &exportMode) >= B_OK) {

							// try to find translator info
							translator_id id = 0;
							const translator_id* idPointer;
							uint32 format;
							ssize_t length = sizeof(translator_id);
							if (message->FindData("translator_id", B_RAW_TYPE,
												  (const void**)&idPointer, &length) < B_OK)
								idPointer = &id;
							if (message->FindInt32("translator_format", (int32 *)&format) < B_OK)
								format = 0;
							// instantiate an Exporter for the given mode
							exporter = Exporter::ExporterFor(exportMode, *idPointer, format);
						}
						if (exporter)
							canvas->SetExporter(exporter);

						Save(canvas, &ref, exporter);
						// maybe we are supposed to close the document or quit even
						// (picking up where we left if we saved the document in
						// the middle of shutdown)
						if (message->HasBool("quit"))
							PostMessage(B_QUIT_REQUESTED);
						else if (message->HasBool("close"))
							CloseDocument(canvas);
					} else
						fprintf(stderr, "export failed - invalid entry: %s\n", strerror(status));
				} else
					fprintf(stderr, "export failed - invalid folder: %s\n", strerror(status));
			} else {
				// we need to setup the file panel
				Canvas* canvas = NULL;
				if (message->FindPointer("canvas", (void**)&canvas) < B_OK
					|| !fDocuments.HasItem((void*)canvas)) {
					canvas = fCanvasView->CurrentCanvas();
					message->AddPointer("canvas", canvas);
				}

				fSavePanel->SetMessage(new BMessage(*message));

				if (canvas) {
					// customize panel to canvas
					BEntry parentFolder;
					if (message->what == MSG_EXPORT_AS) {
						if (canvas->ExportRef())
							parentFolder.SetTo(canvas->ExportRef());
						else {
							if (fLastExportPath)
								parentFolder.SetTo(fLastExportPath);
							else
								parentFolder.SetTo(canvas->DocumentRef());
						}

						// customize panel to current exporter of canvas
						Exporter* exporter = canvas->GetExporter();
						if (exporter) {
							// adjust the panel GUI to indicate current exporter
							exporter->CustomizeExportPanel(fSavePanel);
						} else {
							// automatically use the last used settings
						}
						fSavePanel->SetMode(true);
					} else {
						if (canvas->DocumentRef())
							parentFolder.SetTo(canvas->DocumentRef());
						else {
							if (fLastSavePath)
								parentFolder.SetTo(fLastSavePath);
							else
								parentFolder.SetTo(canvas->ExportRef());
						}
						fSavePanel->SetMode(false);
					}
					// set name
					fSavePanel->SetSaveText(canvas->Name());
					// adjust the name extension
					fSavePanel->AdjustExtension();

					if (parentFolder.GetParent(&parentFolder) >= B_OK)
						fSavePanel->SetPanelDirectory(&parentFolder);
					// show the panel
					fSavePanel->Refresh();
					fSavePanel->Show();
				}
			}
			break;
		}
		case B_SELECT_ALL:
			if (dynamic_cast<BTextView*>(CurrentFocus()) == NULL)
				fCanvasView->MessageReceived(message);
			else
				MWindow::MessageReceived(message);
			break;
		case B_MODIFIERS_CHANGED:
			fCanvasView->ModifiersChanged();
			fLayersListView->ModifiersChanged();
			fHistoryListView->ModifiersChanged();
			break;
		case MSG_UPDATE_HISTORY_ITEMS: {
			LanguageManager* manager = LanguageManager::Default();
			BubbleHelper* bh = BubbleHelper::Default();
			const char* undoString = manager->GetString(UNDO, "Undo");
			const char* redoString = manager->GetString(REDO, "Redo");
			if (Canvas* canvas = fCanvasView->CurrentCanvas()) {
				HistoryManager* history = canvas->GetHistoryManager();
				if (history) {
					BString helper(undoString);
					bool enable = history->GetUndoName(helper);
					fUndoMI->SetLabel(helper.String());
					fUndoMI->SetEnabled(enable);
					fUndoIB->SetEnabled(enable);
					bh->SetHelp(fUndoIB, helper.String());
					helper = redoString;
					enable = history->GetRedoName(helper);
					fRedoMI->SetLabel(helper.String());
					fRedoMI->SetEnabled(enable);
					fRedoIB->SetEnabled(enable);
					bh->SetHelp(fRedoIB, helper.String());
				}
			} else {
				fUndoMI->SetEnabled(false);
				fRedoMI->SetEnabled(false);
				fUndoMI->SetLabel(undoString);
				fRedoMI->SetLabel(redoString);
				fUndoIB->SetEnabled(false);
				fRedoIB->SetEnabled(false);
				bh->SetHelp(fUndoIB, undoString);
				bh->SetHelp(fRedoIB, redoString);
			}
			break;
		}
		case MSG_SET_VISIBLE:
//			PostMessage(message, fLayerConfigView);
			fLayerConfigView->MessageReceived(message);
			break;
		case MSG_BRUSH_PANEL: {
			if (!fBrushPanel) {
				fBrushPanel = new BrushPanel(BRect(50.0, 50.0, 300.0, 250.0),
											 this, fCanvasView);
			} else {
				if (fBrushPanel->Lock()) {
					fBrushPanel->Activate();
					fBrushPanel->Unlock();
				}
			}
			break;
		}
		case MSG_BRUSH_PANEL_QUIT: {
			if (fBrushPanel && fBrushPanel->Lock()) {
				fBrushPanel->Quit();
				fBrushPanel = NULL;
			}
			break;
		}
		case MSG_TOOL_TIPS: {
			BubbleHelper* helper = BubbleHelper::Default();
			helper->SetEnabled(!helper->IsEnabled());
			fToolTipsMI->SetMarked(helper->IsEnabled());
			break;
		}
		case MSG_REDO_MOUSE_MOVED: {
//printf("MSG_REDO_MOUSE_MOVED\n");
			if (BView* view = LastMouseMovedView()) {
				BPoint where;
				uint32 buttons;
				view->GetMouse(&where, &buttons, false);
				message->AddInt32("buttons", buttons);
				view->MouseMoved(where, B_INSIDE_VIEW, NULL);
			}
			break;
		}
		default:
			MWindow::MessageReceived(message);
			break;
	}
}

// Zoom
void
MainWindow::Zoom(BPoint origin, float width, float height)
{
	BRect r;
	if (fZoomToFullscreen) {
		BScreen screen(this);
		r = screen.Frame();
		fNonFullscreenFrame = Frame();
		fWindowIB->SetFullscreen(true);
	} else {
		r = fNonFullscreenFrame;
		fWindowIB->SetFullscreen(false);
	}
	origin = r.LeftTop();
	width = r.Width();
	height = r.Height();
	MWindow::Zoom(origin, width, height);
	// update item
	fFullscreenMI->SetMarked(fZoomToFullscreen);
	fZoomToFullscreen = !fZoomToFullscreen;
}

// NewDocument
Canvas*
MainWindow::NewDocument(BRect bounds, Layer* layer)
{
	Canvas* canvas = NULL;
	if (Lock()) {
		canvas = new Canvas(bounds);
		if (!layer) {
			layer = new Layer(bounds);
			layer->SetName(LanguageManager::Default()->GetString(DEFAULT, "Default"));
		}
		canvas->AddLayer(layer);
		Unlock();
	}
	return canvas;
}

// Save
//
// exporter = NULL means save as native document
status_t
MainWindow::Save(Canvas* canvas, const entry_ref* docRef, Exporter* exporter)
{
	status_t status = B_BAD_VALUE;
	if (canvas && fDocuments.HasItem((void*)canvas)) {

		const entry_ref* ref = docRef;
		entry_ref tempRef;
		BEntry entry(docRef, true);

		if (entry.IsDirectory())
			return B_BAD_VALUE;

#ifndef USE_SERIAL_NUMBERS
#ifndef TARGET_PLATFORM_HAIKU
		// check if we have a valid keyfile
		if (!fKeyfile || !fKeyfile->GetLicenceeInfo() || fKeyfile->GetLicenceeInfo()->GetApplicationVersion() < 150) {
			if (!exporter || exporter->NeedsKeyfile()) {
				LanguageManager* manager = LanguageManager::Default();
				BAlert* alert = new BAlert("show stopper",
										   manager->GetString(DEMO_MODE,
												"WonderBrush is running in demo mode. "
												"Saving project files is disabled.\n\n"
												"Please register to receive your "
												"unlock keyfile at one of these online stores."),
										   "Mensys Store",
										   "Kagi Store",
										   manager->GetString(NEVER_MIND, "Never Mind"),
										   B_WIDTH_AS_USUAL, B_STOP_ALERT);
				int32 ret = alert->Go();
				if (ret == 0) {
					char *argv = "http://shop.mensys.nl/catalogue/mns_Wonderbrush.html";
					be_roster->Launch("text/html", 1, &argv);
				} else if (ret == 1) {
					char *argv = "http://order.kagi.com/?8JD";
					be_roster->Launch("text/html", 1, &argv);
				}
				return B_ERROR;
			}
		}
#endif // !TARGET_PLATFORM_HAIKU
#endif // USE_SERIAL_NUMBERS
		if (entry.Exists()) {
			// if the file exists create a temporary file in the same folder
			// and hope that it doesn't already exist...
			BPath tempPath(docRef);
			if (tempPath.GetParent(&tempPath) >= B_OK) {
				BString helper(docRef->name);
				helper << system_time();
				if (tempPath.Append(helper.String()) >= B_OK
					&& entry.SetTo(tempPath.Path()) >= B_OK
					&& entry.GetRef(&tempRef) >= B_OK) {
					ref = &tempRef;
				}
			}
		}

		const char* fileMIME = NULL;
		if (ref) {
			// do the actual save operation into a file
			BFile outFile(ref, B_CREATE_FILE | B_READ_WRITE | B_ERASE_FILE);
			status = outFile.InitCheck();
			if (status == B_OK) {
				if (!exporter) {
					// save as native document,
					BMessage archive;
					// compress canvas prior to saving
					bool wasMinimized = canvas->IsMinimized();
					canvas->Minimize(true);
					if ((status = canvas->Archive(&archive)) >= B_OK) {
#ifdef TARGET_PLATFORM_ZETA
						status = archive.Flatten(B_MESSAGE_VERSION_1, &outFile);
#else
						status = archive.Flatten(&outFile);
#endif
						if (status >= B_OK) {
							canvas->SetDocumentRef(docRef);
							RenameCanvas(canvas, docRef->name);
							// set file type
							fileMIME = "image/x-WonderBrush";
						} else
							fprintf(stderr, "failed to flatten document: %s\n", strerror(status));
					} else
						fprintf(stderr, "failed to archive document: %s\n", strerror(status));
					// restore minimize state of canvas
					canvas->Minimize(wasMinimized);
				} else {
					status = exporter->Export(canvas, fCanvasView, &outFile, docRef);
					if (status >= B_OK) {
						// success, update export entry_ref,
						// set name if not yet set and get MIME type
						canvas->SetExportRef(docRef);
						if (!canvas->DocumentRef())
							RenameCanvas(canvas, docRef->name);
						fileMIME = exporter->MIMEType();
					} else
						fprintf(stderr, "failed to export bitmap: %s\n", strerror(status));
				}
			} else
				fprintf(stderr, "failed to create output file: %s\n", strerror(status));
			outFile.Unset();
		}

		if (status < B_OK && ref != docRef) {
			// remove temporary file
			entry.Remove();
		}

		if (status >= B_OK && ref != docRef) {
			// move temp file overwriting actual document file
			BEntry docEntry(docRef, true);
			BDirectory dir;
			if ((status = docEntry.GetParent(&dir)) >= B_OK) {
				// copy attributes of previous document file
				BNode sourceNode(&docEntry);
				BNode destNode(&entry);
				if (sourceNode.InitCheck() >= B_OK && destNode.InitCheck() >= B_OK) {
					// lock the nodes
					if (sourceNode.Lock() >= B_OK) {
						if (destNode.Lock() >= B_OK) {
							// iterate over the attributes
							char attrName[B_ATTR_NAME_LENGTH];
							while (sourceNode.GetNextAttrName(attrName) >= B_OK) {
								attr_info info;
								if (sourceNode.GetAttrInfo(attrName, &info) >= B_OK) {
									char *buffer = new char[info.size];
									if (sourceNode.ReadAttr(attrName, info.type, 0,
															buffer, info.size) == info.size) {
										destNode.WriteAttr(attrName, info.type, 0,
														   buffer, info.size);
									}
									delete[] buffer;
								}
							}
							destNode.Unlock();
						}
						sourceNode.Unlock();
					}
				}
				// clobber the orginal file with the new temporary one
				status = entry.MoveTo(&dir, docRef->name, true);
				// if the canvas has an external observer,
				// inform it now, that the image contents might have changed
				if (canvas->ExternalObserver() && canvas->ExternalObserver()->IsValid()) {
					const entry_ref* watchedRef = canvas->ExternalObserverRef();
					if (watchedRef && *watchedRef == *docRef) {
						BMessage editMessage(BBP_SEND_BBITMAP);
						editMessage.AddRef("ref", docRef);
						canvas->ExternalObserver()->SendMessage(&editMessage);
					}
				}
			}
		}
		// inform user of failure at this point
		if (status < B_OK) {
			LanguageManager* manager = LanguageManager::Default();
			BString helper(manager->GetString(SAVING_FAILED, "Saving failed!"));
			helper << "\n\n" << manager->GetString(ERROR, "Error") << ": " << strerror(status);
			BAlert* alert = new BAlert("bad news", helper.String(),
									   manager->GetString(BLIP, "Bleep!"), NULL, NULL);
			// launch alert asynchronously
			alert->Go(NULL);
		} else {
			// success, mark undo history state as saved,
			// add to recent document list
			be_roster->AddToRecentDocuments(docRef);
			if (!exporter) {
				if (HistoryManager* history = canvas->GetHistoryManager()) {
					history->Save();
				}
			}
		}
		if (status >= B_OK && fileMIME) {
			// set file type
			BNode node(docRef);
			if (node.InitCheck() == B_OK) {
				BNodeInfo nodeInfo(&node);
				if (nodeInfo.InitCheck() == B_OK)
					nodeInfo.SetType(fileMIME);
			}
		}
	}
	_CanvasNameChanged(canvas);
	return status;
}









// AddDocument
bool
MainWindow::AddDocument(Canvas* canvas)
{
	bool success = false;
	if (canvas) {
		success = fDocuments.AddItem((void*)canvas);
		BMessage* message = new BMessage(MSG_SET_CANVAS);
		message->AddPointer("canvas", (void*)canvas);
		BMenuItem* item = new BMenuItem(canvas->Name(), message);
		fCanvasM->AddItem(item);
		// show a canvas tab item
#if CANVAS_LISTVIEW
		fCanvasListView->AddCanvas(canvas);
#else
		fCanvasTabView->AddCanvas(canvas);
#endif
		RecalcSize();
	}
	return success;
}

// RemoveDocument
bool
MainWindow::RemoveDocument(Canvas* canvas)
{
	bool success = fDocuments.RemoveItem((void*)canvas);
	if (success) {
		// remove corresponding item from menu
		for (int32 i = 1; BMenuItem* item = fCanvasM->ItemAt(i); i++) {
			BMessage* message = item->Message();
			void* pointer;
			if (message && message->FindPointer("canvas", &pointer) == B_OK
				&& pointer == (void*)canvas) {
				fCanvasM->RemoveItem(item);
				break;
			}
		}
		// remove canvas tab item
#if CANVAS_LISTVIEW
		fCanvasListView->RemoveCanvas(canvas);
#else
		fCanvasTabView->RemoveCanvas(canvas);
#endif
		RecalcSize();
	}
	return success;
}

// CountDocuments
int32
MainWindow::CountDocuments() const
{
	return fDocuments.CountItems();
}

// SetToDocument
void
MainWindow::SetToDocument(Canvas* canvas, bool force)
{
	Canvas* oldCanvas = fCanvasView->CurrentCanvas();
	if (!force && oldCanvas == canvas)
		return;

	// bring us to the users workspace
	SetWorkspaces(B_CURRENT_WORKSPACE);
	Activate();

	// minimize any unused canvas to minimize memory consumtion
	if (canvas && canvas->IsMinimized())
		canvas->Minimize(false);
	fCanvasView->SetTo(canvas);
	fLayersListView->SetToCanvas(canvas);

	if (oldCanvas && oldCanvas != canvas)
		oldCanvas->Minimize(true);

	_CanvasNameChanged(canvas);

	// mark corresponding document item in menu
	for (int32 i = 0; BMenuItem* item = fCanvasM->ItemAt(i); i++) {
		BMessage* message = item->Message();
		void* pointer;
		if (message && message->FindPointer("canvas", &pointer) == B_OK
			&& pointer == (void*)canvas) {
			item->SetMarked(true);
		} else
			item->SetMarked(false);
	}
#if CANVAS_LISTVIEW
	fCanvasListView->SetCanvas(canvas);
#else
	fCanvasTabView->SetCanvas(canvas);
#endif

#if USE_COLORSPACES
	// mark corresponding colorspace item in menu
	uint32 colorSpace = canvas ? canvas->ColorSpace() : COLOR_SPACE_UNKNOWN;
	for (int32 i = 0; BMenuItem* item = fFormatM->ItemAt(i); i++) {
		BMessage* message = item->Message();
		uint32 format;
		if (message && message->FindInt32("color space", (int32*)&format) == B_OK
			&& format == colorSpace) {
			item->SetMarked(true);
			break;
		} else
			item->SetMarked(false);
	}
#endif
	// enable or disable menu items and icon buttons
	bool enable = canvas != NULL;

	fSaveImageIB->SetEnabled(enable);
	fSaveProjectIB->SetEnabled(enable);

	fCloseIB->SetEnabled(enable);

	fExportProjectMI->SetEnabled(enable);
	fExportProjecAstMI->SetEnabled(enable);

	fSaveProjectMI->SetEnabled(enable);
	fSaveProjectAsMI->SetEnabled(enable);

	fSetupPageMI->SetEnabled(enable);
	fPrintMI->SetEnabled(enable);

	PostMessage(MSG_UPDATE_HISTORY_ITEMS);

	if (!enable) {
		fZoomPS->SetValue(100);
		fRadiusDS->SetEnabled(false);
		fAlphaDS->SetEnabled(false);
		fHardnessDS->SetEnabled(false);
		fSpacingDS->SetEnabled(false);
		fBrushView->SetEnabled(false);

	} else {
		_SetTool(fCanvasView->Tool());
	}
	fCropIB->SetEnabled(enable);
	fTranslateIB->SetEnabled(enable);
	fBrushIB->SetEnabled(enable);
	fCloneBrushIB->SetEnabled(enable);
	fPenIB->SetEnabled(enable);
	fEraserIB->SetEnabled(enable);
	fColorPickerIB->SetEnabled(enable);
	fTextIB->SetEnabled(enable);
	fBlurIB->SetEnabled(enable);
	fFillIB->SetEnabled(enable);
	fPenEraserIB->SetEnabled(enable);
	fShapeIB->SetEnabled(enable);
	fCopyIB->SetEnabled(enable);
	fEllipseIB->SetEnabled(enable);
	fRoundRectIB->SetEnabled(enable);
	fGradientIB->SetEnabled(enable);
	fGuidesIB->SetEnabled(enable);
	fPickIB->SetEnabled(enable);

	fZoomPS->SetEnabled(enable);
	fZoomInIB->SetEnabled(enable);
	fZoomOutIB->SetEnabled(enable);
	fFormatM->SetEnabled(enable);
	fResizeCanvasMI->SetEnabled(enable);
	fRotateCanvasM->SetEnabled(enable);
	fClearCanvasMI->SetEnabled(enable);
	fCloseCanvasMI->SetEnabled(enable);
	fDropperSizeLP->SetEnabled(enable);
	fUseAllLayersCB->SetEnabled(enable);

	fBrushSubPixelCB->SetEnabled(enable);
	fSolidCB->SetEnabled(enable);
	fBrushTiltCB->SetEnabled(enable);

	fXOffsetTC->SetEnabled(false);
	fYOffsetTC->SetEnabled(false);
	fRotationTC->SetEnabled(false);
	fScaleXTC->SetEnabled(false);
	fScaleYTC->SetEnabled(false);
	fMoveSubPixelCB->SetEnabled(enable);

	fCropLeftTC->SetEnabled(enable);
	fCropTopTC->SetEnabled(enable);
	fCropWidthTC->SetEnabled(enable);
	fCropHeightTC->SetEnabled(enable);
	fCropSubPixelCB->SetEnabled(enable);
	fCropAllB->SetEnabled(enable);

	fFillOpacityDS->SetEnabled(enable);
	fToleranceDS->SetEnabled(enable);
	fSoftnessDS->SetEnabled(enable);

	fFontLP->SetEnabled(enable);
	fFontSizeSB->SetEnabled(enable);
	fFontOpacityDS->SetEnabled(enable);
	fFontAdvanceScaleDS->SetEnabled(enable);
	fTextTV->SetEnabled(enable);
	fFontAlignModeIOC->SetEnabled(enable);

	fShapeOpacityDS->SetEnabled(enable);
	fShapeOutlineCB->SetEnabled(enable);
	fShapeOutlineWidthDS->SetEnabled(enable ? fShapeOutlineCB->Value() : false);
	_UpdateShapeControls(enable);
	fShapeSubpixelsCB->SetEnabled(enable);
	fShapeCapModeIOC->SetEnabled(enable ? fShapeOutlineCB->Value() : false);
	fShapeJoinModeIOC->SetEnabled(enable ? fShapeOutlineCB->Value() : false);

	fEllipseOpacityDS->SetEnabled(enable);
	fEllipseOutlineCB->SetEnabled(enable);
	fEllipseOutlineWidthDS->SetEnabled(enable ? fEllipseOutlineCB->Value() : false);
	fEllipseSubpixelsCB->SetEnabled(enable);

	fRectOpacityDS->SetEnabled(enable);
	fRectOutlineCB->SetEnabled(enable);
	fRectOutlineWidthDS->SetEnabled(enable ? fRectOutlineCB->Value() : false);
	fRectCornerRadiusDS->SetEnabled(enable);
	fRectSubpixelsCB->SetEnabled(enable);

	fSelectLeftTC->SetEnabled(enable);
	fSelectTopTC->SetEnabled(enable);
	fSelectWidthTC->SetEnabled(enable);
	fSelectHeightTC->SetEnabled(enable);
	fSelectAllLayersCB->SetEnabled(enable);
	fSelectAllB->SetEnabled(enable);

	fGradientControl->SetEnabled(enable);
	fGradientTypeIOC->SetEnabled(enable);
	fGradientInterpolationLP->SetEnabled(enable);
	fGradientInheritsTransformCB->SetEnabled(enable);
	fGradientStopAlphaDS->SetEnabled(enable ? fGradientControl->IsFocus() : false);

	fGuidesShowCB->SetEnabled(enable);
	fGuideCreateIB->SetEnabled(enable);
	fGuideRemoveIB->SetEnabled(false);
	fGuideProportionIB->SetEnabled(false);
	fGuideHPosTC->SetEnabled(enable);
	fGuideVPosTC->SetEnabled(enable);
	fGuideAngleTC->SetEnabled(enable);
	fGuideUnitsLP->SetEnabled(enable);

	_UpdatePickControls(enable);

	_UpdatePermanentStatus();
}

// RenameCanvas
void
MainWindow::RenameCanvas(Canvas* canvas, const char* name)
{
	if (canvas && name) {
		for (int32 i = 1; BMenuItem* item = fCanvasM->ItemAt(i); i++) {
			BMessage* message = item->Message();
			void* pointer;
			if (message && message->FindPointer("canvas", &pointer) == B_OK
				&& pointer == (void*)canvas) {
				item->SetLabel(name);
				canvas->SetName(name);
				break;
			}
		}
	}
}

// CloseDocument
bool
MainWindow::CloseDocument(Canvas* canvas, bool setToNext)
{
	bool success = false;
	if (canvas && fDocuments.HasItem((void*)canvas)) {
		bool close = true;
		// run alert to ask user what to do in case of unsaved changes
		if (!canvas->IsSaved()) {
			// set ourself to the document so user sees which document is not saved
			if (Lock()) {
				if (fCanvasView->CurrentCanvas() != canvas) {
					SetToDocument(canvas);
				}
				Activate();
				Unlock();
			}
			// get appropriate strings
			LanguageManager* manager = LanguageManager::Default();
			const char* question = manager->GetString(AKS_SAVE_CHANGES,
													  "Close canvas and discard unsaved changes?");
			const char* discardString = manager->GetString(DISCARD, "Discard");
			const char* cancelString = manager->GetString(CANCEL, "Cancel");
			const char* saveString = manager->GetString(SAVE, "Save");
			BAlert* alert = new BAlert("last chance", question,
									   discardString, cancelString, saveString);
			int32 answer = alert->Go();
			if (answer > 0)
				close = false;
			if (answer == 2) {
				BMessage message(MSG_SAVE_DOCUMENT);
				message.AddPointer("canvas", (void*)canvas);
				if (setToNext)
					message.AddBool("close", true);
				else
					message.AddBool("quit", true);
				PostMessage(&message);
			}
		}
		// proceed if we can close this baby
		if (close) {
			if (Lock()) {
				int32 index = fDocuments.IndexOf((void*)canvas);
				if (index >= fDocuments.CountItems() - 1)
					index--;
				if ((success = RemoveDocument(canvas))) {
					if (setToNext) {
						Canvas* nextInLine = (Canvas*)fDocuments.ItemAt(index);
						SetToDocument(nextInLine);
					}
					if (fCanvasView->CurrentCanvas() == canvas) {
						fCanvasView->SetTo(NULL);
						fLayersListView->SetToCanvas(NULL);
					}
					delete canvas;
				}
				Unlock();
			}
		}
	}
	return success;
}

// QuitDocuments
bool
MainWindow::QuitDocuments()
{
	status_t err;
	if (fLoadingThread >= B_OK)
		wait_for_thread(fLoadingThread, &err);

	bool quit = true;
	// try to close current document
	if (Canvas* canvas = fCanvasView->CurrentCanvas()) {
		if (!CloseDocument(canvas, false))
			quit = false;
	}
	// if success, close the rest as well
	if (quit) {
		while (Canvas* canvas = (Canvas*)fDocuments.ItemAt(0)) {
			if (!CloseDocument(canvas, false)) {
				quit = false;
				break;
			}
		}
	}
	// at this point we can really quit
	if (quit) {
		if (Lock()) {
			if (fColorPickerPanel) {
				fCanvasView->SetTool(fRememberedTool);
				if (fColorPickerPanel->LockWithTimeout(10000) >= B_OK) {
					fColorPickerFrame = fColorPickerPanel->Frame();
					fColorPickerPanel->Unlock();
				}
			}
			_SaveSettings();

			delete fSavePanel;
			fSavePanel = NULL;

			if (fColorPickerPanel) {
				fColorPickerPanel->Lock();
				fColorPickerPanel->Quit();
			}
			if (fSettingsPanel) {
				fSettingsPanel->Lock();
				fSettingsPanel->Quit();
			}

			Unlock();
		}
	}
	return quit;
}

// SetupPage
status_t
MainWindow::SetupPage(Canvas* canvas) const
{
	status_t status = B_BAD_VALUE;
	if (canvas) {
		BPrintJob printJob(canvas->Name());
		if (BMessage* settings = canvas->PageSetupSettings()) {
			printJob.SetSettings(new BMessage(*settings));
		}
		status = printJob.ConfigPage();
		if (status >= B_OK) {
			canvas->SetPageSetupSettings(printJob.Settings());
		}
	}
	return status;
}

// Print
status_t
MainWindow::Print(Canvas* canvas)// const
{
	status_t status = B_BAD_VALUE;
	if (canvas) {
		BPrintJob job(canvas->Name());
		// if we have no setup message, we should call ConfigPage()
		// we must use the same instance of the BPrintJob object
		// (we can't call the previous "SetupPage()" function, since it
		// creates its own BPrintJob object).
//		BRect paperRect;
		BRect printableRect;

		BMessage* settings = canvas->PageSetupSettings();
		if (!settings || !job.IsSettingsMessageValid(settings)) {
			// no user settings or wrong user settings for this document
printf("Print(): no settings or settings not valid - running Setup\n");
			status = job.ConfigPage();
			if (status >= B_OK) {
				// get the user settings
				canvas->SetPageSetupSettings(job.Settings());

				// use the new settings for your internal use
//				paperRect = job.PaperRect();
				printableRect = job.PrintableRect();
			}
		} else
			status = B_OK;

		if (status >= B_OK) {
			// setup the driver with user settings
			settings = canvas->PageSetupSettings();
			job.SetSettings(new BMessage(*settings));

			status = job.ConfigJob();
			if (status >= B_OK) {
				// get the user settings
				canvas->SetPageSetupSettings(job.Settings());

				// use the new settings for your internal use
				// they may have changed since the last time you read it
//				paperRect = job.PaperRect();
				printableRect = job.PrintableRect();

				// make a bitmap of the canvas, rendered to white background
				BBitmap* bitmap = new BBitmap(canvas->Bounds(), B_BITMAP_CLEAR_TO_WHITE, B_RGB32);
				canvas->Compose(bitmap, bitmap->Bounds());
				printableRect.OffsetTo(0.0, 0.0);
				PrintView* printView = new PrintView(bitmap, printableRect);
				printView->Hide();
				AddChild(printView);
//				float height = printableRect.Height() / 10.0;


				// now we can print the page
				job.BeginJob();
				fInfoView->JobStarted();
//printf("job started\n");

				// divide the bitmap into 10 parts and allow cancellation
				bool canContinue = job.CanContinue();

/*				for (int32 i = 0; canContinue && i < 10 ; i++) {
printf("drawing region:\n");

					// current portion of the bitmap
					BRect r(printableRect.left,
							printableRect.top + (float)i * height,
							printableRect.right,
							printableRect.bottom + (float)(i + 1) * height);

r.PrintToStream();
					job.DrawView(printView, r, printableRect.LeftTop());

					// update info view to indicate progress
					fInfoView->JobProgress(100.0 / ((i + 1) * 10.0));


					// cancel the job if needed.
					// you can for exemple verify if the user pressed the ESC key
					// or (SHIFT + '.')...
//					if (user_has_canceled) {
//						// tell the print_server to cancel the printjob
//						job.CancelJob();
//						canContinue = false;
//						break;
//					}

					// verify that there was no error (disk full for exemple)
					canContinue = job.CanContinue();
				}*/
BRect r(printableRect);
r.OffsetTo(0.0, 0.0);
job.DrawView(printView, r, BPoint(0.0, 0.0));

				// spool the page
//printf("spooling page...\n");
				job.SpoolPage();

				fInfoView->JobDone();
//printf("job done\n");
				// now, you just have to commit the job!
				if (canContinue)
					job.CommitJob();
				else
					status = B_ERROR;

				RemoveChild(printView);
				delete printView;
				delete bitmap;
			}
		}
	}
	return status;
}

// UpdateStrings
void
MainWindow::UpdateStrings()
{
	BubbleHelper* helper = BubbleHelper::Default();
	LanguageManager* manager = LanguageManager::Default();

	BString string;

	string.SetTo(manager->GetString(PICK_OBJECTS, "Pick Objects"));
	helper->SetHelp(fPickIB, string.String());

	string.SetTo(manager->GetString(CLIPBOARD, "Clipboard"));
	helper->SetHelp(fCopyIB, string.String());

	string.SetTo(manager->GetString(CROP, "Crop"));
	string << "   (X)";
	helper->SetHelp(fCropIB, string.String());

	string.SetTo(manager->GetString(TRANSFORM, "Transform"));
	string << "   (V)";
	helper->SetHelp(fTranslateIB, string.String());

	string.SetTo(manager->GetString(EDIT_GRADIENT, "Edit Gradient"));
	string << "   (G)";
	helper->SetHelp(fGradientIB, string.String());


	string.SetTo(manager->GetString(BRUSH, "Brush"));
	string << "   (B)";
	helper->SetHelp(fBrushIB, string.String());

	string.SetTo(manager->GetString(CLONE, "Clone"));
	string << "   (C)";
	helper->SetHelp(fCloneBrushIB, string.String());

	string.SetTo(manager->GetString(PEN, "Pen"));
	string << "   (P)";
	helper->SetHelp(fPenIB, string.String());

	string.SetTo(manager->GetString(PEN_ERASER, "Eraser Pen"));
//	string << "   (S)";
	helper->SetHelp(fPenEraserIB, string.String());

	string.SetTo(manager->GetString(ERASER, "Eraser"));
	string << "   (E)";
	helper->SetHelp(fEraserIB, string.String());

	string.SetTo(manager->GetString(DROPPER, "Dropper"));
	string << "   (D)";
	helper->SetHelp(fColorPickerIB, string.String());

	string.SetTo(manager->GetString(BLUR, "Blur"));
	string << "   (R)";
	helper->SetHelp(fBlurIB, string.String());

	string.SetTo(manager->GetString(BUCKET_FILL, "Fill"));
	string << "   (F)";
	helper->SetHelp(fFillIB, string.String());

	string.SetTo(manager->GetString(TEXT, "Text"));
	string << "   (T)";
	helper->SetHelp(fTextIB, string.String());

	string.SetTo(manager->GetString(SHAPE, "Shape"));
	string << "   (S)";
	helper->SetHelp(fShapeIB, string.String());

	string.SetTo(manager->GetString(ELLIPSE, "Ellipse"));
	helper->SetHelp(fEllipseIB, string.String());

	string.SetTo(manager->GetString(ROUND_RECT, "(Round) Rectangle"));
	helper->SetHelp(fRoundRectIB, string.String());

	string.SetTo(manager->GetString(GUIDES, "Guides"));
	helper->SetHelp(fGuidesIB, string.String());

//	string.SetTo(manager->GetString(EDIT_GRADIENT, "Edit Gradient"));
//	helper->SetHelp(fGradientIB, string.String());


	fZoomPS->SetLabel(manager->GetString(ZOOM, "Zoom"));

	const char* anyModifier = manager->GetString(ANY_MODIFIER, "Any modifier");

	string.SetTo(manager->GetString(ZOOM_IN, "Zoom In"));
	string << "   (" << anyModifier << " +)";
	helper->SetHelp(fZoomInIB, string.String());

	string.SetTo(manager->GetString(ZOOM_OUT, "Zoom Out"));
	string << "   (" << anyModifier << " -)";
	helper->SetHelp(fZoomOutIB, string.String());


	const char* deleteKey = manager->GetString(DELETE_KEY, "Del");

	string.SetTo(manager->GetString(DELETE_LAYER, "Delete Selected Layer"));
	string << "   (" << deleteKey << ")";
	helper->SetHelp(fDeleteLayersIB, string.String());

	string.SetTo(manager->GetString(DELETE_OBJECTS, "Delete Selected Objects"));
	string << "   (" << deleteKey << ")";
	helper->SetHelp(fDeleteObjectsIB, string.String());


	string.SetTo(manager->GetString(CONFIRM, "Confirm"));
	string << "   (" << manager->GetString(RETURN, "Return") << ")";
	helper->SetHelp(fConfirmIB, string.String());

	string.SetTo(manager->GetString(CANCEL, "Cancel"));
	string << "   (" << manager->GetString(ESCAPE, "Escape") << ")";
	helper->SetHelp(fCancelIB, string.String());


	helper->SetHelp(fNewIB,  manager->GetString(NEW_CANVAS, "New Canvas"));
	helper->SetHelp(fOpenIB, manager->GetString(OPEN_CANVAS, "Open File"));
	helper->SetHelp(fSaveImageIB, manager->GetString(EXPORT_CANVAS, "Export Canvas"));
	helper->SetHelp(fSaveProjectIB, manager->GetString(SAVE_CANVAS, "Save Canvas"));
	helper->SetHelp(fCloseIB, manager->GetString(CLOSE_CANVAS, "Close Canvas"));
	helper->SetHelp(fBrushView, manager->GetString(BRUSH_PREVIEW_TIP,
												   "Brush preview. Click to open brush manager panel."));
	helper->SetHelp(fCurrentColorSV, manager->GetString(CURRENT_COLOR_TIP,
														"Current color. Click to open color picker panel."));
	helper->SetHelp(fNavigatorView, manager->GetString(NAVIGATOR_TIP,
													   "Canvas overview. Click to move visible part.\n"
													   "Use mousewheel to zoom in/out."));
	helper->SetHelp(fLayersListView, manager->GetString(LAYER_LIST_TIP,
														"Sort by Drag'N'Drop. Click eye icon to turn\n"
														"visibility on/off. Doubleclick to change name."));
	helper->SetHelp(fHistoryListView, manager->GetString(HISTORY_LIST_TIP,
														"Sort by Drag'N'Drop. Drop color\n"
														"to change selected objects color."));
	helper->SetHelp(fPropertyListView, manager->GetString(PROPERTY_LIST_TIP,
														"Edit, copy and paste object properties."));
	const char* subPixelTip = manager->GetString(SUB_PIXEL_TIP,
												 "Track mouse/pen position with subpixel precision.");
	helper->SetHelp(fBrushSubPixelCB, subPixelTip);
	helper->SetHelp(fMoveSubPixelCB, subPixelTip);
	helper->SetHelp(fCropSubPixelCB, subPixelTip);
	helper->SetHelp(fShapeSubpixelsCB, subPixelTip);
	helper->SetHelp(fEllipseSubpixelsCB, subPixelTip);
	helper->SetHelp(fRectSubpixelsCB, subPixelTip);

	helper->SetHelp(fSolidCB, manager->GetString(SOLID_TIP,
												 "Don't vary opacity per pixel."));
	helper->SetHelp(fBrushTiltCB, manager->GetString(TILT_TIP,
													 "Distort brush shape according to pen tilt."));
	helper->SetHelp(fWindowIB, manager->GetString(FULLSCREEN_TIP,
												 "Toggles between fullscreen and windowed mode."));

	// gradient controls
	helper->SetHelp(fGradientControl, manager->GetString(GRADIENT_CONTROL_TIP,
														 "Drop colors here to add or edit color stops.\n"
														 "Doubleclick onto a stop to pick up it's color."));
/*	fGradientTypeLP->SetLabel(manager->GetString(GRADIENT_TYPE, "Type"));
	fGradientTypeLinearMI->SetLabel(manager->GetString(GRADIENT_TYPE_LINEAR, "Linear"));
	fGradientTypeCircularMI->SetLabel(manager->GetString(GRADIENT_TYPE_RADIAL, "Radial"));
	fGradientTypeDiamontMI->SetLabel(manager->GetString(GRADIENT_TYPE_DIAMONT, "Diamont"));
	fGradientTypeConicMI->SetLabel(manager->GetString(GRADIENT_TYPE_CONIC, "Conic"));
	fGradientTypeXYMI->SetLabel(manager->GetString(GRADIENT_TYPE_XY, "XY"));
	fGradientTypeSqrtXYMI->SetLabel(manager->GetString(GRADIENT_TYPE_XY_SQRT, "Sqrt(X-Y)"));*/
	fGradientTypeIOC->SetLabel(manager->GetString(GRADIENT_TYPE, "Type"));
	helper->SetHelp(fGradientTypeLinearIB, manager->GetString(GRADIENT_TYPE_LINEAR, "Linear"));
	helper->SetHelp(fGradientTypeCircularIB, manager->GetString(GRADIENT_TYPE_RADIAL, "Radial"));
	helper->SetHelp(fGradientTypeDiamontIB, manager->GetString(GRADIENT_TYPE_DIAMONT, "Diamont"));
	helper->SetHelp(fGradientTypeConicIB, manager->GetString(GRADIENT_TYPE_CONIC, "Conic"));
	helper->SetHelp(fGradientTypeXYIB, manager->GetString(GRADIENT_TYPE_XY, "XY"));
	helper->SetHelp(fGradientTypeSqrtXYIB, manager->GetString(GRADIENT_TYPE_XY_SQRT, "Sqrt(X-Y)"));
	helper->SetHelp(fGradientInheritsTransformCB, manager->GetString(GRADIENT_INHERITS_TRANSFORM_TIP, "Inherit transformation from Object"));

	fGradientInterpolationLP->SetLabel(manager->GetString(GRADIENT_INTERPOLATION, "Interpolation"));
	fGradientInterpolationLinearMI->SetLabel(manager->GetString(GRADIENT_INTERPOLATION_LINEAR, "Linear"));
	fGradientInterpolationSmoothMI->SetLabel(manager->GetString(GRADIENT_INTERPOLATION_SMOOTH, "Smooth"));
	fGradientInheritsTransformCB->SetLabel(manager->GetString(GRADIENT_INHERITS_TRANSFORM, "Inherit Transformation"));
	fGradientStopAlphaDS->SetLabel(manager->GetString(OPACITY, "Opacity"));

//	fGradientTypeLP->RefreshItemLabel();
	fGradientInterpolationLP->RefreshItemLabel();

	// brush controls
	fRadiusDS->SetLabel(manager->GetString(RADIUS, "Radius"));
	fAlphaDS->SetLabel(manager->GetString(OPACITY, "Opacity"));
	fHardnessDS->SetLabel(manager->GetString(HARDNESS, "Hardness"));
	fSpacingDS->SetLabel(manager->GetString(SPACING, "Spacing"));
	fBrushSubPixelCB->SetLabel(manager->GetString(SUBPIXELS, "Subpixels"));
	fSolidCB->SetLabel(manager->GetString(SOLID, "Solid"));
	fBrushTiltCB->SetLabel(manager->GetString(TILT, "Tilt"));

	// transform controls
	fTranslationSV->SetText(manager->GetString(TRANSLATE, "Translate"));
	fRotationSV->SetText(manager->GetString(ROTATE, "Rotate"));
	fScaleSV->SetText(manager->GetString(SCALE, "Scale"));
	fXOffsetTC->SetLabel(manager->GetString(TRANSLATION_X, "X"));
	fYOffsetTC->SetLabel(manager->GetString(TRANSLATION_Y, "Y"));
	fRotationTC->SetLabel(manager->GetString(ANGLE, "Angle"));
	fScaleXTC->SetLabel(manager->GetString(SCALE_X, "X"));
	fScaleYTC->SetLabel(manager->GetString(SCALE_Y, "Y"));
	fMoveSubPixelCB->SetLabel(manager->GetString(SUBPIXELS, "Subpixels"));

//	DivideSame(fXOffsetTC, fYOffsetTC, fRotationTC, fScaleXTC, fScaleYTC, NULL);
	DivideSame(fXOffsetTC, fYOffsetTC, NULL);
	DivideSame(fScaleXTC, fScaleYTC, NULL);

	fCropLeftTC->SetLabel(manager->GetString(LEFT, "Left"));
	fCropTopTC->SetLabel(manager->GetString(TOP, "Top"));
	fCropWidthTC->SetLabel(manager->GetString(WIDTH, "Width"));
	fCropHeightTC->SetLabel(manager->GetString(HEIGHT, "Height"));
	fCropSubPixelCB->SetLabel(manager->GetString(SUBPIXELS, "Subpixels"));
	fCropAllB->SetLabel(manager->GetString(ENTIRE_CANVAS, "All"));

	DivideSame(fCropLeftTC, fCropTopTC, fCropWidthTC, fCropHeightTC, NULL);

	fFillOpacityDS->SetLabel(manager->GetString(OPACITY, "Opacity"));
	fToleranceDS->SetLabel(manager->GetString(TOLERANCE, "Tolerance"));
	fSoftnessDS->SetLabel(manager->GetString(SOFTNESS, "Softness"));
	fFillContiguousCB->SetLabel(manager->GetString(CONTIGUOUS_AREA, "Contiguous Area"));

	fFontLP->SetLabel(manager->GetString(FONT, "Font"));
	fFontSizeSB->SetLabel(manager->GetString(SIZE, "Size"));
	fFontOpacityDS->SetLabel(manager->GetString(OPACITY, "Opacity"));
	fFontAdvanceScaleDS->SetLabel(manager->GetString(SPACING, "Spacing"));
	fFontAlignModeIOC->SetLabel(manager->GetString(ALIGNMENT, "Alignment"));
	helper->SetHelp(fFontAlignLeftIB, manager->GetString(ALIGNMENT_LEFT, "Left"));
	helper->SetHelp(fFontAlignCenterIB, manager->GetString(ALIGNMENT_CENTER, "Center"));
	helper->SetHelp(fFontAlignRightIB, manager->GetString(ALIGNMENT_RIGHT, "Right"));
	helper->SetHelp(fFontAlignJustifyIB, manager->GetString(ALIGNMENT_JUSTIFY, "Justify"));

	fFontLP->RefreshItemLabel();

//	DivideSame(fFontLP, fFontAlignModeIOC, NULL);

	fShapeOpacityDS->SetLabel(manager->GetString(OPACITY, "Opacity"));
	fShapeOutlineCB->SetLabel(manager->GetString(OUTLINE, "Outline"));
	fShapeOutlineWidthDS->SetLabel(manager->GetString(WIDTH, "Width"));
	helper->SetHelp(fShapeTransformIB, manager->GetString(TRANSFORM_CONTROL_POINTS, "Transform Control Points"));
	helper->SetHelp(fShapeNewPathIB, manager->GetString(CREATE_NEW_PATH, "New Path"));
	helper->SetHelp(fShapeRemovePointsIB, manager->GetString(REMOVE_CONTROL_POINTS, "Remove Control Points"));
	helper->SetHelp(fShapeReversePathIB, manager->GetString(REVERSE_PATH, "Reverse Path"));
	fShapeClosedCB->SetLabel(manager->GetString(CLOSED, "Closed"));
	fShapeSubpixelsCB->SetLabel(manager->GetString(SUBPIXELS, "Subpixels"));
	fShapeCapModeIOC->SetLabel(manager->GetString(CAP_MODE, "Caps"));
	helper->SetHelp(fCapModeButtIB, manager->GetString(BUTT_CAP, "Butt"));
	helper->SetHelp(fCapModeSquareIB, manager->GetString(SQUARE_CAP, "Square"));
	helper->SetHelp(fCapModeRoundIB, manager->GetString(ROUND_CAP, "Round"));
	fShapeJoinModeIOC->SetLabel(manager->GetString(JOIN_MODE, "Joints"));
	helper->SetHelp(fJoinModeMiterIB, manager->GetString(MITER_JOIN, "Miter"));
	helper->SetHelp(fJoinModeRoundIB, manager->GetString(ROUND_JOIN, "Round"));
	helper->SetHelp(fJoinModeBevelIB, manager->GetString(BEVEL_JOIN, "Bevel"));

	DivideSame(fShapeCapModeIOC, fShapeJoinModeIOC, NULL);

	fEllipseOpacityDS->SetLabel(manager->GetString(OPACITY, "Opacity"));
	fEllipseOutlineCB->SetLabel(manager->GetString(OUTLINE, "Outline"));
	fEllipseOutlineWidthDS->SetLabel(manager->GetString(WIDTH, "Width"));
	fEllipseSubpixelsCB->SetLabel(manager->GetString(SUBPIXELS, "Subpixels"));

	fRectOpacityDS->SetLabel(manager->GetString(OPACITY, "Opacity"));
	fRectOutlineCB->SetLabel(manager->GetString(OUTLINE, "Outline"));
	fRectOutlineWidthDS->SetLabel(manager->GetString(WIDTH, "Width"));
	fRectCornerRadiusDS->SetLabel(manager->GetString(ROUND_CORNER_RADIUS, "Round Corner Radius"));
	fRectSubpixelsCB->SetLabel(manager->GetString(SUBPIXELS, "Subpixels"));

	fDropperSizeLP->SetLabel(manager->GetString(TIP_SIZE, "Tip Size"));
	fDropperSize1x1MI->SetLabel(manager->GetString(PIXEL_1x1, "1x1 pixels"));
	fDropperSize3x3MI->SetLabel(manager->GetString(PIXEL_3x3, "3x3 pixels"));
	fDropperSize5x5MI->SetLabel(manager->GetString(PIXEL_5x5, "5x5 pixels"));

	fDropperSizeLP->RefreshItemLabel();

	fUseAllLayersCB->SetLabel(manager->GetString(INCLUDE_ALL_LAYERS, "Include All Layers"));


	fSelectLeftTC->SetLabel(manager->GetString(LEFT, "Left"));
	fSelectTopTC->SetLabel(manager->GetString(TOP, "Top"));
	fSelectWidthTC->SetLabel(manager->GetString(WIDTH, "Width"));
	fSelectHeightTC->SetLabel(manager->GetString(HEIGHT, "Height"));
	DivideSame(fCropLeftTC, fCropTopTC, fCropWidthTC, fCropHeightTC, NULL);
	fSelectAllLayersCB->SetLabel(manager->GetString(INCLUDE_ALL_LAYERS, "Include All Layers"));
	fSelectAllB->SetLabel(manager->GetString(ENTIRE_CANVAS, "All"));

	fSelectableSV->SetText(manager->GetString(SELECTABLE, "Selectable"));

	fPickMaskAllB->SetLabel(manager->GetString(SELECT_ALL_OBJECTS, "All"));
	fPickMaskNoneB->SetLabel(manager->GetString(SELECT_NO_OBJECTS, "None"));

	fSelectSV->SetText(manager->GetString(OBJECT_SELECTION, "Select"));

	fPickAllB->SetLabel(manager->GetString(SELECT_ALL_OBJECTS, "All"));
	fPickNoneB->SetLabel(manager->GetString(SELECT_NO_OBJECTS, "None"));

	fGuidesShowCB->SetLabel(manager->GetString(SHOW_GUIDES, "Show Guides"));
	helper->SetHelp(fGuideCreateIB, manager->GetString(NEW, "New"));
	helper->SetHelp(fGuideRemoveIB, manager->GetString(REMOVE, "Remove"));
	helper->SetHelp(fGuideProportionIB, "Leonardo da Vinci");
	fGuideHPosTC->SetLabel(manager->GetString(HORIZONTAL, "Horizontal"));
	fGuideVPosTC->SetLabel(manager->GetString(VERTICAL, "Vertical"));
	DivideSame(fGuideHPosTC, fGuideVPosTC, NULL);
	helper->SetHelp(fGuideUnitsLP, manager->GetString(UNIT, "Unit"));
	fGuideUnitsPercentMI->SetLabel(manager->GetString(UNIT_PERCENT, "%"));
	fGuideUnitsPixelsMI->SetLabel(manager->GetString(UNIT_PIXEL, "px"));
	fGuideAngleTC->SetLabel(manager->GetString(ANGLE, "Angle"));

	// refresh item label (this menu is not a LabelPopup but an MPopup)
	if (BMenuItem* item = fGuideUnitsLP->Menu()->FindMarked())
		item->SetMarked(true);
	fGuideUnitsLP->MenuBar()->ResizeToPreferred();

	// tool tips
	const char* penPressureControl = manager->GetString(PRESSURE_CONTROL_TIP,
														"Enables control by pen pressure.");
	fRadiusDS->SetPressureControlTip(penPressureControl);
	fAlphaDS->SetPressureControlTip(penPressureControl);
	fHardnessDS->SetPressureControlTip(penPressureControl);
	fSpacingDS->SetPressureControlTip(penPressureControl);

	// file menu
	if (fFileM->Superitem())
		fFileM->Superitem()->SetLabel(manager->GetString(FILE_MENU, "File"));
	else
		printf("file has no super item!\n");

	string.SetTo(manager->GetString(OPEN, "Open"));
	string << B_UTF8_ELLIPSIS;
	if (fOpenM->Superitem())
		fOpenM->Superitem()->SetLabel(string.String());
	else
		printf("open has no super item!\n");
	fExportProjectMI->SetLabel(manager->GetString(EXPORT, "Export"));
	string.SetTo(manager->GetString(EXPORT_AS, "Export As"));
	string << B_UTF8_ELLIPSIS;
	fExportProjecAstMI->SetLabel(string.String());
	fSaveProjectMI->SetLabel(manager->GetString(SAVE, "Save"));
	string.SetTo(manager->GetString(SAVE_AS, "Save As"));
	string << B_UTF8_ELLIPSIS;
	fSaveProjectAsMI->SetLabel(string.String());
	string.SetTo(manager->GetString(SETUP_PAGE, "Page Setup"));
	string << B_UTF8_ELLIPSIS;
	fSetupPageMI->SetLabel(string.String());
	fPrintMI->SetLabel(manager->GetString(PRINT, "Print"));
	string.SetTo(manager->GetString(ABOUT, "About"));
	string << B_UTF8_ELLIPSIS;
	fAboutMI->SetLabel(string.String());
	fQuitMI->SetLabel(manager->GetString(QUIT, "Quit"));

	// edit menu
	fEditM->Superitem()->SetLabel(manager->GetString(EDIT, "Edit"));
	PostMessage(MSG_UPDATE_HISTORY_ITEMS);
	fPasteMI->SetLabel(manager->GetString(PASTE, "Paste"));

	// settings menu
	fSettingsM->Superitem()->SetLabel(manager->GetString(SETTINGS, "Settings"));
	fFullscreenMI->SetLabel(manager->GetString(FULLSCREEN, "Fullscreen"));
	fShowGridMI->SetLabel(manager->GetString(SHOW_GRID, "Show Pixel Grid"));
	fToolTipsMI->SetLabel(manager->GetString(TOOL_TIPS, "Tool Tips"));
	string.SetTo(manager->GetString(PROGRAM_SETTINGS, "Program Settings"));
	string << B_UTF8_ELLIPSIS;
	fAppSettingsMI->SetLabel(string.String());

	// canvas menu
	fCanvasM->Superitem()->SetLabel(manager->GetString(CANVAS, "Canvas"));
	string.SetTo(manager->GetString(NEW, "New"));
	string << B_UTF8_ELLIPSIS;
	fNewCanvasMI->SetLabel(string.String());
	string.SetTo(manager->GetString(RESIZE, "Resize"));
	string << B_UTF8_ELLIPSIS;
	fResizeCanvasMI->SetLabel(string.String());
	fClearCanvasMI->SetLabel(manager->GetString(CLEAR, "Clear"));
	fCloseCanvasMI->SetLabel(manager->GetString(CLOSE, "Close"));

	fRotateCanvasM->Superitem()->SetLabel(manager->GetString(ROTATE, "Rotate"));
	fRotateCanvas90MI->SetLabel(manager->GetString(ROTATE_90, "90°"));
	fRotateCanvas180MI->SetLabel(manager->GetString(ROTATE_180, "180°"));
	fRotateCanvas270MI->SetLabel(manager->GetString(ROTATE_270, "-90°"));

	// format menu
#if USE_COLORSPACES
	fFormatM->Superitem()->SetLabel(manager->GetString(FORMAT, "Format"));
#endif

	// layers menu
	fLayerM->Superitem()->SetLabel(manager->GetString(LAYER, "Layer"));

	// mode menu
	fModeM->Superitem()->SetLabel(manager->GetString(MODE, "Mode"));

	// history menu
	fHistoryM->Superitem()->SetLabel(manager->GetString(OBJECT, "Object"));

	// swatches menu
	fSwatchesM->Superitem()->SetLabel(manager->GetString(SWATCHES, "Swatches"));
	fLoadSwatchesM->Superitem()->SetLabel(manager->GetString(LOAD, "Load"));
	fSaveSwatchesM->Superitem()->SetLabel(manager->GetString(SAVE_AS, "Save As"));
	string.SetTo(manager->GetString(NEW_PALETTE, "New"));
	string << B_UTF8_ELLIPSIS;
	fSaveSwatchesAsMI->SetLabel(string.String());
	fDeleteSwatchesM->Superitem()->SetLabel(manager->GetString(DELETE, "Delete"));

	fLayersListView->UpdateStrings();
	fHistoryListView->UpdateStrings();
	fPropertyListView->UpdateStrings();
	fSavePanel->UpdateStrings();
	if (fBrushPanel && fBrushPanel->Lock()) {
		fBrushPanel->UpdateStrings();
		fBrushPanel->Unlock();
	}

	_UpdatePermanentStatus();

	RecalcSize();
}

// GetLicenceeName
const char*
MainWindow::GetLicenceeName() const
{
	const char* name = NULL;
#ifndef TARGET_PLATFORM_HAIKU
	if (fKeyfile) {
		if (const LicenceeInfo* info = fKeyfile->GetLicenceeInfo())
			name = info->GetName();
	}
#else
	name = "Enjoy WonderBrush on Haiku!";
#endif
	return name;
}

// SetStatus
void
MainWindow::SetStatus(const char* message, uint32 warningLevel, bool permanent)
{
	if (fStatusBar) {
		if (warningLevel > STATUS_ERROR)
			warningLevel = STATUS_ERROR;

		if (permanent) {
			BString filtered(message);
			filtered.IReplaceAll('\n', ' ');
			fStatusBar->SetDefaultMessage(filtered.String());
			BubbleHelper::Default()->SetHelp(fStatusBar, message);
		} else
			fStatusBar->SetStatus(message, (status_type)warningLevel);
	}
}

// SetBusy
void
MainWindow::SetBusy(bool busy, const char* message)
{
	if (Lock()) {
		if (message)
			fStatusBar->SetDefaultMessage(message);
		if (!busy)
			_UpdatePermanentStatus();
		fStatusBar->SetIndicateBusy(busy);
		Unlock();
	}
}

// SetConfirmationEnabled
void
MainWindow::SetConfirmationEnabled(bool enable) const
{
	fConfirmIB->SetEnabled(enable);
	fCancelIB->SetEnabled(enable);
	BView* parent = fConfirmIB->Parent();
	if (parent)
		parent = parent->Parent();
	if (MView* group = dynamic_cast<MView*>(parent)) {
		rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);
		if (enable) {
			background.red = 255;
			background.green = 217;
			background.blue = 121;
		}
		group->setcolor(background, true);
	}
}

// _CreateMenuBar
MView*
MainWindow::_CreateMenuBar()
{
	MenuBar* menuBar = new MenuBar("main");

	// File menu
	fFileM = new BMenu("File");

	fOpenM = BRecentFilesList::NewFileListMenu("Open"B_UTF8_ELLIPSIS,
											   new BMessage(B_REFS_RECEIVED), NULL,
											   be_app, 10, false, "image", NULL);
	fFileM->AddItem(fOpenM);

	fOpenM->Superitem()->SetShortcut('O', B_COMMAND_KEY);
	fOpenM->Superitem()->SetMessage(new BMessage(MSG_OPEN));
	fOpenM->Superitem()->SetTarget(this);

	fFileM->AddSeparatorItem();

	fExportProjectMI = new BMenuItem("Export", new BMessage(MSG_EXPORT), 'S', B_OPTION_KEY);
	fFileM->AddItem(fExportProjectMI);

	fExportProjecAstMI = new BMenuItem("Export As"B_UTF8_ELLIPSIS,
								   new BMessage(MSG_EXPORT_AS),
								   'S', B_SHIFT_KEY | B_OPTION_KEY);
	fFileM->AddItem(fExportProjecAstMI);

	fFileM->AddSeparatorItem();

	fSaveProjectMI = new BMenuItem("Save", new BMessage(MSG_SAVE_DOCUMENT),
								   'S');
	fFileM->AddItem(fSaveProjectMI);

	fSaveProjectAsMI = new BMenuItem("Save As"B_UTF8_ELLIPSIS,
									 new BMessage(MSG_SAVE_DOCUMENT_AS),
									 'S', B_SHIFT_KEY);
	fFileM->AddItem(fSaveProjectAsMI);

	fFileM->AddSeparatorItem();

	fSetupPageMI = new BMenuItem("Page Setup"B_UTF8_ELLIPSIS,
								 new BMessage(MSG_SETUP_PAGE),
								 'P', B_SHIFT_KEY);
	fFileM->AddItem(fSetupPageMI);

	fPrintMI = new BMenuItem("Print", new BMessage(MSG_PRINT), 'P');
	fFileM->AddItem(fPrintMI);

	fFileM->AddSeparatorItem();

	fAboutMI = new BMenuItem("About"B_UTF8_ELLIPSIS, new BMessage(B_ABOUT_REQUESTED));
	fAboutMI->SetTarget(be_app);
	fFileM->AddItem(fAboutMI);

	fQuitMI = new BMenuItem("Quit", new BMessage(B_QUIT_REQUESTED), 'Q');
	fFileM->AddItem(fQuitMI);

	menuBar->AddItem(fFileM);

	// Edit menu
	fEditM = new BMenu("Edit");
	fUndoMI = new BMenuItem("Undo", new BMessage(MSG_UNDO), 'Z');
	fUndoMI->SetEnabled(false);
	fEditM->AddItem(fUndoMI);
	fRedoMI = new BMenuItem("Redo", new BMessage(MSG_REDO), 'Z', B_SHIFT_KEY);
	fRedoMI->SetEnabled(false);
	fEditM->AddItem(fRedoMI);

	fEditM->AddSeparatorItem();

	fPasteMI = new BMenuItem("Paste", new BMessage(B_PASTE), 'V');
	fPasteMI->SetEnabled(false);
	fEditM->AddItem(fPasteMI);

	menuBar->AddItem(fEditM);

	// Settings menu
	fSettingsM = new BMenu("Settings");
	fFullscreenMI = new BMenuItem("Fullscreen", new BMessage(MSG_GO_FULL_SCREEN), 'F');
	fSettingsM->AddItem(fFullscreenMI);

	fShowGridMI = new BMenuItem("Show Pixel Grid", new BMessage(MSG_SHOW_GRID));
	fSettingsM->AddItem(fShowGridMI);

	fToolTipsMI = new BMenuItem("Tool Tips", new BMessage(MSG_TOOL_TIPS));
	fSettingsM->AddItem(fToolTipsMI);

	fSettingsM->AddSeparatorItem();

	fAppSettingsMI = new BMenuItem("Program Settings"B_UTF8_ELLIPSIS, new BMessage(MSG_GLOBAL_SETTINGS));
	fSettingsM->AddItem(fAppSettingsMI);

//	menuBar->AddItem(fSettingsM);
	fEditM->AddSeparatorItem();
	fEditM->AddItem(fSettingsM);

	return menuBar;
}

// _CanvasIconView
MView*
MainWindow::_CanvasIconView(float width)
{
	return new MBorder
	(
		M_RAISED_BORDER, 4, "Canvas Icons",
		new HGroup
		(
			minimax(width - 8, -1.0, width - 8, -1.0, 1.0),
			fNewIB,
			fOpenIB,
			new Seperator((const char*)NULL, B_VERTICAL),
			fSaveImageIB,
			fSaveProjectIB,
			new Seperator((const char*)NULL, B_VERTICAL),
			fCloseIB,
			0
		)
	);
}

// _ToolsView
MView*
MainWindow::_ToolsView()
{
#if SLIM_GUI
	return new MBorder
	(
		M_RAISED_BORDER, 4, "Tools",
		new HGroup
		(
			new VGroup
			(
				fCopyIB,
				fCropIB,
				0
			),
			new VGroup
			(
				fTranslateIB,
				fGradientIB,
				0
			),
			new Seperator(B_VERTICAL),
			new VGroup
			(
				fBrushIB,
				fPenIB,
				0
			),
			new VGroup
			(
				fEraserIB,
				fPenEraserIB,
				0
			),
			new Seperator(B_VERTICAL),
			new VGroup
			(
				fCloneBrushIB,
				fBlurIB,
				0
			),
			new VGroup
			(
				fFillIB,
				fTextIB,
				0
			),
			new VGroup
			(
				fShapeIB,
				new Space(),
				0
			),
			new VGroup
			(
				fEllipseIB,
				fRoundRectIB,
				0
			),
			new VGroup
			(
				fFillIB,
				fGradientIB,
				0
			),
			0
		)
	);
#else
	return new MBorder
	(
		M_RAISED_BORDER, 4, "Tools",
		new HGroup
		(
			fPickIB,
			fCopyIB,
			fCropIB,
			fTranslateIB,
			fGradientIB,
			new Seperator(B_VERTICAL),
			fBrushIB,
			fPenIB,
			fEraserIB,
			fPenEraserIB,
			new Seperator(B_VERTICAL),
			fCloneBrushIB,
			fBlurIB,
			new Seperator(B_VERTICAL),
			fFillIB,
			fTextIB,
			fShapeIB,
			fEllipseIB,
			fRoundRectIB,
			new Seperator(B_VERTICAL),
			fColorPickerIB,
			fGuidesIB,
			new Space(),
			0
		)
	);
#endif
}

// _PalettesView
MView*
MainWindow::_PalettesView()
{
	MenuBar* menuBar = new MenuBar("swatches");
	fSwatchesM = new BMenu("Swatches");
	fLoadSwatchesM = new BMenu("Load");
	fSwatchesM->AddItem(fLoadSwatchesM);
	fSaveSwatchesM = new BMenu("Save As");
	fSwatchesM->AddItem(fSaveSwatchesM);
	fSwatchesM->AddSeparatorItem();
	fDeleteSwatchesM = new BMenu("Delete");
	fSwatchesM->AddItem(fDeleteSwatchesM);
	menuBar->AddItem(fSwatchesM);
//	menuBar->SetFont(be_plain_font);
	MView* view = new VGroup
	(
		new HGroup
		(
			menuBar,
			fWindowIB = new ToggleFullscreenIB(new BMessage(MSG_GO_FULL_SCREEN), this),
			0
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Palette",
			new VGroup
			(
				new HGroup
				(
					new MBorder
					(
						M_DEPRESSED_BORDER, 1, "current color",
						fCurrentColorSV = new SwatchView("current color",
														 new BMessage(MSG_GET_COLOR),
														 this, kBlack, 28.0, 28.0)
					),
					horizontal_space(),
					new MBorder
					(
						M_DEPRESSED_BORDER, 1, "swatches",
						new VGroup
						(
							new HGroup
							(
								fSwatchViews[0],
								fSwatchViews[1],
								fSwatchViews[2],
								fSwatchViews[3],
								fSwatchViews[4],
								fSwatchViews[5],
								fSwatchViews[6],
								fSwatchViews[7],
								fSwatchViews[8],
								fSwatchViews[9],
								0
							),
							new HGroup
							(
								fSwatchViews[10],
								fSwatchViews[11],
								fSwatchViews[12],
								fSwatchViews[13],
								fSwatchViews[14],
								fSwatchViews[15],
								fSwatchViews[16],
								fSwatchViews[17],
								fSwatchViews[18],
								fSwatchViews[19],
								0
							),
							0
						)
					),
					0
				),
#if !(SLIM_GUI)
				vertical_space(),
				fColorField,
				fColorSlider,
#endif
				0
			)
		),
		0
	);
	fWindowIB->SetFullscreen(false);
	fCurrentColorSV->SetDroppedMessage(new BMessage(MSG_SET_COLOR));
	return view;
}

// _ToolSetupView
MView*
MainWindow::_ToolSetupView()
{
	return fToolSetupLG = new LayeredGroup
	(
		new BrushConfigView
		(
			4.0,
			fBrushView,
			new HGroup
			(
				fAlphaDS,
				horizontal_space(),
				fRadiusDS,
				horizontal_space(),
				fHardnessDS,
				horizontal_space(),
				fSpacingDS,
				0
			),
			new VGroup
			(
				new HGroup
				(
					fSolidCB,
					fBrushTiltCB,
					0
				),
				new HGroup
				(
					fBrushSubPixelCB,
					new Space(),
					0
				),
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Transformation",
			new HGroup
			(
				new VGroup
				(
					fTranslationSV,
					new HGroup
					(
						fXOffsetTC,
						horizontal_space(),
						fYOffsetTC,
						0
					),
					0
				),
				horizontal_space(),
				new Seperator(B_VERTICAL),
				horizontal_space(),
				new VGroup
				(
					minimax(0.0, 0.0, 10000.0, 10000.0, 0.5),
					fRotationSV,
//					new HGroup
//					(
//						new Space(),
						fRotationTC,
//						0
//					),
					0
				),
				horizontal_space(),
				new Seperator(B_VERTICAL),
				horizontal_space(),
				new VGroup
				(
					fScaleSV,
					new HGroup
					(
						fScaleXTC,
						horizontal_space(),
						fScaleYTC,
						0
					),
					0
				),
				horizontal_space(),
				new Seperator(B_VERTICAL),
				horizontal_space(),
				fMoveSubPixelCB,
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Cropping",
			new HGroup
			(
				fCropLeftTC,
				horizontal_space(),
				fCropTopTC,
				horizontal_space(),
				fCropWidthTC,
				horizontal_space(),
				fCropHeightTC,
				new Space(minimax(5.0, 0.0, 10000.0, 10000.0, 0.2)),
				fCropAllB,
				horizontal_space(),
				fCropSubPixelCB,
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Fill",
			new HGroup
			(
				fFillOpacityDS,
				horizontal_space(),
				fToleranceDS,
				horizontal_space(),
				fSoftnessDS,
				horizontal_space(),
				fFillContiguousCB,
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Dropper",
			new HGroup
			(
				fDropperSizeLP,
				horizontal_space(),
				fUseAllLayersCB,
				new Space(minimax(5.0, 0.0, 10000.0, 10000.0, 2.0)),
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Text",
			new HGroup
			(
				new VGroup
				(
					minimax(0.0, 0.0, 10000.0, 10000.0, 1.5),
					fFontLP,
					new HGroup
					(
						fFontAlignModeIOC,
						new Space(),
						0
					),
					0
				),
				horizontal_space(),
				new HGroup
				(
					fFontSizeSB,
					horizontal_space(),
					fFontOpacityDS,
					horizontal_space(),
					fFontAdvanceScaleDS,
					0
				),
				horizontal_space(),
				new BetterMScrollView(fTextTV, false, false),
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Shape",
			new HGroup
			(
				fShapeOpacityDS,
				new Seperator(B_VERTICAL),
				fShapeOutlineCB,
				horizontal_space(),
				fShapeOutlineWidthDS,
				horizontal_space(),
				new VGroup
				(
					fShapeCapModeIOC,
					fShapeJoinModeIOC,

					0
				),
				new Seperator(B_VERTICAL),
				new VGroup
				(
					new HGroup
					(
						fShapeNewPathIB,
						fShapeTransformIB,
						0
					),
					new HGroup
					(
						fShapeReversePathIB,
						fShapeRemovePointsIB,
						0
					),
					0
				),
				horizontal_space(),
				new VGroup
				(
					minimax(0.0, 0.0, 10000.0, 10000.0, 0.7),
					fShapeClosedCB,
					fShapeSubpixelsCB,
					0
				),
				horizontal_space(),
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Clipboard",
			new HGroup
			(
				fSelectLeftTC,
				horizontal_space(),
				fSelectTopTC,
				horizontal_space(),
				fSelectWidthTC,
				horizontal_space(),
				fSelectHeightTC,
				new Space(minimax(5.0, 0.0, 10000.0, 10000.0, 0.2)),
				fSelectAllB,
				horizontal_space(),
				fSelectAllLayersCB,
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Ellipse",
			new HGroup
			(
				fEllipseOpacityDS,
				new Seperator(B_VERTICAL),
				fEllipseOutlineCB,
				horizontal_space(),
				fEllipseOutlineWidthDS,
				horizontal_space(),
				new Space(minimax(0.0, 0.0, 10000.0, 10000.0, 1.0)),
				horizontal_space(),
				fEllipseSubpixelsCB,
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Round Rect",
			new HGroup
			(
				fRectOpacityDS,
				new Seperator(B_VERTICAL),
				fRectOutlineCB,
				horizontal_space(),
				fRectOutlineWidthDS,
				horizontal_space(),
				fRectCornerRadiusDS,
				horizontal_space(),
				fRectSubpixelsCB,
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Edit Gradient",
			new HGroup
			(
				fGradientControl,
				horizontal_space(),
				fGradientStopAlphaDS,
				horizontal_space(),
//				fGradientTypeLP,
				new VGroup
				(
					fGradientTypeIOC,
					fGradientInheritsTransformCB,
					0
				),
				fGradientInterpolationLP,
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Guides",
			new HGroup
			(
				fGuidesShowCB,
				new Seperator(B_VERTICAL),
				fGuideCreateIB,
				fGuideRemoveIB,
				fGuideProportionIB,
				horizontal_space(),
				fGuideHPosTC,
				horizontal_space(),
				fGuideVPosTC,
				horizontal_space(),
				fGuideUnitsLP,
				horizontal_space(),
				fGuideAngleTC,
				0
			)
		),
		new MBorder
		(
			M_RAISED_BORDER, 4, "Pick Objects",
			new HGroup
			(
				new VGroup
				(
					fSelectableSV,
					new HGroup
					(
						fPickMaskBrushIB,
						fPickMaskPenIB,
						fPickMaskEraserIB,
						fPickMaskPenEraserIB,
						fPickMaskCloneIB,
						fPickMaskBlurIB,
						fPickMaskFillIB,
						fPickMaskTextIB,
						fPickMaskShapeIB,
						fPickMaskBitmapIB,
						horizontal_space(),
						fPickMaskAllB,
						fPickMaskNoneB,
						0
					),
					0
				),
				new Seperator(B_VERTICAL),
				new VGroup
				(
					fSelectSV,
					new HGroup
					(
						fPickAllB,
						fPickNoneB,
						0
					),
					0
				),
				0
			)
		),
		0
	);
}

// _ConfirmGroup
MView*
MainWindow::_ConfirmGroup()
{
#if SLIM_GUI
	return new MBorder
		(
			M_RAISED_BORDER, 4, "confirm group",
			new VGroup
			(
				fConfirmIB,
				fCancelIB,
				0
			)
		);
#else
	return new MBorder
		(
			M_RAISED_BORDER, 4, "confirm group",
			new HGroup
			(
				fUndoIB,
				fRedoIB,
				new Seperator((const char*)NULL, B_VERTICAL),
				fConfirmIB,
				fCancelIB,
				0
			)
		);
#endif
}

// _LoadSettings()
void
MainWindow::_LoadSettings()
{
	load_settings(fSettings, "main_settings", "WonderBrush");
	// restore window frame
	BRect frame;
	if (fSettings->FindRect("window frame", &frame) == B_OK) {
		make_sure_frame_is_on_screen(frame, this);
		MoveTo(frame.LeftTop());
		ResizeTo(frame.Width(), frame.Height());
		// test if this is full screen
		BScreen screen(this);
		if (screen.Frame() == frame) {
			fNonFullscreenFrame = frame;
			fNonFullscreenFrame.InsetBy(50.0, 50.0);
			fZoomToFullscreen = false;
			fFullscreenMI->SetMarked(true);
			fWindowIB->SetFullscreen(true);
		} else
			fWindowIB->SetFullscreen(false);
	}
	if (fSettings->FindRect("color picker frame", &frame) == B_OK) {
		make_sure_frame_is_on_screen(frame, this);
		fColorPickerFrame = frame;
	}
	if (fSettings->FindRect("new canvas frame", &frame) == B_OK) {
		make_sure_frame_is_on_screen(frame, this);
		fNewCanvasFrame = frame;
	}
	if (fSettings->FindRect("resize canvas frame", &frame) == B_OK) {
		make_sure_frame_is_on_screen(frame, this);
		fResizeCanvasFrame = frame;
	}
	if (fSettings->FindRect("global settings frame", &frame) == B_OK) {
		make_sure_frame_is_on_screen(frame, this);
		fSettingsPanelFrame = frame;
	}
	bool bubbleHelp;
	if (fSettings->FindBool("bubble help", &bubbleHelp) >= B_OK) {
		BubbleHelper* helper = BubbleHelper::Default();
		helper->SetEnabled(bubbleHelp);
		fToolTipsMI->SetMarked(helper->IsEnabled());
	} else
		fToolTipsMI->SetMarked(true);

	// restore swatches
	_LoadSwatches(fSettings);

	uint32 value;
	if (fSettings->FindInt32("tool", (int32*)&value) == B_OK) {
		fCanvasView->SetTool(value);
		// set tool after actually running, otherwise we crash in liblayout
		BMessage msg(MSG_SET_TOOL);
		msg.AddInt32("id", value);
		PostMessage(&msg);
	}

	// Brush tool settings
	if (fSettings->FindInt32("brush flags", (int32*)&value) == B_OK)
		_SetBrushFlags(value);
	if (fSettings->FindInt32("color picker mode", (int32*)&value) == B_OK)
		fColorPickerMode = (selected_color_mode)value;
	float min;
	float max;
	if (fSettings->FindFloat("min alpha", &min) == B_OK
		&& fSettings->FindFloat("max alpha", &max) == B_OK) {
		fCanvasView->SetMaxAlpha(min, max);
		fAlphaDS->SetValues(min, max);
	}
	if (fSettings->FindFloat("min brush radius", &min) == B_OK
		&& fSettings->FindFloat("max brush radius", &max) == B_OK) {
		fCanvasView->SetBrushRadius(min, max);
		fRadiusDS->SetValues(min / 100.0, max / 100.0);
	}
	if (fSettings->FindFloat("min brush hardness", &min) == B_OK
		&& fSettings->FindFloat("max brush hardness", &max) == B_OK) {
		fCanvasView->SetBrushHardness(min, max);
		fHardnessDS->SetValues(min, max);
	}
	if (fSettings->FindFloat("min brush spacing", &min) == B_OK
		&& fSettings->FindFloat("max brush spacing", &max) == B_OK) {
		fCanvasView->SetBrushSpacing(min, max);
		fSpacingDS->SetValues(min, max);
	}

	// subpixel tool settings
	bool subPixels;
	if (fSettings->FindBool("crop subpixels", &subPixels) >= B_OK) {
		fCanvasView->SetPrecise(subPixels, TOOL_CROP);
		fCropSubPixelCB->SetValue(subPixels);
	}
	if (fSettings->FindBool("translate subpixels", &subPixels) >= B_OK) {
		fCanvasView->SetPrecise(subPixels, TOOL_TRANSLATE);
		fMoveSubPixelCB->SetValue(subPixels);
	}
	if (fSettings->FindBool("stroke subpixels", &subPixels) >= B_OK) {
		fCanvasView->SetPrecise(subPixels, TOOL_BRUSH);
		fBrushSubPixelCB->SetValue(subPixels);
	}
	if (fSettings->FindBool("shape subpixels", &subPixels) >= B_OK) {
		fCanvasView->SetPrecise(subPixels, TOOL_SHAPE);
		fShapeSubpixelsCB->SetValue(subPixels);
	}
	if (fSettings->FindBool("ellipse subpixels", &subPixels) >= B_OK) {
		fCanvasView->SetPrecise(subPixels, TOOL_ELLIPSE);
		fEllipseSubpixelsCB->SetValue(subPixels);
	}
	if (fSettings->FindBool("rect subpixels", &subPixels) >= B_OK) {
		fCanvasView->SetPrecise(subPixels, TOOL_ROUND_RECT);
		fRectSubpixelsCB->SetValue(subPixels);
	}

	// Fill tool settings
	if (fSettings->FindInt32("fill opacity", (int32*)&value) >= B_OK) {
		fCanvasView->SetOpacity(value, TOOL_BUCKET_FILL);
		fFillOpacityDS->SetMaxValue(value / 255.0);
	}
	if (fSettings->FindInt32("fill tolerance", (int32*)&value) >= B_OK) {
		fCanvasView->SetTolerance(value);
		fToleranceDS->SetMaxValue(value / 255.0);
	}
	if (fSettings->FindInt32("fill softness", (int32*)&value) >= B_OK) {
		fCanvasView->SetSoftness(value);
		fSoftnessDS->SetMaxValue(value / 255.0);
	}
	bool contiguous;
	if (fSettings->FindBool("fill contiguous", &contiguous) >= B_OK) {
		fCanvasView->SetFillContiguous(contiguous);
		fFillContiguousCB->SetValue(contiguous);
	}

	// Dropper tool settings
	uint32 dropperTipSize;
	if (fSettings->FindInt32("dropper tip size", (int32*)&dropperTipSize) >= B_OK) {
		switch (dropperTipSize) {
			case 1:
				fDropperSize1x1MI->SetMarked(true);
				fCanvasView->SetDropperTipSize(dropperTipSize);
				break;
			case 3:
				fDropperSize3x3MI->SetMarked(true);
				fCanvasView->SetDropperTipSize(dropperTipSize);
				break;
			case 5:
				fDropperSize5x5MI->SetMarked(true);
				fCanvasView->SetDropperTipSize(dropperTipSize);
				break;
		}
	}

	bool dropperUsesAllLayers;
	if (fSettings->FindBool("dropper uses all layers", &dropperUsesAllLayers) >= B_OK) {
		fCanvasView->SetDropperUsesAllLayers(dropperUsesAllLayers);
		fUseAllLayersCB->SetValue(dropperUsesAllLayers);
	}

	// Text tool settings
	const char* text;
	if (fSettings->FindString("text text", &text) >= B_OK) {
		fCanvasView->SetText(text);
		fTextTV->SetText(text);
	}
	const char* family;
	const char* style;
	if (fSettings->FindString("text font family", &family) >= B_OK
		&& fSettings->FindString("text font style", &style) >= B_OK)
		fCanvasView->SetFamilyAndStyle(family, style);
	float fontSize;
	if (fSettings->FindFloat("text font size", &fontSize) >= B_OK) {
		fCanvasView->SetFontSize(fontSize);
		fFontSizeSB->SetMaxValue(sqrtf(fontSize / 50.0));
	}
	uint32 fontOpacity;
	if (fSettings->FindInt32("text opacity", (int32*)&fontOpacity) >= B_OK) {
		fCanvasView->SetFontOpacity(fontOpacity);
		fFontOpacityDS->SetMaxValue(fontOpacity / 255.0);
	}
	float scale;
	if (fSettings->FindFloat("text advance scale", &scale) >= B_OK) {
		fCanvasView->SetFontAdvanceScale(scale);
		fFontAdvanceScaleDS->SetMaxValue(scale);
	}
	uint32 alignment;
	if (fSettings->FindInt32("text alignment", (int32*)&alignment) >= B_OK) {
		fCanvasView->SetTextAlignment(alignment);
		fFontAlignModeIOC->SetValue(alignment);
	}
	fCanvasView->RestoreNonGUIFontSettings(fSettings);

	// Shape tool settings
	uint32 shapeOpacity;
	if (fSettings->FindInt32("shape opacity", (int32*)&shapeOpacity) >= B_OK) {
		fCanvasView->SetOpacity(shapeOpacity, TOOL_SHAPE);
		fShapeOpacityDS->SetMaxValue(shapeOpacity / 255.0);
	}
	bool shapeOutline;
	if (fSettings->FindBool("shape outline", &shapeOutline) >= B_OK) {
		fCanvasView->SetOutline(shapeOutline, TOOL_SHAPE);
		fShapeOutlineCB->SetValue(shapeOutline);
	}
	float shapeOutlineWidth;
	if (fSettings->FindFloat("shape outline width", &shapeOutlineWidth) >= B_OK) {
		fCanvasView->SetOutlineWidth(shapeOutlineWidth, TOOL_SHAPE);
		fShapeOutlineWidthDS->SetMaxValue(sqrtf(shapeOutlineWidth / 100.0));
	}
	uint32 lineMode;
	if (fSettings->FindInt32("shape cap mode", (int32*)&lineMode) >= B_OK) {
		fCanvasView->SetShapeCapMode(lineMode);
		fShapeCapModeIOC->SetValue(lineMode);
	}
	if (fSettings->FindInt32("shape join mode", (int32*)&lineMode) >= B_OK) {
		fCanvasView->SetShapeJoinMode(lineMode);
		fShapeJoinModeIOC->SetValue(lineMode);
	}
	fCanvasView->RestoreNonGUIShapeSettings(fSettings);

	// Ellipse tool settings
	if (fSettings->FindInt32("ellipse opacity", (int32*)&shapeOpacity) >= B_OK) {
		fCanvasView->SetOpacity(shapeOpacity, TOOL_ELLIPSE);
		fEllipseOpacityDS->SetMaxValue(shapeOpacity / 255.0);
	}
	if (fSettings->FindBool("ellipse outline", &shapeOutline) >= B_OK) {
		fCanvasView->SetOutline(shapeOutline, TOOL_ELLIPSE);
		fEllipseOutlineCB->SetValue(shapeOutline);
		fEllipseOutlineWidthDS->SetEnabled(shapeOutline);
	}
	if (fSettings->FindFloat("ellipse outline width", &shapeOutlineWidth) >= B_OK) {
		fCanvasView->SetOutlineWidth(shapeOutlineWidth, TOOL_ELLIPSE);
		fEllipseOutlineWidthDS->SetMaxValue(sqrtf(shapeOutlineWidth / 100.0));
	}

	// Round Rect tool settings
	if (fSettings->FindInt32("rect opacity", (int32*)&shapeOpacity) >= B_OK) {
		fCanvasView->SetOpacity(shapeOpacity, TOOL_ROUND_RECT);
		fRectOpacityDS->SetMaxValue(shapeOpacity / 255.0);
	}
	if (fSettings->FindBool("rect outline", &shapeOutline) >= B_OK) {
		fCanvasView->SetOutline(shapeOutline, TOOL_ROUND_RECT);
		fRectOutlineCB->SetValue(shapeOutline);
		fRectOutlineWidthDS->SetEnabled(shapeOutline);
	}
	if (fSettings->FindFloat("rect outline width", &shapeOutlineWidth) >= B_OK) {
		fCanvasView->SetOutlineWidth(shapeOutlineWidth, TOOL_ROUND_RECT);
		fRectOutlineWidthDS->SetMaxValue(sqrtf(shapeOutlineWidth / 100.0));
	}
	if (fSettings->FindFloat("rect corner radius", &shapeOutlineWidth) >= B_OK) {
		fCanvasView->SetRoundCornerRadius(shapeOutlineWidth);
		fRectCornerRadiusDS->SetMaxValue(shapeOutlineWidth);
	}

	// Edit Gradient tool settings
	BMessage gradientArchive;
	if (fSettings->FindMessage("current gradient", &gradientArchive) >= B_OK) {
		Gradient gradient(&gradientArchive);
		fGradientControl->SetGradient(&gradient);
	}

	// Pick Objects tool settings
	uint32 pickMask;
	if (fSettings->FindInt32("pick mask", (int32*)&pickMask) >= B_OK) {
		fCanvasView->SetPickMask(pickMask);
	}
	_UpdatePickControls(fCanvasView->CurrentCanvas() != NULL);

	// canvas view settings
	bool showGrid;
	if (fSettings->FindBool("show grid", &showGrid) >= B_OK) {
		fCanvasView->SetShowGrid(showGrid);
		fShowGridMI->SetMarked(showGrid);
	}

	// restore current color
	const void* colorPointer;
	ssize_t size = sizeof(rgb_color);
	if (fSettings->FindData("color", B_RGB_COLOR_TYPE,
							&colorPointer, &size) == B_OK) {
		rgb_color color = *(const rgb_color*)colorPointer;
		fCanvasView->SetColor(color);
		fCurrentColorSV->SetColor(color);

		// update colorfield
		float h, s, v;
		RGB_to_HSV(color.red / 255.0, color.green / 255.0, color.blue / 255.0, h, s, v);
		fColorField->SetFixedValue(h);
		fColorField->SetMarkerToColor(color);

		fColorSlider->SetOtherValues(s, v);
		fColorSlider->SetMarkerToColor(color);
	}

	// language
	LanguageManager* lm = LanguageManager::Default();
	if (GlobalSettings::CreateDefault()->UseSystemLanguage()) {
		lm->SetLanguage(lm->PosixSystemLanguage());
		UpdateStrings();
	} else {
		const char* language;
		if (fSettings->FindString("language name", &language) >= B_OK
			&& strcmp(language, lm->LanguageName()) != 0) {
			lm->SetLanguage(language);
			UpdateStrings();
		}
	}

	// save panel settings
	int32 exportMode;
	if (fSettings->FindInt32("export mode", &exportMode) < B_OK)
		exportMode = EXPORT_TRANSLATOR;
	fSavePanel->SetExportMode(exportMode);
	if (exportMode == EXPORT_TRANSLATOR) {
		translator_id id;
		uint32 format;
		if (fSettings->FindInt32("translator id", (int32*)&id) == B_OK
			&& fSettings->FindInt32("translator format", (int32*)&format) == B_OK)
			fSavePanel->SetTranslator(id, format);
	}
	entry_ref ref;
	if (fSettings->FindRef("last save path", &ref) >= B_OK) {
		if (!fLastSavePath)
			fLastSavePath = new entry_ref;
		*fLastSavePath = ref;
	}
	if (fSettings->FindRef("last export path", &ref) >= B_OK) {
		if (!fLastExportPath)
			fLastExportPath = new entry_ref;
		*fLastExportPath = ref;
	}

	// layer/history group weighting
	float layerWeight;
	float historyWeight;
	float propertyWeight;
	if (fSettings->FindFloat("layer group weight", &layerWeight) >= B_OK
		&& fSettings->FindFloat("history group weight", &historyWeight) >= B_OK
		&& fSettings->FindFloat("property group weight", &propertyWeight) >= B_OK) {
		fLayerVG->ct_mpm.weight = layerWeight;
		fHistoryVG->ct_mpm.weight = historyWeight;
		fPropertyVG->ct_mpm.weight = 3.0 - (layerWeight + historyWeight); //propertyWeight;
	}

}

// _SaveSettings()
void
MainWindow::_SaveSettings()
{
	// store window frame
	if (fSettings->ReplaceRect("window frame", Frame()) != B_OK)
		fSettings->AddRect("window frame", Frame());
	// store swatches
	_SaveSwatches(fSettings, NULL);

	if (fSettings->ReplaceInt32("tool", fCanvasView->Tool()) != B_OK)
		fSettings->AddInt32("tool", fCanvasView->Tool());
	if (fSettings->ReplaceInt32("brush flags", fCanvasView->BrushFlags()) != B_OK)
		fSettings->AddInt32("brush flags", fCanvasView->BrushFlags());
	if (fSettings->ReplaceFloat("min alpha", fCanvasView->BrushAlpha().min) != B_OK)
		fSettings->AddFloat("min alpha", fCanvasView->BrushAlpha().min);
	if (fSettings->ReplaceFloat("max alpha", fCanvasView->BrushAlpha().max) != B_OK)
		fSettings->AddFloat("max alpha", fCanvasView->BrushAlpha().max);
	if (fSettings->ReplaceFloat("min brush radius", fCanvasView->BrushRadius().min) != B_OK)
		fSettings->AddFloat("min brush radius", fCanvasView->BrushRadius().min);
	if (fSettings->ReplaceFloat("max brush radius", fCanvasView->BrushRadius().max) != B_OK)
		fSettings->AddFloat("max brush radius", fCanvasView->BrushRadius().max);
	if (fSettings->ReplaceFloat("min brush hardness", fCanvasView->BrushHardness().min) != B_OK)
		fSettings->AddFloat("min brush hardness", fCanvasView->BrushHardness().min);
	if (fSettings->ReplaceFloat("max brush hardness", fCanvasView->BrushHardness().max) != B_OK)
		fSettings->AddFloat("max brush hardness", fCanvasView->BrushHardness().max);
	if (fSettings->ReplaceFloat("min brush spacing", fCanvasView->BrushSpacing().min) != B_OK)
		fSettings->AddFloat("min brush spacing", fCanvasView->BrushSpacing().min);
	if (fSettings->ReplaceFloat("max brush spacing", fCanvasView->BrushSpacing().max) != B_OK)
		fSettings->AddFloat("max brush spacing", fCanvasView->BrushSpacing().max);

	// canvas view settings
	bool subPixels = fCanvasView->Precise(TOOL_CROP);
	if (fSettings->ReplaceBool("crop subpixels", subPixels) != B_OK)
		fSettings->AddBool("crop subpixels", subPixels);
	subPixels = fCanvasView->Precise(TOOL_TRANSLATE);
	if (fSettings->ReplaceBool("translate subpixels", subPixels) != B_OK)
		fSettings->AddBool("translate subpixels", subPixels);
	subPixels = fCanvasView->Precise(TOOL_BRUSH);
	if (fSettings->ReplaceBool("stroke subpixels", subPixels) != B_OK)
		fSettings->AddBool("stroke subpixels", subPixels);
	subPixels = fCanvasView->Precise(TOOL_SHAPE);
	if (fSettings->ReplaceBool("shape subpixels", subPixels) != B_OK)
		fSettings->AddBool("shape subpixels", subPixels);
	subPixels = fCanvasView->Precise(TOOL_ELLIPSE);
	if (fSettings->ReplaceBool("ellipse subpixels", subPixels) != B_OK)
		fSettings->AddBool("ellipse subpixels", subPixels);
	subPixels = fCanvasView->Precise(TOOL_ROUND_RECT);
	if (fSettings->ReplaceBool("rect subpixels", subPixels) != B_OK)
		fSettings->AddBool("rect subpixels", subPixels);

	// Fill tool settings
	if (fSettings->ReplaceInt32("fill opacity", fCanvasView->Opacity(TOOL_BUCKET_FILL)) != B_OK)
		fSettings->AddInt32("fill opacity", fCanvasView->Opacity(TOOL_BUCKET_FILL));
	if (fSettings->ReplaceInt32("fill tolerance", fCanvasView->Tolerance()) != B_OK)
		fSettings->AddInt32("fill tolerance", fCanvasView->Tolerance());
	if (fSettings->ReplaceInt32("fill softness", fCanvasView->Softness()) != B_OK)
		fSettings->AddInt32("fill softness", fCanvasView->Softness());
	if (fSettings->ReplaceBool("fill contiguous", fCanvasView->FillContiguous()) != B_OK)
		fSettings->AddBool("fill contiguous", fCanvasView->FillContiguous());

	// Text tool settings
	if (fSettings->ReplaceString("text text", fCanvasView->Text()) < B_OK)
		fSettings->AddString("text text", fCanvasView->Text());
	if (fSettings->ReplaceString("text font family", fCanvasView->FontFamily()) < B_OK)
		fSettings->AddString("text font family", fCanvasView->FontFamily());
	if (fSettings->ReplaceString("text font style", fCanvasView->FontStyle()) < B_OK)
		fSettings->AddString("text font style", fCanvasView->FontStyle());
	if (fSettings->ReplaceFloat("text font size", fCanvasView->FontSize()) < B_OK)
		fSettings->AddFloat("text font size", fCanvasView->FontSize());
	if (fSettings->ReplaceInt32("text opacity", fCanvasView->FontOpacity()) < B_OK)
		fSettings->AddInt32("text opacity", fCanvasView->FontOpacity());
	if (fSettings->ReplaceFloat("text advance scale", fCanvasView->FontAdvanceScale()) < B_OK)
		fSettings->AddFloat("text advance scale", fCanvasView->FontAdvanceScale());
	if (fSettings->ReplaceInt32("text alignment", fCanvasView->TextAlignment()) < B_OK)
		fSettings->AddInt32("text alignment", fCanvasView->TextAlignment());
	fCanvasView->StoreNonGUIFontSettings(fSettings);

	// Shape tool settings
	if (fSettings->ReplaceInt32("shape opacity", fCanvasView->Opacity(TOOL_SHAPE)) < B_OK)
		fSettings->AddInt32("shape opacity", fCanvasView->Opacity(TOOL_SHAPE));
	if (fSettings->ReplaceBool("shape outline", fCanvasView->Outline(TOOL_SHAPE)) < B_OK)
		fSettings->AddBool("shape outline", fCanvasView->Outline(TOOL_SHAPE));
	if (fSettings->ReplaceFloat("shape outline width", fCanvasView->OutlineWidth(TOOL_SHAPE)) < B_OK)
		fSettings->AddFloat("shape outline width", fCanvasView->OutlineWidth(TOOL_SHAPE));
	if (fSettings->ReplaceInt32("shape cap mode", fCanvasView->ShapeCapMode()) < B_OK)
		fSettings->AddInt32("shape cap mode", fCanvasView->ShapeCapMode());
	if (fSettings->ReplaceInt32("shape join mode", fCanvasView->ShapeJoinMode()) < B_OK)
		fSettings->AddInt32("shape join mode", fCanvasView->ShapeJoinMode());
	fCanvasView->StoreNonGUIShapeSettings(fSettings);

	// Ellipse tool settings
	if (fSettings->ReplaceInt32("ellipse opacity", fCanvasView->Opacity(TOOL_ELLIPSE)) < B_OK)
		fSettings->AddInt32("ellipse opacity", fCanvasView->Opacity(TOOL_ELLIPSE));
	if (fSettings->ReplaceBool("ellipse outline", fCanvasView->Outline(TOOL_ELLIPSE)) < B_OK)
		fSettings->AddBool("ellipse outline", fCanvasView->Outline(TOOL_ELLIPSE));
	if (fSettings->ReplaceFloat("ellipse outline width", fCanvasView->OutlineWidth(TOOL_ELLIPSE)) < B_OK)
		fSettings->AddFloat("ellipse outline width", fCanvasView->OutlineWidth(TOOL_ELLIPSE));

	// Round Rect tool settings
	if (fSettings->ReplaceInt32("rect opacity", fCanvasView->Opacity(TOOL_ROUND_RECT)) < B_OK)
		fSettings->AddInt32("rect opacity", fCanvasView->Opacity(TOOL_ROUND_RECT));
	if (fSettings->ReplaceBool("rect outline", fCanvasView->Outline(TOOL_ROUND_RECT)) < B_OK)
		fSettings->AddBool("rect outline", fCanvasView->Outline(TOOL_ROUND_RECT));
	if (fSettings->ReplaceFloat("rect outline width", fCanvasView->OutlineWidth(TOOL_ROUND_RECT)) < B_OK)
		fSettings->AddFloat("rect outline width", fCanvasView->OutlineWidth(TOOL_ROUND_RECT));
	if (fSettings->ReplaceFloat("rect corner radius", fCanvasView->RoundCornerRadius()) < B_OK)
		fSettings->AddFloat("rect corner radius", fCanvasView->RoundCornerRadius());

	// Edit Gradient tool settings
	BMessage gradientArchive;
	if (fGradientControl->GetGradient()->Archive(&gradientArchive) >= B_OK) {
		if (fSettings->ReplaceMessage("current gradient", &gradientArchive) < B_OK)
			fSettings->AddMessage("current gradient", &gradientArchive);
	}

	if (fSettings->ReplaceInt32("pick mask", fCanvasView->PickMask()) < B_OK)
		fSettings->AddInt32("pick mask", fCanvasView->PickMask());

	// canvas view settings
	if (fSettings->ReplaceBool("show grid", fCanvasView->ShowGrid()) != B_OK)
		fSettings->AddBool("show grid", fCanvasView->ShowGrid());

	if (fSettings->ReplaceInt32("dropper tip size", fCanvasView->DropperTipSize()) < B_OK)
		fSettings->AddInt32("dropper tip size", fCanvasView->DropperTipSize());

	if (fSettings->ReplaceBool("dropper uses all layers", fCanvasView->DropperUsesAllLayers()) < B_OK)
		fSettings->AddBool("dropper uses all layers", fCanvasView->DropperUsesAllLayers());

	rgb_color color = fCanvasView->Color();
	ssize_t size = sizeof(rgb_color);
	if (fSettings->ReplaceData("color", B_RGB_COLOR_TYPE, &color, size) != B_OK)
		fSettings->AddData("color", B_RGB_COLOR_TYPE, &color, size);

	if (fSettings->ReplaceRect("color picker frame", fColorPickerFrame) != B_OK)
		fSettings->AddRect("color picker frame", fColorPickerFrame);
	if (fSettings->ReplaceInt32("color picker mode", fColorPickerMode) != B_OK)
		fSettings->AddInt32("color picker mode", fColorPickerMode);
	if (fSettings->ReplaceRect("new canvas frame", fNewCanvasFrame) != B_OK)
		fSettings->AddRect("new canvas frame", fNewCanvasFrame);
	if (fSettings->ReplaceRect("resize canvas frame", fResizeCanvasFrame) != B_OK)
		fSettings->AddRect("resize canvas frame", fResizeCanvasFrame);
	if (fSettings->ReplaceRect("global settings frame", fSettingsPanelFrame) != B_OK)
		fSettings->AddRect("global settings frame", fSettingsPanelFrame);

	// bubble help
	BubbleHelper* helper = BubbleHelper::Default();
	if (fSettings->ReplaceBool("bubble help", helper->IsEnabled()) < B_OK)
		fSettings->AddBool("bubble help", helper->IsEnabled());

	// language
	const char* language = LanguageManager::Default()->LanguageName();
	if (fSettings->ReplaceString("language name", language) < B_OK)
		fSettings->AddString("language name", language);

	// save panel settings
	int32 exportMode = fSavePanel->ExportMode();
	translator_id id = fSavePanel->TranslatorID();
	uint32 format = fSavePanel->TranslatorFormat();
	if (fSettings->ReplaceInt32("export mode", exportMode) < B_OK)
		fSettings->AddInt32("export mode", exportMode);
	if (fSettings->ReplaceInt32("translator id", id) < B_OK)
		fSettings->AddInt32("translator id", id);
	if (fSettings->ReplaceInt32("translator format", format) < B_OK)
		fSettings->AddInt32("translator format", format);
	if (fLastSavePath) {
		if (fSettings->ReplaceRef("last save path", fLastSavePath) < B_OK)
			fSettings->AddRef("last save path", fLastSavePath);
	}
	if (fLastExportPath) {
		if (fSettings->ReplaceRef("last export path", fLastExportPath) < B_OK)
			fSettings->AddRef("last export path", fLastExportPath);
	}

	// layer/history weighting
	float layerWeight = fLayerVG->mpm.weight;
	float historyWeight = fHistoryVG->mpm.weight;
	float propertyWeight = fPropertyVG->mpm.weight;
	if (fSettings->ReplaceFloat("layer group weight", layerWeight) < B_OK)
		fSettings->AddFloat("layer group weight", layerWeight);
	if (fSettings->ReplaceFloat("history group weight", historyWeight) < B_OK)
		fSettings->AddFloat("history group weight", historyWeight);
	if (fSettings->ReplaceFloat("property group weight", propertyWeight) < B_OK)
		fSettings->AddFloat("property group weight", propertyWeight);

	save_settings(fSettings, "main_settings", "WonderBrush");
}

// _SetTool
void
MainWindow::_SetTool(uint32 id)
{
	fBrushIB->SetPressed(id == fBrushIB->ID());
	fPenIB->SetPressed(id == fPenIB->ID());
	fPenEraserIB->SetPressed(id == fPenEraserIB->ID());
	fEraserIB->SetPressed(id == fEraserIB->ID());
	fCloneBrushIB->SetPressed(id == fCloneBrushIB->ID());
	fColorPickerIB->SetPressed(id == fColorPickerIB->ID());
	fBlurIB->SetPressed(id == fBlurIB->ID());
	fTranslateIB->SetPressed(id == fTranslateIB->ID());
	fCropIB->SetPressed(id == fCropIB->ID());
	fFillIB->SetPressed(id == fFillIB->ID());
	fTextIB->SetPressed(id == fTextIB->ID());
	fShapeIB->SetPressed(id == fShapeIB->ID());
	fCopyIB->SetPressed(id == fCopyIB->ID());
	fEllipseIB->SetPressed(id == fEllipseIB->ID());
	fRoundRectIB->SetPressed(id == fRoundRectIB->ID());
	fGradientIB->SetPressed(id == fGradientIB->ID());
	fGuidesIB->SetPressed(id == fGuidesIB->ID());
	fPickIB->SetPressed(id == fPickIB->ID());

	// defocus any text views
	fSelectLeftTC->MakeFocus(false);
	fSelectTopTC->MakeFocus(false);
	fSelectWidthTC->MakeFocus(false);
	fSelectHeightTC->MakeFocus(false);

	fCropLeftTC->MakeFocus(false);
	fCropTopTC->MakeFocus(false);
	fCropWidthTC->MakeFocus(false);
	fCropHeightTC->MakeFocus(false);

	fXOffsetTC->MakeFocus(false);
	fYOffsetTC->MakeFocus(false);
	fRotationTC->MakeFocus(false);
	fScaleXTC->MakeFocus(false);
	fScaleYTC->MakeFocus(false);

	fTextTV->MakeFocus(false);

	fGuideHPosTC->MakeFocus(false);
	fGuideVPosTC->MakeFocus(false);
	fGuideAngleTC->MakeFocus(false);

	bool enable = fCanvasView->CurrentCanvas();
	// activate correct tool setup
	if (id == fPenIB->ID() || id == fPenEraserIB->ID()) {
		fAlphaDS->SetEnabled(enable);
		fRadiusDS->SetEnabled(false);
		fHardnessDS->SetEnabled(false);
		fSpacingDS->SetEnabled(false);
		fBrushView->SetEnabled(false);
	} else {
		fAlphaDS->SetEnabled(enable);
		fRadiusDS->SetEnabled(enable);
		fHardnessDS->SetEnabled(enable && !(fCanvasView->BrushFlags() & FLAG_SOLID));
		fSpacingDS->SetEnabled(enable);
		fBrushView->SetEnabled(enable);
	}
	_UpdatePermanentStatus();

	int32 layer = 0;
	if (id == TOOL_TRANSLATE)
		layer = 1;
	else if (id == TOOL_CROP)
		layer = 2;
	else if (id == TOOL_BUCKET_FILL)
		layer = 3;
	else if (id == TOOL_COLOR_PICKER)
		layer = 4;
	else if (id == TOOL_TEXT)
		layer = 5;
	else if (id == TOOL_SHAPE)
		layer = 6;
	else if (id == TOOL_SELECT)
		layer = 7;
	else if (id == TOOL_ELLIPSE)
		layer = 8;
	else if (id == TOOL_ROUND_RECT)
		layer = 9;
	else if (id == TOOL_EDIT_GRADIENT)
		layer = 10;
	else if (id == TOOL_GUIDES)
		layer = 11;
	else if (id == TOOL_PICK)
		layer = 12;

	BMessage message(MSG_ACTIVATE_LAYER);
	message.AddInt32("value", layer);
	PostMessage(&message);
}

// _BuildSwatchMenu
void
MainWindow::_BuildSwatchMenu()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) >= B_OK
		&& path.Append("WonderBrush/swatches") >= B_OK
		&& create_directory(path.Path(), 0777) >= B_OK) {
		BDirectory dir(path.Path());
		if (dir.InitCheck() >= B_OK) {
			// empty the menus
			while (BMenuItem* item = fLoadSwatchesM->RemoveItem((int32)0)) {
				delete item;
			}
			while (BMenuItem* item = fSaveSwatchesM->RemoveItem((int32)0)) {
				delete item;
			}
			while (BMenuItem* item = fDeleteSwatchesM->RemoveItem((int32)0)) {
				delete item;
			}
			LanguageManager* manager = LanguageManager::Default();
			BString string(manager->GetString(NEW_PALETTE, "New"));
			string << B_UTF8_ELLIPSIS;
			fSaveSwatchesAsMI = new BMenuItem(string.String(),
											  new BMessage(MSG_SAVE_SWATCHES));
			fSaveSwatchesM->AddItem(fSaveSwatchesAsMI);
			bool separator = false;
			// traverse files in swatches folder
			entry_ref ref;
			while (dir.GetNextRef(&ref) >= B_OK) {
				BFile file(&ref, B_READ_ONLY);
				BMessage* loadMessage = new BMessage();
				ssize_t size = sizeof(rgb_color);
				if (loadMessage->Unflatten(&file) >= B_OK
					&& loadMessage->HasData("color swatch", B_RGB_COLOR_TYPE, size)) {
					loadMessage->what = MSG_SET_SWATCHES;
					// load
					BMenuItem* loadItem = new BMenuItem(ref.name, loadMessage);
					loadItem->SetTarget(this);
					fLoadSwatchesM->AddItem(loadItem);
					// save
					if (!separator) {
						fSaveSwatchesM->AddSeparatorItem();
						separator = true;
					}
					BMessage* saveMessage = new BMessage(MSG_SAVE_SWATCHES);
					saveMessage->AddRef("ref", &ref);
					BMenuItem* saveItem = new BMenuItem(ref.name, saveMessage);
					saveItem->SetTarget(this);
					fSaveSwatchesM->AddItem(saveItem);
					// delete
					BMessage* deleteMessage = new BMessage(MSG_DELETE_SWATCHES);
					deleteMessage->AddRef("ref", &ref);
					BMenuItem* deleteItem = new BMenuItem(ref.name, deleteMessage);
					deleteItem->SetTarget(this);
					fDeleteSwatchesM->AddItem(deleteItem);
				} else
					delete loadMessage;
			}
		}
	}
}

// _LoadSwatches
void
MainWindow::_LoadSwatches(const BMessage* message)
{
	// restore swatches
	if (message) {
		const void* colorPointer;
		ssize_t size = sizeof(rgb_color);
		for (int32 i = 0; i < 20; i++) {
			if (message->FindData("color swatch", B_RGB_COLOR_TYPE, i,
								  &colorPointer, &size) == B_OK) {
				fSwatchViews[i]->SetColor(*(const rgb_color*)colorPointer);
			}
		}
	}
}

// _SaveSwatches
void
MainWindow::_SaveSwatches(BMessage* message, const entry_ref* ref)
{
	// store swatches
	ssize_t size = sizeof(rgb_color);
	rgb_color color = fSwatchViews[0]->Color();
	// replace or add first color item
	if (message->ReplaceData("color swatch", B_RGB_COLOR_TYPE,
							 0, &color, size) != B_OK) {
		message->AddData("color swatch", B_RGB_COLOR_TYPE,
						 &color, size, true, 10);
	}
	for (int32 i = 1; i < 20; i++) {
		color = fSwatchViews[i]->Color();
		if (message->ReplaceData("color swatch", B_RGB_COLOR_TYPE,
								 i, &color, size) != B_OK) {
			message->AddData("color swatch", B_RGB_COLOR_TYPE,
							 &color, size);
		}
	}
	if (ref) {
		BFile file(ref, B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
		if (message->Flatten(&file) >= B_OK) {
			// rebuild the swatches menu
			_BuildSwatchMenu();
		}
	}
}

// _SetBrushFlags
void
MainWindow::_SetBrushFlags(uint32 flags)
{
	fCanvasView->SetBrushFlags(flags);
	fRadiusDS->SetMinEnabled(flags & FLAG_PRESSURE_CONTROLS_RADIUS);
	fAlphaDS->SetMinEnabled(flags & FLAG_PRESSURE_CONTROLS_APHLA);
	fHardnessDS->SetMinEnabled(flags & FLAG_PRESSURE_CONTROLS_HARDNESS);

	bool solid = flags & FLAG_SOLID;
	bool tilt = flags & FLAG_TILT_CONTROLS_SHAPE;
	uint32 tool = fCanvasView->Tool();
	if (tool == TOOL_CLONE
		|| tool == TOOL_BRUSH
		|| tool == TOOL_ERASER) {
		fHardnessDS->SetEnabled(fCanvasView->CurrentCanvas() && !solid);
	}

	fSolidCB->SetValue(solid);
	fBrushTiltCB->SetValue(tilt);
}

// _CanvasNameChanged
void
MainWindow::_CanvasNameChanged(Canvas* canvas)
{
	if (canvas == fCanvasView->CurrentCanvas()) {
		BString helper("WonderBrush");
		if (canvas) {
			// set to last work layer
			int32 index = canvas->CurrentLayer();
			if (index < 0 || index >= canvas->CountLayers())
				index = canvas->CountLayers() - 1;
			fCanvasView->SetToLayer(index);
			// set window title
			helper << " | " << canvas->Name();
		}
		SetTitle(helper.String());
	}

	// change the label of the corresponding document item in menu
	for (int32 i = 0; BMenuItem* item = fCanvasM->ItemAt(i); i++) {
		BMessage* message = item->Message();
		void* pointer;
		if (message && message->FindPointer("canvas", &pointer) == B_OK
			&& pointer == (void*)canvas) {
			item->SetLabel(canvas->Name());
			break;
		}
	}
#if CANVAS_LISTVIEW
	fCanvasListView->UpdateCanvasName(canvas);
#else
	fCanvasTabView->Invalidate();
#endif
}

// _UpdatePermanentStatus
void
MainWindow::_UpdatePermanentStatus()
{
	const char* message = NULL;
	LanguageManager* manager = LanguageManager::Default();
	if (fCanvasView->CurrentCanvas()) {
		switch (fCanvasView->Tool()) {
			case TOOL_CROP:
				message = manager->GetString(CROP_HELP, "Define the crop rectangle, hit <Return> to apply.\n<Shift> constrains the aspect ratio.");
				break;
			case TOOL_TRANSLATE:
				message = manager->GetString(TRANSLATE_HELP, "Select Objects to be translated from the list.\n<Shift> constrains the direction or aspect ratio.");
				break;
			case TOOL_EDIT_GRADIENT:
				message = manager->GetString(EDIT_GRADIENT_HELP, "Select an Object with a \"Gradient\" mode from the list.\nEdit gradient color stops, type and transformation.");
				break;
			case TOOL_BRUSH:
			case TOOL_BLUR:
			case TOOL_PEN:
			case TOOL_PEN_ERASER:
			case TOOL_ERASER:
				message = manager->GetString(STROKE_HELP, "Use <Shift> to extend a previous stroke and draw a straight line.");
				break;
			case TOOL_CLONE:
				message = manager->GetString(CLONE_HELP, "First click defines the source location, second the offset.\nHit <Command> to reset the offset.");
				break;
			case TOOL_COLOR_PICKER:
				message = manager->GetString(NO_HELP, "No additional tricks.");
				break;
			case TOOL_BUCKET_FILL:
				message = manager->GetString(BUCKET_FILL_HELP, "\"Softness\" controls the solidness of the fill color.");
				break;
			case TOOL_TEXT:
				message = manager->GetString(TEXT_HELP, "Hit <Return> when you're done to apply the tool.");
				break;
			case TOOL_SHAPE:
				message = manager->GetString(SHAPE_HELP, "Hold down <Ctrl>, <Shift>, <Alt> or <Option> for\nadditional point editing options.\nHit <Return> when finished.");
				break;
			case TOOL_SELECT:
				message = manager->GetString(SELECT_HELP, "Define an area. Hit <Return> to copy it to the system clipboard.");
				break;
			case TOOL_ELLIPSE:
				message = manager->GetString(ELLIPSE_HELP, "Click and drag to create an ellipse.\n<Shift> constrains the aspect ratio.");
				break;
			case TOOL_ROUND_RECT:
				message = manager->GetString(ROUND_RECT_HELP, "Click and drag to create a rectangle.\n<Shift> constrains the aspect ratio.");
				break;
			case TOOL_GUIDES:
				message = manager->GetString(GUIDES_HELP, "Create, edit or remove guides.");
				break;
			case TOOL_PICK:
				message = manager->GetString(PICK_OBJECTS_HELP, "Select Objects by clicking them or dragging a box.\n<Shift> keeps the previous selection.");
				break;
			default:
				message = manager->GetString(READY, "Ready.");
		}
	} else {
		message = manager->GetString(NO_CANVAS_HELP, "Create a new canvas or load a project or image file.");
	}
	SetStatus(message, 0, true);
}

// _RememberPath
void
MainWindow::_RememberPath(const entry_ref* ref, bool save)
{
	// depending on wether an export or save path was already remembered (!= NULL),
	// the export is also remembered for save and the other way arround
	if (save) {
		if (!fLastExportPath) {
			fLastExportPath = new entry_ref;
			*fLastExportPath = *ref;
		}
		if (!fLastSavePath)
			fLastSavePath = new entry_ref;
		*fLastSavePath = *ref;
	} else {
		if (!fLastExportPath)
			fLastExportPath = new entry_ref;
		*fLastExportPath = *ref;
		if (!fLastSavePath) {
			fLastSavePath = new entry_ref;
			*fLastSavePath = *ref;
		}
	}
}

// _UpdateShapeControls
void
MainWindow::_UpdateShapeControls(bool enable)
{
	if (enable) {
		uint32 flags = fCanvasView->ShapeControlFlags();

		fShapeTransformIB->SetEnabled(flags & SHAPE_UI_FLAGS_HAS_SELECTION);
		fShapeTransformIB->SetPressed(flags & SHAPE_UI_FLAGS_IS_TRANSFORMING);
		fShapeNewPathIB->SetEnabled(flags & SHAPE_UI_FLAGS_CAN_CREATE_PATH);
		fShapeNewPathIB->SetPressed(flags & SHAPE_UI_FLAGS_IS_CREATING_PATH);
		fShapeRemovePointsIB->SetEnabled(flags & SHAPE_UI_FLAGS_HAS_SELECTION);
		fShapeReversePathIB->SetEnabled(flags & SHAPE_UI_FLAGS_CAN_REVERSE_PATH);
		fShapeClosedCB->SetEnabled(flags & SHAPE_UI_FLAGS_CAN_CLOSE_PATH);
		fShapeClosedCB->SetValue(flags & SHAPE_UI_FLAGS_PATH_IS_CLOSED);
	} else {
		fShapeTransformIB->SetEnabled(false);
		fShapeTransformIB->SetPressed(false);
		fShapeNewPathIB->SetEnabled(false);
		fShapeNewPathIB->SetPressed(false);
		fShapeRemovePointsIB->SetEnabled(false);
		fShapeReversePathIB->SetEnabled(false);
		fShapeClosedCB->SetEnabled(false);
		fShapeClosedCB->SetValue(B_CONTROL_OFF);
	}
}

// _UpdatePickControls
void
MainWindow::_UpdatePickControls(bool enable)
{
	uint32 mask = fCanvasView->PickMask();

	fPickMaskBrushIB->SetEnabled(enable);
	fPickMaskBrushIB->SetPressed(mask & PICK_MASK_BRUSH);

	fPickMaskEraserIB->SetEnabled(enable);
	fPickMaskEraserIB->SetPressed(mask & PICK_MASK_ERASER);

	fPickMaskPenIB->SetEnabled(enable);
	fPickMaskPenIB->SetPressed(mask & PICK_MASK_PEN);

	fPickMaskPenEraserIB->SetEnabled(enable);
	fPickMaskPenEraserIB->SetPressed(mask & PICK_MASK_PEN_ERASER);

	fPickMaskCloneIB->SetEnabled(enable);
	fPickMaskCloneIB->SetPressed(mask & PICK_MASK_CLONE_BRUSH);

	fPickMaskBlurIB->SetEnabled(enable);
	fPickMaskBlurIB->SetPressed(mask & PICK_MASK_BLUR_BRUSH);

	fPickMaskFillIB->SetEnabled(enable);
	fPickMaskFillIB->SetPressed(mask & PICK_MASK_FILL);

	fPickMaskTextIB->SetEnabled(enable);
	fPickMaskTextIB->SetPressed(mask & PICK_MASK_TEXT);

	fPickMaskShapeIB->SetEnabled(enable);
	fPickMaskShapeIB->SetPressed(mask & PICK_MASK_SHAPE);

	fPickMaskBitmapIB->SetEnabled(enable);
	fPickMaskBitmapIB->SetPressed(mask & PICK_MASK_BITMAP);

	fPickMaskAllB->SetEnabled(enable && mask != PICK_MASK_ALL);
	fPickMaskNoneB->SetEnabled(enable && mask != 0);

	fPickAllB->SetEnabled(enable);
	fPickNoneB->SetEnabled(enable);
}

// _wait_for_file
int32
MainWindow::_wait_for_file(void* cookie)
{
	wait_info* info = (wait_info*)cookie;

	BEntry entry(&info->ref);
	for (int32 i = 0; i < 100; i++) {
		if (entry.Exists())
			break;
		snooze(100000);
	}

	// NOTE: contrary to the BeBook, it seems that the Lock() call will
	// block until the other app releases the file, B_BUSY is not returned
	BNode node(&entry);
	for (int32 i = 0; i < 1000; i++) {
		status_t ret = node.Lock();
		if (ret >= B_OK)
			break;
		snooze(100000);
	}

	node.Unlock();
	BMessage message(B_REFS_RECEIVED);
	message.AddRef("refs", &info->ref);
	info->window->PostMessage(&message);

	delete info;

	return 0;
}


