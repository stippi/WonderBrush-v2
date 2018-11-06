// FilterObject.h

#ifndef FILTER_OBJECT_H
#define FILTER_OBJECT_H

#include "Stroke.h"

class FilterObject : public Stroke {
 public:
								FilterObject(int32 id);
								FilterObject(const FilterObject& other);
								FilterObject(BMessage* archive);
	virtual						~FilterObject();

	virtual	bool				SetTo(const Stroke* from);

								// BArchivable protocoll
	virtual	status_t			Archive(BMessage* into, bool deep = true) const;

	virtual	bool				DrawLastLine(BBitmap* bitmap,
											 BRect& updateRect);
	virtual	void				Draw(BBitmap* bitmap);
	virtual	void				Draw(BBitmap* bitmap, BRect constrainRect);

	virtual	void				Finish(BBitmap* bitmap);
	virtual	void				Reset();

	virtual	bool				IsTransformable() const
									{ return false; }

	virtual	void				ProcessBitmap(BBitmap* bitmap,
											  BBitmap* alphaMap,
											  BRect constrainRect) const = 0;

	virtual	void				MergeWithBitmap(BBitmap* dest,
												BBitmap* strokeBitmap,
												BRect area,
												uint32 colorSpace) const;
	virtual	void				MergeWithBitmap(BBitmap* from,
												BBitmap* dest,
												BBitmap* strokeBitmap,
												BRect area,
												uint32 colorSpace) const;
	virtual	const char*			Name() const;

	virtual	int32				ToolID() const;

	virtual	PropertyObject*		MakePropertyObject() const
									{ return NULL; }
	virtual	bool				SetToPropertyObject(PropertyObject* object)
									{ return false; }

			void				SetFilterID(int32 id)
									{ fFilterID = id; }
	virtual	int32				FilterID() const
									{ return fFilterID; }

			void				SaveSettings() const;

 private:
			int32				fFilterID;
};

#endif	// FILTER_OBJECT_H
