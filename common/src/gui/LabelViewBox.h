// LabelViewBox

#ifndef LABEL_VIEW_BOX_H
#define LABEL_VIEW_BOX_H

#include <View.h>
#include <String.h>

#include <MGroup.h>

class LabelViewBox : public MGroup, public BView {
 public:
								LabelViewBox(uint32 borderType,
											 float innerSpacing,
											 const char* name = NULL,
											 MView* fLabelView = NULL,
											 MView* fInnerView = NULL,
											 bool refuseLayout = true,
											 float layoutWeight = 1.0);
								LabelViewBox(uint32 borderType,
											 float innerSpacing,
											 const char* name = NULL,
											 const char* label = NULL,
											 MView* fInnerView = NULL,
											 bool refuseLayout = true,
											 float layoutWeight = 1.0);
	virtual						~LabelViewBox();

								// BView
	virtual void				Draw(BRect);
	virtual void				FrameResized(float width, float height);
	virtual	void				AttachedToWindow();

								// MView
	virtual	minimax				layoutprefs();
	virtual BRect				layout(BRect rect);

								// LabelViewBox
			void				SetLabel(const char* label);

 private:
			void				_StrokeFrame(BRect frame,
											 rgb_color high, rgb_color low);
			float				_BorderSize() const;

			uint32				fBorderType;
			float				fInnerSpacing;
			float				fLabelHeight;
			float				fLayoutWeight;
			MView*				fLabelView;
			MView*				fInnerView;
			BRect				fLabelViewRect;
			bool				fRefuseLayout;
			BString				fLabelString;
};

#endif // LABEL_VIEW_BOX_H
