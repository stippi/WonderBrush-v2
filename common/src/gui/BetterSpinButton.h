// BetterSpinButton.h

#ifndef BETTER_SPIN_BUTTON_H
#define BETTER_SPIN_BUTTON_H

#include <SpinButton.h>

class BetterSpinButton : public SpinButton {
 public:
								BetterSpinButton(const char* label,
												 spinmode mode,
												 BHandler* target = NULL);
	virtual						~BetterSpinButton();

	virtual	void				SetLabel(const char* label);
	virtual	void				SetEnabled(bool enable);
	virtual BTextView*			TextView();
};

#endif // BETTER_SPIN_BUTTON_H
