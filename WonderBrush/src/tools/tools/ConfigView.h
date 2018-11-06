// ConfigView.h

#ifndef CONFIG_VIEW_H
#define CONFIG_VIEW_H

#include <View.h>

#include <layout.h>

class CanvasView;
class MView;

class ConfigView : public BView, public MView {
 public:
								ConfigView(CanvasView* view,
										   Tool* tool);
	virtual						~ConfigView();

	// ie the interface language has changed
	virtual	void				UpdateStrings() = 0;
	// user switched tool
	virtual	void				SetActive(bool active) = 0;
	// en/disable controls
	virtual	void				SetEnabled(bool enable) = 0;

			Tool*				GetTool() const
									{ return fTool; }

 protected:
	CanvasView*					fCanvasView;
	Tool*						fTool;
};

#endif	// CONFIG_VIEW_H
