// TextColumnListItem.h

#ifndef TEXT_COLUMN_LIST_ITEM_H
#define TEXT_COLUMN_LIST_ITEM_H

#include "ColumnListItem.h"

class TextColumnListItem : public ColumnListItem {
 public:
								TextColumnListItem(float height);
	virtual						~TextColumnListItem();

	virtual	void				Draw(BView* view, Column* column, BRect frame,
									 BRect updateRect, uint32 flags,
									 const column_list_item_colors* colors);

			void				SetText(const char* text, int32 index);
			const char*			TextAt(int32 index) const;

 private:
			BString*			fTexts;
			int32				fColumnCount;
};


#endif	// TEXT_COLUMN_LIST_ITEM_H
