// Seperator.h

#ifndef SEPERATOR_H
#define SEPERATOR_H

#include <String.h>
#include <View.h>

#include <layout.h>

class Seperator : public MView, public BView {
 public:
							Seperator(const char *label = NULL,
									  orientation direction = B_HORIZONTAL);
							Seperator(MView *label,
									  orientation direction = B_HORIZONTAL);
							Seperator(orientation direction = B_HORIZONTAL);
	virtual					~Seperator();

							// BView
	virtual	void			Draw(BRect update);

							// MView
			minimax			layoutprefs();
			BRect			layout(BRect rect);

							// Seperator
			void			SetLabel(const char* label);

 private:
	MView*					fLabelView;
	BString					fLabel;
	orientation				fOrient;
	float					fLabelWidth;
	float					fLabelHeight;
	float					fAscent;
};

#endif // SEPERATOR_H
