// Queue.h

#ifndef QUEUE_H
#define QUEUE_H

#include "List.h"

template<class T, bool delete_on_destruction = true> class Queue :
		public List<T, delete_on_destruction>
{
public:
	inline	Queue(int32 count = 10) : List<T, delete_on_destruction>(count) {}
	inline	~Queue() {}
	inline	void Push(T value)	{ AddItem(value); }
	inline	T Pop()				{ return RemoveItem(0L); }
	inline	T Top() const		{ return FirstItem(); }
	void	Flush()				{ while (T value = Pop())
									delete value; }
};

#endif	// QUEUE_H

