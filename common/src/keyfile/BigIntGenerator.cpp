// BigIntGenerator.cpp

#include <algorithm>
#include <cstdio>
#include <new>

#include "BigIntGenerator.h"

using namespace std;

#include <OS.h>


// constants

const int kFirstPrimes[] = { 2, 3, 5, 7, 11, 0 };


// helper functions

// bit_filter
static inline
unsigned long
bit_filter(int count)
{
	return (~0UL >> (sizeof(unsigned long) * 8 - count));
}


// constructor
BigIntGenerator::BigIntGenerator()
	: fSieveSize(0),
	  fCandidates(0),
	  fCandidateCount(0)
{
	int firstPrimesCount = 0;
	fSieveSize = 1;
	for (; kFirstPrimes[firstPrimesCount]; firstPrimesCount++)
		fSieveSize *= kFirstPrimes[firstPrimesCount];
	// init the sieve
	bool* sieve = new bool[fSieveSize];
	for (int n = 0; n < fSieveSize; n++)
		sieve[n] = true;
	for (int n = 0; n < fSieveSize; n++) {
		for (int i = 0; i < firstPrimesCount; i++)
			sieve[n] &= (bool)(n % kFirstPrimes[i]);
	}
	// count the remaining numbers
	for (int n = 0; n < fSieveSize; n++) {
		if (sieve[n])
			fCandidateCount++;
	}
	// create the candidate array
	fCandidates = new int[fCandidateCount];
	int index = 0;
	for (int n = 0; n < fSieveSize; n++) {
		if (sieve[n])
			fCandidates[index++] = n;
	}
	delete[] sieve;
//cout << "sieve size     : " << fSieveSize << endl;
//cout << "candidate count: " << fCandidateCount << endl;
	// init the random number generator
	srand((unsigned)system_time());
}

// destructor
BigIntGenerator::~BigIntGenerator()
{
	delete[] fCandidates;
}

// CreateDefault
BigIntGenerator*
BigIntGenerator::CreateDefault()
{
	if (!fDefaultGenerator)
		fDefaultGenerator = new(nothrow) BigIntGenerator;
	return fDefaultGenerator;
}

// DeleteDefault
void
BigIntGenerator::DeleteDefault()
{
	delete fDefaultGenerator;
	fDefaultGenerator = NULL;
}

// GetDefault
BigIntGenerator&
BigIntGenerator::GetDefault()
{
	return *fDefaultGenerator;
}

// _GenerateRandomNumber
bigint
BigIntGenerator::_GenerateRandomNumber(unsigned length)
{
	bigint n(0);
	while (n.ld() + 1 < (int)length) {
		int bitsToPush = min(8U, n.ld() + 1 - length);
		unsigned int byte = (unsigned int)((rand() >> 2));
		n <<= bitsToPush;
		n |= byte & bit_filter(bitsToPush);
	}
	return n;
}

// _GenerateRandomNumber
bigint
BigIntGenerator::_GenerateRandomNumber(const bigint& maximum)
{
	bigint n(GenerateRandomNumber(maximum.ld() + 1));
	while (n >= maximum)
		n >>= 1;
	return n;
}

// _GeneratePrimeCandidate
bigint
BigIntGenerator::_GeneratePrimeCandidate(unsigned length)
{
	int cookie;
	return GeneratePrimeCandidate(length, cookie);
}

// _GeneratePrimeCandidate
//
// Generates a prime candidate and initializes the /cookie/ for subsequent
// calls to GenerateNextPrimeCandidate().
bigint
BigIntGenerator::_GeneratePrimeCandidate(unsigned length, int& cookie)
{
	bigint p = GenerateRandomNumber(length);
	int offset = (p % fSieveSize).toi();
	// find a the index of a near candidate (binary search)
	int lower = 0;
	int upper = fCandidateCount;
	while (lower < upper) {
		int mid = (lower + upper) / 2;
		if (fCandidates[mid] == offset)
			lower = upper = mid;
		else if (fCandidates[mid] > offset)
			upper = mid;
		else
			lower = mid + 1;
	}
	if (fCandidates[lower] != offset)
		p += fCandidates[lower] - offset;
	cookie = lower;
	return p;
}

// _GetNextPrimeCandidate
//
// p is set to a greater candidate.
// Note: /p/ and /cookie/ must be the result of a call to
// GeneratePrimeCandidate().
void
BigIntGenerator::_GetNextPrimeCandidate(bigint& p, int& cookie)
{
//cout << "BigIntGenerator::_GetNextPrimeCandidate(" << p << ", " << cookie << ")" << endl;
	if (cookie < 0 || cookie >= fCandidateCount)
		cookie = 0;
	int lastOffset = fCandidates[cookie];
	cookie = (cookie + 1) % fCandidateCount;
	int newOffset = fCandidates[cookie];
	if (newOffset < lastOffset)
		newOffset += fCandidateCount;
	p += (newOffset - lastOffset);
//cout << "BigIntGenerator::_GetNextPrimeCandidate() done: " << p << endl;
}

// _GeneratePrime
bigint
BigIntGenerator::_GeneratePrime(unsigned length)
{
//printf("generate_prime(%d)\n", length);
	printf("searching %d bit prime number: .", length);
	fflush(stdout);

	bigint p;
	do {
		int cookie;
		p = GeneratePrimeCandidate(length, cookie);
		while (!IsPrime(p)) {
			printf(".");
			fflush(stdout);
	
			GetNextPrimeCandidate(p, cookie);
		}
	} while (p.ld() + 1 != (int)length);

//printf("generate_prime() done\n");
	printf("\n");

	return p;
}

// IsPrime
bool
BigIntGenerator::IsPrime(const bigint& n)
{
//printf("is_prime()\n");
//cout << "is_prime(" << n << ")" << endl;
	enum { ITERATIONS = 50 };
	// Rabin-Miller algorithm
	// factorize n - 1 so that n - 1 = 2^q * m, m odd
	bigint m(n - 1);
	int q = 0;
	while ((m & 1) == 0) {
		m >>= 1;
		q++;
	}
//cout << n - 1 << " = 2^" << q << " * " << m << endl;
	// choose a random number b in [1, n - 1] and test if
	// (a) b^m % n - 1 = 0 or
	// (b) an i exists in [0, q - 1] so that (b^(m * 2^i) + 1) % n = 0
	// If neither (a) nor (b) holds, n is composite, otherwise the result
	// is inconclusive. At most 1/4 of the bases yield `inconclusive' for
	// a composite number (but all for a prime). Thus the probability
	// for misidentifying a composite number as a prime for ITERATIONS
	// iterations is 1/4^ITERATIONS.
	bool inconclusive = true;
	for (int it = 0; inconclusive && it < ITERATIONS; it++) {
//printf("  it: %d\n", it);
		bigint b = GenerateRandomNumber(n - 1) + 1;
		bigint bm = b.mod_pow_copy(m, n);
//cout << "bm: " << bm << endl;
//cout << "bm: " << bm.toui() << endl;
//cout << "bm: " << mod_pow(b.toui(), m.toui(), n.toui()) << endl;

		inconclusive = ((bm - 1) % n == 0);
		for (int i = 0; !inconclusive && i < q; i++) {
//printf("    i: %d\n", i);
			inconclusive = ((bm + 1) % n == 0);
			if (!inconclusive)
				bm = bm.mod_mult_copy(bm, n);
		}
	}
//printf("is_prime() done\n");
	return inconclusive;
}


// static variables

// fDefaultGenerator
BigIntGenerator* BigIntGenerator::fDefaultGenerator = NULL;

