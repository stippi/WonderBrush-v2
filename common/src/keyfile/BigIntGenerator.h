// BigIntGenerator.h

#ifndef BIG_INT_GENERATOR_H
#define BIG_INT_GENERATOR_H

#include "bigint.h"

class BigIntGenerator {
 private:
								BigIntGenerator();
								~BigIntGenerator();

 public:
	static	BigIntGenerator*	CreateDefault();
	static	void				DeleteDefault();
	static	BigIntGenerator&	GetDefault();

	static inline	bigint		GenerateRandomNumber(unsigned length);
	static inline	bigint		GenerateRandomNumber(const bigint& maximum);
	static inline	bigint		GeneratePrimeCandidate(unsigned length);
	static inline	bigint		GeneratePrimeCandidate(unsigned length,
													   int& cookie);
	static inline	void		GetNextPrimeCandidate(bigint& p, int& cookie);
	static inline	bigint		GeneratePrime(unsigned length);

	static inline	bool		IsPrime(const bigint& p);

 private:
			bigint				_GenerateRandomNumber(unsigned length);
			bigint				_GenerateRandomNumber(const bigint& maximum);
			bigint				_GeneratePrimeCandidate(unsigned length);
			bigint				_GeneratePrimeCandidate(unsigned length,
														int& cookie);
			void				_GetNextPrimeCandidate(bigint& p, int& cookie);
			bigint				_GeneratePrime(unsigned length);

 private:
 			int					fSieveSize;
 			int*				fCandidates;
 			int					fCandidateCount;
	static	BigIntGenerator*	fDefaultGenerator;
};

// GenerateRandomNumber
bigint
BigIntGenerator::GenerateRandomNumber(unsigned length)
{
	return GetDefault()._GenerateRandomNumber(length);
}

// GenerateRandomNumber
bigint
BigIntGenerator::GenerateRandomNumber(const bigint& maximum)
{
	return GetDefault()._GenerateRandomNumber(maximum);
}

// GeneratePrimeCandidate
bigint
BigIntGenerator::GeneratePrimeCandidate(unsigned length)
{
	return GetDefault()._GeneratePrimeCandidate(length);
}

// GeneratePrimeCandidate
//
// Generates a prime candidate and initializes the /cookie/ for subsequent
// calls to GenerateNextPrimeCandidate().
bigint
BigIntGenerator::GeneratePrimeCandidate(unsigned length, int& cookie)
{
	return GetDefault()._GeneratePrimeCandidate(length, cookie);
}

// GetNextPrimeCandidate
//
// p is set to a greater candidate.
// Note: /p/ and /cookie/ must be the result of a call to
// GeneratePrimeCandidate().
void
BigIntGenerator::GetNextPrimeCandidate(bigint& p, int& cookie)
{
	GetDefault()._GetNextPrimeCandidate(p, cookie);
}

// GeneratePrime
bigint
BigIntGenerator::GeneratePrime(unsigned length)
{
	return GetDefault()._GeneratePrime(length);
}


#endif	// BIG_INT_GENERATOR_H
