// History.cpp

#include <stdio.h>

#include "Stroke.h"

#include "History.h"

// constructor
History::History()
	: BList(1024)
{
}

// copy constructor
History::History(const History& clone)
	: BList(1024)
{
	for (int32 i = 0; Stroke* stroke = clone.ModifierAt(i); i++) {
		Stroke* clone = stroke->Clone();
		if (!AddModifier(clone))
			delete clone;
	}
}

// destructor
History::~History()
{
}

// AddModifier
bool
History::AddModifier(Stroke* stroke)
{
	if (stroke)
		return AddItem((void*)stroke);
	return false;
}

// AddModifier
bool
History::AddModifier(Stroke* stroke, int32 index)
{
	if (stroke)
		return AddItem((void*)stroke, index);
	return false;
}

// RemoveModifier
Stroke*
History::RemoveModifier(int32 index)
{
	return (Stroke*)RemoveItem(index);
}

// RemoveModifier
bool
History::RemoveModifier(Stroke* stroke)
{
	return RemoveItem((void*)stroke);
}

// ModifierAt
Stroke*
History::ModifierAt(int32 index) const
{
	return (Stroke*)ItemAt(index);
}

// IndexOf
int32
History::IndexOf(Stroke* stroke) const
{
	return BList::IndexOf((void*)stroke);
}

// CountModifiers
int32
History::CountModifiers() const
{
	return CountItems();
}

// HasModifier
bool
History::HasModifier(Stroke* modifier) const
{
	return HasItem((void*)modifier);
}

