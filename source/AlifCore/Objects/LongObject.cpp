#include "alif.h"

#include "AlifCore_BitUtils.h"
#include "AlifCore_Long.h"
#include "AlifCore_Object.h"
#include "AlifCore_Abstract.h"


#define MEDIUM_VALUE(_x) ((stwodigits)alifLong_compactValue(_x)) // 23

#define IS_SMALL_INT(_iVal) (-ALIF_NSMALLNEGINTS <= (_iVal) and (_iVal) < ALIF_NSMALLPOSINTS) // 25
#define IS_SMALL_UINT(_iVal) ((_iVal) < ALIF_NSMALLPOSINTS)


//#define WITH_ALIFLONG_MODULE 1 // 32



static inline AlifIntT is_mediumInt(stwodigits _x) { // 41
	/* Take care that we are comparing unsigned values. */
	twodigits xPlusMask = ((twodigits)_x) + ALIFLONG_MASK;
	return xPlusMask < ((twodigits)ALIFLONG_MASK) + ALIFLONG_BASE;
}


static AlifObject* get_smallInt(sdigit _iVal) { // 50
	return (AlifObject*)&ALIFLONG_SMALL_INTS[ALIF_NSMALLNEGINTS + _iVal];
}

static AlifLongObject* maybe_smallLong(AlifLongObject* _v) { // 56
	if (_v and alifLong_isCompact(_v)) {
		stwodigits ival = MEDIUM_VALUE(_v);
		if (IS_SMALL_INT(ival)) {
			ALIF_DECREF(_v); //_ALIF_DECREF_INT(_v);
			return (AlifLongObject*)get_smallInt((sdigit)ival);
		}
	}
	return _v;
}


static AlifLongObject* long_normalize(AlifLongObject* _v) { // 114
	AlifSizeT j = alifLong_digitCount(_v);
	AlifSizeT i_ = j;

	while (i_ > 0 and _v->longValue.digit[i_ - 1] == 0)
		--i_;
	if (i_ != j) {
		if (i_ == 0) {
			_alifLong_setSignAndDigitCount(_v, 0, 0);
		}
		else {
			_alifLong_setDigitCount(_v, i_);
		}
	}
	return _v;
}

 // 136
#define MAX_LONG_DIGITS \
    ((ALIF_SIZET_MAX - offsetof(AlifLongObject, longValue.digit))/sizeof(digit))

AlifLongObject* alifLong_new(AlifSizeT _size) { // 139
	AlifLongObject* result{};
	if (_size > (AlifSizeT)MAX_LONG_DIGITS) {
		//alifErr_setString(_alifExcOverflowError_,
		//	"too many digits in integer");
		return nullptr;
	}
	/* Fast operations for single digit integers (including zero)
	 * assume that there is always at least one digit present. */
	AlifSizeT ndigits = _size ? _size : 1;
	/* Number of bytes needed is: offsetof(AlifLongObject, digit) +
	   sizeof(digit)*size.  Previous incarnations of this code used
	   sizeof() instead of the offsetof, but this risks being
	   incorrect in the presence of padding between the header
	   and the digits. */
	result = (AlifLongObject*)alifMem_objAlloc(offsetof(AlifLongObject, longValue.digit) +
		ndigits * sizeof(digit));
	if (!result) {
		//alifErr_noMemory();
		return nullptr;
	}
	_alifLong_setSignAndDigitCount(result, _size != 0, _size);
	_alifObject_init((AlifObject*)result, &_alifLongType_);
	/* The digit has to be initialized explicitly to avoid
	 * use-of-uninitialized-value. */
	result->longValue.digit[0] = 0;
	return result;
}

AlifLongObject* _alifLong_fromDigits(AlifIntT _negative,
	AlifSizeT _digitCount, digit* _digits) { // 172
	if (_digitCount == 0) {
		return (AlifLongObject*)_alifLong_getZero();
	}
	AlifLongObject* result = alifLong_new(_digitCount);
	if (result == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	_alifLong_setSignAndDigitCount(result, _negative ? -1 : 1, _digitCount);
	memcpy(result->longValue.digit, _digits, _digitCount * sizeof(digit));
	return result;
}

AlifObject* _alifLong_copy(AlifLongObject* _src) { // 189
	if (alifLong_isCompact(_src)) {
		stwodigits iVal = MEDIUM_VALUE(_src);
		if (IS_SMALL_INT(iVal)) {
			return get_smallInt((sdigit)iVal);
		}
	}
	AlifSizeT size = alifLong_digitCount(_src);
	return (AlifObject*)_alifLong_fromDigits(_alifLong_isNegative(_src), size, _src->longValue.digit);
}



static AlifObject* _alifLong_fromMedium(sdigit _x) { // 203
	/* We could use a freelist here */
	AlifLongObject* v_ = (AlifLongObject*)alifMem_objAlloc(sizeof(AlifLongObject));
	if (v_ == nullptr) {
		//alifErr_noMemory();
		return nullptr;
	}
	digit abs_x = _x < 0 ? -_x : _x;
	_alifLong_setSignAndDigitCount(v_, _x < 0 ? -1 : 1, 1);
	_alifObject_init((AlifObject*)v_, &_alifLongType_);
	v_->longValue.digit[0] = abs_x;
	return (AlifObject*)v_;
}

static AlifObject* _alifLong_fromLarge(stwodigits _iVal) { // 221
	twodigits absIVal{};
	AlifIntT sign{};

	if (_iVal < 0) {
		/* negate: can't write this as abs_ival = -ival since that
		   invokes undefined behaviour when ival is LONG_MIN */
		absIVal = 0U - (twodigits)_iVal;
		sign = -1;
	}
	else {
		absIVal = (twodigits)_iVal;
		sign = 1;
	}
	/* Must be at least two digits */
	twodigits t = absIVal >> (ALIFLONG_SHIFT * 2);
	AlifSizeT ndigits = 2;
	while (t) {
		++ndigits;
		t >>= ALIFLONG_SHIFT;
	}
	AlifLongObject* v = alifLong_new(ndigits);
	if (v != nullptr) {
		digit* p = v->longValue.digit;
		_alifLong_setSignAndDigitCount(v, sign, ndigits);
		t = absIVal;
		while (t) {
			*p++ = ALIF_SAFE_DOWNCAST(
				t & ALIFLONG_MASK, twodigits, digit);
			t >>= ALIFLONG_SHIFT;
		}
	}
	return (AlifObject*)v;
}

static inline AlifObject* _alifLong_fromSTwoDigits(stwodigits _x) { // 261
	if (IS_SMALL_INT(_x)) {
		return get_smallInt((sdigit)_x);
	}
	if (is_mediumInt(_x)) {
		return _alifLong_fromMedium((sdigit)_x);
	}
	return _alifLong_fromLarge(_x);
}


AlifObject* alifLong_fromLong(long _iVal) { // 293
	AlifLongObject* v_{};
	unsigned long absIVal{}, t_{};
	AlifIntT ndigits{};

	/* Handle small and medium cases. */
	if (IS_SMALL_INT(_iVal)) {
		return get_smallInt((sdigit)_iVal);
	}
	if (-(long)ALIFLONG_MASK <= _iVal and _iVal <= (long)ALIFLONG_MASK) {
		return _alifLong_fromMedium((sdigit)_iVal);
	}

	/* Count digits (at least two - smaller cases were handled above). */
	absIVal = _iVal < 0 ? 0U - (unsigned long)_iVal : (unsigned long)_iVal;
	/* Do shift in two steps to avoid possible undefined behavior. */
	t_ = absIVal >> ALIFLONG_SHIFT >> ALIFLONG_SHIFT;
	ndigits = 2;
	while (t_) {
		++ndigits;
		t_ >>= ALIFLONG_SHIFT;
	}

	/* Construct output value. */
	v_ = alifLong_new(ndigits);
	if (v_ != nullptr) {
		digit* p = v_->longValue.digit;
		_alifLong_setSignAndDigitCount(v_, _iVal < 0 ? -1 : 1, ndigits);
		t_ = absIVal;
		while (t_) {
			*p++ = (digit)(t_ & ALIFLONG_MASK);
			t_ >>= ALIFLONG_SHIFT;
		}
	}
	return (AlifObject*)v_;
}


 // 332
#define ALIFLONG_FROM_UINT(_intType, _iVal) \
    do { \
        if (IS_SMALL_UINT(_iVal)) { \
            return get_smallInt((sdigit)(_iVal)); \
        } \
        /* Count the number of Alif digits. */ \
        AlifSizeT ndigits = 0; \
        _intType t = (_iVal); \
        while (t) { \
            ++ndigits; \
            t >>= ALIFLONG_SHIFT; \
        } \
        AlifLongObject *v_ = alifLong_new(ndigits); \
        if (v_ == nullptr) { \
            return nullptr; \
        } \
        digit *p = v_->longValue.digit; \
        while ((_iVal)) { \
            *p++ = (digit)((_iVal) & ALIFLONG_MASK); \
            (_iVal) >>= ALIFLONG_SHIFT; \
        } \
        return (AlifObject *)v_; \
    } while(0)


AlifObject* alifLong_fromUnsignedLong(unsigned long _ival) { // 358
	ALIFLONG_FROM_UINT(unsigned long, _ival);
}

AlifObject* alifLong_fromUnsignedLongLong(unsigned long long _ival) { // 366
	ALIFLONG_FROM_UINT(unsigned long long, _ival);
}



// 446
#define ALIF_ABS_LONG_MIN       (0-(unsigned long)LONG_MIN)
#define ALIF_ABS_SIZET_MIN      (0-(AlifUSizeT)ALIF_SIZET_MIN)


long alifLong_asLongAndOverflow(AlifObject* _vv, AlifIntT* _overflow) { // 460
	AlifLongObject* v_{};
	unsigned long x_{}, prev{};
	long res_{};
	AlifSizeT i_{};
	AlifIntT sign{};
	AlifIntT doDecref = 0;

	*_overflow = 0;
	if (_vv == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	if (ALIFLONG_CHECK(_vv)) {
		v_ = (AlifLongObject*)_vv;
	}
	else {
		v_ = (AlifLongObject*)_alifNumber_index(_vv);
		if (v_ == nullptr)
			return -1;
		doDecref = 1;
	}
	if (alifLong_isCompact(v_)) {
#if SIZEOF_LONG < SIZEOF_SIZE_T
		AlifSizeT tmp = alifLong_compactValue(v_);
		if (tmp < LONG_MIN) {
			*_overflow = -1;
			res_ = -1;
		}
		else if (tmp > LONG_MAX) {
			*_overflow = 1;
			res_ = -1;
		}
		else {
			res_ = (long)tmp;
		}
#else
		res_ = alifLong_compactValue(v_);
#endif
	}
	else {
		res_ = -1;
		i_ = alifLong_digitCount(v_);
		sign = alifLong_nonCompactSign(v_);
		x_ = 0;
		while (--i_ >= 0) {
			prev = x_;
			x_ = (x_ << ALIFLONG_SHIFT) | v_->longValue.digit[i_];
			if ((x_ >> ALIFLONG_SHIFT) != prev) {
				*_overflow = sign;
				goto exit;
			}
		}
		
		if (x_ <= (unsigned long)LONG_MAX) {
			res_ = (long)x_ * sign;
		}
		else if (sign < 0 and x_ == ALIF_ABS_LONG_MIN) {
			res_ = LONG_MIN;
		}
		else {
			*_overflow = sign;
		}
	}
exit:
	if (doDecref) {
		ALIF_DECREF(v_);
	}
	return res_;
}

long alifLong_asLong(AlifObject* _obj) { // 540
	AlifIntT overflow{};
	long result = alifLong_asLongAndOverflow(_obj, &overflow);
	if (overflow) {
		//alifErr_setString(_alifExcOverflowError_,
			//"alif AlifIntT too large to convert to C long");
	}
	return result;
}


AlifSizeT alifLong_asSizeT(AlifObject* _vv) { // 575
	AlifLongObject* v_{};
	AlifUSizeT x_{}, prev{};
	AlifSizeT i_;
	AlifIntT sign{};

	if (_vv == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	if (!ALIFLONG_CHECK(_vv)) {
		//alifErr_setString(_alifExcTypeError_, "an integer is required");
		return -1;
	}

	v_ = (AlifLongObject*)_vv;
	if (alifLong_isCompact(v_)) {
		return alifLong_compactValue(v_);
	}
	i_ = alifLong_digitCount(v_);
	sign = alifLong_nonCompactSign(v_);
	x_ = 0;
	while (--i_ >= 0) {
		prev = x_;
		x_ = (x_ << ALIFLONG_SHIFT) | v_->longValue.digit[i_];
		if ((x_ >> ALIFLONG_SHIFT) != prev)
			goto overflow;
	}
	if (x_ <= (AlifUSizeT)ALIF_SIZET_MAX) {
		return (AlifSizeT)x_ * sign;
	}
	else if (sign < 0 and x_ == ALIF_ABS_SIZET_MIN) {
		return ALIF_SIZET_MIN;
	}
	/* else overflow */

overflow:
	//alifErr_setString(_alifExcOverflowError_,
	//	"alif AlifIntT too large to convert to CPP AlifUSizeT");
	return -1;
}


static AlifIntT bit_lengthDigit(digit _x) { // 797
	// digit can be larger than unsigned long, but only ALIFLONG_SHIFT bits
	// of it will be ever used.
	static_assert(ALIFLONG_SHIFT <= sizeof(unsigned long) * 8,
		"digit is larger than unsigned long");
	return alifBit_length((unsigned long)_x);
}

AlifUSizeT _alifLong_numBits(AlifObject* _vv) { // 807
	AlifLongObject* v = (AlifLongObject*)_vv;
	AlifUSizeT result = 0;
	AlifSizeT ndigits{};
	AlifIntT msdBits{};

	ndigits = alifLong_digitCount(v);
	if (ndigits > 0) {
		digit msd = v->longValue.digit[ndigits - 1];
		if ((AlifUSizeT)(ndigits - 1) > SIZE_MAX / (AlifUSizeT)ALIFLONG_SHIFT)
			goto Overflow;
		result = (AlifUSizeT)(ndigits - 1) * (AlifUSizeT)ALIFLONG_SHIFT;
		msdBits = bit_lengthDigit(msd);
		if (SIZE_MAX - msdBits < result)
			goto Overflow;
		result += msdBits;
	}
	return result;

Overflow:
	//alifErr_setString(_alifExcOverflowError_, "int has too many bits "
	//	"to express in a platform AlifUSizeT");
	return (AlifUSizeT)-1;
}



AlifObject* alifLong_fromVoidPtr(void* _p) { // 1349
#if SIZEOF_VOID_P <= SIZEOF_LONG
	return alifLong_fromUnsignedLong((unsigned long)(uintptr_t)_p);
#else

#if SIZEOF_LONG_LONG < SIZEOF_VOID_P
#   error "alifLong_fromVoidPtr: sizeof(long long) < sizeof(void*)"
#endif
	return alifLong_fromUnsignedLongLong((unsigned long long)(uintptr_t)_p);
#endif /* SIZEOF_VOID_P <= SIZEOF_LONG */

}



 // 1820
#define CHECK_BINOP(_v,_w)									\
    do {													\
        if (!ALIFLONG_CHECK(_v) or !ALIFLONG_CHECK(_w))     \
            return ALIF_NOTIMPLEMENTED;						\
    } while(0)






unsigned char _alifLongDigitValue_[256] = { // 2527
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
	0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  37, 37, 37, 37, 37, 37,
	37, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 37, 37, 37, 37, 37,
	37, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
	37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
};


static AlifIntT long_fromBinaryBase(const char* _start,
	const char* _end, AlifSizeT _digits, AlifIntT _base, AlifLongObject** _res) { // 2559
	const char* p_{};
	AlifIntT bitsPerChar{};
	AlifSizeT n_{};
	AlifLongObject* z_{};
	twodigits accum{};
	AlifIntT bitsInAccum{};
	digit* pDigit{};

	n_ = _base;
	for (bitsPerChar = -1; n_; ++bitsPerChar) {
		n_ >>= 1;
	}

	/* n <- the number of alif digits needed,
			= ceiling((digits * bits_per_char) / ALIFLONG_SHIFT). */
	if (_digits > (ALIF_SIZET_MAX - (ALIFLONG_SHIFT - 1)) / bitsPerChar) {
		//alifErr_setString(_alifExcValueError_,
		//	"AlifIntT string too large to convert");
		*_res = nullptr;
		return 0;
	}
	n_ = (_digits * bitsPerChar + ALIFLONG_SHIFT - 1) / ALIFLONG_SHIFT;
	z_ = alifLong_new(n_);
	if (z_ == nullptr) {
		*_res = nullptr;
		return 0;
	}
	/* Read string from right, and fill in AlifIntT from left; i.e.,
	 * from least to most significant in both.
	 */
	accum = 0;
	bitsInAccum = 0;
	pDigit = z_->longValue.digit;
	p_ = _end;
	while (--p_ >= _start) {
		AlifIntT k;
		if (*p_ == '_') {
			continue;
		}
		k = (AlifIntT)_alifLongDigitValue_[ALIF_CHARMASK(*p_)];
		accum |= (twodigits)k << bitsInAccum;
		bitsInAccum += bitsPerChar;
		if (bitsInAccum >= ALIFLONG_SHIFT) {
			*pDigit++ = (digit)(accum & ALIFLONG_MASK);
			accum >>= ALIFLONG_SHIFT;
			bitsInAccum -= ALIFLONG_SHIFT;
		}
	}
	if (bitsInAccum) {
		*pDigit++ = (digit)accum;
	}
	while (pDigit - z_->longValue.digit < n_) *pDigit++ = 0;
	*_res = z_;
	return 0;
}


static AlifIntT long_fromNonBinaryBase(const char* _start,
	const char* _end, AlifSizeT _digits, AlifIntT _base, AlifLongObject** _res) { // 2749
	twodigits c_{};           /* current input character */
	AlifSizeT sizeZ{};
	AlifIntT i_{};
	AlifIntT convwidth{};
	twodigits convmultmax, convmult;
	digit* pz{}, * pzstop{};
	AlifLongObject* z_{};
	const char* p_{};

	static double log_base_BASE[37] = { 0.0e0, };
	static AlifIntT convwidth_base[37] = { 0, };
	static twodigits convmultmax_base[37] = { 0, };

	if (log_base_BASE[_base] == 0.0) {
		twodigits convmax = _base;
		AlifIntT i_ = 1;

		log_base_BASE[_base] = (log((double)_base) /
			log((double)ALIFLONG_BASE));
		for (;;) {
			twodigits next = convmax * _base;
			if (next > ALIFLONG_BASE) {
				break;
			}
			convmax = next;
			++i_;
		}
		convmultmax_base[_base] = convmax;
		convwidth_base[_base] = i_;
	}

	double fsize_z = (double)_digits * log_base_BASE[_base] + 1.0;
	if (fsize_z > (double)MAX_LONG_DIGITS) {
		/* The same exception as in _PyLong_New(). */
		//alifErr_setString(_alifExcOverflowError_,
		//	"too many digits in integer");
		*_res = nullptr;
		return 0;
	}
	sizeZ = (AlifSizeT)fsize_z;
	/* Uncomment next line to test exceedingly rare copy code */
	/* size_z = 1; */
	z_ = alifLong_new(sizeZ);
	if (z_ == nullptr) {
		*_res = nullptr;
		return 0;
	}
	_alifLong_setSignAndDigitCount(z_, 0, 0);

	/* `convwidth` consecutive input digits are treated as a single
	 * digit in base `convmultmax`.
	 */
	convwidth = convwidth_base[_base];
	convmultmax = convmultmax_base[_base];

	/* Work ;-) */
	p_ = _start;
	while (p_ < _end) {
		if (*p_ == '_') {
			p_++;
			continue;
		}
		/* grab up to convwidth digits from the input string */
		c_ = (digit)_alifLongDigitValue_[ALIF_CHARMASK(*p_++)];
		for (i_ = 1; i_ < convwidth and p_ != _end; ++p_) {
			if (*p_ == '_') {
				continue;
			}
			i_++;
			c_ = (twodigits)(c_ * _base +
				(AlifIntT)_alifLongDigitValue_[ALIF_CHARMASK(*p_)]);
		}

		convmult = convmultmax;
		/* Calculate the shift only if we couldn't get
		 * convwidth digits.
		 */
		if (i_ != convwidth) {
			convmult = _base;
			for (; i_ > 1; --i_) {
				convmult *= _base;
			}
		}

		/* Multiply z by convmult, and add c. */
		pz = z_->longValue.digit;
		pzstop = pz + alifLong_digitCount(z_);
		for (; pz < pzstop; ++pz) {
			c_ += (twodigits)*pz * convmult;
			*pz = (digit)(c_ & ALIFLONG_MASK);
			c_ >>= ALIFLONG_SHIFT;
		}
		/* carry off the current end? */
		if (c_) {
			if (alifLong_digitCount(z_) < sizeZ) {
				*pz = (digit)c_;
				_alifLong_setSignAndDigitCount(z_, 1, alifLong_digitCount(z_) + 1);
			}
			else {
				AlifLongObject* tmp{};
				/* Extremely rare.  Get more space. */
				tmp = alifLong_new(sizeZ + 1);
				if (tmp == nullptr) {
					ALIF_DECREF(z_);
					*_res = nullptr;
					return 0;
				}
				memcpy(tmp->longValue.digit,
					z_->longValue.digit,
					sizeof(digit) * sizeZ);
				ALIF_SETREF(z_, tmp);
				z_->longValue.digit[sizeZ] = (digit)c_;
				++sizeZ;
			}
		}
	}
	*_res = z_;
	return 0;
}


static AlifIntT long_fromStringBase(const char** _str,
	AlifIntT _base, AlifLongObject** _res) { // 2902
	const char* start{}, * end{}, * p{};
	char prev = 0;
	AlifSizeT digits = 0;
	AlifIntT isBinaryBase = (_base & (_base - 1)) == 0;

	/* Here we do four things:
	 *
	 * - Find the `end` of the string.
	 * - Validate the string.
	 * - Count the number of `digits` (rather than underscores)
	 * - Point *str to the end-of-string or first invalid character.
	 */
	start = p = *_str;
	/* Leading underscore not allowed. */
	if (*start == '_') {
		return -1;
	}
	/* Verify all characters are digits and underscores. */
	while (_alifLongDigitValue_[ALIF_CHARMASK(*p)] < _base or *p == '_') {
		if (*p == '_') {
			/* Double underscore not allowed. */
			if (prev == '_') {
				*_str = p - 1;
				return -1;
			}
		}
		else {
			++digits;
		}
		prev = *p;
		++p;
	}
	/* Trailing underscore not allowed. */
	if (prev == '_') {
		*_str = p - 1;
		return -1;
	}
	*_str = end = p;
	/* Reject empty strings */
	if (start == end) {
		return -1;
	}
	/* Allow only trailing whitespace after `end` */
	while (*p and ALIF_ISSPACE(*p)) {
		p++;
	}
	*_str = p;
	if (*p != '\0') {
		return -1;
	}

	/*
	 * Pass a validated string consisting of only valid digits and underscores
	 * to long_from_xxx_base.
	 */
	if (isBinaryBase) {
		/* Use the linear algorithm for binary bases. */
		return long_fromBinaryBase(start, end, digits, _base, _res);
	}
	else {
		/* Limit the size to avoid excessive computation attacks exploiting the
		 * quadratic algorithm. */
		if (digits > ALIF_LONG_MAX_STR_DIGITS_THRESHOLD) {
			AlifInterpreter* interp = _alifInterpreter_get();
			AlifIntT maxStrDigits = interp->longState.maxStrDigits;
			if ((maxStrDigits > 0) && (digits > maxStrDigits)) {
				//alifErr_format(_alifExcValueError_, _MAX_STR_DIGITS_ERROR_FMT_TO_INT,
				//	max_str_digits, digits);
				*_res = nullptr;
				return 0;
			}
		}
//#if WITH_ALIFLONG_MODULE
//		if (digits > 6000 and _base == 10) {
//			/* Switch to _pylong.int_from_string() */
//			return alifLong_intFromString(start, end, _res);
//		}
//#endif
		/* Use the quadratic algorithm for non binary bases. */
		return long_fromNonBinaryBase(start, end, digits, _base, _res);
	}
}



AlifObject* alifLong_fromString(const char* _str, char** _pend, AlifIntT _base) { // 2995
	AlifIntT sign = 1, errorIfNonZero = 0;
	const char* orig_str = _str;
	AlifLongObject* z_ = nullptr;
	AlifObject* strObj{};
	AlifSizeT slen{};

	if ((_base != 0 and _base < 2) or _base > 36) {
		//alifErr_setString(_alifExcValueError_,
		//	"AlifIntT() arg 2 must be >= 2 and <= 36");
		return nullptr;
	}
	while (*_str != '\0' and ALIF_ISSPACE(*_str)) {
		++_str;
	}
	if (*_str == '+') {
		++_str;
	}
	else if (*_str == '-') {
		++_str;
		sign = -1;
	}
	if (_base == 0) {
		if (_str[0] != '0') {
			_base = 10;
		}
		else if (_str[1] == 'x' or _str[1] == 'X') {
			_base = 16;
		}
		else if (_str[1] == 'o' or _str[1] == 'O') {
			_base = 8;
		}
		else if (_str[1] == 'b' or _str[1] == 'B') {
			_base = 2;
		}
		else {
			/* "old" (C-style) octal literal, now invalid.
			   it might still be zero though */
			errorIfNonZero = 1;
			_base = 10;
		}
	}
	if (_str[0] == '0' and
		((_base == 16 and (_str[1] == 'x' or _str[1] == 'X')) or
			(_base == 8 and (_str[1] == 'o' or _str[1] == 'O')) or
			(_base == 2 and (_str[1] == 'b' or _str[1] == 'B')))) {
		_str += 2;
		/* One underscore allowed here. */
		if (*_str == '_') {
			++_str;
		}
	}

	/* long_fromStringBase is the main workhorse here. */
	AlifIntT ret = long_fromStringBase(&_str, _base, &z_);
	if (ret == -1) {
		/* Syntax error. */
		goto onError;
	}
	if (z_ == nullptr) {
		/* Error. exception already set. */
		return nullptr;
	}

	if (errorIfNonZero) {
		/* reset the base to 0, else the exception message
		   doesn't make too much sense */
		_base = 0;
		if (!_alifLong_isZero(z_)) {
			goto onError;
		}
		/* there might still be other problems, therefore base
		   remains zero here for the same reason */
	}

	/* Set sign and normalize */
	if (sign < 0) {
		_alifLong_flipSign(z_);
	}
	long_normalize(z_);
	z_ = maybe_smallLong(z_);

	if (_pend != nullptr) {
		*_pend = (char*)_str;
	}
	return (AlifObject*)z_;

onError:
	if (_pend != nullptr) {
		*_pend = (char*)_str;
	}
	ALIF_XDECREF(z_);
	slen = strlen(orig_str) < 200 ? strlen(orig_str) : 200;
	strObj = alifUStr_fromStringAndSize(orig_str, slen);
	if (strObj == nullptr) {
		return nullptr;
	}
	//alifErr_format(_alifExcValueError_,
	//	"invalid literal for AlifIntT() with base %d: %.200R",
	//	_base, strObj);
	ALIF_DECREF(strObj);
	return nullptr;
}





static AlifLongObject* x_add(AlifLongObject* _a, AlifLongObject* _b) { // 3675
	AlifSizeT sizeA = alifLong_digitCount(_a), sizeB = alifLong_digitCount(_b);
	AlifLongObject* z{};
	AlifSizeT i{};
	digit carry = 0;

	/* Ensure a is the larger of the two: */
	if (sizeA < sizeB) {
		{ AlifLongObject* temp = _a; _a = _b; _b = temp; }
		{
			AlifSizeT size_temp = sizeA;
			sizeA = sizeB;
			sizeB = size_temp;
		}
	}
	z = alifLong_new(sizeA + 1);
	if (z == nullptr) return nullptr;
	for (i = 0; i < sizeB; ++i) {
		carry += _a->longValue.digit[i] + _b->longValue.digit[i];
		z->longValue.digit[i] = carry & ALIFLONG_MASK;
		carry >>= ALIFLONG_SHIFT;
	}
	for (; i < sizeA; ++i) {
		carry += _a->longValue.digit[i];
		z->longValue.digit[i] = carry & ALIFLONG_MASK;
		carry >>= ALIFLONG_SHIFT;
	}
	z->longValue.digit[i] = carry;
	return long_normalize(z);
}

static AlifLongObject* x_sub(AlifLongObject* a, AlifLongObject* b) { // 3709
	AlifSizeT sizeA = alifLong_digitCount(a), sizeB = alifLong_digitCount(b);
	AlifLongObject* z{};
	AlifSizeT i{};
	AlifIntT sign = 1;
	digit borrow = 0;

	/* Ensure a is the larger of the two: */
	if (sizeA < sizeB) {
		sign = -1;
		{ AlifLongObject* temp = a; a = b; b = temp; }
		{
			AlifSizeT size_temp = sizeA;
			sizeA = sizeB;
			sizeB = size_temp;
		}
	}
	else if (sizeA == sizeB) {
		/* Find highest digit where a and b differ: */
		i = sizeA;
		while (--i >= 0 and a->longValue.digit[i] == b->longValue.digit[i])
			;
		if (i < 0)
			return (AlifLongObject*)alifLong_fromLong(0);
		if (a->longValue.digit[i] < b->longValue.digit[i]) {
			sign = -1;
			{ AlifLongObject* temp = a; a = b; b = temp; }
		}
		sizeA = sizeB = i + 1;
	}
	z = alifLong_new(sizeA);
	if (z == nullptr) return nullptr;
	for (i = 0; i < sizeB; ++i) {
		/* The following assumes unsigned arithmetic
		   works module 2**N for some N>ALIFLONG_SHIFT. */
		borrow = a->longValue.digit[i] - b->longValue.digit[i] - borrow;
		z->longValue.digit[i] = borrow & ALIFLONG_MASK;
		borrow >>= ALIFLONG_SHIFT;
		borrow &= 1; /* Keep only one sign bit */
	}
	for (; i < sizeA; ++i) {
		borrow = a->longValue.digit[i] - borrow;
		z->longValue.digit[i] = borrow & ALIFLONG_MASK;
		borrow >>= ALIFLONG_SHIFT;
		borrow &= 1; /* Keep only one sign bit */
	}
	if (sign < 0) {
		_alifLong_flipSign(z);
	}
	return maybe_smallLong(long_normalize(z));
}


AlifObject* _alifLong_add(AlifLongObject* _a, AlifLongObject* _b) { // 3763
	if (_alifLong_bothAreCompact(_a, _b)) {
		return _alifLong_fromSTwoDigits(MEDIUM_VALUE(_a) + MEDIUM_VALUE(_b));
	}

	AlifLongObject* z{};
	if (_alifLong_isNegative(_a)) {
		if (_alifLong_isNegative(_b)) {
			z = x_add(_a, _b);
			if (z != nullptr) {
				/* x_add received at least one multiple-digit int,
				   and thus z must be a multiple-digit int.
				   That also means z is not an element of
				   small_ints, so negating it in-place is safe. */
				_alifLong_flipSign(z);
			}
		}
		else
			z = x_sub(_b, _a);
	}
	else {
		if (_alifLong_isNegative(_b))
			z = x_sub(_a, _b);
		else
			z = x_add(_a, _b);
	}
	return (AlifObject*)z;
}


static AlifObject* long_add(AlifLongObject* _a, AlifLongObject* _b) { // 3795
	CHECK_BINOP(_a, _b);
	return _alifLong_add(_a, _b);
}











static AlifObject* long_long(AlifObject* _v) { // 5630
	if (ALIFLONG_CHECKEXACT(_v)) {
		return ALIF_NEWREF(_v);
	}
	else {
		return _alifLong_copy((AlifLongObject*)_v);
	}
}










static AlifNumberMethods _longAsNumber_ = { // 6560
	.add_ = (BinaryFunc)long_add,  
	.subtract = (BinaryFunc)long_sub,
	.multiply = (BinaryFunc)long_mul,  
	.remainder = long_mod,              
	.divmod = long_divmod,           
	.power = long_pow,              
	.negative = (UnaryFunc)long_neg,   
	.positive = long_long,             
	.absolute = (UnaryFunc)long_abs,
	.sqrt = long_sqrt,
	.bool_ = (Inquiry)long_bool,    
	.invert = (UnaryFunc)long_invert,
	.lshift = long_lshift,           
	.rshift = long_rshift,           
	.and_ = long_and,              
	.xor_ = long_xor,              
	.or_ = long_or,               
	.int_ = long_long,             
	.reserved = 0,                     
	.float_ = long_float,            
	.inplaceAdd = 0,                     
	.inplaceSubtract = 0,                     
	.inplaceMultiply = 0,                     
	.inplaceRemainder = 0,                     
	.inplacePower = 0,                     
	.inplaceLshift = 0,                     
	.inplaceRshift = 0,                     
	.inplaceAnd = 0,                     
	.inplaceXor = 0,                     
	.inplaceOr = 0,                     
	.floorDivide = long_div,              
	.trueDivide = long_true_divide,      
	.inplaceFloorDivide = 0,                     
	.inplaceTrueDivide = 0,                     
	.index = long_long,
};

AlifTypeObject _alifLongType_ = { // 6597
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "عدد_صحيح",                                   
	.basicSize = offsetof(AlifLongObject, longValue.digit),
	.itemSize = sizeof(digit),                              
	//.dealloc = long_dealloc,                               
                                        
	//.repr = long_toDecimalString,                     
	.asNumber = &_longAsNumber_,
                                        
	//.hash = long_hash,                                  
                                        
	.getAttro = alifObject_genericGetAttr,                    
                                        
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		ALIF_TPFLAGS_LONG_SUBCLASS |
		_ALIF_TPFLAGS_MATCH_SELF,                                                     
                                       
	.free = alifMem_objFree,                            
};
