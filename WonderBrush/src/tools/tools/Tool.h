// Tool.h

#ifndef TOOL_H
#define TOOL_H

#include <Handler.h>

class MView;

class Tool {
 public:
								Tool(const char* name = NULL);
	virtual						~Tool();

	// save state
	virtual	status_t			SaveSettings(BMessage* message);
	virtual	status_t			LoadSettings(BMessage* message);

	// GUI
	virtual	ConfigView*			MakeConfigView(CanvasView* view);
	virtual	IconButton*			Icon();

	virtual	const char*			ShortHelpMessage();

	virtual CanvasViewState*	State();

	// set GUI to parameters of passed modifier and control it
	virtual	status_t			SetModifier(Modifier* modifier);

	// some tools might need this
	virtual	status_t			Confirm();
	virtual	status_t			Cancel();

 protected:
};

#endif	// TOOL_H
