// BitQueue.cpp

#include <cstdio>

#include "BitQueue.h"
#include "bigint.h"

// helper functions

static inline
unsigned long
bit_filter(int count)
{
	return (~0UL >> (sizeof(unsigned long) * 8 - count));
}

static inline
void
print_bit(unsigned long bits, int bit)
{
	printf("%ld", (bits >> bit) & 1);
}

static inline
void
print_bits(unsigned long bits, int first, int count)
{
	for (int i = 0; i < count; i++)
		print_bit(bits, first - i);
}

static inline
void
print_lower_bits(unsigned long bits, int count)
{
	print_bits(bits, count - 1, count);
}


//bool BitQueue::DEBUG = false;


// constructor
BitQueue::BitQueue()
	: fByteCount(0),
	  fStartBits(0),
	  fEndBits(0),
	  fStartBitCount(0),
	  fEndBitCount(0),
	  fBytes()
{
}

// PushBits
void
BitQueue::PushBits(unsigned long bits, int count)
{
//printf("BitQueue::PushBits(%lx, %d)\n", bits, count);
//printf("BitQueue::PushBits(%d): bytes: %d, bits: %d\n", count, fByteCount, CountBits());
//printf("[%08lx", bits);
	enum { MAX_BITS =  sizeof(unsigned long) * 8 };
	if (count < 0)
		count = 0;
	if (count > MAX_BITS)
		count = MAX_BITS;
	while (count > 0) {
		bits &= bit_filter(count);
		int bitsToPush = min(count, MAX_BITS - fEndBitCount);
//printf("count: %d\n", count);
//printf("bitsToPush: %lu\n", bitsToPush);
//printf("fEndBitCount: %d\n", fEndBitCount);
//printf("(%d)|%08lx:%08lx:%08lx", bitsToPush, fEndBits, fEndBits << bitsToPush,
//bits >> (count - bitsToPush));
		unsigned long endBits = bits >> (count - bitsToPush);
		if (bitsToPush < MAX_BITS)
			endBits |= (fEndBits << bitsToPush);
		fEndBitCount += bitsToPush;
//printf("|%08lx|", endBits);
		while (fEndBitCount >= 8) {
//printf("%02lx", (unsigned char)(endBits >> (fEndBitCount - 8)));
			fBytes.push_back((unsigned char)(endBits >> (fEndBitCount - 8)));
			fByteCount++;
			fEndBitCount -= 8;
			endBits &= bit_filter(fEndBitCount);
//printf("(%08lx):", endBits);
		}
		fEndBits = (unsigned char)endBits;
		count -= bitsToPush;
	}
//printf("]\n");
//printf("BitQueue::PushBits() done\n");
//printf("BitQueue::PushBits() done: bytes: %d, bits: %d\n", fByteCount, CountBits());
}

// PushBits
void
BitQueue::PushBits(const bigint& bits, int count)
{
//printf("BitQueue::PushBits()\n");
	const int MAX_BITS =  sizeof(unsigned long) * 8;
	while (count > 0) {
		int bitsToPush = min(count, MAX_BITS);
//cout << bits << endl << " -> ";
//cout << " >> " << count - bitsToPush << ": " << (bits >> (count - bitsToPush)) << endl << " -> ";
//cout << ((bits >> (count - bitsToPush)) & bit_filter(bitsToPush)) << endl;
		unsigned long b =
			((bits >> (count - bitsToPush)) & bit_filter(bitsToPush)).toul();
		PushBits(b, bitsToPush);
//printf("[%lx]", b);
//PrintToStream();
//printf("\n");
		count -= bitsToPush;
	}
//printf("\n");
//printf("BitQueue::PushBits() done\n");
}

// PushByte
void
BitQueue::PushByte(unsigned char byte)
{
	PushBits(byte, 8);
}

// PushBytes
void
BitQueue::PushBytes(const void* bytes, int count)
{
	const unsigned char* inBytes = (const unsigned char*)bytes;
	for (int i = 0; i < count; i++, inBytes++)
		PushByte(*inBytes);
}

// PopBits
unsigned long
BitQueue::PopBits(int count)
{
//if (DEBUG) {
//printf("BitQueue::PopBits(%d)\n", count);
//printf("BitQueue::PopBits(%d): bytes: %d, bits: %d\n", count, fByteCount, CountBits());
//printf("fByteCount: %d\n", fByteCount);
//PrintToStream();
//}
	unsigned long result = 0;
	enum { MAX_BITS =  sizeof(unsigned long) * 8 };
	if (count < 0)
		count = 0;
	if (count > MAX_BITS)
		count = MAX_BITS;
	int bitsLeft = 0;
	if (count > CountBits()) {
		bitsLeft = count - CountBits();
		count -= bitsLeft;
	}
	while (count > 0) {
//if (DEBUG) {
//printf("count: %d\n", count);
//printf("fStartBitCount: %d\n", fStartBitCount);
//}
		// move bits to the front of the queue
		if (fStartBitCount == 0) {
			if (fByteCount > 0) {
				// move a byte from the byte queue to the start bits
//if (DEBUG) {
//printf("check: %d\n", fBytes.size());
//}
				fStartBits = fBytes.front();
//printf("check\n");
				fBytes.pop_front();
//printf("check\n");
				fByteCount--;
//printf("check\n");
				fStartBitCount = 8;
//printf("check\n");
			} else {
				// move the end bits to the start bits
				fStartBits = fEndBits;
				fStartBitCount = fEndBitCount;
				fEndBits = 0;
				fEndBitCount = 0;
			}
		}
		int bitsToPop = min(count, fStartBitCount);
//if (DEBUG) {
//printf("bitsToPop: %d\n", bitsToPop);
//}
		if (bitsToPop < MAX_BITS)
			result <<= bitsToPop;
		else
			result = 0;
		result |= fStartBits >> (fStartBitCount - bitsToPop);
		fStartBits &= bit_filter(fStartBitCount - bitsToPop);
		fStartBitCount -= bitsToPop;
		count -= bitsToPop;
//if (DEBUG) {
//printf("result: %lx\n", result);
//}
	}
	if (bitsLeft > 0)
		result <<= bitsLeft;
//if (DEBUG) {
//printf("BitQueue::PopBits() done: result: %lx, bytes: %d, bits: %d\n", result, fByteCount, CountBits());
//}
	return result;
}

// PopBits
void
BitQueue::PopBits(bigint& bits, int count)
{
//printf("BitQueue::PopBits(, %d)\n", count);
	const int MAX_BITS =  sizeof(unsigned long) * 8;
	bits = 0;
	while (count > 0) {
		int bitsToPop = min(count, MAX_BITS);
//printf("bitsToPop: %d\n", bitsToPop);
		unsigned long value = PopBits(bitsToPop);
//printf("bits: "); print_lower_bits(value, bitsToPop); printf("\n");
		bits <<= bitsToPop;
		bits |= value;
		count -= bitsToPop;
	}
//printf("BitQueue::PopBits() done\n");
}

// PopByte
unsigned char
BitQueue::PopByte()
{
	return (unsigned char)PopBits(8);
}

// PopBytes
void
BitQueue::PopBytes(void* bytes, int count)
{
	unsigned char* outBytes = (unsigned char*)bytes;
	for (int i = 0; i < count; i++, outBytes++)
		*outBytes = PopByte();
}

// PrintToStream
void
BitQueue::PrintToStream() const
{
	bool space = false;
	// print the start bits
	if (fStartBitCount > 0) {
		print_lower_bits(fStartBits, fStartBitCount);
		space = true;
	}
	// print the bytes
	if (fByteCount > 0) {
		for (bytelist::const_iterator it = fBytes.begin();
			 it != fBytes.end();
			 it++) {
			if (space)
				printf(" ");
			print_lower_bits(*it, 8);
			space = true;
		}
	}
	// print the end bits
	if (fEndBitCount > 0) {
		if (space)
			printf(" ");
		print_lower_bits(fEndBits, fEndBitCount);
	}
}


