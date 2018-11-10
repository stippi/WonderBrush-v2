// bigint.h

#ifndef BIGINT_H
#define BIGINT_H

#include <algorithm>
#include <iostream>
#include <string.h>

using namespace std;

class bigint {
 private:
	typedef unsigned int		digit;
	static const int			DIGIT_BLOCK_SIZE = 4;
	static const int			BITS_PER_DIGIT = sizeof(digit) * 8;
	static const int			BITS_PER_WORD = BITS_PER_DIGIT / 2;
	static const digit			LOWER_WORD = (1 << BITS_PER_WORD) - 1;
	static const digit			UPPER_WORD = LOWER_WORD << BITS_PER_WORD;

 public:
								bigint(const bigint& bi);
								bigint(unsigned int value = 0);
								bigint(int value);
								bigint(unsigned long value);
								bigint(long value);
								bigint(unsigned long long value);
								bigint(long long value);
								~bigint();

			bigint&				divide(const bigint& operand,
									   bigint& remainder);
			bigint				divide_copy(const bigint& operand,
											bigint& remainder) const;
	inline	int					ld() const;
	inline	bool				test_bit(int bit) const;
			bigint&				mod_mult(const bigint& factor,
										 const bigint& mod);
			bigint				mod_mult_copy(const bigint& factor,
											  const bigint& mod) const;
			bigint&				mod_pow(const bigint& exponent,
										const bigint& mod);
			bigint				mod_pow_copy(const bigint& exponent,
											 const bigint& mod) const;
			unsigned int		toui() const;
			int					toi() const;
			unsigned long		toul() const;
			long				tol() const;
			unsigned long long	toull() const;
			long long			toll() const;

			bigint&				operator =(const bigint& operand);
			bigint&				operator +=(const bigint& operand);
			bigint&				operator -=(const bigint& operand);
			bigint&				operator *=(const bigint& operand);
			bigint&				operator /=(const bigint& operand);
			bigint&				operator %=(const bigint& operand);
	inline	bigint&				operator <<=(int shift);
	inline	bigint&				operator >>=(int shift);
			bigint&				operator &=(const bigint& operand);
			bigint&				operator |=(const bigint& operand);

	inline	bigint				operator +(const bigint& operand) const;
	inline	bigint				operator -(const bigint& operand) const;
	inline	bigint				operator -() const;
	inline	bigint				operator ++();
	inline	bigint				operator ++(int);
	inline	bigint				operator --();
	inline	bigint				operator --(int);
	inline	bigint				operator *(const bigint& operand) const;
	inline	bigint				operator /(const bigint& operand) const;
	inline	bigint				operator %(const bigint& operand) const;
	inline	bigint				operator <<(int shift) const;
	inline	bigint				operator >>(int shift) const;
	inline	bigint				operator &(const bigint& operand) const;
	inline	bigint				operator |(const bigint& operand) const;

			bool				operator ==(const bigint& operand) const;
	inline	bool				operator !=(const bigint& operand) const;
	inline	bool				operator <(const bigint& operand) const;
	inline	bool				operator >(const bigint& operand) const;
	inline	bool				operator <=(const bigint& operand) const;
	inline	bool				operator >=(const bigint& operand) const;

//								operator bool() const { *this != 0; }
//								operator unsigned int() const;
//								operator int() const;
//								operator unsigned long long() const;
//								operator long long() const;

 private:
			bool				_abs_less(const bigint& operand) const;
	inline	bool				_abs_greater(const bigint& operand) const;
			bool				_abs_less_shift(const bigint& operand,
												int shift) const;

			void				_add_digits(const bigint& operand);
			void				_sub_digits(const bigint& operand);
			void				_sub_digits_shift(const bigint& operand,
												  int shift);
			void				_inverse_sub_digits(const bigint& operand);
			void				_shift_digits(int shift);
			void				_multiply_digits(const bigint& operand);
			void				_divide_abs(const bigint& operand);
			void				_mod_abs(const bigint* operand);

	inline	void				_zero_check();

	inline	void				_clear_digits(int index, int count);
	inline	void				_alloc_digits(int digitCount);
	inline	void				_alloc_digits_dont_clear(int digitCount);
	inline	void				_alloc_bits(int bits);
	inline	void				_realloc_digits(int digitCount);
	inline	void				_ensure_capacity(int digitCount);
	static inline int			_count_bits(digit d);
	inline	void				_recalculate_bit_count(int lastDigit);
	inline	void				_recalculate_bit_count();

 private:
			digit*				fDigits;		// little endian
			bool				fNegative;		// sign
			int					fBitCount;		// number of bits
			int					fCapacity;		// number of available digits

	friend	bigint 				gcd(const bigint& a, const bigint& b);
	friend	bigint 				gcd(const bigint& a, const bigint& b,
									bigint& u, bigint& v);
	friend	ostream&			operator <<(ostream& out, const bigint& bi);
};

// non-member functions

bigint gcd(const bigint& a, const bigint& b);
bigint gcd(const bigint& a, const bigint& b, bigint& u, bigint& v);
ostream& operator <<(ostream& out, const bigint& bi);

// abs
//
// Returns the absolute value of /a/.
static inline
bigint
abs(const bigint& a)
{
	if (a < 0)
		return -a;
	return a;
}


// inline member functions

// ld
//
// Returns the index of the most significant set bit, that is for bigints
// > 0 the integer dual logarithm of their absolute value.
int
bigint::ld() const
{
	return fBitCount - 1;
}

// test_bit
bool
bigint::test_bit(int bit) const
{
	return (bit < fBitCount)
		   && (fDigits[bit / BITS_PER_DIGIT] & (1 << bit % BITS_PER_DIGIT));
}

// <<=
bigint&
bigint::operator <<=(int shift)
{
	_shift_digits(shift);
	return *this;
}

// >>=
bigint&
bigint::operator >>=(int shift)
{
	_shift_digits(-shift);
	return *this;
}

// +
bigint
bigint::operator +(const bigint& operand) const
{
	bigint bi(*this);
	return (bi += operand);
}

// -
bigint
bigint::operator -(const bigint& operand) const
{
	bigint bi(*this);
	return (bi -= operand);
}

// -
bigint
bigint::operator -() const
{
	bigint bi(*this);
	if (bi.fBitCount != 0)
		bi.fNegative = !fNegative;
	return bi;
}

// ++
bigint
bigint::operator ++()
{
	return *this += 1;
}

// ++
bigint
bigint::operator ++(int)
{
	bigint result(*this);
	++*this;
	return result;
}

// --
bigint
bigint::operator --()
{
	return *this -= 1;
}

// --
bigint
bigint::operator --(int)
{
	bigint result(*this);
	--*this;
	return result;
}

// *
bigint
bigint::operator *(const bigint& operand) const
{
	bigint bi(*this);
	return (bi *= operand);
}

// /
bigint
bigint::operator /(const bigint& operand) const
{
	bigint bi(*this);
	return (bi /= operand);
}

// %
bigint
bigint::operator %(const bigint& operand) const
{
	bigint bi(*this);
	return (bi %= operand);
}

// <<
bigint
bigint::operator <<(int shift) const
{
	bigint bi(*this);
	return (bi <<= shift);
}

// >>
bigint
bigint::operator >>(int shift) const
{
	bigint bi(*this);
	return (bi >>= shift);
}

// &
bigint
bigint::operator &(const bigint& operand) const
{
	bigint bi(*this);
	return (bi &= operand);
}

// |
bigint
bigint::operator |(const bigint& operand) const
{
	bigint bi(*this);
	return (bi |= operand);
}

// !=
bool
bigint::operator !=(const bigint& operand) const
{
	return !(*this == operand);
}

// <
bool
bigint::operator <(const bigint& operand) const
{
	if (fNegative != operand.fNegative)
		return fNegative;
	if (fNegative)
		return (_abs_greater(operand));
	return _abs_less(operand);
}

// >
bool
bigint::operator >(const bigint& operand) const
{
	return (operand < *this);
}

// <=
bool
bigint::operator <=(const bigint& operand) const
{
	return !(*this > operand);
}

// >=
bool
bigint::operator >=(const bigint& operand) const
{
	return !(*this < operand);
}

// _abs_greater
bool
bigint::_abs_greater(const bigint& operand) const
{
	return operand._abs_less(*this);
}

// _zero_check
void
bigint::_zero_check()
{
	if (fBitCount == 0)
		fNegative = false;
}

// _clear_digits
void
bigint::_clear_digits(int index, int count)
{
	digit* d = fDigits + index;
	const digit* end = d + count;
	for (; d < end; d++)
		*d = 0;
}

// _alloc_digits
void
bigint::_alloc_digits(int digitCount)
{
	_alloc_digits_dont_clear(digitCount);
	_clear_digits(0, fCapacity);
}

// _alloc_digits_dont_clear
void
bigint::_alloc_digits_dont_clear(int digitCount)
{
	digitCount += DIGIT_BLOCK_SIZE - 1;
	digitCount -= digitCount % DIGIT_BLOCK_SIZE;
	delete[] fDigits;
	fDigits = new digit[digitCount];
	fCapacity = digitCount;
}

// _alloc_bits
void
bigint::_alloc_bits(int bits)
{
	_alloc_digits((bits + BITS_PER_DIGIT - 1) % BITS_PER_DIGIT);
}

// _realloc_digits
//
// Note: If digitCount < fCapacity, fBitCount may become invalid.
void
bigint::_realloc_digits(int digitCount)
{
	digitCount += DIGIT_BLOCK_SIZE - 1;
	digitCount -= digitCount % DIGIT_BLOCK_SIZE;
	if (digitCount != fCapacity) {
		digit* oldDigits = fDigits;
		int oldCapacity = fCapacity;
		fDigits = new digit[digitCount];
		fCapacity = digitCount;
		int digitsToCopy = min(oldCapacity, fCapacity);
		memcpy(fDigits, oldDigits, sizeof(digit) * digitsToCopy);
		_clear_digits(digitsToCopy, fCapacity - digitsToCopy);
		delete[] oldDigits;
	}
}

// _ensure_capacity
void
bigint::_ensure_capacity(int digitCount)
{
	if (digitCount > fCapacity)
		_realloc_digits(digitCount);
}

// _count_bits
int
bigint::_count_bits(digit d)
{
	int bits = 0;
	if (d) {
		bits = 1;
		int range = BITS_PER_DIGIT / 2;
		digit filter = ~(digit)0 << range;
		while (range > 0) {
			if (d & filter) {
				bits += range;
				range /= 2;
				filter <<= range;
			} else {
				range /= 2;
				filter >>= range;
			}
		}
	}
	return bits;
}

// _recalculate_bit_count
//
// The most significant digit != 0 is searched (backwards) starting at
// /lastDigit/.
void
bigint::_recalculate_bit_count(int lastDigit)
{
	if (lastDigit < 0)
		lastDigit = 0;
	for (; lastDigit > 0 && fDigits[lastDigit] == 0; lastDigit--);
	fBitCount = lastDigit * BITS_PER_DIGIT + _count_bits(fDigits[lastDigit]);
}

// _recalculate_bit_count
void
bigint::_recalculate_bit_count()
{
	_recalculate_bit_count(fCapacity - 1);
}


#endif	// BIGINT_H
