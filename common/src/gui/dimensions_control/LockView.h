// LockView.h

#ifndef LOCK_VIEW_H
#define LOCK_VIEW_H

#include <View.h>

#include <layout.h>

class BBitmap;

class LockView : public MView, public BView {
 public:
								LockView();
	virtual						~LockView();

								// BView
	virtual	void				Draw(BRect update);
	virtual	void				MouseDown(BPoint where);

								// MView
			minimax				layoutprefs();
			BRect				layout(BRect rect);

								// LockView
	virtual	void				SetEnabled(bool enabled);
	virtual	bool				IsEnabled();
	virtual	void				SetLocked(bool locked);
	virtual	bool				IsLocked();

 private:
	bool						fEnabled;
	bool						fLocked;
	BBitmap*					fOpenEnabledBitmap;
	BBitmap*					fOpenDisabledBitmap;
	BBitmap*					fClosedEnabledBitmap;
	BBitmap*					fClosedDisabledBitmap;
};

#endif // LOCK_VIEW_H
