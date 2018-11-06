// BrushTool.h

#ifndef BRUSH_TOOL_H
#define BRUSH_TOOL_H

#include "Tool.h"

class DualSlider;
class MCheckBox;

class BrushTool : public Tool {
 public:
								BrushTool();
	virtual						~BrushTool();

	virtual	status_t			SaveSettings(BMessage* message);
	virtual	status_t			LoadSettings(BMessage* message);

	virtual	MView*				ConfigView();

	virtual CanvasViewState*	State();

	virtual	status_t			SetModifier(Modifier* modifier);

	virtual	status_t			Confirm();
	virtual	status_t			Cancel();


 protected:
			IconButton*			fIconButton;

			DualSlider*			fRadiusDS;
			DualSlider*			fAlphaDS;
			DualSlider*			fHardnessDS;
			DualSlider*			fSpacingDS;
			MCheckBox*			fBrushSubPixelCB;
			MCheckBox*			fSolidCB;

};

#endif	// BRUSH_TOOL_H
