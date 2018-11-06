// LayerConfigView.h

#ifndef LAYER_CONFIG_VIEW_H
#define LAYER_CONFIG_VIEW_H

#include <MBorder.h>

class CanvasView;
class Layer;
class PopupSlider;
class MCheckBox;

enum {
	MSG_SET_VISIBLE			= 'stvs',
	MSG_START_ALPHA_CHANGE	= 'stac',
	MSG_STOP_ALPHA_CHANGE	= 'spac',
};

class LayerConfigView : public MBorder {
 public:
								LayerConfigView(CanvasView* canvasView);

	virtual						~LayerConfigView();

								// BView
	virtual void				AttachtedToWindow();
	virtual	void				MessageReceived(BMessage* message);

								// LayerConfigView
			void				SetLayer(Layer* layer);

			void				UpdateStrings();

 private:
			Layer*				fLayer;
			CanvasView*			fCanvasView;

//			MCheckBox*			fVisibleCB;
			PopupSlider*		fAlphaPS;

			bool				fUpdate;
			float				fOldAlpha;
};

#endif // LAYER_CONFIG_VIEW_H
