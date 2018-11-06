// BetterMCheckBox.h

#ifndef BETTER_M_CHECK_BOX_H
#define BETTER_M_CHECK_BOX_H

#include <MCheckBox.h>

class BetterMCheckBox : public MCheckBox {
 public:
								BetterMCheckBox(const char* label,
												ulong id = 0,
												bool state = false);
								BetterMCheckBox(const char* label,
												BMessage* message,
												BHandler* handler = NULL,
												bool state = false);
								BetterMCheckBox(BMessage* archive);

	virtual	minimax				layoutprefs();
};

#endif // BETTER_M_CHECK_BOX_H
