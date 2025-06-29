#include "alif.h"

#include "AlifCore_Long.h"





static const unsigned long _smallMax_[] = { // 26
	0, /* bases 0 and 1 are invalid */
	0,
	ULONG_MAX / 2,
	ULONG_MAX / 3,
	ULONG_MAX / 4,
	ULONG_MAX / 5,
	ULONG_MAX / 6,
	ULONG_MAX / 7,
	ULONG_MAX / 8,
	ULONG_MAX / 9,
	ULONG_MAX / 10,
	ULONG_MAX / 11,
	ULONG_MAX / 12,
	ULONG_MAX / 13,
	ULONG_MAX / 14,
	ULONG_MAX / 15,
	ULONG_MAX / 16,
	ULONG_MAX / 17,
	ULONG_MAX / 18,
	ULONG_MAX / 19,
	ULONG_MAX / 20,
	ULONG_MAX / 21,
	ULONG_MAX / 22,
	ULONG_MAX / 23,
	ULONG_MAX / 24,
	ULONG_MAX / 25,
	ULONG_MAX / 26,
	ULONG_MAX / 27,
	ULONG_MAX / 28,
	ULONG_MAX / 29,
	ULONG_MAX / 30,
	ULONG_MAX / 31,
	ULONG_MAX / 32,
	ULONG_MAX / 33,
	ULONG_MAX / 34,
	ULONG_MAX / 35,
	ULONG_MAX / 36,
};






 // 70
#if SIZEOF_LONG == 4
static const AlifIntT _digitLimit_[] = {
	0,  0, 32, 20, 16, 13, 12, 11, 10, 10,  /*  0 -  9 */
	9,  9,  8,  8,  8,  8,  8,  7,  7,  7,  /* 10 - 19 */
	7,  7,  7,  7,  6,  6,  6,  6,  6,  6,  /* 20 - 29 */
	6,  6,  6,  6,  6,  6,  6 };             /* 30 - 36 */
#elif SIZEOF_LONG == 8
/* [int(math.floor(math.log(2**64, i))) for i in range(2, 37)] */
static const AlifIntT _digitLimit_[] = {
		 0,   0, 64, 40, 32, 27, 24, 22, 21, 20,  /*  0 -  9 */
	19,  18, 17, 17, 16, 16, 16, 15, 15, 15,  /* 10 - 19 */
	14,  14, 14, 14, 13, 13, 13, 13, 13, 13,  /* 20 - 29 */
	13,  12, 12, 12, 12, 12, 12 };             /* 30 - 36 */
#else
#  error "Need table for SIZEOF_LONG"
#endif





unsigned long alifOS_strToULong(const char* _str, char** _ptr, AlifIntT _base) { // 100
	unsigned long result = 0; /* return value of the function */
	AlifIntT c_{};             /* current input character */
	AlifIntT ovLimit{};       /* required digits to overflow */

	/* skip leading white space */
	while (*_str and ALIF_ISSPACE(*_str))
		++_str;

	/* check for leading 0b, 0o or 0x for auto-base or base 16 */
	switch (_base) {
	case 0:             /* look for leading 0b, 0o or 0x */
		if (*_str == '0') {
			++_str;
			if (*_str == 'x' or *_str == 'X') {
				/* there must be at least one digit after 0x */
				if (_alifLongDigitValue_[ALIF_CHARMASK(_str[1])] >= 16) {
					if (_ptr)
						*_ptr = (char*)_str;
					return 0;
				}
				++_str;
				_base = 16;
			}
			else if (*_str == 'o' or *_str == 'O') {
				/* there must be at least one digit after 0o */
				if (_alifLongDigitValue_[ALIF_CHARMASK(_str[1])] >= 8) {
					if (_ptr)
						*_ptr = (char*)_str;
					return 0;
				}
				++_str;
				_base = 8;
			}
			else if (*_str == 'b' or *_str == 'B') {
				/* there must be at least one digit after 0b */
				if (_alifLongDigitValue_[ALIF_CHARMASK(_str[1])] >= 2) {
					if (_ptr)
						*_ptr = (char*)_str;
					return 0;
				}
				++_str;
				_base = 2;
			}
			else {
				/* skip all zeroes... */
				while (*_str == '0')
					++_str;
				while (ALIF_ISSPACE(*_str))
					++_str;
				if (_ptr)
					*_ptr = (char*)_str;
				return 0;
			}
		}
		else
			_base = 10;
		break;

		/* even with explicit base, skip leading 0? prefix */
	case 16:
		if (*_str == '0') {
			++_str;
			if (*_str == 'x' or *_str == 'X') {
				/* there must be at least one digit after 0x */
				if (_alifLongDigitValue_[ALIF_CHARMASK(_str[1])] >= 16) {
					if (_ptr)
						*_ptr = (char*)_str;
					return 0;
				}
				++_str;
			}
		}
		break;
	case 8:
		if (*_str == '0') {
			++_str;
			if (*_str == 'o' or *_str == 'O') {
				/* there must be at least one digit after 0o */
				if (_alifLongDigitValue_[ALIF_CHARMASK(_str[1])] >= 8) {
					if (_ptr)
						*_ptr = (char*)_str;
					return 0;
				}
				++_str;
			}
		}
		break;
	case 2:
		if (*_str == '0') {
			++_str;
			if (*_str == 'b' or *_str == 'B') {
				/* there must be at least one digit after 0b */
				if (_alifLongDigitValue_[ALIF_CHARMASK(_str[1])] >= 2) {
					if (_ptr)
						*_ptr = (char*)_str;
					return 0;
				}
				++_str;
			}
		}
		break;
	}

	/* catch silly bases */
	if (_base < 2 or _base > 36) {
		if (_ptr)
			*_ptr = (char*)_str;
		return 0;
	}

	/* skip leading zeroes */
	while (*_str == '0')
		++_str;

	/* base is guaranteed to be in [2, 36] at this point */
	ovLimit = _digitLimit_[_base];

	/* do the conversion until non-digit character encountered */
	while ((c_ = _alifLongDigitValue_[ALIF_CHARMASK(*_str)]) < _base) {
		if (ovLimit > 0) /* no overflow check required */
			result = result * _base + c_;
		else { /* requires overflow check */
			unsigned long temp_result;

			if (ovLimit < 0) /* guaranteed overflow */
				goto overflowed;

			/* there could be an overflow */
			/* check overflow just from shifting */
			if (result > _smallMax_[_base])
				goto overflowed;

			result *= _base;

			/* check overflow from the digit's value */
			temp_result = result + c_;
			if (temp_result < result)
				goto overflowed;

			result = temp_result;
		}

		++_str;
		--ovLimit;
	}

	/* set pointer to point to the last character scanned */
	if (_ptr)
		*_ptr = (char*)_str;

	return result;

overflowed:
	if (_ptr) {
		/* spool through remaining digit characters */
		while (_alifLongDigitValue_[ALIF_CHARMASK(*_str)] < _base)
			++_str;
		*_ptr = (char*)_str;
	}
	errno = ERANGE;
	return (unsigned long)-1;
}




#define ALIF_ABS_LONG_MIN         (0-(unsigned long)LONG_MIN)

long alifOS_strToLong(const char* _str, char** _ptr, AlifIntT _base) { // 268
	long result{};
	unsigned long uresult{};
	char sign{};

	while (*_str and ALIF_ISSPACE(*_str))
		_str++;

	sign = *_str;
	if (sign == '+' or sign == '-')
		_str++;

	uresult = alifOS_strToULong(_str, _ptr, _base);

	if (uresult <= (unsigned long)LONG_MAX) {
		result = (long)uresult;
		if (sign == '-')
			result = -result;
	}
	else if (sign == '-' and uresult == ALIF_ABS_LONG_MIN) {
		result = LONG_MIN;
	}
	else {
		errno = ERANGE;
		result = LONG_MAX;
	}
	return result;
}
