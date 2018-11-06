// DualSliderInputGroup.h

#ifndef DUAL_SLIDER_INPUT_GROUP_H
#define DUAL_SLIDER_INPUT_GROUP_H

#include <View.h>

#include <MGroup.h>

class DualSlider;
class MTextControl;

class DualSliderInputGroup : public MGroup, public BView {
 public:
								DualSliderInputGroup(DualSlider* slider);
	virtual						~DualSliderInputGroup();

								// MGroup
	virtual	minimax				layoutprefs();
	virtual	BRect				layout(BRect frame);

								// BView
	virtual void				AllAttachted();
	virtual	void				MessageReceived(BMessage* message);

								// DualSliderInputGroup
			DualSlider*			Slider() const
									{ return fSlider; }

			MTextControl*		MinTextControl() const
									{ return fMinTC; }
			MTextControl*		MaxTextControl() const
									{ return fMaxTC; }

 private:
			DualSlider*			fSlider;
			MTextControl*		fMinTC;
			MTextControl*		fMaxTC;
};

#endif // DUAL_SLIDER_INPUT_GROUP_H
