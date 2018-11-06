// List.h

#ifndef LIST_H
#define LIST_H

#include <List.h>

template<class T, bool delete_on_destruction = true> class List :
		protected BList
{
public:
	inline List(int32 count = 10) {}
	inline ~List() {
		if (delete_on_destruction) {
			// delete all values
			for (int32 i = 0; T value = ItemAt(i); i++)
				delete value;
		}
	}
	inline T FirstItem() const
		{ return (T)BList::FirstItem(); }
	inline T LastItem() const
		{ return (T)BList::LastItem(); }
	inline void AddItem(T value)
		{ BList::AddItem((void *)value); }
	inline void AddItem(T value, int32 index)
		{ BList::AddItem((void *)value, index); }
	inline bool HasItem(T value) const
		{ return BList::HasItem((void *)value); }
	inline int32 IndexOf(T value) const
		{ return BList::IndexOf((void *)value); }
	inline bool IsEmpty() const
		{ return BList::IsEmpty(); }
	inline T ItemAt(int32 index) const
		{ return (T)BList::ItemAt(index); }
	inline void MakeEmpty()	// doesn't delete the removed items
		{ BList::MakeEmpty(); }
	inline bool RemoveItem(T value)
		{ return BList::RemoveItem((void *)value); }
	inline T RemoveItem(int32 index)
		{ return (T)BList::RemoveItem(index); }
	inline bool RemoveItems(int32 index, int32 count)
		{ return BList::RemoveItems(index, count); }

	inline T RemoveFirstItem()
		{ return (T)BList::FirstItem(); }
};

#endif	// LIST_H
