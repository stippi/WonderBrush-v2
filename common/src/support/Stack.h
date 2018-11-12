// Stack.h

#ifndef STACK_H
#define STACK_H

#include "List.h"

template<class T, bool delete_on_destruction = true> class Stack :
		public List<T, delete_on_destruction>
{
public:
	inline	Stack(int32 count = 10) : List<T, delete_on_destruction>(count) {}
	inline	~Stack() {}
	inline	void Push(T value)	{ BList::AddItem(value); }
	inline	T Pop()				{ return (T)BList::RemoveItem(BList::CountItems() - 1); }
	inline	T Top() const		{ return (T)BList::LastItem(); }
};

#endif	// STACK_H

