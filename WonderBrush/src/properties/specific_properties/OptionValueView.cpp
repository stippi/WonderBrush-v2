// OptionValueView.cpp

#include <stdio.h>

#include <Font.h>
#include <MenuItem.h>
#include <Message.h>
#include <PopUpMenu.h>
#include <Region.h>

#include "OptionProperty.h"

#include "OptionValueView.h"

enum {
	MSG_OPTION_CHANGED = 'opch',
};

// constructor
OptionValueView::OptionValueView(OptionProperty* property)
	: PropertyItemValueView(property),
	  fProperty(property),
	  fCurrentOption("")
{
	if (fProperty)
		fProperty->GetCurrentOption(&fCurrentOption);
}

// destructor
OptionValueView::~OptionValueView()
{
}

// Draw
void
OptionValueView::Draw(BRect updateRect)
{
	BRect b(Bounds());
	// focus indication
	if (IsFocus()) {
		SetHighColor(ui_color(B_KEYBOARD_NAVIGATION_COLOR));
		StrokeRect(b);
		b.InsetBy(1.0, 1.0);
		BRegion clipping;
		clipping.Include(b);
		ConstrainClippingRegion(&clipping);
		b.left --;
	}
	// background
	FillRect(b, B_SOLID_LOW);

	SetHighColor(0, 0, 0, 255);

	b.InsetBy(2.0, 1.0);

	float center = floorf(b.top + b.Height() / 2.0);

	BPoint arrow[3];
	arrow[0] = BPoint(b.left, center - 3.0);
	arrow[1] = BPoint(b.left, center + 3.0);
	arrow[2] = BPoint(b.left + 3.0, center);

	FillPolygon(arrow, 3);

	b.left += 6.0;

	BFont font;
	GetFont(&font);

	font_height fh;
	font.GetHeight(&fh);

	BString truncated(fCurrentOption);
	font.TruncateString(&truncated, B_TRUNCATE_END, b.Width());

	DrawString(fCurrentOption.String(), BPoint(b.left, floorf(center + fh.ascent / 2.0)));
}

// FrameResized
void
OptionValueView::FrameResized(float width, float height)
{
/*	float radius = ceilf((height - 6.0) / 2.0);
	float centerX = floorf(Bounds().left + width / 2.0);
	float centerY = floorf(Bounds().top + height / 2.0);
	fCheckBoxRect.Set(centerX - radius, centerY - radius,
					  centerX + radius, centerY + radius);*/
}

// MakeFocus
void
OptionValueView::MakeFocus(bool focused)
{
	PropertyItemValueView::MakeFocus(focused);
	Invalidate();
}

// MessageReceived
void
OptionValueView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case MSG_OPTION_CHANGED:
			if (fProperty) {
				int32 id;
				if (message->FindInt32("id", &id) >= B_OK) {
					fProperty->SetCurrentOptionID(id);
					ValueChanged();
				}
			}
			break;
		default:
			PropertyItemValueView::MessageReceived(message);
			break;
	}
}

// MouseDown
void
OptionValueView::MouseDown(BPoint where)
{
	if (BView* parent = Parent())
		parent->MouseDown(ConvertToParent(where));
	
	if (fProperty) {
		BPopUpMenu* menu = new BPopUpMenu("option popup", false, false);
		BString name;
		int32 id;
		for (int32 i = 0; fProperty->GetOption(i, &name, &id); i++) {
			BMessage* message = new BMessage(MSG_OPTION_CHANGED);
			message->AddInt32("id", id);
			BMenuItem* item = new BMenuItem(name.String(), message);
			menu->AddItem(item);
			if (id == fProperty->CurrentOptionID())
				item->SetMarked(true);
		}
		menu->SetTargetForItems(this);
		menu->SetAsyncAutoDestruct(true);
		menu->SetFont(be_plain_font);

		where = ConvertToScreen(where);
		BRect mouseRect(where, where);
		mouseRect.InsetBy(-10.0, -10.0);
		where += BPoint(5.0, 5.0);
		menu->Go(where, true, false, mouseRect, true);
	}
}

// KeyDown
void
OptionValueView::KeyDown(const char* bytes, int32 numBytes)
{
	bool handled = true;
	if (numBytes > 0) {
		switch (bytes[0]) {
			case B_LEFT_ARROW:
			case B_UP_ARROW:
				fProperty->SetOptionAtOffset(-1);
				ValueChanged();
				break;

			case B_RIGHT_ARROW:
			case B_DOWN_ARROW:
				fProperty->SetOptionAtOffset(1);
				ValueChanged();
				break;
			default:
				handled = false;
				break;
		}
	}
	if (!handled)
		PropertyItemValueView::KeyDown(bytes, numBytes);
}

// ValueChanged
void
OptionValueView::ValueChanged()
{
	if (fProperty) {
		fProperty->GetCurrentOption(&fCurrentOption);
		BRect b(Bounds());
		b.InsetBy(1.0, 1.0);
		b.left += 5.0;
		Invalidate(b);
	}
	PropertyItemValueView::ValueChanged();	
}

// SetToProperty
bool
OptionValueView::SetToProperty(const Property* property)
{
	const OptionProperty* p = dynamic_cast<const OptionProperty*>(property);
	if (p) {
		int32 optionID = p->CurrentOptionID();
		if (optionID != fProperty->CurrentOptionID()) {
			fProperty->SetCurrentOptionID(optionID);
			fProperty->GetCurrentOption(&fCurrentOption);
			BRect b(Bounds());
			b.InsetBy(1.0, 1.0);
			b.left += 5.0;
			Invalidate(b);
		}
		return true;
	}
	return false;
}

