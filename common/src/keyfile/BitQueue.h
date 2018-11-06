// BitQueue.h

#ifndef BIT_QUEUE_H
#define BIT_QUEUE_H

#include <list>

using namespace std;

class bigint;

class BitQueue {
 private:
	typedef list<unsigned char>	bytelist;

 public:
								BitQueue();
//	virtual						~BitQueue();

			void				PushBits(unsigned long bits, int count);
			void				PushBits(const bigint& bits, int count);
			void				PushByte(unsigned char byte);
			void				PushBytes(const void* bytes, int count);
			unsigned long		PopBits(int count);
			void				PopBits(bigint& bits, int count);
			unsigned char		PopByte();
			void				PopBytes(void* bytes, int count);

	inline	int					CountBits() const;

			void				PrintToStream() const;

//static bool DEBUG;

 private:
			int					fByteCount;
			unsigned char		fStartBits;
			unsigned char		fEndBits;
			int					fStartBitCount;
			int					fEndBitCount;
			bytelist			fBytes;
};


// inline functions

// CountBits
int
BitQueue::CountBits() const
{
	return fStartBitCount + fByteCount * 8 + fEndBitCount;
}

#endif	// BIT_QUEUE_H
