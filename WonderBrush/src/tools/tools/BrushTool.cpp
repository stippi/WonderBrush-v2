// BrushTool.cpp

#include <stdio.h>

#include "BubbleHelper.h"
#include "LanguageManager.h"

#include "BrushTool.h"

// constructor
BrushTool::BrushTool()
	: Tool("brush tool")
{
	// config controls
	fBrushView = new BrushView("brush preview", new BMessage(MSG_BRUSH_PANEL), this);

	fAlphaDS = new DualSlider("max alpha", "Opacity",
							  new BMessage(MSG_SET_ALPHA),
							  new BMessage(MSG_CONTROL_ALPHA),
							  fCanvasView, 0.0, 1.0);
	fRadiusDS = new DualSlider("max radius", "Radius",
							   new BMessage(MSG_SET_RADIUS),
							   new BMessage(MSG_CONTROL_RADIUS),
							   fCanvasView, 0.0, 0.1);
	fHardnessDS = new DualSlider("hardness", "Hardness",
								 new BMessage(MSG_SET_HARDNESS),
								 new BMessage(MSG_CONTROL_HARDNESS),
								 fCanvasView, 1.0, 1.0);
	fSpacingDS = new DualSlider("spacing", "Spacing",
								new BMessage(MSG_SET_SPACING),
								NULL,//new BMessage(MSG_CONTROL_SPACING),
								fCanvasView, 0.1, 0.1);
fSpacingDS->SetMinEnabled(false);


	BMessage* message = new BMessage(MSG_SUB_PIXEL_PRECISION);
	message->AddInt32("tool", TOOL_BRUSH);
	fBrushSubPixelCB = new MCheckBox("Subpixels", message, fCanvasView, true);
	fSolidCB = new MCheckBox("Solid", new BMessage(MSG_SET_SOLID), fCanvasView, false);

	// icon button
	msg = new BMessage(MSG_SET_TOOL);
	msg->AddInt32("tool", TOOL_BRUSH);
	fBrushIB = new IconButton("brush icon", TOOL_BRUSH, NULL, msg, fCanvasView);
	fBrushIB->SetIcon(kBrushIconBits, kIconWidth, kIconHeight, kIconColorSpace);
	fBrushIB->SetPressed(true);

}

// destructor
BrushTool::~BrushTool()
{
}

// SaveSettings
status_t
BrushTool::SaveSettings(BMessage* message)
{
	status_t ret = Tool::SaveSettings(message);
	if (ret >= B_OK) {

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

	subPixels = fCanvasView->Precise(TOOL_BRUSH);
	if (fSettings->ReplaceBool("stroke subpixels", subPixels) != B_OK)
		fSettings->AddBool("stroke subpixels", subPixels);
	}
	return ret;
}

// LoadSettings
status_t
BrushTool::LoadSettings(BMessage* message)
{
	status_t ret = Tool::LoadSettings(message);
	if (ret >= B_OK) {


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


	if (fSettings->FindBool("stroke subpixels", &subPixels) >= B_OK) {
		fCanvasView->SetPrecise(subPixels, TOOL_BRUSH);
		fBrushSubPixelCB->SetValue(subPixels);
	}

	}
	return ret;
}

// ConfigView
MView*
BrushTool::ConfigView()
{
	return	new BrushConfigView
		(
			4.0,
			fBrushView,
			new HGroup
			(
				fAlphaDS,
				fRadiusDS,
				fHardnessDS,
				fSpacingDS,
				0
			),
			new VGroup
			(
				fBrushSubPixelCB,
				fSolidCB,
				0
			)
		);
}

// Icon
IconButton*
BrushTool::Icon()
{
	return fIconButton;
}

// UpdateStrings
void
BrushTool::UpdateStrings()
{
	BubbleHelper* helper = BubbleHelper::Default();
	LanguageManager* manager = LanguageManager::Default();

	// labels
	fRadiusDS->SetLabel(manager->GetString(RADIUS, "Radius"));
	fAlphaDS->SetLabel(manager->GetString(OPACITY, "Opacity"));
	fHardnessDS->SetLabel(manager->GetString(HARDNESS, "Hardness"));
	fSpacingDS->SetLabel(manager->GetString(SPACING, "Spacing"));
	fBrushSubPixelCB->SetLabel(manager->GetString(SUBPIXELS, "Subpixels"));
	fSolidCB->SetLabel(manager->GetString(SOLID, "Solid"));

	// tool tips
	BString string;
	const char* toolString = manager->GetString(TOOL, "Tool");
	string.SetTo(manager->GetString(BRUSH, "Brush"));
	string << " " << toolString << "   (B)";
	helper->SetHelp(fBrushIB, string.String());

	const char* subPixelTip = manager->GetString(SUB_PIXEL_TIP,
												 "Track mouse/pen position with subpixel precision.");
	helper->SetHelp(fBrushSubPixelCB, subPixelTip);

	helper->SetHelp(fSolidCB, manager->GetString(SOLID_TIP,
												 "Don't vary opacity per pixel."));

	const char* penPressureControl = manager->GetString(PRESSURE_CONTROL_TIP,
														"Enables control by pen pressure.");
	fRadiusDS->SetPressureControlTip(penPressureControl);
	fAlphaDS->SetPressureControlTip(penPressureControl);
	fHardnessDS->SetPressureControlTip(penPressureControl);
	fSpacingDS->SetPressureControlTip(penPressureControl);
}

// SetActive
void
BrushTool::SetActive(bool active)
{
	fIconButton->SetPressed(active);
}

// SetEnabled
void
BrushTool::SetEnabled(bool enable)
{
	fAlphaDS->SetEnabled(enable);
	fRadiusDS->SetEnabled(enable);
	fHardnessDS->SetEnabled(enable);
	fSpacingDS->SetEnabled(enable);
	fBrushView->SetEnabled(enable);
}








// State
CanvasViewState*
BrushTool::State()
{
	return NULL;
}

// SetModifier
status_t
BrushTool::SetModifier(Modifier* modifier)
{
	return B_OK;
}

// Confirm
status_t
BrushTool::Confirm()
{
	return B_OK;
}

// Cancel
status_t
BrushTool::Cancel()
{
	return B_OK;
}


