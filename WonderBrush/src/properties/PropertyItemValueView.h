// PropertyItemValueView.h

#ifndef PROPERTY_ITEM_VALUE_VIEW_H
#define PROPERTY_ITEM_VALUE_VIEW_H

#include <View.h>

class Property;
class PropertyItemView;

class PropertyItemValueView : public BView {
 public:
								PropertyItemValueView(Property* property);
	virtual						~PropertyItemValueView();

								// BView
	virtual	void				Draw(BRect updateRect);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
										   const BMessage* dragMessage);

								// PropertyItemValueView
	virtual	float				PreferredHeight() const;

			const Property*		GetProperty() const
									{ return fProperty; }

			void				SetSelected(bool selected);
			bool				IsSelected() const
									{ return fSelected; }
	virtual	bool				IsFocused() const
									{ return IsFocus(); }

			void				SetItemView(PropertyItemView* parent);

								// used to trigger an update on the
								// represented object
	virtual	void				ValueChanged();

	virtual	bool				SetToProperty(const Property* property) = 0;

 protected:
	PropertyItemView*			fParent;

 private:
	Property*					fProperty;

	bool						fSelected;
};

#endif // PROPERTY_ITEM_VALUE_VIEW_H


