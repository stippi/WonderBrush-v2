#ifndef M_STATUS_BAR_H
#define M_STATUS_BAR_H

#include <layout.h>
#include <StatusBar.h>

class MStatusBar : public MView, public BStatusBar {
 public:
								MStatusBar(const char* label,
										   const char* trailingLabel);
	virtual						~MStatusBar();

	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);
};

#endif // M_STATUS_BAR_H
