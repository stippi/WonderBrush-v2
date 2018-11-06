// LabelPopup.h

#ifndef LABEL_POPUP_H
#define LABEL_POPUP_H

#include <MenuField.h>

#include <layout.h>
#include <MDividable.h>

class LabelPopup : public MView,
				   public MDividable,
				   public BMenuField {
 public:
								LabelPopup(const char* label,
										   BMenu* menu = NULL,
										   bool fixedSize = true,
										   bool asLabel = false);
	virtual						~LabelPopup();

								// MView
	virtual	minimax				layoutprefs();
	virtual BRect				layout(BRect rect);

								// MDividable
	virtual float				LabelWidth();

								// BMenuField
	virtual	void				AttachedToWindow();
	virtual	void				SetLabel(const char* label);

								// LabelPopup
	virtual	void				RefreshItemLabel();

 private:
			BRect				_PreferredSize(const char* label,
											   BMenu *menu,
											   bool asLabel);
			void				_LayoutMenuBar();

			bool				fIsLabel;

	static	int32				sSystemVersion;
};

#endif // LABEL_POPUP_H
