// BrushConfigView.h

#ifndef BRUSH_CONFIG_VIEW_H
#define BRUSH_CONFIG_VIEW_H

#include <View.h>

#include <MGroup.h>

class BrushView;
class MCheckBox;

class BrushConfigView : public MGroup, public BView {
 public:
								BrushConfigView(float borderInset,
												BrushView* brushView,
												MView* group1,
												MView* group2);

	virtual						~BrushConfigView();

								// BView
	virtual void				AllAttached();
	virtual void				Draw(BRect updateRect);
	virtual	void				MessageReceived(BMessage* message);

								// MView
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect rect);

								// BrushConfigView
			void				SetTool(int32 id);

 private:
			void				_LayoutView(MView* view, BRect frame);

			float				fBorderInset;

			BrushView*			fBrushView;
			MView*				fSliderGroup;
			MView*				fCheckBoxGroup;
};

#endif // INFO_VIEW_H
