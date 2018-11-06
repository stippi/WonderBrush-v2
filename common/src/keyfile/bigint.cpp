// bigint.cpp

#include <cstdio>
#include <iostream>

//#include <OS.h>

#include "bigint.h"

static const int SIZEOF_UL = sizeof(unsigned long);

// copy constructor
bigint::bigint(const bigint& bi)
	: fDigits(NULL),
	  fNegative(false),
	  fBitCount(0),
	  fCapacity(0)
{
	*this = bi;
}

// constructor
bigint::bigint(unsigned int value)
	: fDigits(NULL),
	  fNegative(false),
	  fBitCount(0),
	  fCapacity(0)
{
	_alloc_bits(sizeof(unsigned int));
	fDigits[0] = (digit)value;
	fBitCount = _count_bits(fDigits[0]);
}

// constructor
bigint::bigint(int value)
	: fDigits(NULL),
	  fNegative(value < 0),
	  fBitCount(0),
	  fCapacity(0)
{
	_alloc_bits(sizeof(int));
	fDigits[0] = (digit)((value < 0) ? -value : value);
	fBitCount = _count_bits(fDigits[0]);
}

// constructor
bigint::bigint(unsigned long value)
	: fDigits(NULL),
	  fNegative(false),
	  fBitCount(0),
	  fCapacity(0)
{
	_alloc_bits(sizeof(unsigned long));
#if SIZEOF_UL - BITS_PER_DIGIT
	int digitCount = 0;
	for (; value > 0; value >>= BITS_PER_DIGIT) {
		fDigits[digitCount] = (digit)(value & ~(digit)0);
		digitCount++;
	}
	if (digitCount > 0) {
		fBitCount = (digitCount - 1) * BITS_PER_DIGIT
			+ _count_bits(fDigits[digitCount - 1]);
	}
#else
	fDigits[0] = (digit)value;
	fBitCount = _count_bits(fDigits[0]);
#endif
}

// constructor
bigint::bigint(long value)
	: fDigits(NULL),
	  fNegative(value < 0),
	  fBitCount(0),
	  fCapacity(0)
{
	unsigned long val = (value < 0) ? -value : value;
	_alloc_bits(sizeof(unsigned long));
#if SIZEOF_UL - BITS_PER_DIGIT
	int digitCount = 0;
	for (; val > 0; val >>= BITS_PER_DIGIT) {
		fDigits[digitCount] = (digit)(val & ~(digit)0);
		digitCount++;
	}
	if (digitCount > 0) {
		fBitCount = (digitCount - 1) * BITS_PER_DIGIT
			+ _count_bits(fDigits[digitCount - 1]);
	}
#else
	fDigits[0] = (digit)val;
	fBitCount = _count_bits(fDigits[0]);
#endif
}

// constructor
bigint::bigint(unsigned long long value)
	: fDigits(NULL),
	  fNegative(false),
	  fBitCount(0),
	  fCapacity(0)
{
	_alloc_bits(sizeof(unsigned long long));
	int digitCount = 0;
	for (; value > 0; value >>= BITS_PER_DIGIT) {
		fDigits[digitCount] = (digit)(value & ~(digit)0);
		digitCount++;
	}
	if (digitCount > 0) {
		fBitCount = (digitCount - 1) * BITS_PER_DIGIT
			+ _count_bits(fDigits[digitCount - 1]);
	}
}

// constructor
bigint::bigint(long long value)
	: fDigits(NULL),
	  fNegative(value < 0),
	  fBitCount(0),
	  fCapacity(0)
{
	unsigned long long val = (value < 0) ? -value : value;
	_alloc_bits(sizeof(unsigned long long));
	int digitCount = 0;
	for (; val > 0; val >>= BITS_PER_DIGIT) {
		fDigits[digitCount] = (digit)(val & ~(digit)0);
		digitCount++;
	}
	if (digitCount > 0) {
		fBitCount = (digitCount - 1) * BITS_PER_DIGIT
			+ _count_bits(fDigits[digitCount - 1]);
	}
}

// destructor
bigint::~bigint()
{
	delete[] fDigits;
}

// divide
//
// Note: This function does not allow passing this bigint as operand or
// remainder as well or the same bigint as operand and remainder.
bigint&
bigint::divide(const bigint& operand, bigint& remainder)
{
	bool sign = fNegative;
	remainder = *this;
	bigint divider(operand);
	remainder.fNegative = divider.fNegative = false;
	*this = 0;
	int shift = remainder.ld() - operand.ld();
	if (shift > 0)
		divider <<= shift;
	for (; shift >= 0; shift--) {
		*this <<= 1;
		if (remainder >= divider) {
			remainder -= divider;
//			*this |= 1;
			*fDigits |= 1;
			fBitCount = max(fBitCount, 1);
		}
		if (shift > 0)
			divider >>= 1;
	}
	fNegative = (sign != operand.fNegative);
	remainder.fNegative = sign;
	// clear negative for 0
	_zero_check();
	remainder._zero_check();
	return *this;
}

// divide_copy
//
// Note: This function does not allow passing the same bigint as operand
// and remainder.
bigint
bigint::divide_copy(const bigint& operand, bigint& remainder) const
{
	bigint divident(*this);
	return divident.divide(operand, remainder);
}

// mod_mult
bigint&
bigint::mod_mult(const bigint& factor, const bigint& mod)
{
	if (&factor == this || &mod == this)
		return (*this = bigint(*this).mod_mult(factor, mod));
	return (*this *= factor) %= mod;
}

// mod_mult_copy
bigint
bigint::mod_mult_copy(const bigint& factor, const bigint& mod) const
{
	bigint result(*this);
	return result.mod_mult(factor, mod);
}

// mod_pow
//
// Note: The exponent's sign is ignored, thus if exponent < 0,
// *this^(-exponent) is calculated.
bigint&
bigint::mod_pow(const bigint& exponent, const bigint& mod)
{
	return (*this = mod_pow_copy(exponent, mod));
}

static int dbgShiftCount;
static int dbgAddCount;
static int dbgSubCount;
static int dbgMultCount;
static int dbgModAbsCount;

// mod_pow_copy
//
// Note: The exponent's sign is ignored, thus if exponent < 0,
// *this^(-exponent) is calculated.
bigint
bigint::mod_pow_copy(const bigint& exponent, const bigint& mod) const
{
/*
printf("bigint::mod_pow_copy()\n");
bigtime_t startTime = system_time();
dbgShiftCount = 0;
dbgAddCount = 0;
dbgSubCount = 0;
dbgMultCount = 0;
dbgModAbsCount = 0;
*/

/*
	bigint result(1);
	bigint factor(*this);
	bigint exp(exponent);
	for (int i = 0; i < exp.fBitCount; i++) {
//printf("  i: %d\n", i);
		if (exp.test_bit(i))
			result.mod_mult(factor, mod);
		factor.mod_mult(factor, mod);
	}
*/

	// calculate shifted versions of /mod/
	bigint operands[BITS_PER_DIGIT];
	operands[0] = mod;
	for (int i = 1; i < BITS_PER_DIGIT; i++)
		operands[i] = mod << i;
	// do the calculation
	bigint result(1);
	bigint factor(*this);
	bigint exp(exponent);
	for (int i = 0; i < exp.fBitCount; i++) {
		if (exp.test_bit(i)) {
			result *= factor;
			result._mod_abs(operands);
//if (dbgMultCount == 2)
//delete[] result.fDigits;
		}
		factor *= factor;
		factor._mod_abs(operands);
//if (dbgMultCount == 2)
//delete[] result.fDigits;
	}

/*
bigtime_t passedTime = system_time() - startTime;
printf("mod_pow_copy took  : %Ld\n", passedTime);
printf("per bit time (avg) : %Ld\n", passedTime / exp.fBitCount);
printf("shift count        : %d\n", dbgShiftCount);
printf("add count          : %d\n", dbgAddCount);
printf("sub count          : %d\n", dbgSubCount);
printf("mult count         : %d\n", dbgMultCount);
printf("mod abs count      : %d\n", dbgModAbsCount);
printf("bigint::mod_pow_copy() done\n");
*/
	return result;
}

// toui
unsigned int
bigint::toui() const
{
	unsigned int result = *fDigits;
	return result;
}

// toi
int
bigint::toi() const
{
	int result = *fDigits;
	if (fNegative)
		result = -result;
	return result;
}

// toul
unsigned long
bigint::toul() const
{
#if SIZEOF_UL - BITS_PER_DIGIT
	unsigned long result = 0;
	int digitCount = (fBitCount + BITS_PER_DIGIT - 1) / BITS_PER_DIGIT;
	digitCount = min(digitCount, (int)(sizeof(unsigned long) / sizeof(digit)));
	for (int i = digitCount - 1; i >= 0; i--)
		result = (result << BITS_PER_DIGIT) | fDigits[i];
#else
	unsigned long result = *fDigits;
#endif
	return result;
}

// tol
long
bigint::tol() const
{
	long result = (long)toul();
	if (fNegative)
		result = -result;
	return result;
}

// toull
unsigned long long
bigint::toull() const
{
	unsigned long long result = 0;
	int digitCount = (fBitCount + BITS_PER_DIGIT - 1) / BITS_PER_DIGIT;
	digitCount = min(digitCount,
					 (int)(sizeof(unsigned long long) / sizeof(digit)));
	for (int i = digitCount - 1; i >= 0; i--)
		result = (result << BITS_PER_DIGIT) | fDigits[i];
	return result;
}

// toll
long long
bigint::toll() const
{
	long long result = (long long)toull();
	if (fNegative)
		result = -result;
	return result;
}

// =
bigint&
bigint::operator =(const bigint& operand)
{
	if (&operand != this) {
		_alloc_digits_dont_clear(operand.fCapacity);
		memcpy(fDigits, operand.fDigits, operand.fCapacity * sizeof(digit));
		fNegative = operand.fNegative;
		fBitCount = operand.fBitCount;
	}
	return *this;
}

// +=
bigint&
bigint::operator +=(const bigint& operand)
{
	if (&operand == this)
		return (*this = *this + operand);
	if (fNegative == operand.fNegative)
		_add_digits(operand);
	else {
		if (_abs_less(operand)) {
			_inverse_sub_digits(operand);
			fNegative = operand.fNegative;	// |*this| > 0
		} else {
			_sub_digits(operand);			// |*this| >= 0 !
			_zero_check();
		}
	}
	return *this;
}

// -=
bigint&
bigint::operator -=(const bigint& operand)
{
//cout << "bigint::operator -=(" << *this << ", " << operand << ")" << endl;
	if (&operand == this)
		return (*this = bigint(0));
	if (fNegative != operand.fNegative)
		_add_digits(operand);
	else {
		if (_abs_less(operand)) {
//cout << "_abs_less()\n";
			_inverse_sub_digits(operand);
			fNegative = !operand.fNegative;	// |*this| > 0
		} else {
//cout << "else\n";
			_sub_digits(operand);			// |*this| >= 0 !
			_zero_check();
		}
	}
//cout << "bigint::operator -=() done: " << *this << endl;
	return *this;
}

// *=
bigint&
bigint::operator *=(const bigint& operand)
{
//cout << "bigint::operator *=(" << *this << ", " << operand << ")" << endl;
	if (&operand == this)
		return (*this = *this * operand);
	_multiply_digits(operand);
	fNegative = (fNegative != operand.fNegative);
	// clear negative for 0
	_zero_check();
//cout << "bigint::operator *=() done: " << *this << endl;
	return *this;
}

// /=
bigint&
bigint::operator /=(const bigint& operand)
{
//cout << "bigint::operator /=(" << *this << ", " << operand << ")" << endl;
	bigint remainder;
	return divide(operand, remainder);
//	if (&operand == this)
//		return (*this = bigint(1));
//	_divide_abs(operand);
//	fNegative = fNegative != operand.fNegative;
//	_zero_check();
//cout << "bigint::operator /=() done: " << endl;
	return *this;
}

// %=
bigint&
bigint::operator %=(const bigint& operand)
{
	if (&operand == this)
		return (*this = bigint(0));
	bool sign = fNegative;
	bigint divider(operand);
	fNegative = divider.fNegative = false;
	int shift = ld() - operand.ld();
	if (shift > 0)
		divider <<= shift;
//	for (; shift >= 0; shift--) {
//		if (*this >= divider)
//			*this -= divider;
//		if (shift > 0)
//			divider >>= 1;
//	}
	while (shift > 0) {
		if (*this < divider) {
			divider >>= 1;
			shift--;
		}
		*this -= divider;
		int newShift = ld() - operand.ld();
		if (newShift > 0)
			divider >>= (shift - newShift);
		shift = newShift;
	}
	if (shift == 0 && !_abs_less(operand))
		_sub_digits(operand);

	fNegative = sign;
	// clear negative for 0
	_zero_check();
	return *this;
}

// &=
bigint&
bigint::operator &=(const bigint& operand)
{
//cout << "bigint::operator &=(" << *this << ", " << operand << ")" << endl;
	if (&operand == this)
		return *this;
	int digitCount = (fBitCount + BITS_PER_DIGIT - 1) / BITS_PER_DIGIT;
	int opDigitCount
		= (operand.fBitCount + BITS_PER_DIGIT - 1) / BITS_PER_DIGIT;
	int maxDigitCount = min(digitCount, opDigitCount);
//printf("digitCount: %d, opDigitCount: %d, maxDigitCount: %d\n",
//digitCount, opDigitCount, maxDigitCount);
	digit* d = fDigits;
	const digit* opd = operand.fDigits;
	const digit* endDigit = d + (maxDigitCount - 1);
	for (; d <= endDigit; d++, opd++)
//{
		*d &= *opd;
//printf("d: %u\n", *d);
//}
//printf("check\n");
	_clear_digits(maxDigitCount, maxDigitCount - digitCount);
//printf("check\n");
	_recalculate_bit_count(maxDigitCount - 1);
//printf("check\n");
	fNegative &= operand.fNegative;
	_zero_check();
//cout << "bigint::operator &=() done: " << *this << endl;
	return *this;
}

// |=
bigint&
bigint::operator |=(const bigint& operand)
{
//cout << "bigint::operator |=(" << *this << ", " << operand << ")" << endl;
	if (&operand == this)
		return *this;
	int digitCount = (fBitCount + BITS_PER_DIGIT - 1) / BITS_PER_DIGIT;
	int opDigitCount
		= (operand.fBitCount + BITS_PER_DIGIT - 1) / BITS_PER_DIGIT;
	int newDigitCount = max(digitCount, opDigitCount);
//printf("digitCount: %d, opDigitCount: %d, newDigitCount: %d\n",
//digitCount, opDigitCount, newDigitCount);
	_ensure_capacity(newDigitCount);
	digit* d = fDigits;
	const digit* opd = operand.fDigits;
	const digit* endDigit = d + (opDigitCount - 1);
	for (; d <= endDigit; d++, opd++)
//{
		*d |= *opd;
//printf("d: %u\n", *d);
//}
	fBitCount = max(fBitCount, operand.fBitCount);
//printf("fBitCount: %d\n", fBitCount);
	fNegative |= operand.fNegative;
//cout << "bigint::operator |=() done: " << *this << endl;
	return *this;
}

// ==
bool
bigint::operator ==(const bigint& operand) const
{
	// compare the sign and the number of digits
	if (fNegative != operand.fNegative || fBitCount != operand.fBitCount)
		return false;
	if (fBitCount == 0)	// both numbers are 0
		return true;
	int lastIndex = (fBitCount - 1) / BITS_PER_DIGIT;
	digit* d = fDigits + lastIndex;
	digit* opd = operand.fDigits + lastIndex;
	for (; d >= fDigits; d--, opd--) {
		if (*d != *opd)
			return false;
	}
	// numbers are equal
	return true;
}

/*
// unsigned int
bigint::operator unsigned int() const
{
	unsigned int result = digits.front();
	return result;
}

// int
bigint::operator int() const
{
	int result = digits.front();
	if (negative)
		result = -result;
	return result;
}

// unsigned long long
bigint::operator unsigned long long() const
{
	unsigned long long result = 0;
	for (digitlist::const_reverse_iterator it = digits.rbegin();
		 it != digits.rend();
		 it++) {
		result = (result << BITS_PER_DIGIT) | *it;
	}
	return result;
}

// long long
bigint::operator long long() const
{
	long long result = (long long)(unsigned long long)*this;
	if (negative)
		result = -result;
	return result;
}
*/

// _abs_less
bool
bigint::_abs_less(const bigint& operand) const
{
	// compare the number of digits first
	if (fBitCount < operand.fBitCount)
		return true;
	if (fBitCount > operand.fBitCount)
		return false;
	if (fBitCount == 0)	// both numbers are 0
		return false;
	int lastIndex = (fBitCount - 1) / BITS_PER_DIGIT;
	digit* d = fDigits + lastIndex;
	digit* opd = operand.fDigits + lastIndex;
	for (; d >= fDigits; d--, opd--) {
		if (*d < *opd)
			return true;
		if (*d > *opd)
			return false;
	}
	// numbers are equal
	return false;
}

// _abs_less_shift
//
// The same semantics as _abs_less(operand << (shift * BITS_PER_DIGIT)).
// PRE: shift >= 0
bool
bigint::_abs_less_shift(const bigint& operand, int shift) const
{
	// compare the number of digits first
	if (operand.fBitCount == 0)		// operand == 0
		return false;
	int opBitsCount = operand.fBitCount + shift * BITS_PER_DIGIT;
	if (fBitCount < opBitsCount)
		return true;
	if (fBitCount > opBitsCount)
		return false;
	int lastIndex = (fBitCount - 1) / BITS_PER_DIGIT;
	const digit* d = fDigits + lastIndex;
	const digit* opd = operand.fDigits + (lastIndex - shift);
	const digit* firstDigit = fDigits + shift;
	for (; d >= firstDigit; d--, opd--) {
		if (*d < *opd)
			return true;
		if (*d > *opd)
			return false;
	}
	// The upper digits are equal and the operand's shift lower digits are 0.
	// Therefore the operand is equal to or less than this number.
	return false;
}

// _add_digits
void
bigint::_add_digits(const bigint& operand)
{
dbgAddCount++;
	// catch special cases
	if (operand.fBitCount == 0)	// operand == 0
		return;
	int maxBitCount = max(fBitCount, operand.fBitCount) + 1;
	int digitCount = (fBitCount + BITS_PER_DIGIT - 1) / BITS_PER_DIGIT;
	int opDigitCount = (operand.fBitCount - 1) / BITS_PER_DIGIT + 1;
	int maxDigitCount = (maxBitCount - 1) / BITS_PER_DIGIT + 1;
	_ensure_capacity(maxDigitCount);
	// Add the digits, that both numbers have.
	int addDigitCount = min(digitCount, opDigitCount);
	digit* d = fDigits;
	const digit* opd = operand.fDigits;
	const digit* endDigit = d + addDigitCount;
	digit carry = 0;
	for (; d != endDigit; d++, opd++) {
		digit value = *d + *opd + carry;
		carry = (value < *d || value < *opd) ? 1 : 0;
		*d = value;
	}
	// Add the remaining digits.
	if (digitCount > opDigitCount) {
		endDigit = fDigits + digitCount;
		for (; carry && d != endDigit; d++) {
			digit value = *d + carry;
			carry = (value < *d) ? 1 : 0;
			*d = value;
		}
	} else {
		// Note: opDigitCount may be < maxDigitCount
		endDigit = fDigits + opDigitCount;
		for (; d != endDigit; d++, opd++) {
			digit value = *opd + carry;
			carry = (value < *opd) ? 1 : 0;
			*d = value;
		}
	}
	// Append carry, if necessary.
	if (carry)
		*d = carry;
	_recalculate_bit_count(maxDigitCount - 1);
}

/*
// _add_digits
void
bigint::_add_digits(const digitlist& operand)
{
dbgAddCount++;
	unsigned addlen = min(digits.size(), operand.size());
	digit carry = 0;
	digitlist::iterator it = digits.begin();
	digitlist::const_iterator opit = operand.begin();
	// Add up to the last digit, that both lists have.
	for (unsigned i = 0; i < addlen; i++, it++, opit++) {
		digit d = *it + *opit + carry;
		carry = (d < *it || d < *opit) ? 1 : 0;
		*it = d;
	}
	// Append the remaining digits of the longer list.
	if (digits.size() > operand.size()) {
		for (; it != digits.end(); it++) {
			digit d = *it + carry;
			carry = (d < *it) ? 1 : 0;
			*it = d;
		}
	} else {
		for (; opit != operand.end(); opit++) {
			digit d = *opit + carry;
			carry = (d < *opit) ? 1 : 0;
			digits.push_back(d);
		}
	}
	// Append carry, if necessary.
	if (carry)
		digits.push_back(carry);
}

// _add_digits
void
bigint::_add_digits(const digitlist& operand, int shift)
{
dbgAddCount++;
	int addlen = min((int)digits.size() - shift, (int)operand.size());
	digit carry = 0;
	digitlist::iterator it = digits.begin();
	digitlist::const_iterator opit = operand.begin();
	// Position iterator to the first digit to be added.
	if (addlen >= 0)
		for (int i = 0; i < shift; i++, it++);
	else {
		// insert 0 digits
		for (int i = 0; i > addlen; i--)
			digits.push_back(0);
	}
	// Add up to the last digit, that both lists have.
	for (int i = 0; i < addlen; i++, it++, opit++) {
		digit d = *it + *opit + carry;
		carry = (d < *it || d < *opit) ? 1 : 0;
		*it = d;
	}
	// Append the remaining digits of the longer list.
	if (digits.size() - shift > operand.size()) {
		for (; it != digits.end(); it++) {
			digit d = *it + carry;
			carry = (d < *it) ? 1 : 0;
			*it = d;
		}
	} else {
		for (; opit != operand.end(); opit++) {
			digit d = *opit + carry;
			carry = (d < *opit) ? 1 : 0;
			digits.push_back(d);
		}
	}
	// Append carry, if necessary.
	if (carry)
		digits.push_back(carry);
	_simplify();
}
*/

// _sub_digits
//
// PRE: !_abs_less(operand)
void
bigint::_sub_digits(const bigint& operand)
{
dbgSubCount++;
	// catch special cases
	if (operand.fBitCount == 0)	// operand == 0
		return;
	int digitCount = (fBitCount + BITS_PER_DIGIT - 1) / BITS_PER_DIGIT;
	int opDigitCount = (operand.fBitCount - 1) / BITS_PER_DIGIT + 1;
	// Sub the digits, that both numbers have.
	digit* d = fDigits;
	const digit* opd = operand.fDigits;
	const digit* endDigit = d + opDigitCount;
	digit carry = 0;
	for (; d != endDigit; d++, opd++) {
		digit value = *d - *opd - carry;
		carry = (value > *d) ? 1 : 0;
		*d = value;
	}
	// Sub the remaining digits.
	endDigit = fDigits + digitCount;
	for (; carry && d != endDigit; d++) {
		digit value = *d - carry;
		carry = (value > *d) ? 1 : 0;
		*d = value;
	}
	_recalculate_bit_count(digitCount - 1);
}

// _sub_digits_shift
//
// PRE: shift >= 0 && !_abs_less(operand << (shift * BITS_PER_DIGIT))
void
bigint::_sub_digits_shift(const bigint& operand, int shift)
{
dbgSubCount++;
	// catch special cases
	if (operand.fBitCount == 0)	// operand == 0
		return;
	int digitCount = (fBitCount + BITS_PER_DIGIT - 1) / BITS_PER_DIGIT;
	int opDigitCount = (operand.fBitCount - 1) / BITS_PER_DIGIT + 1 + shift;
	// Sub the digits, that both numbers have.
	digit* d = fDigits + shift;
	const digit* opd = operand.fDigits;
	const digit* endDigit = fDigits + opDigitCount;
	digit carry = 0;
	for (; d != endDigit; d++, opd++) {
		digit value = *d - *opd - carry;
		carry = (value > *d) ? 1 : 0;
		*d = value;
	}
	// Sub the remaining digits.
	endDigit = fDigits + digitCount;
	for (; carry && d != endDigit; d++) {
		digit value = *d - carry;
		carry = (value > *d) ? 1 : 0;
		*d = value;
	}
	_recalculate_bit_count(digitCount - 1);
}

/*
// _sub_digits
//
// PRE: !_abs_less(operand)
void
bigint::_sub_digits(const digitlist& operand)
{
dbgSubCount++;
	unsigned sublen = min(digits.size(), operand.size());
	digit carry = 0;
	digitlist::iterator it = digits.begin();
	digitlist::const_iterator opit = operand.begin();
	// Sub up to the last digit, that both lists have.
	for (unsigned i = 0; i < sublen; i++, it++, opit++) {
		digit d = *it - *opit - carry;
		carry = (d > *it) ? 1 : 0;
		*it = d;
	}
	// Append the remaining digits of the longer list.
	if (digits.size() > operand.size()) {
		for (; it != digits.end(); it++) {
			digit d = *it - carry;
			carry = (d > *it) ? 1 : 0;
			*it = d;
		}
	} else;	// must not occur
	_simplify();
}
*/

// _inverse_sub_digits
//
// PRE: !operand._abs_less(*this)
void
bigint::_inverse_sub_digits(const bigint& operand)
{
dbgSubCount++;
//printf("bigint::_inverse_sub_digits()\n");
	// catch special cases
	if (operand.fBitCount == 0)	// operand == 0 => *this == 0
{
//printf("bigint::_inverse_sub_digits() done1\n");
		return;
}
	int digitCount = (fBitCount + BITS_PER_DIGIT - 1) / BITS_PER_DIGIT;
	int opDigitCount = (operand.fBitCount - 1) / BITS_PER_DIGIT + 1;
//printf("digitCount: %d, opDigitCount: %d\n", digitCount, opDigitCount);
	_ensure_capacity(opDigitCount);
	// Sub the digits, that both numbers have.
	digit* d = fDigits;
	const digit* opd = operand.fDigits;
	const digit* endDigit = d + digitCount;
	digit carry = 0;
	for (; d != endDigit; d++, opd++) {
		digit value = *opd - *d - carry;
//printf("  sub digit: %u\n", value);
		carry = (value > *opd) ? 1 : 0;
		*d = value;
	}
	// Sub the remaining digits.
	endDigit = fDigits + opDigitCount;
	for (; d != endDigit; d++, opd++) {
		digit value = *opd - carry;
//printf("  sub digit2: %u\n", value);
		carry = (value > *opd) ? 1 : 0;
		*d = value;
	}
	_recalculate_bit_count(opDigitCount - 1);
//printf("bigint::_inverse_sub_digits() done\n");
}

/*
// _inverse_sub_digits
//
// PRE: !operand._abs_less(*this)
void
bigint::_inverse_sub_digits(const digitlist& operand)
{
dbgSubCount++;
	unsigned sublen = min(digits.size(), operand.size());
	digit carry = 0;
	digitlist::iterator it = digits.begin();
	digitlist::const_iterator opit = operand.begin();
	// Sub up to the last digit, that both lists have.
	for (unsigned i = 0; i < sublen; i++, it++, opit++) {
		digit d = *opit - *it - carry;
		carry = (d > *opit) ? 1 : 0;
		*it = d;
	}
	// Append the remaining digits of the longer list.
	if (digits.size() > operand.size()) {
		// must not occur
	} else {
		for (; opit != operand.end(); opit++) {
			digit d = *opit - carry;
			carry = (d > *opit) ? 1 : 0;
			digits.push_back(d);
		}
	}
	_simplify();
}
*/

// _shift_digits
//
// Shift digits left for /shift/ > 0, shift right for /shift/ < 0.
void
bigint::_shift_digits(int shift)
{
	if (shift == 0 || fBitCount == 0)
		return;
dbgShiftCount++;
	int newBitCount = fBitCount + shift;
	if (shift > 0) {
		// shift right
		int shiftDigits = shift / BITS_PER_DIGIT;
		shift %= BITS_PER_DIGIT;
		int reverseShift = BITS_PER_DIGIT - shift;
		int digitCount = (fBitCount - 1) / BITS_PER_DIGIT + 1;
		int newDigitCount = (newBitCount - 1) / BITS_PER_DIGIT + 1;
		_ensure_capacity(newDigitCount);
		if (shift > 0) {
			// shift not digit aligned
			const digit* src = fDigits + (digitCount - 1);
			digit* dest = fDigits + (newDigitCount - 1);
//			digit oldDigit = *src;
			digit oldDigit = 0;
			if (newDigitCount == digitCount + shiftDigits) {
				oldDigit = *src;
				src--;
			}
			for (; src >= fDigits; src--, dest--) {
				digit d = *src;
				*dest = (oldDigit << shift) | (d >> reverseShift);
				oldDigit = d;
			}
			*dest = oldDigit << shift;
			_clear_digits(0, shiftDigits);
		} else {
			// shift digit aligned
			const digit* src = fDigits + (digitCount - 1);
			digit* dest = fDigits + (newDigitCount - 1);
			for (; src >= fDigits; src--, dest--)
				*dest = *src;
			_clear_digits(0, shiftDigits);
		}
		fBitCount = newBitCount;
	} else {
		// shift left
		int digitCount = (fBitCount - 1) / BITS_PER_DIGIT + 1;
		if (newBitCount <= 0) {
			// shift away all bits
			_clear_digits(0, digitCount);
			fBitCount = 0;
			fNegative = false;
		} else {
			// some bits will be left
			shift = -shift;
			int shiftDigits = shift / BITS_PER_DIGIT;
			shift %= BITS_PER_DIGIT;
			int reverseShift = BITS_PER_DIGIT - shift;
			int newDigitCount = (newBitCount - 1) / BITS_PER_DIGIT + 1;
			if (shift > 0) {
				// shift not digit aligned
				const digit* src = fDigits + shiftDigits;
				digit* dest = fDigits;
				const digit* endDigit = fDigits + digitCount;
				digit oldDigit = *src;
				src++;
				for (; src != endDigit; src++, dest++) {
					digit d = *src;
					*dest = (oldDigit >> shift) | (d << reverseShift);
					oldDigit = d;
				}
				*dest = oldDigit >> shift;
				_clear_digits(newDigitCount, digitCount - newDigitCount);
			} else {
				// shift digit aligned
				const digit* src = fDigits + shiftDigits;
				digit* dest = fDigits;
				const digit* endDigit = fDigits + digitCount;
				for (; src != endDigit; src++, dest++)
					*dest = *src;
				_clear_digits(newDigitCount, shiftDigits);
			}
			fBitCount = newBitCount;
		}
	}
}

// _multiply_digits
void
bigint::_multiply_digits(const bigint& operand)
{
dbgMultCount++;
//printf("bigint::_multiply_digits(): dbgMultCount: %d\n", dbgMultCount);
//printf("bigint::_multiply_digits()\n");
	// catch special cases
	if (fBitCount == 0)				// *this == 0
{
//printf("bigint::_multiply_digits() done1\n");
		return;
}
	if (operand.fBitCount == 0) {	// operand == 0
		*this = 0;
//printf("bigint::_multiply_digits() done2\n");
		return;
	}
	int digitCount = (fBitCount - 1) / BITS_PER_DIGIT + 1;
	int opDigitCount = (operand.fBitCount - 1) / BITS_PER_DIGIT + 1;
	int newBitCount = fBitCount + operand.fBitCount;
	int newDigitCount = (newBitCount - 1) / BITS_PER_DIGIT + 1;
//printf("digitCount: %d, opDigitCount: %d, newBitCount: %d, newDigitCount: %d\n",
//digitCount, opDigitCount, newBitCount, newDigitCount);
	bigint operand1(*this);
	_ensure_capacity(newDigitCount);
	_clear_digits(0, newDigitCount);

	const digit* factor1 = operand1.fDigits;
	const digit* factor2 = operand.fDigits;
	digit* result = fDigits;
	for (int i = 0; i < digitCount; i++) {
		unsigned long long f1 = factor1[i];
		unsigned long long value = 0;
		for (int k = 0; k < opDigitCount; k++) {
			value += f1 * (unsigned long long)factor2[k];
			value += result[i + k];
			result[i + k] = (digit)(value & ~(digit)0);
			value >>= BITS_PER_DIGIT;
		}
		// Note: The array is exactly large enough to contain all non-0
		// bits, thus, if the last carry is 0, it must not be written.
		if (i + opDigitCount < newDigitCount)
			result[i + opDigitCount] = (digit)value;
	}
	_recalculate_bit_count(newDigitCount - 1);
//printf("bigint::_multiply_digits() done\n");
}

// _divide_abs
void
bigint::_divide_abs(const bigint& operand)
{
	bigint divident(*this);
	bigint divider(operand);
	divident.fNegative = divider.fNegative = false;
	*this = 0;
	int shift = divident.ld() - operand.ld();
	if (shift > 0)
		divider <<= shift;
	for (; shift >= 0; shift--) {
//cout << "shift: " << shift << ", ";
//cout << "divident: " << divident << ", divider: " << divider << endl;
		*this <<= 1;
		if (divident >= divider) {
//cout << "divident >= divider" << endl;
			divident -= divider;
//			*this |= 1;
			*fDigits |= 1;
			fBitCount = max(fBitCount, 1);
		}
		if (shift > 0)
			divider >>= 1;
	}
}

// _mod_abs
//
// /operands/ is an array containing BITS_PER_DIGIT numbers -- the 0 - 31
// bits shifted versions of the actual operand.
void
bigint::_mod_abs(const bigint* operands)
{
dbgModAbsCount++;
	int shift = ld() - operands[0].ld();
	while (shift > 0) {
		if (_abs_less_shift(operands[shift % BITS_PER_DIGIT],
							shift / BITS_PER_DIGIT)) {
			shift--;
		}
		_sub_digits_shift(operands[shift % BITS_PER_DIGIT],
						  shift / BITS_PER_DIGIT);
		shift = ld() - operands[0].ld();
	}
	if (shift == 0 && !_abs_less(operands[0]))
		_sub_digits(operands[0]);
	// clear negative for 0
	_zero_check();
}


// non-member functions

// gcd
//
// Returns the greatest common divider of /a/ and /b/.
bigint
gcd(const bigint& a, const bigint& b)
{
	bigint f = a;
	bigint g = b;
	while (g != 0) {
		bigint r = f % g;
		f = g;
		g = r;
	}
	// normalize
	f.fNegative = false;
	return f;
}

// gcd
//
// Returns the greatest common divider of /a/ and /b/ and a representation
// of kind: gcd = u*b + v*b.
bigint
gcd(const bigint& a, const bigint& b, bigint& u, bigint& v)
{
	bigint f = a;
	bigint g = b;
	u = 0;
	v = 1;
	bigint oldu(1);
	bigint oldv(0);
	bigint r;
	bigint q = f.divide_copy(g, r);
	while (r != 0) {
		f = g;
		g = r;
		bigint lastu = u;
		bigint lastv = v;
		u = oldu - q * u;
		v = oldv - q * v;
		oldu = lastu;
		oldv = lastv;
		q = f.divide_copy(g, r);
	}
	// normalize
	bool sign = g.fNegative;
	u.fNegative = (u.fNegative != sign);
	v.fNegative = (v.fNegative != sign);
	u._zero_check();
	v._zero_check();
	g.fNegative = false;
	return g;
}

// output stream operator
ostream&
operator <<(ostream& out, const bigint& bi)
{
	if (bi.fBitCount == 0)	// bi = 0
		out << '0';
	else {
		if (bi.fNegative)
			out << '-';
		bool first = true;
		int digitCount = (bi.fBitCount + bigint::BITS_PER_DIGIT - 1)
						 / bigint::BITS_PER_DIGIT;
		const bigint::digit* d = bi.fDigits + (digitCount - 1);
		for (; d >= bi.fDigits; d--) {
			if (first) {
				first = false;
// PPC: no form() ?!
//				out.form("%x", *d);
				char buffer[16];
				sprintf(buffer, "%x", *d);
				out << buffer;
			} else {
// PPC: no form() ?!
//				out.form(" %08x", *d);
				char buffer[16];
				sprintf(buffer, " %08x", *d);
				out << buffer;
			}
		}
	}
	out << "(" << bi.fBitCount << ", " << bi.fCapacity << ")";
	return out;
}

