// WonderBrushApp.h

#ifndef ART_FLOW_APP_H
#define ART_FLOW_APP_H

#include "MApplication.h"

class BFilePanel;
class MainWindow;

class WonderBrushApp : public MApplication {
public:
								WonderBrushApp();
	virtual						~WonderBrushApp();

	virtual	void				AboutRequested();
	virtual	bool				QuitRequested();
	virtual	void				ReadyToRun();
	virtual	void				RefsReceived(BMessage* message);
	virtual	void				ArgvReceived(int32 argc,
											 char** argv);
	virtual	void				MessageReceived(BMessage* message);

private:
			void				_InstallMimeTypes();

	MainWindow*					fMainWindow;
	BFilePanel*					fOpenPanel;

	bool						fRefsMessageSent;
	BMessage*					fInitialRefsMessage;
	bool						fOpenPathAdjusted;
};

#endif // ART_FLOW_APP_H
