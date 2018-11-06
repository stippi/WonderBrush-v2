// PropertyItemView.h

#ifndef PROPERTY_ITEM_VIEW_H
#define PROPERTY_ITEM_VIEW_H

#include <View.h>

class Property;
class PropertyItemValueView;
class PropertyListView;

class PropertyItemView : public BView {
 public:
								PropertyItemView(Property* property);
	virtual						~PropertyItemView();

								// BView
	virtual	void				Draw(BRect updateRect);
	virtual	void				FrameResized(float width, float height);
	virtual	void				MakeFocus(bool focused);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseUp(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 transit,
										   const BMessage* dragMessage);

								// PropertyItemView
			float				PreferredHeight() const;
			float				PreferredLabelWidth() const;
			void				SetLabelWidth(float width);

			const Property*		GetProperty() const;
			bool				SetProperty(const Property* property);

			void				SetSelected(bool selected);
			bool				IsSelected() const
									{ return fSelected; }
	virtual	bool				IsFocused() const;

			void				SetListView(PropertyListView* parent);

	virtual	void				UpdateObject();

 private:
	PropertyListView*			fParent;

	PropertyItemValueView*		fValueView;

	bool						fSelected;

	float						fLabelWidth;
};

#endif // PROPERTY_ITEM_VIEW_H


