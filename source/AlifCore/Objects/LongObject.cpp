#include "alif.h"

#include "AlifCore_Abstract.h" //* alif
#include "AlifCore_BitUtils.h"
#include "AlifCore_InitConfig.h"
#include "AlifCore_Call.h"
#include "AlifCore_Long.h"
#include "AlifCore_Object.h"
#include "AlifCore_DureRun.h"

#include <float.h>


#define MEDIUM_VALUE(_x) ((stwodigits)alifLong_compactValue(_x)) // 23

#define IS_SMALL_INT(_iVal) (-ALIF_NSMALLNEGINTS <= (_iVal) and (_iVal) < ALIF_NSMALLPOSINTS) // 25
#define IS_SMALL_UINT(_iVal) ((_iVal) < ALIF_NSMALLPOSINTS)


#define WITH_ALIFLONG_MODULE 1 // 32

static AlifLongObject* long_neg(AlifLongObject*); // 35
static AlifLongObject* x_divrem(AlifLongObject*, AlifLongObject*, AlifLongObject**);
static AlifObject* long_long(AlifObject*);
static AlifObject* long_lShiftInt64(AlifLongObject*, int64_t);

static inline void _alif_decrefInt(AlifLongObject* _op) { // 41
	_alif_decrefSpecialized((AlifObject*)_op, (Destructor)alifMem_objFree);
}

static inline AlifIntT is_mediumInt(stwodigits _x) { // 48
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


#define KARATSUBA_CUTOFF 70 // 73
#define KARATSUBA_SQUARE_CUTOFF (2 * KARATSUBA_CUTOFF) // 74


#define SIGCHECK(_alifTryBlock)							\
    do {												\
        /*if (alifErr_checkSignals()) _alifTryBlock*/		\
    } while(0)

#define EXP_WINDOW_SIZE 5 // 82
#define EXP_TABLE_LEN (1 << (EXP_WINDOW_SIZE - 1)) // 83

#define HUGE_EXP_CUTOFF 60 // 103


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
#if SIZEOF_SIZE_T < 8
	#define MAX_LONG_DIGITS \
		((ALIF_SIZET_MAX - offsetof(AlifLongObject, longValue.digit))/sizeof(digit))
#else
	#define MAX_LONG_DIGITS ((INT64_MAX-1) / ALIFLONG_SHIFT)
#endif

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
	AlifLongObject* v_ = alifLong_new(ndigits);
	if (v_ != nullptr) {
		digit* p = v_->longValue.digit;
		_alifLong_setSignAndDigitCount(v_, sign, ndigits);
		t = absIVal;
		while (t) {
			*p++ = ALIF_SAFE_DOWNCAST(
				t & ALIFLONG_MASK, twodigits, digit);
			t >>= ALIFLONG_SHIFT;
		}
	}
	return (AlifObject*)v_;
}

static inline AlifLongObject* _alifLong_fromSTwoDigits(stwodigits _x) { // 261
	if (IS_SMALL_INT(_x)) {
		return (AlifLongObject*)get_smallInt((sdigit)_x);
	}
	if (is_mediumInt(_x)) {
		return (AlifLongObject*)_alifLong_fromMedium((sdigit)_x);
	}
	return (AlifLongObject*)_alifLong_fromLarge(_x);
}


ALIF_LOCAL_INLINE(void) _alifLong_negate(AlifLongObject** _xP) { // 276
	AlifLongObject* x{};

	x = (AlifLongObject*)*_xP;
	if (ALIF_REFCNT(x) == 1) {
		_alifLong_flipSign(x);
		return;
	}

	*_xP = _alifLong_fromSTwoDigits(-MEDIUM_VALUE(x));
	ALIF_DECREF(x);
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





AlifObject* alifLong_fromDouble(double _dVal) { // 382
	const double intMax = (unsigned long)LONG_MAX + 1;
	if (-intMax < _dVal and _dVal < intMax) {
		return alifLong_fromLong((long)_dVal);
	}

	AlifLongObject* v{};
	double frac{};
	AlifIntT i{}, ndig{}, expo{}, neg{};
	neg = 0;
	if (isinf(_dVal)) {
		//alifErr_setString(_alifExcOverflowError_,
		//	"cannot convert float infinity to integer");
		return nullptr;
	}
	if (isnan(_dVal)) {
		//alifErr_setString(_alifExcValueError_,
		//	"cannot convert float NaN to integer");
		return nullptr;
	}
	if (_dVal < 0.0) {
		neg = 1;
		_dVal = -_dVal;
	}
	frac = frexp(_dVal, &expo); /* dval = frac*2**expo; 0.0 <= frac < 1.0 */
	ndig = (expo - 1) / ALIFLONG_SHIFT + 1; /* Number of 'digits' in result */
	v = alifLong_new(ndig);
	if (v == nullptr) return nullptr;
	frac = ldexp(frac, (expo - 1) % ALIFLONG_SHIFT + 1);
	for (i = ndig; --i >= 0; ) {
		digit bits = (digit)frac;
		v->longValue.digit[i] = bits;
		frac = frac - (double)bits;
		frac = ldexp(frac, ALIFLONG_SHIFT);
	}
	if (neg) {
		_alifLong_flipSign(v);
	}
	return (AlifObject*)v;
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

AlifIntT alifLong_asInt(AlifObject* _obj) { // 557
	AlifIntT overflow{};
	long result = alifLong_asLongAndOverflow(_obj, &overflow);
	if (overflow or result > INT_MAX or result < INT_MIN) {
		/* XXX: could be cute and give a different
		   message for overflow == -1 */
		//alifErr_setString(_alifExcOverflowError_,
		//	"Alif AlifIntT too large to convert to Cpp AlifIntT");
		return -1;
	}
	return (AlifIntT)result;
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


static unsigned long _alifLong_asUnsignedLongMask(AlifObject* _vv) { // 721
	AlifLongObject* v_{};
	unsigned long x_{};
	AlifSizeT i_{};

	if (_vv == nullptr or !ALIFLONG_CHECK(_vv)) {
		//ALIFERR_BADINTERNALCALL();
		return (unsigned long)-1;
	}
	v_ = (AlifLongObject*)_vv;
	if (alifLong_isCompact(v_)) {
#if SIZEOF_LONG < SIZEOF_SIZE_T
		return (unsigned long)(AlifUSizeT)alifLong_compactValue(v_);
#else
		return (unsigned long)(long)alifLong_compactValue(v_);
#endif
	}
	i_ = alifLong_digitCount(v_);
	AlifIntT sign = alifLong_nonCompactSign(v_);
	x_ = 0;
	while (--i_ >= 0) {
		x_ = (x_ << ALIFLONG_SHIFT) | v_->longValue.digit[i_];
	}
	return x_ * sign;
}

unsigned long alifLong_asUnsignedLongMask(AlifObject* _op) { // 748
	AlifLongObject* lo{};
	unsigned long val{};

	if (_op == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return (unsigned long)-1;
	}

	if (ALIFLONG_CHECK(_op)) {
		return _alifLong_asUnsignedLongMask(_op);
	}

	lo = (AlifLongObject*)_alifNumber_index(_op);
	if (lo == nullptr)
		return (unsigned long)-1;

	val = _alifLong_asUnsignedLongMask((AlifObject*)lo);
	ALIF_DECREF(lo);
	return val;
}




AlifIntT _alifLong_sign(AlifObject* _vv) { // 772
	AlifLongObject* v = (AlifLongObject*)_vv;

	if (alifLong_isCompact(v)) {
		return _alifLong_compactSign(v);
	}
	return alifLong_nonCompactSign(v);
}


static AlifIntT bit_lengthDigit(digit _x) { // 797
	// digit can be larger than unsigned long, but only ALIFLONG_SHIFT bits
	// of it will be ever used.
	static_assert(ALIFLONG_SHIFT <= sizeof(unsigned long) * 8,
		"digit is larger than unsigned long");
	return alifBit_length((unsigned long)_x);
}

int64_t _alifLong_numBits(AlifObject* _vv) { // 807
	AlifLongObject* v_ = (AlifLongObject*)_vv;
	int64_t result = 0;
	AlifSizeT ndigits{};
	AlifIntT msdBits{};

	ndigits = alifLong_digitCount(v_);
	if (ndigits > 0) {
		digit msd = v_->longValue.digit[ndigits - 1];
		result = (int64_t)(ndigits - 1) * ALIFLONG_SHIFT;
		msdBits = bit_lengthDigit(msd);
		result += msdBits;
	}
	return result;
}

AlifObject* _alifLong_fromByteArray(const unsigned char* _bytes, AlifUSizeT _n,
	AlifIntT _littleEndian, AlifIntT _isSigned) { // 844
	const unsigned char* pstartbyte{};    /* LSB of bytes */
	AlifIntT incr{};                           /* direction to move pstartbyte */
	const unsigned char* pendbyte;      /* MSB of bytes */
	AlifUSizeT numsignificantbytes;         /* number of bytes that matter */
	AlifSizeT ndigits{};                 /* number of Alif int digits */
	AlifLongObject* v{};                    /* result */
	AlifSizeT idigit = 0;              /* next free index in v->longValue.digit */

	if (_n == 0)
		return alifLong_fromLong(0L);

	if (_littleEndian) {
		pstartbyte = _bytes;
		pendbyte = _bytes + _n - 1;
		incr = 1;
	}
	else {
		pstartbyte = _bytes + _n - 1;
		pendbyte = _bytes;
		incr = -1;
	}

	if (_isSigned)
		_isSigned = *pendbyte >= 0x80;

	/* Compute numsignificantbytes.  This consists of finding the most
	   significant byte.  Leading 0 bytes are insignificant if the number
	   is positive, and leading 0xff bytes if negative. */
	{
		AlifUSizeT i;
		const unsigned char* p = pendbyte;
		const AlifIntT pincr = -incr;  /* search MSB to LSB */
		const unsigned char insignificant = _isSigned ? 0xff : 0x00;

		for (i = 0; i < _n; ++i, p += pincr) {
			if (*p != insignificant)
				break;
		}
		numsignificantbytes = _n - i;
		/* 2's-comp is a bit tricky here, e.g. 0xff00 == -0x0100, so
		   actually has 2 significant bytes.  OTOH, 0xff0001 ==
		   -0x00ffff, so we wouldn't *need* to bump it there; but we
		   do for 0xffff = -0x0001.  To be safe without bothering to
		   check every case, bump it regardless. */
		if (_isSigned and numsignificantbytes < _n)
			++numsignificantbytes;
	}

	/* How many Alif int digits do we need?  We have
	   8*numsignificantbytes bits, and each Alif int digit has
	   ALIFLONG_SHIFT bits, so it's the ceiling of the quotient. */
	   /* catch overflow before it happens */
	if (numsignificantbytes > (ALIF_SIZET_MAX - ALIFLONG_SHIFT) / 8) {
		//alifErr_setString(_alifExcOverflowError_,
		//	"byte array too long to convert to int");
		return nullptr;
	}
	ndigits = (numsignificantbytes * 8 + ALIFLONG_SHIFT - 1) / ALIFLONG_SHIFT;
	v = alifLong_new(ndigits);
	if (v == nullptr)
		return nullptr;

	/* Copy the bits over.  The tricky parts are computing 2's-comp on
	   the fly for signed numbers, and dealing with the mismatch between
	   8-bit bytes and (probably) 15-bit Alif digits.*/
	{
		AlifUSizeT i{};
		twodigits carry = 1;                    /* for 2's-comp calculation */
		twodigits accum = 0;                    /* sliding register */
		AlifUIntT accumbits = 0;             /* number of bits in accum */
		const unsigned char* p = pstartbyte;

		for (i = 0; i < numsignificantbytes; ++i, p += incr) {
			twodigits thisbyte = *p;
			/* Compute correction for 2's comp, if needed. */
			if (_isSigned) {
				thisbyte = (0xff ^ thisbyte) + carry;
				carry = thisbyte >> 8;
				thisbyte &= 0xff;
			}
			/* Because we're going LSB to MSB, thisbyte is
			   more significant than what's already in accum,
			   so needs to be prepended to accum. */
			accum |= thisbyte << accumbits;
			accumbits += 8;
			if (accumbits >= ALIFLONG_SHIFT) {
				/* There's enough to fill a Alif digit. */
				v->longValue.digit[idigit] = (digit)(accum & ALIFLONG_MASK);
				++idigit;
				accum >>= ALIFLONG_SHIFT;
				accumbits -= ALIFLONG_SHIFT;
			}
		}
		if (accumbits) {
			v->longValue.digit[idigit] = (digit)accum;
			++idigit;
		}
	}

	int sign = _isSigned ? -1 : 1;
	if (idigit == 0) {
		sign = 0;
	}
	_alifLong_setSignAndDigitCount(v, sign, idigit);
	return (AlifObject*)maybe_smallLong(long_normalize(v));
}

AlifIntT _alifLong_asByteArray(AlifLongObject* _v,
	unsigned char* _bytes, AlifUSizeT _n,
	AlifIntT _littleEndian, AlifIntT _isSigned,
	AlifIntT _withExceptions) { // 953
	AlifSizeT i_{};               /* index into v->long_value.ob_digit */
	AlifSizeT nDigits{};         /* number of digits */
	twodigits accum{};            /* sliding register */
	AlifUIntT accumBits{};     /* # bits in accum */
	AlifIntT doTwosComp{};           /* store 2's-comp?  is_signed and v < 0 */
	digit carry{};                /* for computing 2's-comp */
	AlifUSizeT j_{};                   /* # bytes filled */
	unsigned char* p_{};           /* pointer to next byte in bytes */
	AlifIntT pincr{};                  /* direction to move p */


	nDigits = alifLong_digitCount(_v);
	if (_alifLong_isNegative(_v)) {
		if (!_isSigned) {
			if (_withExceptions) {
				//alifErr_setString(_alifExcOverflowError_,
					//"can't convert negative int to unsigned");
			}
			return -1;
		}
		doTwosComp = 1;
	}
	else {
		doTwosComp = 0;
	}

	if (_littleEndian) {
		p_ = _bytes;
		pincr = 1;
	}
	else {
		p_ = _bytes + _n - 1;
		pincr = -1;
	}

	j_ = 0;
	accum = 0;
	accumBits = 0;
	carry = doTwosComp ? 1 : 0;
	for (i_ = 0; i_ < nDigits; ++i_) {
		digit thisdigit = _v->longValue.digit[i_];
		if (doTwosComp) {
			thisdigit = (thisdigit ^ ALIFLONG_MASK) + carry;
			carry = thisdigit >> ALIFLONG_SHIFT;
			thisdigit &= ALIFLONG_MASK;
		}
		accum |= (twodigits)thisdigit << accumBits;

		if (i_ == nDigits - 1) {
	
			digit s = doTwosComp ? thisdigit ^ ALIFLONG_MASK : thisdigit;
			while (s != 0) {
				s >>= 1;
				accumBits++;
			}
		}
		else
			accumBits += ALIFLONG_SHIFT;

		while (accumBits >= 8) {
			if (j_ >= _n)
				goto Overflow;
			++j_;
			*p_ = (unsigned char)(accum & 0xff);
			p_ += pincr;
			accumBits -= 8;
			accum >>= 8;
		}
	}

	if (accumBits > 0) {
		if (j_ >= _n)
			goto Overflow;
		++j_;
		if (doTwosComp) {
			accum |= (~(twodigits)0) << accumBits;
		}
		*p_ = (unsigned char)(accum & 0xff);
		p_ += pincr;
	}
	else if (j_ == _n and _n > 0 and _isSigned) {
		unsigned char msb_ = *(p_ - pincr);
		AlifIntT signBitSet = msb_ >= 0x80;
		if (signBitSet == doTwosComp)
			return 0;
		else
			goto Overflow;
	}

	{
		unsigned char signbyte = doTwosComp ? 0xffU : 0U;
		for (; j_ < _n; ++j_, p_ += pincr)
			*p_ = signbyte;
	}

	return 0;

Overflow:
	if (_withExceptions) {
		//alifErr_setString((alifExcOverflowError_, "int too big to convert");
	}
	return -1;

}


static inline AlifIntT fits_inNBits(AlifSizeT v, AlifSizeT n) { // 1095
	if (n >= (AlifSizeT)sizeof(AlifSizeT) * 8) {
		return 1;
	}
	AlifSizeT v_extended = v >> ((int)n - 1);
	return v_extended == 0 or v_extended == -1;
}

static inline AlifIntT _resolve_endianness(AlifIntT* endianness) { // 1107
	if (*endianness == -1 || (*endianness & 2)) {
		*endianness = ALIF_LITTLE_ENDIAN;
	}
	else {
		*endianness &= 1;
	}
	return 0;
}

AlifSizeT alifLong_asNativeBytes(AlifObject* vv,
	void* buffer, AlifSizeT n, AlifIntT flags) { // 1119
	AlifLongObject* v{};
	union {
		AlifSizeT v;
		unsigned char b[sizeof(AlifSizeT)];
	} cv{};
	AlifIntT do_decref = 0;
	AlifSizeT res = 0;

	if (vv == nullptr or n < 0) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	AlifIntT little_endian = flags;
	if (_resolve_endianness(&little_endian) < 0) {
		return -1;
	}

	if (ALIFLONG_CHECK(vv)) {
		v = (AlifLongObject*)vv;
	}
	else if (flags != -1 and (flags & ALIF_ASNATIVEBYTES_ALLOW_INDEX)) {
		v = (AlifLongObject*)_alifNumber_index(vv);
		if (v == nullptr) {
			return -1;
		}
		do_decref = 1;
	}
	else {
		//alifErr_format(_alifExcTypeError_, "expect int, got %T", vv);
		return -1;
	}

	if ((flags != -1 and (flags & ALIF_ASNATIVEBYTES_REJECT_NEGATIVE))
		and _alifLong_isNegative(v)) {
		//alifErr_setString(_alifExcValueError_, "Cannot convert negative int");
		if (do_decref) {
			ALIF_DECREF(v);
		}
		return -1;
	}

	if (alifLong_isCompact(v)) {
		res = 0;
		cv.v = alifLong_compactValue(v);
		res = sizeof(cv.b);
		if (n <= 0) {
			// nothing to do!
		}
		else if (n <= (AlifSizeT)sizeof(cv.b)) {
#if ALIF_LITTLE_ENDIAN
			if (little_endian) {
				memcpy(buffer, cv.b, n);
			}
			else {
				for (AlifSizeT i = 0; i < n; ++i) {
					((unsigned char*)buffer)[n - i - 1] = cv.b[i];
				}
			}
#else
			if (little_endian) {
				for (AlifSizeT i = 0; i < n; ++i) {
					((unsigned char*)buffer)[i] = cv.b[sizeof(cv.b) - i - 1];
				}
			}
			else {
				memcpy(buffer, &cv.b[sizeof(cv.b) - n], n);
			}
#endif

			/* If we fit, return the requested number of bytes */
			if (fits_inNBits(cv.v, n * 8)) {
				res = n;
			}
			else if (cv.v > 0 and fits_inNBits(cv.v, n * 8 + 1)) {
				if (flags == -1 or (flags & ALIF_ASNATIVEBYTES_UNSIGNED_BUFFER)) {
					res = n;
				}
				else {
					res = n + 1;
				}
			}
		}
		else {
			unsigned char fill = cv.v < 0 ? 0xFF : 0x00;
#if ALIF_LITTLE_ENDIAN
			if (little_endian) {
				memcpy(buffer, cv.b, sizeof(cv.b));
				memset((char*)buffer + sizeof(cv.b), fill, n - sizeof(cv.b));
			}
			else {
				unsigned char* b = (unsigned char*)buffer;
				for (AlifSizeT i = 0; i < n - (int)sizeof(cv.b); ++i) {
					*b++ = fill;
				}
				for (AlifSizeT i = sizeof(cv.b); i > 0; --i) {
					*b++ = cv.b[i - 1];
				}
			}
#else
			if (little_endian) {
				unsigned char* b = (unsigned char*)buffer;
				for (AlifSizeT i = sizeof(cv.b); i > 0; --i) {
					*b++ = cv.b[i - 1];
				}
				for (AlifSizeT i = 0; i < n - (int)sizeof(cv.b); ++i) {
					*b++ = fill;
				}
			}
			else {
				memset(buffer, fill, n - sizeof(cv.b));
				memcpy((char*)buffer + n - sizeof(cv.b), cv.b, sizeof(cv.b));
			}
#endif
		}
	}
	else {
		if (n > 0) {
			_alifLong_asByteArray(v, (unsigned char*)buffer, (AlifUSizeT)n, little_endian, 1, 0);
		}

		int64_t nb = _alifLong_numBits((AlifObject*)v);
		res = (AlifSizeT)(nb / 8) + 1;

		if (n > 0 && res == n + 1 && nb % 8 == 0) {
			if (_alifLong_isNegative(v)) {
				int is_edge_case = 1;
				unsigned char* b = (unsigned char*)buffer;
				for (AlifSizeT i = 0; i < n && is_edge_case; ++i, ++b) {
					if (i == 0) {
						is_edge_case = (*b == (little_endian ? 0 : 0x80));
					}
					else if (i < n - 1) {
						is_edge_case = (*b == 0);
					}
					else {
						is_edge_case = (*b == (little_endian ? 0x80 : 0));
					}
				}
				if (is_edge_case) {
					res = n;
				}
			}
			else {
				unsigned char* b = (unsigned char*)buffer;
				if (b[little_endian ? n - 1 : 0] & 0x80) {
					if (flags == -1 or (flags & ALIF_ASNATIVEBYTES_UNSIGNED_BUFFER)) {
						res = n;
					}
					else {
						res = n + 1;
					}
				}
			}
		}
	}

	if (do_decref) {
		ALIF_DECREF(v);
	}

	return res;
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



AlifObject* alifLong_fromSizeT(AlifSizeT _iVal) { // 1447
	AlifLongObject* v{};
	AlifUSizeT absIVal{};
	AlifUSizeT t{};  /* unsigned so >> doesn't propagate sign bit */
	AlifIntT ndigits = 0;
	AlifIntT negative = 0;

	if (IS_SMALL_INT(_iVal)) {
		return get_smallInt((sdigit)_iVal);
	}

	if (_iVal < 0) {
		/* avoid signed overflow when ival = SIZE_T_MIN */
		absIVal = (AlifUSizeT)(-1 - _iVal) + 1;
		negative = 1;
	}
	else {
		absIVal = (AlifUSizeT)_iVal;
	}

	/* Count the number of Alif digits. */
	t = absIVal;
	while (t) {
		++ndigits;
		t >>= ALIFLONG_SHIFT;
	}
	v = alifLong_new(ndigits);
	if (v != nullptr) {
		digit* p = v->longValue.digit;
		_alifLong_setSignAndDigitCount(v, negative ? -1 : 1, ndigits);
		t = absIVal;
		while (t) {
			*p++ = (digit)(t & ALIFLONG_MASK);
			t >>= ALIFLONG_SHIFT;
		}
	}
	return (AlifObject*)v;
}




long long alifLong_asLongLong(AlifObject* vv) { // 1491
	AlifLongObject* v{};
	long long bytes{};
	AlifIntT res{};
	AlifIntT do_decref = 0; /* if PyNumber_Index was called */

	if (vv == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}

	if (ALIFLONG_CHECK(vv)) {
		v = (AlifLongObject*)vv;
	}
	else {
		v = (AlifLongObject*)_alifNumber_index(vv);
		if (v == nullptr)
			return -1;
		do_decref = 1;
	}

	if (alifLong_isCompact(v)) {
		res = 0;
		bytes = alifLong_compactValue(v);
	}
	else {
		res = _alifLong_asByteArray((AlifLongObject*)v, (unsigned char*)&bytes,
			SIZEOF_LONG_LONG, ALIF_LITTLE_ENDIAN, 1, 1);
	}
	if (do_decref) {
		ALIF_DECREF(v);
	}

	/* Plan 9 can't handle long long in ? : expressions */
	if (res < 0)
		return (long long)-1;
	else
		return bytes;
}

AlifObject* alifNumber_long(AlifObject* o) { // 1505
	AlifObject* result{};
	AlifNumberMethods* m;
	AlifBuffer view{};

	if (o == nullptr) {
		//return null_error();
	}

	if (ALIFLONG_CHECKEXACT(o)) {
		return ALIF_NEWREF(o);
	}
	m = ALIF_TYPE(o)->asNumber;
	if (m and m->int_) { /* This should include subclasses of int */
		result = m->int_(o);
		if (!result or ALIFLONG_CHECKEXACT(result)) {
			return result;
		}

		if (!ALIFLONG_CHECK(result)) {
			//alifErr_format(_alifExc_typeError,
			//	"__int__ returned non-int (type %.200s)",
			//	ALIF_TYPE(result)->name);
			ALIF_DECREF(result);
			return nullptr;
		}
		//if (alifErr_warnFormat(_alifExcDeprecationWarning_, 1,
		//	"__int__ returned non-int (type %.200s).  "
		//	"The ability to return an instance of a strict subclass of int "
		//	"is deprecated, and may be removed in a future version of Alif.",
		//	ALIF_TYPE(result)->name)) {
		//	ALIF_DECREF(result);
		//	return nullptr;
		//}
		ALIF_SETREF(result, _alifLong_copy((AlifLongObject*)result));
		return result;
	}
	if (m and m->index) {
		return alifNumber_index(o);
	}

	if (ALIFUSTR_CHECK(o))
		return alifLong_fromUStrObject(o, 10);

	if (ALIFBYTES_CHECK(o))
		return _alifLong_fromBytes(ALIFBYTES_AS_STRING(o),
			ALIFBYTES_GET_SIZE(o), 10);

	if (ALIFBYTEARRAY_CHECK(o))
		return _alifLong_fromBytes(ALIFBYTEARRAY_AS_STRING(o),
			ALIFBYTEARRAY_GET_SIZE(o), 10);

	if (alifObject_getBuffer(o, &view, ALIFBUF_SIMPLE) == 0) {
		AlifObject* bytes{};

		/* Copy to NUL-terminated buffer. */
		bytes = alifBytes_fromStringAndSize((const char*)view.buf, view.len);
		if (bytes == nullptr) {
			alifBuffer_release(&view);
			return nullptr;
		}
		result = _alifLong_fromBytes(ALIFBYTES_AS_STRING(bytes),
			ALIFBYTES_GET_SIZE(bytes), 10);
		ALIF_DECREF(bytes);
		alifBuffer_release(&view);
		return result;
	}

	//return type_error("int() argument must be a string, a bytes-like object "
	//	"or a real number, not '%.200s'", o);
}

static unsigned long long _alifLong_asUnsignedLongLongMask(AlifObject* _vv) { // 1583
	AlifLongObject* v_{};
	unsigned long long x_{};
	AlifSizeT i_{};
	AlifIntT sign{};

	if (_vv == nullptr or !ALIFLONG_CHECK(_vv)) {
		//ALIFERR_BADINTERNALCALL();
		return (unsigned long long) - 1;
	}
	v_ = (AlifLongObject*)_vv;
	if (alifLong_isCompact(v_)) {
#if SIZEOF_LONG_LONG < SIZEOF_SIZE_T
		return (unsigned long long)(AlifUSizeT)alifLong_compactValue(v_);
#else
		return (unsigned long long)(long long)alifLong_compactValue(v_);
#endif
	}
	i_ = alifLong_digitCount(v_);
	sign = alifLong_nonCompactSign(v_);
	x_ = 0;
	while (--i_ >= 0) {
		x_ = (x_ << ALIFLONG_SHIFT) | v_->longValue.digit[i_];
	}
	return x_ * sign;
}

unsigned long long alifLong_asUnsignedLongLongMask(AlifObject* _op) { // 1612
	AlifLongObject* lo{};
	unsigned long long val{};

	if (_op == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return (unsigned long long) - 1;
	}

	if (ALIFLONG_CHECK(_op)) {
		return _alifLong_asUnsignedLongLongMask(_op);
	}

	lo = (AlifLongObject*)_alifNumber_index(_op);
	if (lo == nullptr)
		return (unsigned long long) - 1;

	val = _alifLong_asUnsignedLongLongMask((AlifObject*)lo);
	ALIF_DECREF(lo);
	return val;
}








 // 1820
#define CHECK_BINOP(_v,_w)									\
    do {													\
        if (!ALIFLONG_CHECK(_v) or !ALIFLONG_CHECK(_w))     \
            return ALIF_NOTIMPLEMENTED;						\
    } while(0)


static digit v_iadd(digit* _x, AlifSizeT _m, digit* _y, AlifSizeT _n) { // 1830
	AlifSizeT i_{};
	digit carry = 0;

	for (i_ = 0; i_ < _n; ++i_) {
		carry += _x[i_] + _y[i_];
		_x[i_] = carry & ALIFLONG_MASK;
		carry >>= ALIFLONG_SHIFT;
	}
	for (; carry and i_ < _m; ++i_) {
		carry += _x[i_];
		_x[i_] = carry & ALIFLONG_MASK;
		carry >>= ALIFLONG_SHIFT;
	}
	return carry;
}

static digit v_isub(digit* _x, AlifSizeT _m, digit* _y, AlifSizeT _n) { // 1856
	AlifSizeT i_{};
	digit borrow = 0;

	for (i_ = 0; i_ < _n; ++i_) {
		borrow = _x[i_] - _y[i_] - borrow;
		_x[i_] = borrow & ALIFLONG_MASK;
		borrow >>= ALIFLONG_SHIFT;
		borrow &= 1;            /* keep only 1 sign bit */
	}
	for (; borrow and i_ < _m; ++i_) {
		borrow = _x[i_] - borrow;
		_x[i_] = borrow & ALIFLONG_MASK;
		borrow >>= ALIFLONG_SHIFT;
		borrow &= 1;
	}
	return borrow;
}

static digit v_lShift(digit* _z, digit* _a, AlifSizeT _m, AlifIntT _d) { // 1882
	AlifSizeT i_{};
	digit carry = 0;

	for (i_ = 0; i_ < _m; i_++) {
		twodigits acc_ = (twodigits)_a[i_] << _d | carry;
		_z[i_] = (digit)acc_ & ALIFLONG_MASK;
		carry = (digit)(acc_ >> ALIFLONG_SHIFT);
	}
	return carry;
}

static digit v_rShift(digit* _z, digit* _a, AlifSizeT _m, AlifIntT _d) { // 1900
	AlifSizeT i_{};
	digit carry = 0;
	digit mask = ((digit)1 << _d) - 1U;

	for (i_ = _m; i_-- > 0;) {
		twodigits acc_ = (twodigits)carry << ALIFLONG_SHIFT | _a[i_];
		carry = (digit)acc_ & mask;
		_z[i_] = (digit)(acc_ >> _d);
	}
	return carry;
}

static digit inplace_divrem1(digit* _pOut, digit* _pin, AlifSizeT _size, digit _n) { // 1937
	digit remainder = 0;

	while (--_size >= 0) {
		twodigits dividend{};
		dividend = ((twodigits)remainder << ALIFLONG_SHIFT) | _pin[_size];
		digit quotient{};
		quotient = (digit)(dividend / _n);
		remainder = dividend % _n;
		_pOut[_size] = quotient;
	}
	return remainder;
}


static AlifLongObject* divrem1(AlifLongObject* _a, digit _n, digit* _pRem) { // 1959
	const AlifSizeT size = alifLong_digitCount(_a);
	AlifLongObject* z_{};

	z_ = alifLong_new(size);
	if (z_ == nullptr)
		return nullptr;
	*_pRem = inplace_divrem1(z_->longValue.digit, _a->longValue.digit, size, _n);
	return long_normalize(z_);
}

static digit inplace_rem1(digit* _pin, AlifSizeT _size, digit _n) { // 1975
	twodigits rem = 0;

	while (--_size >= 0)
		rem = ((rem << ALIFLONG_SHIFT) | _pin[_size]) % _n;
	return (digit)rem;
}

static AlifLongObject* rem1(AlifLongObject* _a, digit _n) { // 1990
	const AlifSizeT size = alifLong_digitCount(_a);

	return (AlifLongObject*)alifLong_fromLong(
		(long)inplace_rem1(_a->longValue.digit, size, _n));
}

#ifdef WITH_ALIFLONG_MODULE
static AlifIntT alifLong_intToDecimalString(AlifObject* _aa,
	AlifObject** _pOutput, AlifUStrWriter* _writer,
	AlifBytesWriter* _bytesWriter, char** _bytesStr) { // 2003
	AlifObject* s_ = nullptr;
	//AlifObject* mod_ = alifImport_importModule("alifLong");
	//if (mod_ == nullptr) {
	//	return -1;
	//}
	//s_ = alifObject_callMethod(mod_, "intToDecimalString", "O", _aa);
	//if (s_ == nullptr) {
	//	goto error;
	//}
	if (!ALIFUSTR_CHECK(s_)) {
		//alifErr_setString(_alifExcTypeError_,
			//"aliflong.intToDecimalString did not return a str");
		goto error;
	}
	if (_writer) {
		AlifSizeT size = ALIFUSTR_GET_LENGTH(s_);
		if (ALIFUSTRWRITER_PREPARE(_writer, size, '9') == -1) {
			goto error;
		}
		if (alifUStrWriter_writeStr(_writer, s_) < 0) {
			goto error;
		}
		goto success;
	}
	else if (_bytesWriter) {
		AlifSizeT size = ALIFUSTR_GET_LENGTH(s_);
		const void* data = ALIFUSTR_DATA(s_);
		AlifIntT kind = ALIFUSTR_KIND(s_);
		*_bytesStr = (char*)alifBytesWriter_prepare(_bytesWriter, *_bytesStr, size);
		if (*_bytesStr == nullptr) {
			goto error;
		}
		char* p_ = *_bytesStr;
		for (AlifSizeT i = 0; i < size; i++) {
			AlifUCS4 ch_ = ALIFUSTR_READ(kind, data, i);
			*p_++ = (char)ch_;
		}
		(*_bytesStr) = p_;
		goto success;
	}
	else {
		*_pOutput = ALIF_NEWREF(s_);
		goto success;
	}

error:
	//ALIF_DECREF(mod_);
	ALIF_XDECREF(s_);
	return -1;

success:
	//ALIF_DECREF(mod_);
	ALIF_XDECREF(s_);
	return 0;
}
#endif /* WITH_ALIFLONG_MODULE */


static AlifIntT longTo_decimalStringInternal(AlifObject* _aa,
	AlifObject** _pOutput, AlifUStrWriter* _writer,
	AlifBytesWriter* _bytesWriter, char** _bytesStr) { // 2072
	AlifLongObject* scratch{}, * a_{};
	AlifObject* str_ = nullptr;
	AlifSizeT size{}, strLen{}, sizeA{}, i_{}, j_{};
	digit* pOut{}, * pin_{}, rem_{}, tenPow{};
	AlifIntT negative{};
	AlifIntT d_{};

	a_ = (AlifLongObject*)_aa;
	if (a_ == nullptr or !ALIFLONG_CHECK(a_)) {
		//ALIFERR_BADINTERNALCALL();
		return -1;
	}
	sizeA = alifLong_digitCount(a_);
	negative = _alifLong_isNegative(a_);

	if (sizeA >= 10 * ALIF_LONG_MAX_STR_DIGITS_THRESHOLD
		/ (3 * ALIFLONG_SHIFT) + 2) {
		AlifInterpreter* interp = _alifInterpreter_get();
		AlifIntT maxStrDigits = interp->longState.maxStrDigits;
		if ((maxStrDigits > 0) and
			(maxStrDigits / (3 * ALIFLONG_SHIFT) <= (sizeA - 11) / 10)) {
			//alifErr_format(_alifExcValueError_, _MAX_STR_DIGITS_ERROR_FMT_TO_STR,
				//max_str_digits);
			return -1;
		}
	}

#if WITH_ALIFLONG_MODULE
	if (sizeA > 1000) {
		return alifLong_intToDecimalString(_aa,
			_pOutput,
			_writer,
			_bytesWriter,
			_bytesStr);
	}
#endif
	d_ = (33 * ALIFLONG_DECIMAL_SHIFT) /
		(10 * ALIFLONG_SHIFT - 33 * ALIFLONG_DECIMAL_SHIFT);
	size = 1 + sizeA + sizeA / d_;
	scratch = alifLong_new(size);
	if (scratch == nullptr)
		return -1;

	pin_ = a_->longValue.digit;
	pOut = scratch->longValue.digit;
	size = 0;
	for (i_ = sizeA; --i_ >= 0; ) {
		digit hi = pin_[i_];
		for (j_ = 0; j_ < size; j_++) {
			twodigits z = (twodigits)pOut[j_] << ALIFLONG_SHIFT | hi;
			hi = (digit)(z / ALIFLONG_DECIMAL_BASE);
			pOut[j_] = (digit)(z - (twodigits)hi *
				ALIFLONG_DECIMAL_BASE);
		}
		while (hi) {
			pOut[size++] = hi % ALIFLONG_DECIMAL_BASE;
			hi /= ALIFLONG_DECIMAL_BASE;
		}
		SIGCHECK({
				ALIF_DECREF(scratch);
				return -1;
			});
	}

	if (size == 0)
		pOut[size++] = 0;

	strLen = negative + 1 + (size - 1) * ALIFLONG_DECIMAL_SHIFT;
	tenPow = 10;
	rem_ = pOut[size - 1];
	while (rem_ >= tenPow) {
		tenPow *= 10;
		strLen++;
	}
	if (strLen > ALIF_LONG_MAX_STR_DIGITS_THRESHOLD) {
		AlifInterpreter* interp = _alifInterpreter_get();
		AlifIntT maxStrDigits = interp->longState.maxStrDigits;
		AlifSizeT strLenNoSign = strLen - negative;
		if ((maxStrDigits > 0) and (strLenNoSign > maxStrDigits)) {
			ALIF_DECREF(scratch);
			//alifErr_format(_alifExcValueError_, _MAX_STR_DIGITS_ERROR_FMT_TO_STR,
				//max_str_digits);
			return -1;
		}
	}
	if (_writer) {
		if (ALIFUSTRWRITER_PREPARE(_writer, strLen, '9') == -1) {
			ALIF_DECREF(scratch);
			return -1;
		}
	}
	else if (_bytesWriter) {
		*_bytesStr = (char*)alifBytesWriter_prepare(_bytesWriter, *_bytesStr, strLen);
		if (*_bytesStr == nullptr) {
			ALIF_DECREF(scratch);
			return -1;
		}
	}
	else {
		str_ = alifUStr_new(strLen, '9');
		if (str_ == nullptr) {
			ALIF_DECREF(scratch);
			return -1;
		}
	}

#define WRITE_DIGITS(p)                                               \
    do {                                                              \
        for (i_=0; i_ < size - 1; i_++) {                                \
            rem_ = pOut[i_];                                            \
            for (j_ = 0; j_ < ALIFLONG_DECIMAL_SHIFT; j_++) {             \
                *--p = '0' + rem_ % 10;                                \
                rem_ /= 10;                                            \
            }                                                         \
        }                                                             \
        rem_ = pOut[i_];                                                \
        do {                                                          \
            *--p = '0' + rem_ % 10;                                    \
            rem_ /= 10;                                                \
        } while (rem_ != 0);                                           \
        if (negative)                                                 \
            *--p = '-';                                               \
    } while (0)

#define WRITE_UNICODE_DIGITS(_type)                                    \
    do {                                                              \
        if (_writer)                                                   \
            p = (_type*)ALIFUSTR_DATA(_writer->buffer) + _writer->pos + strLen; \
        else                                                          \
            p = (_type*)ALIFUSTR_DATA(str_) + strLen;                  \
        WRITE_DIGITS(p);                                              \
    } while (0)
	if (_bytesWriter) {
		char* p = *_bytesStr + strLen;
		WRITE_DIGITS(p);
	}
	else {
		AlifIntT kind = _writer ? _writer->kind : ALIFUSTR_KIND(str_);
		if (kind == AlifUStrKind_::AlifUStr_1Byte_Kind) {
			AlifUCS1* p;
			WRITE_UNICODE_DIGITS(AlifUCS1);
		}
		else if (kind == AlifUStrKind_::AlifUStr_2Byte_Kind) {
			AlifUCS2* p;
			WRITE_UNICODE_DIGITS(AlifUCS2);
		}
		else {
			AlifUCS4* p;
			WRITE_UNICODE_DIGITS(AlifUCS4);
		}
	}

#undef WRITE_DIGITS
#undef WRITE_USTR_DIGITS

	_alif_decrefInt(scratch);
	if (_writer) {
		_writer->pos += strLen;
	}
	else if (_bytesWriter) {
		(*_bytesStr) += strLen;
	}
	else {
		*_pOutput = (AlifObject*)str_;
	}
	return 0;
}


static AlifObject* longTo_decimalString(AlifObject* _aa) { // 2294
	AlifObject* v{};
	if (longTo_decimalStringInternal(_aa, &v, nullptr, nullptr, nullptr) == -1)
		return nullptr;
	return v;
}




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
		AlifIntT k_;
		if (*p_ == '_') {
			continue;
		}
		k_ = (AlifIntT)_alifLongDigitValue_[ALIF_CHARMASK(*p_)];
		accum |= (twodigits)k_ << bitsInAccum;
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


#ifdef WITH_ALIFLONG_MODULE
static AlifIntT alifLong_intFromString(const char* _start,
	const char* _end, AlifLongObject** _res) { // 2629
//	AlifObject* mod = alifImport_importModule("_aliflong");
//	if (mod == nullptr) {
//		goto error;
//	}
//	AlifObject* s = alifUStr_fromStringAndSize(_start, _end - _start);
//	if (s == nullptr) {
//		ALIF_DECREF(mod);
//		goto error;
//	}
//	AlifObject* result = alifObject_callMethod(mod, "int_from_string", "O", s);
//	ALIF_DECREF(s);
//	ALIF_DECREF(mod);
//	if (result == nullptr) {
//		goto error;
//	}
//	if (!ALIFLONG_CHECK(result)) {
//		ALIF_DECREF(result);
//		alifErr_setString(_alifExcTypeError_,
//			"_aliflong.int_from_string did not return an AlifIntT");
//		goto error;
//	}
//	*_res = (AlifLongObject*)result;
//	return 0;
//error:
	*_res = nullptr;
	return 0; 
}
#endif /* WITH_ALIFLONG_MODULE */


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
			if ((maxStrDigits > 0) and (digits > maxStrDigits)) {
				//alifErr_format(_alifExcValueError_, _MAX_STR_DIGITS_ERROR_FMT_TO_INT,
				//	max_str_digits, digits);
				*_res = nullptr;
				return 0;
			}
		}
#if WITH_ALIFLONG_MODULE
		if (digits > 6000 and _base == 10) {
			/* Switch to alifLong_intFromString() */
			return alifLong_intFromString(start, end, _res);
		}
#endif
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
	AlifIntT ret_ = long_fromStringBase(&_str, _base, &z_);
	if (ret_ == -1) {
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



AlifObject* _alifLong_fromBytes(const char* s, AlifSizeT len, AlifIntT base) { // 3106
	AlifObject* result{}, * strobj{};
	char* end{};

	result = alifLong_fromString(s, &end, base);
	if (end == nullptr || (result != nullptr && end == s + len))
		return result;
	ALIF_XDECREF(result);
	strobj = alifBytes_fromStringAndSize(s, ALIF_MIN(len, 200));
	if (strobj != nullptr) {
		//alifErr_format(_alifExcValueError_,
		//	"invalid literal for int() with base %d: %.200R",
		//	base, strobj);
		ALIF_DECREF(strobj);
	}
	return nullptr;
}



AlifObject* alifLong_fromUStrObject(AlifObject* u, AlifIntT base) { // 3126
	AlifObject* result{}, * asciidig{};
	const char* buffer{};
	char* end = nullptr;
	AlifSizeT buflen{};

	asciidig = _alifUStr_transformDecimalAndSpaceToASCII(u);
	if (asciidig == nullptr)
		return nullptr;
	/* Simply get a pointer to existing ASCII characters. */
	buffer = alifUStr_asUTF8AndSize(asciidig, &buflen);

	result = alifLong_fromString(buffer, &end, base);
	if (end == nullptr or (result != nullptr and end == buffer + buflen)) {
		ALIF_DECREF(asciidig);
		return result;
	}
	ALIF_DECREF(asciidig);
	ALIF_XDECREF(result);
	//alifErr_format(_alifExcValueError_,
	//	"invalid literal for int() with base %d: %.200R",
	//	base, u);
	return nullptr;
}


/* forward */
static AlifLongObject* x_divrem(AlifLongObject*, AlifLongObject*, AlifLongObject**); // 3155
static AlifObject* long_long(AlifObject*);

static AlifIntT long_divrem(AlifLongObject* _a, AlifLongObject* _b,
	AlifLongObject** _pDiv, AlifLongObject** _pRem) { // 3163
	AlifSizeT sizeA = alifLong_digitCount(_a), sizeB = alifLong_digitCount(_b);
	AlifLongObject* z_{};

	if (sizeB == 0) {
		//alifErr_setString(_alifExcZeroDivisionError_, "division by zero");
		return -1;
	}
	if (sizeA < sizeB or
		(sizeA == sizeB and
			_a->longValue.digit[sizeA - 1] < _b->longValue.digit[sizeB - 1])) {
		*_pRem = (AlifLongObject*)long_long((AlifObject*)_a);
		if (*_pRem == nullptr) {
			return -1;
		}
		*_pDiv = (AlifLongObject*)_alifLong_getZero();
		return 0;
	}
	if (sizeB == 1) {
		digit rem = 0;
		z_ = divrem1(_a, _b->longValue.digit[0], &rem);
		if (z_ == nullptr)
			return -1;
		*_pRem = (AlifLongObject*)alifLong_fromLong((long)rem);
		if (*_pRem == nullptr) {
			ALIF_DECREF(z_);
			return -1;
		}
	}
	else {
		z_ = x_divrem(_a, _b, _pRem);
		*_pRem = maybe_smallLong(*_pRem);
		if (z_ == nullptr)
			return -1;
	}
	if ((_alifLong_isNegative(_a)) != (_alifLong_isNegative(_b))) {
		_alifLong_negate(&z_);
		if (z_ == nullptr) {
			ALIF_CLEAR(*_pRem);
			return -1;
		}
	}
	if (_alifLong_isNegative(_a) and !_alifLong_isZero(*_pRem)) {
		_alifLong_negate(_pRem);
		if (*_pRem == nullptr) {
			ALIF_DECREF(z_);
			ALIF_CLEAR(*_pRem);
			return -1;
		}
	}
	*_pDiv = maybe_smallLong(z_);
	return 0;
}

static AlifIntT long_rem(AlifLongObject* _a, AlifLongObject* _b, AlifLongObject** _prem) { // 3226
	AlifSizeT sizeA = alifLong_digitCount(_a), sizeB = alifLong_digitCount(_b);

	if (sizeB == 0) {
		//alifErr_setString(_alifExcZeroDivisionError_,
		//	"division by zero");
		return -1;
	}
	if (sizeA < sizeB or
		(sizeA == sizeB and
			_a->longValue.digit[sizeA - 1] < _b->longValue.digit[sizeB - 1])) {
		/* |a| < |b|. */
		*_prem = (AlifLongObject*)long_long((AlifObject*)_a);
		return -(*_prem == nullptr);
	}
	if (sizeB == 1) {
		*_prem = rem1(_a, _b->longValue.digit[0]);
		if (*_prem == nullptr)
			return -1;
	}
	else {
		/* Slow path using divrem. */
		ALIF_XDECREF(x_divrem(_a, _b, _prem));
		*_prem = maybe_smallLong(*_prem);
		if (*_prem == nullptr)
			return -1;
	}
	/* Set the sign. */
	if (_alifLong_isNegative(_a) and !_alifLong_isZero(*_prem)) {
		_alifLong_negate(_prem);
		if (*_prem == nullptr) {
			ALIF_CLEAR(*_prem);
			return -1;
		}
	}
	return 0;
}

static AlifLongObject* x_divrem(AlifLongObject* _v1,
	AlifLongObject* _w1, AlifLongObject** _pRem) { // 3270
	AlifLongObject* v_{}, * w_{}, * a_{};
	AlifSizeT i_{}, k_{}, sizeV{}, sizeW{};
	AlifIntT d_{};
	digit wm1_{}, wm2_{}, carry{}, q_{}, r_{}, vTop{}, * v0_{}, * vk_{}, * w0_{}, * ak_{};
	twodigits vv_{};
	sdigit zhi_{};
	stwodigits z_{};

	sizeV = alifLong_digitCount(_v1);
	sizeW = alifLong_digitCount(_w1);
	v_ = alifLong_new(sizeV + 1);
	if (v_ == nullptr) {
		*_pRem = nullptr;
		return nullptr;
	}
	w_ = alifLong_new(sizeW);
	if (w_ == nullptr) {
		ALIF_DECREF(v_);
		*_pRem = nullptr;
		return nullptr;
	}
	d_ = ALIFLONG_SHIFT - bit_lengthDigit(_w1->longValue.digit[sizeW - 1]);
	carry = v_lShift(w_->longValue.digit, _w1->longValue.digit, sizeW, d_);
	carry = v_lShift(v_->longValue.digit, _v1->longValue.digit, sizeV, d_);
	if (carry != 0 or v_->longValue.digit[sizeV - 1] >= w_->longValue.digit[sizeW - 1]) {
		v_->longValue.digit[sizeV] = carry;
		sizeV++;
	}

	k_ = sizeV - sizeW;
	a_ = alifLong_new(k_);
	if (a_ == nullptr) {
		ALIF_DECREF(w_);
		ALIF_DECREF(v_);
		*_pRem = nullptr;
		return nullptr;
	}
	v0_ = v_->longValue.digit;
	w0_ = w_->longValue.digit;
	wm1_ = w0_[sizeW - 1];
	wm2_ = w0_[sizeW - 2];
	for (vk_ = v0_ + k_, ak_ = a_->longValue.digit + k_; vk_-- > v0_;) {


		//SIGCHECK({
		//		ALIF_DECREF(a_);
		//		ALIF_DECREF(w_);
		//		ALIF_DECREF(v_);
		//		*_pRem = nullptr;
		//		return nullptr;
		//});

		vTop = vk_[sizeW];
		vv_ = ((twodigits)vTop << ALIFLONG_SHIFT) | vk_[sizeW - 1];

		q_ = (digit)(vv_ / wm1_);
		r_ = (digit)(vv_ % wm1_);
		while ((twodigits)wm2_ * q_ > (((twodigits)r_ << ALIFLONG_SHIFT)
			| vk_[sizeW - 2])) {
			--q_;
			r_ += wm1_;
			if (r_ >= ALIFLONG_BASE)
				break;
		}

		zhi_ = 0;
		for (i_ = 0; i_ < sizeW; ++i_) {

			z_ = (sdigit)vk_[i_] + zhi_ -
				(stwodigits)q_ * (stwodigits)w0_[i_];
			vk_[i_] = (digit)z_ & ALIFLONG_MASK;
			zhi_ = (sdigit)ALIF_ARITHMETIC_RIGHT_SHIFT(stwodigits,
				z_, ALIFLONG_SHIFT);
		}

		if ((sdigit)vTop + zhi_ < 0) {
			carry = 0;
			for (i_ = 0; i_ < sizeW; ++i_) {
				carry += vk_[i_] + w0_[i_];
				vk_[i_] = carry & ALIFLONG_MASK;
				carry >>= ALIFLONG_SHIFT;
			}
			--q_;
		}

		*--ak_ = q_;
	}

	carry = v_rShift(w0_, v0_, sizeW, d_);
	ALIF_DECREF(v_);

	*_pRem = long_normalize(w_);
	return long_normalize(a_);
}

 // 3408
#if DBL_MANT_DIG == 53
#define EXP2_DBL_MANT_DIG 9007199254740992.0
#else
#define EXP2_DBL_MANT_DIG (ldexp(1.0, DBL_MANT_DIG))
#endif

double _alifLong_frexp(AlifLongObject* a, int64_t* e) { // 3414
	AlifSizeT aSize{}, shiftDigits{}, xSize{};
	AlifIntT shiftBits{};
	int64_t aBits{};
	digit rem{};
	digit xDigits[2 + (DBL_MANT_DIG + 1) / ALIFLONG_SHIFT] = { 0, };
	double dx{};
	static const AlifIntT halfEvenCorrection[8] = { 0, -1, -2, 1, 0, -1, 2, 1 };

	aSize = alifLong_digitCount(a);
	if (aSize == 0) {
		*e = 0;
		return 0.0;
	}
	aBits = _alifLong_numBits((AlifObject*)a);

	if (aBits <= DBL_MANT_DIG + 2) {
		shiftDigits = (DBL_MANT_DIG + 2 - (AlifSizeT)aBits) / ALIFLONG_SHIFT;
		shiftBits = (DBL_MANT_DIG + 2 - (AlifIntT)aBits) % ALIFLONG_SHIFT;
		xSize = shiftDigits;
		rem = v_lShift(xDigits + xSize, a->longValue.digit, aSize,
			shiftBits);
		xSize += aSize;
		xDigits[xSize++] = rem;
	}
	else {
		shiftDigits = (AlifSizeT)(aBits - DBL_MANT_DIG - 2) / ALIFLONG_SHIFT;
		shiftBits = (AlifIntT)(aBits - DBL_MANT_DIG - 2) % ALIFLONG_SHIFT;
		rem = v_rShift(xDigits, a->longValue.digit + shiftDigits,
			aSize - shiftDigits, shiftBits);
		xSize = aSize - shiftDigits;
		if (rem)
			xDigits[0] |= 1;
		else
			while (shiftDigits > 0)
				if (a->longValue.digit[--shiftDigits]) {
					xDigits[0] |= 1;
					break;
				}
	}

	/* Round, and convert to double. */
	xDigits[0] += halfEvenCorrection[xDigits[0] & 7];
	dx = xDigits[--xSize];
	while (xSize > 0)
		dx = dx * ALIFLONG_BASE + xDigits[--xSize];

	/* Rescale;  make correction if result is 1.0. */
	dx /= 4.0 * EXP2_DBL_MANT_DIG;
	if (dx == 1.0) {
		dx = 0.5;
		aBits += 1;
	}

	*e = aBits;
	return _alifLong_isNegative(a) ? -dx : dx;
}




double alifLong_asDouble(AlifObject* _v) { // 3526
	int64_t exponent{};
	double x{};

	if (_v == nullptr) {
		//ALIFERR_BADINTERNALCALL();
		return -1.0;
	}
	if (!ALIFLONG_CHECK(_v)) {
		//alifErr_setString(_alifExcTypeError_, "an integer is required");
		return -1.0;
	}
	if (alifLong_isCompact((AlifLongObject*)_v)) {
		/* Fast path; single digit long (31 bits) will cast safely
		   to double.  This improves performance of FP/long operations
		   by 20%.
		*/
		return (double)MEDIUM_VALUE((AlifLongObject*)_v);
	}
	x = _alifLong_frexp((AlifLongObject*)_v, &exponent);
	if (exponent > DBL_MAX_EXP) {
		//alifErr_setString(_alifExcOverflowError_,
		//	"AlifIntT too large to convert to float");
		return -1.0;
	}
	return ldexp(x, (AlifIntT)exponent);
}

static AlifSizeT long_compare(AlifLongObject* _a, AlifLongObject* _b) { // 3563
	if (_alifLong_bothAreCompact(_a, _b)) {
		return alifLong_compactValue(_a) - alifLong_compactValue(_b);
	}
	AlifSizeT sign = _alifLong_signedDigitCount(_a) - _alifLong_signedDigitCount(_b);
	if (sign == 0) {
		AlifSizeT i_ = alifLong_digitCount(_a);
		sdigit diff = 0;
		while (--i_ >= 0) {
			diff = (sdigit)_a->longValue.digit[i_] - (sdigit)_b->longValue.digit[i_];
			if (diff) {
				break;
			}
		}
		sign = _alifLong_isNegative(_a) ? -diff : diff;
	}
	return sign;
}

static AlifObject* long_richCompare(AlifObject* _self, AlifObject* _other, AlifIntT _op) { // 3583
	AlifSizeT result{};
	CHECK_BINOP(_self, _other);
	if (_self == _other)
		result = 0;
	else
		result = long_compare((AlifLongObject*)_self, (AlifLongObject*)_other);
	ALIF_RETURN_RICHCOMPARE(result, 0, _op);
}

static void long_dealloc(AlifObject* _self) { // 3595
	AlifLongObject* alifLong = (AlifLongObject*)_self;
	if (alifLong and alifLong_isCompact(alifLong)) {
		stwodigits ival = MEDIUM_VALUE(alifLong);
		if (IS_SMALL_INT(ival)) {
			AlifLongObject* smallAlifLong = (AlifLongObject*)get_smallInt((sdigit)ival);
			if (alifLong == smallAlifLong) {
				alif_setImmortal(_self);
				return;
			}
		}
	}
	ALIF_TYPE(_self)->free(_self);
}

static AlifHashT long_hash(AlifObject* _obj) { // 3616
	AlifLongObject* v = (AlifLongObject*)_obj;
	AlifUHashT x{};
	AlifSizeT i{};
	AlifIntT sign{};

	if (alifLong_isCompact(v)) {
		x = (AlifUHashT)alifLong_compactValue(v);
		if (x == (AlifUHashT)-1) {
			x = (AlifUHashT)-2;
		}
		return x;
	}
	i = alifLong_digitCount(v);
	sign = alifLong_nonCompactSign(v);
	x = 0;
	while (--i >= 0) {
		x = ((x << ALIFLONG_SHIFT) & ALIFHASH_MODULUS) |
			(x >> (ALIFHASH_BITS - ALIFLONG_SHIFT));
		x += v->longValue.digit[i];
		if (x >= ALIFHASH_MODULUS)
			x -= ALIFHASH_MODULUS;
	}
	x = x * sign;
	if (x == (AlifUHashT)-1) x = (AlifUHashT)-2;
	return (AlifHashT)x;
}

static AlifLongObject* x_add(AlifLongObject* _a, AlifLongObject* _b) { // 3675
	AlifSizeT sizeA = alifLong_digitCount(_a), sizeB = alifLong_digitCount(_b);
	AlifLongObject* z{};
	AlifSizeT i_{};
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
	for (i_ = 0; i_ < sizeB; ++i_) {
		carry += _a->longValue.digit[i_] + _b->longValue.digit[i_];
		z->longValue.digit[i_] = carry & ALIFLONG_MASK;
		carry >>= ALIFLONG_SHIFT;
	}
	for (; i_ < sizeA; ++i_) {
		carry += _a->longValue.digit[i_];
		z->longValue.digit[i_] = carry & ALIFLONG_MASK;
		carry >>= ALIFLONG_SHIFT;
	}
	z->longValue.digit[i_] = carry;
	return long_normalize(z);
}

static AlifLongObject* x_sub(AlifLongObject* _a, AlifLongObject* _b) { // 3709
	AlifSizeT sizeA = alifLong_digitCount(_a), sizeB = alifLong_digitCount(_b);
	AlifLongObject* z{};
	AlifSizeT i_{};
	AlifIntT sign = 1;
	digit borrow = 0;

	/* Ensure a is the larger of the two: */
	if (sizeA < sizeB) {
		sign = -1;
		{ AlifLongObject* temp = _a; _a = _b; _b = temp; }
		{
			AlifSizeT size_temp = sizeA;
			sizeA = sizeB;
			sizeB = size_temp;
		}
	}
	else if (sizeA == sizeB) {
		/* Find highest digit where a and b differ: */
		i_ = sizeA;
		while (--i_ >= 0 and _a->longValue.digit[i_] == _b->longValue.digit[i_])
			;
		if (i_ < 0)
			return (AlifLongObject*)alifLong_fromLong(0);
		if (_a->longValue.digit[i_] < _b->longValue.digit[i_]) {
			sign = -1;
			{ AlifLongObject* temp = _a; _a = _b; _b = temp; }
		}
		sizeA = sizeB = i_ + 1;
	}
	z = alifLong_new(sizeA);
	if (z == nullptr) return nullptr;
	for (i_ = 0; i_ < sizeB; ++i_) {
		/* The following assumes unsigned arithmetic
		   works module 2**N for some N>ALIFLONG_SHIFT. */
		borrow = _a->longValue.digit[i_] - _b->longValue.digit[i_] - borrow;
		z->longValue.digit[i_] = borrow & ALIFLONG_MASK;
		borrow >>= ALIFLONG_SHIFT;
		borrow &= 1; /* Keep only one sign bit */
	}
	for (; i_ < sizeA; ++i_) {
		borrow = _a->longValue.digit[i_] - borrow;
		z->longValue.digit[i_] = borrow & ALIFLONG_MASK;
		borrow >>= ALIFLONG_SHIFT;
		borrow &= 1; /* Keep only one sign bit */
	}
	if (sign < 0) {
		_alifLong_flipSign(z);
	}
	return maybe_smallLong(long_normalize(z));
}


static AlifLongObject* long_add(AlifLongObject* _a, AlifLongObject* _b) { // 3763
	if (_alifLong_bothAreCompact(_a, _b)) {
		stwodigits z = MEDIUM_VALUE(_a) + MEDIUM_VALUE(_b);
		return _alifLong_fromSTwoDigits(z);
	}

	AlifLongObject* z{};
	if (_alifLong_isNegative(_a)) {
		if (_alifLong_isNegative(_b)) {
			z = x_add(_a, _b);
			if (z != nullptr) {
				/* x_add received at least one multiple-digit AlifIntT,
				   and thus z must be a multiple-digit AlifIntT.
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
	return z;
}


static AlifObject* long_addMethod(AlifObject* _a, AlifObject* _b) { // 3795
	CHECK_BINOP(_a, _b);
	return (AlifObject*)long_add((AlifLongObject*)_a, (AlifLongObject*)_b);
}

static AlifLongObject* long_sub(AlifLongObject* _a, AlifLongObject* _b) { // 3802
	if (_alifLong_bothAreCompact(_a, _b)) {
		return _alifLong_fromSTwoDigits(MEDIUM_VALUE(_a) - MEDIUM_VALUE(_b));
	}

	AlifLongObject* z_{};
	if (_alifLong_isNegative(_a)) {
		if (_alifLong_isNegative(_b)) {
			z_ = x_sub(_b, _a);
		}
		else {
			z_ = x_add(_a, _b);
			if (z_ != nullptr) {
				_alifLong_flipSign(z_);
			}
		}
	}
	else {
		if (_alifLong_isNegative(_b))
			z_ = x_add(_a, _b);
		else
			z_ = x_sub(_a, _b);
	}
	return z_;
}

static AlifObject* long_subMethod(AlifObject* _a, AlifObject* _b) { // 3832
	CHECK_BINOP(_a, _b);
	return (AlifObject*)long_sub((AlifLongObject*)_a, (AlifLongObject*)_b);
}

static AlifLongObject* x_mul(AlifLongObject* a, AlifLongObject* b) { // 3842
	AlifLongObject* z{};
	AlifSizeT sizeA = alifLong_digitCount(a);
	AlifSizeT sizeB = alifLong_digitCount(b);
	AlifSizeT i_{};

	z = alifLong_new(sizeA + sizeB);
	if (z == nullptr) return nullptr;

	memset(z->longValue.digit, 0, alifLong_digitCount(z) * sizeof(digit));
	if (a == b) {

		digit* paend = a->longValue.digit + sizeA;
		for (i_ = 0; i_ < sizeA; ++i_) {
			twodigits carry{};
			twodigits f = a->longValue.digit[i_];
			digit* pz = z->longValue.digit + (i_ << 1);
			digit* pa = a->longValue.digit + i_ + 1;

			//SIGCHECK({
			//	ALIF_DECREF(z);
			//	return nullptr;
			//});

			carry = *pz + f * f;
			*pz++ = (digit)(carry & ALIFLONG_MASK);
			carry >>= ALIFLONG_SHIFT;
			f <<= 1;
			while (pa < paend) {
				carry += *pz + *pa++ * f;
				*pz++ = (digit)(carry & ALIFLONG_MASK);
				carry >>= ALIFLONG_SHIFT;
			}
			if (carry) {
				carry += *pz;
				*pz = (digit)(carry & ALIFLONG_MASK);
				carry >>= ALIFLONG_SHIFT;
				if (carry) {

					pz[1] = (digit)carry;
				}
			}
		}
	}
	else {      /* a is not the same as b -- gradeschool AlifIntT mult */
		for (i_ = 0; i_ < sizeA; ++i_) {
			twodigits carry = 0;
			twodigits f = a->longValue.digit[i_];
			digit* pz = z->longValue.digit + i_;
			digit* pb = b->longValue.digit;
			digit* pbend = b->longValue.digit + sizeB;

			//SIGCHECK({
			//		ALIF_DECREF(z);
			//		return nullptr;
			//	});

			while (pb < pbend) {
				carry += *pz + *pb++ * f;
				*pz++ = (digit)(carry & ALIFLONG_MASK);
				carry >>= ALIFLONG_SHIFT;
			}
			if (carry)
				*pz += (digit)(carry & ALIFLONG_MASK);
		}
	}
	return long_normalize(z);
}


static AlifIntT kMul_split(AlifLongObject* _n, AlifSizeT _size,
	AlifLongObject** _high, AlifLongObject** _low) { // 3946
	AlifLongObject* hi{}, * lo{};
	AlifSizeT sizeLo{}, sizeHi{};
	const AlifSizeT size_n = alifLong_digitCount(_n);

	sizeLo = ALIF_MIN(size_n, _size);
	sizeHi = size_n - sizeLo;

	if ((hi = alifLong_new(sizeHi)) == nullptr)
		return -1;
	if ((lo = alifLong_new(sizeLo)) == nullptr) {
		ALIF_DECREF(hi);
		return -1;
	}

	memcpy(lo->longValue.digit, _n->longValue.digit, sizeLo * sizeof(digit));
	memcpy(hi->longValue.digit, _n->longValue.digit + sizeLo, sizeHi * sizeof(digit));

	*_high = long_normalize(hi);
	*_low = long_normalize(lo);
	return 0;
}

static AlifLongObject* kLopsided_mul(AlifLongObject*, AlifLongObject*); // 3974

static AlifLongObject* k_mul(AlifLongObject* _a, AlifLongObject* _b) { // 3981
	AlifSizeT aSize = alifLong_digitCount(_a);
	AlifSizeT bSize = alifLong_digitCount(_b);
	AlifLongObject* ah_ = nullptr;
	AlifLongObject* al_ = nullptr;
	AlifLongObject* bh_ = nullptr;
	AlifLongObject* bl_ = nullptr;
	AlifLongObject* ret_ = nullptr;
	AlifLongObject* t1_{}, * t2_{}, * t3_{};
	AlifSizeT shift{};
	AlifSizeT i_{};


	if (aSize > bSize) {
		t1_ = _a;
		_a = _b;
		_b = t1_;

		i_ = aSize;
		aSize = bSize;
		bSize = i_;
	}

	i_ = _a == _b ? KARATSUBA_SQUARE_CUTOFF : KARATSUBA_CUTOFF;
	if (aSize <= i_) {
		if (aSize == 0)
			return (AlifLongObject*)alifLong_fromLong(0);
		else
			return x_mul(_a, _b);
	}

	if (2 * aSize <= bSize)
		return kLopsided_mul(_a, _b);

	shift = bSize >> 1;
	if (kMul_split(_a, shift, &ah_, &al_) < 0) goto fail;

	if (_a == _b) {
		bh_ = (AlifLongObject*)ALIF_NEWREF(ah_);
		bl_ = (AlifLongObject*)ALIF_NEWREF(al_);
	}
	else if (kMul_split(_b, shift, &bh_, &bl_) < 0) goto fail;

	ret_ = alifLong_new(aSize + bSize);
	if (ret_ == nullptr) goto fail;

	if ((t1_ = k_mul(ah_, bh_)) == nullptr) goto fail;
	memcpy(ret_->longValue.digit + 2 * shift, t1_->longValue.digit,
		alifLong_digitCount(t1_) * sizeof(digit));

	/* Zero-out the digits higher than the ah_*bh_ copy. */
	i_ = alifLong_digitCount(ret_) - 2 * shift - alifLong_digitCount(t1_);
	if (i_)
		memset(ret_->longValue.digit + 2 * shift + alifLong_digitCount(t1_), 0,
			i_ * sizeof(digit));

	if ((t2_ = k_mul(al_, bl_)) == nullptr) {
		ALIF_DECREF(t1_);
		goto fail;
	}
	memcpy(ret_->longValue.digit, t2_->longValue.digit, alifLong_digitCount(t2_) * sizeof(digit));

	i_ = 2 * shift - alifLong_digitCount(t2_);          /* number of uninitialized digits */
	if (i_)
		memset(ret_->longValue.digit + alifLong_digitCount(t2_), 0, i_ * sizeof(digit));

	i_ = alifLong_digitCount(ret_) - shift;  /* # digits after shift */
	(void)v_isub(ret_->longValue.digit + shift, i_, t2_->longValue.digit, alifLong_digitCount(t2_));
	_alif_decrefInt(t2_);

	(void)v_isub(ret_->longValue.digit + shift, i_, t1_->longValue.digit, alifLong_digitCount(t1_));
	_alif_decrefInt(t1_);

	/* 6. t3_ <- (ah_+al_)(bh_+bl_), and add into result. */
	if ((t1_ = x_add(ah_, al_)) == nullptr) goto fail;
	_alif_decrefInt(ah_);
	_alif_decrefInt(al_);
	ah_ = al_ = nullptr;

	if (_a == _b) {
		t2_ = (AlifLongObject*)ALIF_NEWREF(t1_);
	}
	else if ((t2_ = x_add(bh_, bl_)) == nullptr) {
		ALIF_DECREF(t1_);
		goto fail;
	}
	_alif_decrefInt(bh_);
	_alif_decrefInt(bl_);
	bh_ = bl_ = nullptr;

	t3_ = k_mul(t1_, t2_);
	_alif_decrefInt(t1_);
	_alif_decrefInt(t2_);
	if (t3_ == nullptr) goto fail;
	(void)v_iadd(ret_->longValue.digit + shift, i_, t3_->longValue.digit, alifLong_digitCount(t3_));
	_alif_decrefInt(t3_);

	return long_normalize(ret_);

fail:
	ALIF_XDECREF(ret_);
	ALIF_XDECREF(ah_);
	ALIF_XDECREF(al_);
	ALIF_XDECREF(bh_);
	ALIF_XDECREF(bl_);
	return nullptr;
}


static AlifLongObject* kLopsided_mul(AlifLongObject* _a, AlifLongObject* _b) { // 4198
	const AlifSizeT asize = alifLong_digitCount(_a);
	AlifSizeT bsize = alifLong_digitCount(_b);
	AlifSizeT nbdone{};          /* # of b digits already multiplied */
	AlifLongObject* ret{};
	AlifLongObject* bslice = nullptr;

	/* Allocate result space, and zero it out. */
	ret = alifLong_new(asize + bsize);
	if (ret == nullptr) return nullptr;
	memset(ret->longValue.digit, 0, alifLong_digitCount(ret) * sizeof(digit));

	/* Successive slices of b are copied into bslice. */
	bslice = alifLong_new(asize);
	if (bslice == nullptr) goto fail;

	nbdone = 0;
	while (bsize > 0) {
		AlifLongObject* product{};
		const AlifSizeT nbtouse = ALIF_MIN(bsize, asize);

		/* Multiply the next slice of b by a. */
		memcpy(bslice->longValue.digit, _b->longValue.digit + nbdone,
			nbtouse * sizeof(digit));
		_alifLong_setSignAndDigitCount(bslice, 1, nbtouse);
		product = k_mul(_a, bslice);
		if (product == nullptr) goto fail;

		/* Add into result. */
		(void)v_iadd(ret->longValue.digit + nbdone, alifLong_digitCount(ret) - nbdone,
			product->longValue.digit, alifLong_digitCount(product));
		_alif_decrefInt(product);

		bsize -= nbtouse;
		nbdone += nbtouse;
	}

	_alif_decrefInt(bslice);
	return long_normalize(ret);

fail:
	ALIF_DECREF(ret);
	ALIF_XDECREF(bslice);
	return nullptr;
}


static AlifLongObject* long_mul(AlifLongObject* _a, AlifLongObject* _b) { // 4254
	if (_alifLong_bothAreCompact(_a, _b)) {
		stwodigits v_ = MEDIUM_VALUE(_a) * MEDIUM_VALUE(_b);
		return _alifLong_fromSTwoDigits(v_);
	}

	AlifLongObject* z_ = k_mul(_a, _b);
	if (!_alifLong_sameSign(_a, _b) and z_) {
		_alifLong_negate(&z_);
		if (z_ == nullptr)
			return nullptr;
	}
	return z_;
}

static AlifObject* long_mulMethod(AlifObject* _a, AlifObject* _b) { // 4275
	CHECK_BINOP(_a, _b);
	return (AlifObject*)long_mul((AlifLongObject*)_a, (AlifLongObject*)_b);
}


static AlifObject* fast_mod(AlifLongObject* _a, AlifLongObject* _b) { // 4282
	sdigit left = _a->longValue.digit[0];
	sdigit right = _b->longValue.digit[0];
	sdigit mod_{};

	sdigit sign = _alifLong_compactSign(_b);
	if (_alifLong_sameSign(_a, _b)) {
		mod_ = left % right;
	}
	else {
		/* Either 'a' or 'b' is negative. */
		mod_ = right - 1 - (left - 1) % right;
	}

	return alifLong_fromLong(mod_ * sign);
}

static AlifObject* fast_floorDiv(AlifLongObject* _a, AlifLongObject* _b) { // 4305
	sdigit left = _a->longValue.digit[0];
	sdigit right = _b->longValue.digit[0];
	sdigit div_{};

	if (_alifLong_sameSign(_a, _b)) {
		div_ = left / right;
	}
	else {
		div_ = -1 - (left - 1) / right;
	}

	return alifLong_fromLong(div_);
}

#ifdef WITH_ALIFLONG_MODULE
static AlifIntT alifLong_intDivmod(AlifLongObject* _v, AlifLongObject* _w,
	AlifLongObject** _pdiv, AlifLongObject** _pmod) { // 4325
	//AlifObject* mod = alifImport_importModule("_aliflong");
	//if (mod == nullptr) {
	//	return -1;
	//}
	//AlifObject* result = alifObject_callMethod(mod, "int_divmod", "OO", _v, _w);
	//ALIF_DECREF(mod);
	//if (result == nullptr) {
	//	return -1;
	//}
	//if (!ALIFTUPLE_CHECK(result)) {
	//	ALIF_DECREF(result);
	//	alifErr_setString(_alifExcValueError_,
	//		"tuple is required from int_divmod()");
	//	return -1;
	//}
	//AlifObject* q = ALIFTUPLE_GET_ITEM(result, 0);
	//AlifObject* r = ALIFTUPLE_GET_ITEM(result, 1);
	//if (!ALIFLONG_CHECK(q) or !ALIFLONG_CHECK(r)) {
	//	ALIF_DECREF(result);
	//	alifErr_setString(_alifExcValueError_,
	//		"tuple of AlifIntT is required from int_divmod()");
	//	return -1;
	//}
	//if (_pdiv != nullptr) {
	//	*_pdiv = (AlifLongObject*)ALIF_NEWREF(q);
	//}
	//if (_pmod != nullptr) {
	//	*_pmod = (AlifLongObject*)ALIF_NEWREF(r);
	//}
	//ALIF_DECREF(result);
	//return 0;

	return -1; //* alif
}
#endif /* WITH_ALIFLONG_MODULE */

static AlifIntT l_divmod(AlifLongObject* _v, AlifLongObject* _w,
	AlifLongObject** _pDiv, AlifLongObject** _pMod) { // 4387
	AlifLongObject* div_{}, * mod_{};

	if (alifLong_digitCount(_v) == 1 and alifLong_digitCount(_w) == 1) {
		div_ = nullptr;
		if (_pDiv != nullptr) {
			div_ = (AlifLongObject*)fast_floorDiv(_v, _w);
			if (div_ == nullptr) {
				return -1;
			}
		}
		if (_pMod != nullptr) {
			mod_ = (AlifLongObject*)fast_mod(_v, _w);
			if (mod_ == nullptr) {
				ALIF_XDECREF(div_);
				return -1;
			}
			*_pMod = mod_;
		}
		if (_pDiv != nullptr) {
			*_pDiv = div_;
		}
		return 0;
	}
#if WITH_ALIFLONG_MODULE
	AlifSizeT sizeV = alifLong_digitCount(_v); /* digits in numerator */
	AlifSizeT sizeW = alifLong_digitCount(_w); /* digits in denominator */
	if (sizeW > 300 and (sizeV - sizeW) > 150) {
		return alifLong_intDivmod(_v, _w, _pDiv, _pMod);
	}
#endif
	if (long_divrem(_v, _w, &div_, &mod_) < 0)
		return -1;
	if ((_alifLong_isNegative(mod_) and _alifLong_isPositive(_w)) or
		(_alifLong_isPositive(mod_) and _alifLong_isNegative(_w))) {
		AlifLongObject* temp{};
		temp = long_add(mod_, _w);
		ALIF_SETREF(mod_, temp);
		if (mod_ == nullptr) {
			ALIF_DECREF(div_);
			return -1;
		}
		temp = long_sub(div_, (AlifLongObject*)_alifLong_getOne());
		if (temp == nullptr) {
			ALIF_DECREF(mod_);
			ALIF_DECREF(div_);
			return -1;
		}
		ALIF_SETREF(div_, temp);
	}
	if (_pDiv != nullptr)
		*_pDiv = div_;
	else
		ALIF_DECREF(div_);

	if (_pMod != nullptr)
		*_pMod = mod_;
	else
		ALIF_DECREF(mod_);

	return 0;
}

static AlifIntT l_mod(AlifLongObject* _v, AlifLongObject* _w, AlifLongObject** _pmod) { // 4464
	AlifLongObject* mod_{};

	if (alifLong_digitCount(_v) == 1 and alifLong_digitCount(_w) == 1) {
		/* Fast path for single-digit longs */
		*_pmod = (AlifLongObject*)fast_mod(_v, _w);
		return -(*_pmod == nullptr);
	}
	if (long_rem(_v, _w, &mod_) < 0)
		return -1;
	if ((_alifLong_isNegative(mod_) and _alifLong_isPositive(_w)) or
		(_alifLong_isPositive(mod_) and _alifLong_isNegative(_w))) {
		AlifLongObject* temp{};
		temp = long_add(mod_, _w);
		ALIF_SETREF(mod_, temp);
		if (mod_ == nullptr) return -1;
	}
	*_pmod = mod_;

	return 0;
}

static AlifObject* long_div(AlifObject* _a, AlifObject* _b) { // 4490
	AlifLongObject* div_{};

	CHECK_BINOP(_a, _b);

	if (alifLong_digitCount((AlifLongObject*)_a) == 1
		and alifLong_digitCount((AlifLongObject*)_b) == 1) {
		return fast_floorDiv((AlifLongObject*)_a, (AlifLongObject*)_b);
	}

	if (l_divmod((AlifLongObject*)_a, (AlifLongObject*)_b, &div_, nullptr) < 0) div_ = nullptr;
	return (AlifObject*)div_;
}

#define MANT_DIG_DIGITS (DBL_MANT_DIG / ALIFLONG_SHIFT) // 4508
#define MANT_DIG_BITS (DBL_MANT_DIG % ALIFLONG_SHIFT) // 4509


static AlifObject* long_trueDivide(AlifObject* _v, AlifObject* _w) { // 4512
	AlifLongObject* a_{}, * b_{}, * x_{};
	AlifSizeT aSize{}, bSize{}, shift{}, extraBits{}, diff{}, xSize{}, xBits{};
	digit mask{}, low_{};
	AlifIntT inexact{}, negate{}, aIsSmall{}, bIsSmall{};
	double dx_{}, result{};

	CHECK_BINOP(_v, _w);
	a_ = (AlifLongObject*)_v;
	b_ = (AlifLongObject*)_w;
	aSize = alifLong_digitCount(a_);
	bSize = alifLong_digitCount(b_);
	negate = (_alifLong_isNegative(a_)) != (_alifLong_isNegative(b_));
	if (bSize == 0) {
		//alifErr_setString(_alifExcZeroDivisionError_,
			//"division by zero");
		goto error;
	}
	if (aSize == 0)
		goto underflow_or_zero;

	aIsSmall = aSize <= MANT_DIG_DIGITS or
		(aSize == MANT_DIG_DIGITS + 1 and
			a_->longValue.digit[MANT_DIG_DIGITS] >> MANT_DIG_BITS == 0);
	bIsSmall = bSize <= MANT_DIG_DIGITS or
		(bSize == MANT_DIG_DIGITS + 1 and
			b_->longValue.digit[MANT_DIG_DIGITS] >> MANT_DIG_BITS == 0);
	if (aIsSmall and bIsSmall) {
		double da_{}, db_{};
		da_ = a_->longValue.digit[--aSize];
		while (aSize > 0)
			da_ = da_ * ALIFLONG_BASE + a_->longValue.digit[--aSize];
		db_ = b_->longValue.digit[--bSize];
		while (bSize > 0)
			db_ = db_ * ALIFLONG_BASE + b_->longValue.digit[--bSize];
		result = da_ / db_;
		goto success;
	}

	diff = aSize - bSize;
	if (diff > ALIF_SIZET_MAX / ALIFLONG_SHIFT - 1)
		goto overflow;
	else if (diff < 1 - ALIF_SIZET_MAX / ALIFLONG_SHIFT)
		goto underflow_or_zero;
	diff = diff * ALIFLONG_SHIFT + bit_lengthDigit(a_->longValue.digit[aSize - 1]) -
		bit_lengthDigit(b_->longValue.digit[bSize - 1]);
	if (diff > DBL_MAX_EXP)
		goto overflow;
	else if (diff < DBL_MIN_EXP - DBL_MANT_DIG - 1)
		goto underflow_or_zero;

	shift = ALIF_MAX(diff, DBL_MIN_EXP) - DBL_MANT_DIG - 2;

	inexact = 0;

	if (shift <= 0) {
		AlifSizeT i_{}, shiftDigits = -shift / ALIFLONG_SHIFT;
		digit rem_{};
		if (aSize >= ALIF_SIZET_MAX - 1 - shiftDigits) {
			
			//alifErr_setString(_alifExcOverflowError_,
				//"intermediate overflow during division");
			goto error;
		}
		x_ = alifLong_new(aSize + shiftDigits + 1);
		if (x_ == nullptr)
			goto error;
		for (i_ = 0; i_ < shiftDigits; i_++)
			x_->longValue.digit[i_] = 0;
		rem_ = v_lShift(x_->longValue.digit + shiftDigits, a_->longValue.digit,
			aSize, -shift % ALIFLONG_SHIFT);
		x_->longValue.digit[aSize + shiftDigits] = rem_;
	}
	else {
		AlifSizeT shiftDigits = shift / ALIFLONG_SHIFT;
		digit rem_{};
		x_ = alifLong_new(aSize - shiftDigits);
		if (x_ == nullptr)
			goto error;
		rem_ = v_rShift(x_->longValue.digit, a_->longValue.digit + shiftDigits,
			aSize - shiftDigits, shift % ALIFLONG_SHIFT);
		if (rem_)
			inexact = 1;
		while (!inexact and shiftDigits > 0)
			if (a_->longValue.digit[--shiftDigits])
				inexact = 1;
	}
	long_normalize(x_);
	xSize = _alifLong_signedDigitCount(x_);

	if (bSize == 1) {
		digit rem_ = inplace_divrem1(x_->longValue.digit, x_->longValue.digit, xSize,
			b_->longValue.digit[0]);
		long_normalize(x_);
		if (rem_)
			inexact = 1;
	}
	else {
		AlifLongObject* div_{}, * rem_{};
		div_ = x_divrem(x_, b_, &rem_);
		ALIF_SETREF(x_, div_);
		if (x_ == nullptr)
			goto error;
		if (!_alifLong_isZero(rem_))
			inexact = 1;
		ALIF_DECREF(rem_);
	}
	xSize = alifLong_digitCount(x_);
	xBits = (xSize - 1) * ALIFLONG_SHIFT + bit_lengthDigit(x_->longValue.digit[xSize - 1]);

	extraBits = ALIF_MAX(xBits, DBL_MIN_EXP - shift) - DBL_MANT_DIG;

	mask = (digit)1 << (extraBits - 1);
	low_ = x_->longValue.digit[0] | inexact;
	if ((low_ & mask) and (low_ & (3U * mask - 1U)))
		low_ += mask;
	x_->longValue.digit[0] = low_ & ~(2U * mask - 1U);

	dx_ = x_->longValue.digit[--xSize];
	while (xSize > 0)
		dx_ = dx_ * ALIFLONG_BASE + x_->longValue.digit[--xSize];
	ALIF_DECREF(x_);

	if (shift + xBits >= DBL_MAX_EXP and
		(shift + xBits > DBL_MAX_EXP or dx_ == ldexp(1.0, (AlifIntT)xBits)))
		goto overflow;
	result = ldexp(dx_, (AlifIntT)shift);

success:
	return alifFloat_fromDouble(negate ? -result : result);

underflow_or_zero:
	return alifFloat_fromDouble(negate ? -0.0 : 0.0);

overflow:
	//alifErr_setString(_alifExcOverflowError_,
		//"integer division result too large for a float");
error:
	return nullptr;
}

static AlifObject* long_mod(AlifObject* _a, AlifObject* _b) { // 4768
	AlifLongObject* mod_{};

	CHECK_BINOP(_a, _b);

	if (l_mod((AlifLongObject*)_a, (AlifLongObject*)_b, &mod_) < 0) mod_ = nullptr;
	return (AlifObject*)mod_;
}

static AlifObject* long_divmod(AlifObject* _a, AlifObject* _b) { // 4781
	AlifLongObject* div_{}, * mod_{};
	AlifObject* z_{};

	CHECK_BINOP(_a, _b);

	if (l_divmod((AlifLongObject*)_a, (AlifLongObject*)_b, &div_, &mod_) < 0) {
		return nullptr;
	}
	z_ = alifTuple_new(2);
	if (z_ != nullptr) {
		ALIFTUPLE_SET_ITEM(z_, 0, (AlifObject*)div_);
		ALIFTUPLE_SET_ITEM(z_, 1, (AlifObject*)mod_);
	}
	else {
		ALIF_DECREF(div_);
		ALIF_DECREF(mod_);
	}
	return z_;
}

static AlifLongObject* long_invmod(AlifLongObject* _a, AlifLongObject* _n) { // 4825
	AlifLongObject* b_{}, * c_{};

	b_ = (AlifLongObject*)alifLong_fromLong(1L);
	if (b_ == nullptr) {
		return nullptr;
	}
	c_ = (AlifLongObject*)alifLong_fromLong(0L);
	if (c_ == nullptr) {
		ALIF_DECREF(b_);
		return nullptr;
	}
	ALIF_INCREF(_a);
	ALIF_INCREF(_n);

	while (!_alifLong_isZero(_n)) {
		AlifLongObject* q_{}, * r_{}, * s_{}, * t_{};

		if (l_divmod(_a, _n, &q_, &r_) == -1) {
			goto Error;
		}
		ALIF_SETREF(_a, _n);
		_n = r_;
		t_ = (AlifLongObject*)long_mul(q_, c_);
		ALIF_DECREF(q_);
		if (t_ == nullptr) {
			goto Error;
		}
		s_ = long_sub(b_, t_);
		ALIF_DECREF(t_);
		if (s_ == nullptr) {
			goto Error;
		}
		ALIF_SETREF(b_, c_);
		c_ = s_;
	}

	ALIF_DECREF(c_);
	ALIF_DECREF(_n);
	if (long_compare(_a, (AlifLongObject*)_alifLong_getOne())) {
		ALIF_DECREF(_a);
		ALIF_DECREF(b_);
		//alifErr_setString(_alifExcValueError_,
			//"base is not invertible for the given modulus");
		return nullptr;
	}
	else {
		ALIF_DECREF(_a);
		return b_;
	}

Error:
	ALIF_DECREF(_a);
	ALIF_DECREF(b_);
	ALIF_DECREF(c_);
	ALIF_DECREF(_n);
	return nullptr;
}

static AlifObject* long_pow(AlifObject* _v, AlifObject* _w, AlifObject* _x) { // 4895
	AlifLongObject* a_{}, * b_{}, * c_{};
	AlifIntT negativeOutput = 0;

	AlifLongObject* z_ = nullptr; 
	AlifSizeT i_{}, j_{};
	AlifLongObject* temp = nullptr;
	AlifLongObject* a2_ = nullptr; 

	AlifLongObject* table[EXP_TABLE_LEN];
	AlifSizeT numTableEntries = 0;

	digit bi_{};
	digit bit_{};

	CHECK_BINOP(_v, _w);
	a_ = (AlifLongObject*)ALIF_NEWREF(_v);
	b_ = (AlifLongObject*)ALIF_NEWREF(_w);
	if (ALIFLONG_CHECK(_x)) {
		c_ = (AlifLongObject*)ALIF_NEWREF(_x);
	}
	else if (_x == ALIF_NONE)
		c_ = nullptr;
	else {
		ALIF_DECREF(a_);
		ALIF_DECREF(b_);
		return ALIF_NOTIMPLEMENTED;
	}

	if (_alifLong_isNegative(b_) and c_ == nullptr) {
		ALIF_DECREF(a_);
		ALIF_DECREF(b_);
		return _alifFloatType_.asNumber->power(_v, _w, _x);
	}

	if (c_) {

		if (_alifLong_isZero(c_)) {
			//_alifErr_setString(_alifExcValueError_,
				//"pow() 3rd argument cannot be 0");
			goto error;
		}

		if (_alifLong_isNegative(c_)) {
			negativeOutput = 1;
			temp = (AlifLongObject*)_alifLong_copy(c_);
			if (temp == nullptr)
				goto error;
			ALIF_SETREF(c_, temp);
			temp = nullptr;
			_alifLong_negate(&c_);
			if (c_ == nullptr)
				goto error;
		}
		if (_alifLong_isNonNegativeCompact(c_) and (c_->longValue.digit[0] == 1)) {
			z_ = (AlifLongObject*)alifLong_fromLong(0L);
			goto done;
		}

		if (_alifLong_isNegative(b_)) {
			temp = (AlifLongObject*)_alifLong_copy(b_);
			if (temp == nullptr)
				goto error;
			ALIF_SETREF(b_, temp);
			temp = nullptr;
			_alifLong_negate(&b_);
			if (b_ == nullptr)
				goto error;

			temp = long_invmod(a_, c_);
			if (temp == nullptr)
				goto error;
			ALIF_SETREF(a_, temp);
			temp = nullptr;
		}
		if (_alifLong_isNegative(a_) or alifLong_digitCount(a_) > alifLong_digitCount(c_)) {
			if (l_mod(a_, c_, &temp) < 0)
				goto error;
			ALIF_SETREF(a_, temp);
			temp = nullptr;
		}
	}

	z_ = (AlifLongObject*)alifLong_fromLong(1L);
	if (z_ == nullptr)
		goto error;


#define REDUCE(_x)                                       \
    do {                                                \
        if (c_ != nullptr) {                                \
            if (l_mod(_x, c_, &temp) < 0)                 \
                goto error;                             \
            ALIF_XDECREF(_x);                              \
            _x = temp;                                   \
            temp = nullptr;                                \
        }                                               \
    } while(0)

#define MULT(_x, _y, _result)                      \
    do {                                        \
        temp = (AlifLongObject *)long_mul(_x, _y);  \
        if (temp == nullptr)                       \
            goto error;                         \
        ALIF_XDECREF(_result);                     \
        _result = temp;                          \
        temp = nullptr;                            \
        REDUCE(_result);                         \
    } while(0)

	i_ = _alifLong_signedDigitCount(b_);
	bi_ = i_ ? b_->longValue.digit[i_ - 1] : 0;
	if (i_ <= 1 and bi_ <= 3) {
		if (bi_ >= 2) {
			MULT(a_, a_, z_);
			if (bi_ == 3) {
				MULT(z_, a_, z_);
			}
		}
		else if (bi_ == 1) {

			MULT(a_, z_, z_);
		}
	}
	else if (i_ <= HUGE_EXP_CUTOFF / ALIFLONG_SHIFT) {
		ALIF_SETREF(z_, (AlifLongObject*)ALIF_NEWREF(a_));
		for (bit_ = 2; ; bit_ <<= 1) {
			if (bit_ > bi_) { 
				bit_ >>= 1;
				break;
			}
		}
		for (--i_, bit_ >>= 1;;) {
			for (; bit_ != 0; bit_ >>= 1) {
				MULT(z_, z_, z_);
				if (bi_ & bit_) {
					MULT(z_, a_, z_);
				}
			}
			if (--i_ < 0) {
				break;
			}
			bi_ = b_->longValue.digit[i_];
			bit_ = (digit)1 << (ALIFLONG_SHIFT - 1);
		}
	}
	else {

		table[0] = (AlifLongObject*)ALIF_NEWREF(a_);
		numTableEntries = 1;
		MULT(a_, a_, a2_);
		for (i_ = 1; i_ < EXP_TABLE_LEN; ++i_) {
			table[i_] = nullptr;
			MULT(table[i_ - 1], a2_, table[i_]);
			++numTableEntries; 
		}
		ALIF_CLEAR(a2_);

		AlifIntT pending = 0, bLen = 0;
#define ABSORB_PENDING  do { \
            AlifIntT ntz_ = 0;  \
            while ((pending & 1) == 0) { \
                ++ntz_; \
                pending >>= 1; \
            } \
            bLen -= ntz_; \
            do { \
                MULT(z_, z_, z_); \
            } while (--bLen); \
            MULT(z_, table[pending >> 1], z_); \
            while (ntz_-- > 0) \
                MULT(z_, z_, z_); \
            pending = 0; \
        } while(0)

		for (i_ = _alifLong_signedDigitCount(b_) - 1; i_ >= 0; --i_) {
			const digit bi_ = b_->longValue.digit[i_];
			for (j_ = ALIFLONG_SHIFT - 1; j_ >= 0; --j_) {
				const AlifIntT bit_ = (bi_ >> j_) & 1;
				pending = (pending << 1) | bit_;
				if (pending) {
					++bLen;
					if (bLen == EXP_WINDOW_SIZE)
						ABSORB_PENDING;
				}
				else 
					MULT(z_, z_, z_);
			}
		}
		if (pending)
			ABSORB_PENDING;
	}

	if (negativeOutput and !_alifLong_isZero(z_)) {
		temp = long_sub(z_, c_);
		if (temp == nullptr)
			goto error;
		ALIF_SETREF(z_, temp);
		temp = nullptr;
	}
	goto done;

error:
	ALIF_CLEAR(z_);
done:
	for (i_ = 0; i_ < numTableEntries; ++i_)
		ALIF_DECREF(table[i_]);
	ALIF_DECREF(a_);
	ALIF_DECREF(b_);
	ALIF_XDECREF(c_);
	ALIF_XDECREF(a2_);
	ALIF_XDECREF(temp);
	return (AlifObject*)z_;
}


static AlifObject* long_sqrt(AlifObject* _v) { //* alif
	AlifLongObject* a_{}, *res{};

	a_ = (AlifLongObject*)ALIF_NEWREF(_v);
	AlifIntT sizeA = alifLong_digitCount(a_);

	if (_alifLong_isNegative(a_)) {
		//error
		return nullptr;
	}

	AlifSizeT val = alifLong_asSizeT((AlifObject*)a_);

	AlifSizeT sqrtVal = sqrt(val);

	res = alifLong_new(sizeA);
	if (res == nullptr) return nullptr;

	res->longValue.digit[0] = sqrtVal;

	ALIF_DECREF(a_);
	return (AlifObject*)res;
}


static AlifObject* long_invert(AlifObject* _self) { // 5175
	AlifLongObject* v = ALIFLONG_CAST(_self);

	/* Implement ~x as -(x+1) */
	if (alifLong_isCompact(v))
		return (AlifObject*)_alifLong_fromSTwoDigits(~MEDIUM_VALUE(v));

	AlifLongObject* x = long_add(v, (AlifLongObject*)_alifLong_getOne());
	if (x == nullptr) return nullptr;
	_alifLong_negate(&x);
	/* No need for maybe_small_long here, since any small longs
	   will have been caught in the alifLong_isCompact() fast path. */
	return (AlifObject*)x;
}

static AlifLongObject* long_neg(AlifLongObject* _v) { // 5191

	if (alifLong_isCompact(_v))
		return _alifLong_fromSTwoDigits(-MEDIUM_VALUE(_v));

	AlifLongObject* z = (AlifLongObject*)_alifLong_copy(_v);
	if (z != nullptr) _alifLong_flipSign(z);
	return z;
}

static AlifObject* long_negMethod(AlifObject* _v) { // 5211
	return (AlifObject*)long_neg(ALIFLONG_CAST(_v));
}

static AlifLongObject* long_abs(AlifLongObject* _v) { // 5203
	if (_alifLong_isNegative(_v))
		return long_neg(_v);
	else
		return (AlifLongObject*)long_long((AlifObject*)_v);
}

static AlifObject* long_absMethod(AlifObject* _v) { // 5226
	return (AlifObject*)long_abs(ALIFLONG_CAST(_v));
}

static AlifIntT long_bool(AlifObject* _v) { // 5212
	return !_alifLong_isZero(ALIFLONG_CAST(_v));
}



static AlifObject* long_rShift1(AlifLongObject* _a,
	AlifSizeT _wordShift, digit _remShift) { // 5256
	AlifLongObject* z_ = nullptr;
	AlifSizeT newSize{}, hiShift{}, sizeA{};
	twodigits accum{};
	AlifIntT aNegative{};

	if (alifLong_isCompact(_a)) {
		stwodigits m_{}, x_{};
		digit shift{};
		m_ = MEDIUM_VALUE(_a);
		shift = _wordShift == 0 ? _remShift : ALIFLONG_SHIFT;
		x_ = m_ < 0 ? ~(~m_ >> shift) : m_ >> shift;
		return (AlifObject*)_alifLong_fromSTwoDigits(x_);
	}

	aNegative = _alifLong_isNegative(_a);
	sizeA = alifLong_digitCount(_a);

	if (aNegative) {

		if (_remShift == 0) {
			if (_wordShift == 0) {
				return long_long((AlifObject*)_a);
			}
			_remShift = ALIFLONG_SHIFT;
			--_wordShift;
		}
	}

	newSize = sizeA - _wordShift;
	if (newSize <= 0) {
		return alifLong_fromLong(-aNegative);
	}
	z_ = alifLong_new(newSize);
	if (z_ == nullptr) {
		return nullptr;
	}
	hiShift = ALIFLONG_SHIFT - _remShift;

	accum = _a->longValue.digit[_wordShift];
	if (aNegative) {

		_alifLong_setSignAndDigitCount(z_, -1, newSize);

		digit sticky = 0;
		for (AlifSizeT j = 0; j < _wordShift; j++) {
			sticky |= _a->longValue.digit[j];
		}
		accum += (ALIFLONG_MASK >> hiShift) + (digit)(sticky != 0);
	}

	accum >>= _remShift;
	for (AlifSizeT i = 0, j = _wordShift + 1; j < sizeA; i++, j++) {
		accum += (twodigits)_a->longValue.digit[j] << hiShift;
		z_->longValue.digit[i] = (digit)(accum & ALIFLONG_MASK);
		accum >>= ALIFLONG_SHIFT;
	}
	z_->longValue.digit[newSize - 1] = (digit)accum;

	z_ = maybe_smallLong(long_normalize(z_));
	return (AlifObject*)z_;
}

static AlifObject* long_rShift(AlifObject* _a, AlifObject* _b) { // 5294
	int64_t shiftBy{};

	CHECK_BINOP(_a, _b);

	if (_alifLong_isNegative((AlifLongObject*)_b)) {
		//alifErr_setString(_alifExcValueError, "negative shift count");
		return nullptr;
	}
	if (_alifLong_isZero((AlifLongObject*)_a)) {
		return alifLong_fromLong(0);
	}
	if (alifLong_asInt64(_b, &shiftBy) < 0) {
		//if (!alifErr_exceptionMatches(_alifExcOverflowError_)) {
		//	return nullptr;
		//}
		//alifErr_clear();
		if (_alifLong_isNegative((AlifLongObject*)_a)) {
			return alifLong_fromLong(-1);
		}
		else {
			return alifLong_fromLong(0);
		}
	}
	return _alifLong_rShift(_a, shiftBy);
}

AlifObject* _alifLong_rShift(AlifObject* _a, int64_t _shiftBy) { // 5324
	AlifSizeT wordshift{};
	digit remshift{};

	if (_alifLong_isZero((AlifLongObject*)_a)) {
		return alifLong_fromLong(0);
	}
#if ALIF_SIZET_MAX <= INT64_MAX / ALIFLONG_SHIFT
	if (_shiftBy > (int64_t)ALIF_SIZET_MAX * ALIFLONG_SHIFT) {
		if (_alifLong_isNegative((AlifLongObject*)_a)) {
			return alifLong_fromLong(-1);
		}
		else {
			return alifLong_fromLong(0);
		}
	}
#endif
	wordshift = (AlifSizeT)(_shiftBy / ALIFLONG_SHIFT);
	remshift = (digit)(_shiftBy % ALIFLONG_SHIFT);
	return long_rShift1((AlifLongObject*)_a, wordshift, remshift);
}

static AlifObject* long_lShift1(AlifLongObject* _a, AlifSizeT _wordShift, digit _remShift) { // 5377
	AlifLongObject* z_ = nullptr;
	AlifSizeT oldSize{}, newSize{}, i_{}, j_{};
	twodigits accum{};

	if (_wordShift == 0 and alifLong_isCompact(_a)) {
		stwodigits m_ = MEDIUM_VALUE(_a);
		stwodigits x_ = m_ < 0 ? -(-m_ << _remShift) : m_ << _remShift;
		return (AlifObject*)_alifLong_fromSTwoDigits(x_);
	}

	oldSize = alifLong_digitCount(_a);
	newSize = oldSize + _wordShift;
	if (_remShift)
		++newSize;
	z_ = alifLong_new(newSize);
	if (z_ == nullptr)
		return nullptr;
	if (_alifLong_isNegative(_a)) {
		_alifLong_flipSign(z_);
	}
	for (i_ = 0; i_ < _wordShift; i_++)
		z_->longValue.digit[i_] = 0;
	accum = 0;
	for (j_ = 0; j_ < oldSize; i_++, j_++) {
		accum |= (twodigits)_a->longValue.digit[j_] << _remShift;
		z_->longValue.digit[i_] = (digit)(accum & ALIFLONG_MASK);
		accum >>= ALIFLONG_SHIFT;
	}
	if (_remShift)
		z_->longValue.digit[newSize - 1] = (digit)accum;
	else
	z_ = long_normalize(z_);
	return (AlifObject*)maybe_smallLong(z_);
}

static AlifObject* long_lShiftMethod(AlifObject* _aa, AlifObject* _bb) { // 5419

	CHECK_BINOP(_aa, _bb);
	AlifLongObject* a = (AlifLongObject*)_aa;
	AlifLongObject* b = (AlifLongObject*)_bb;

	if (_alifLong_isNegative(b)) {
		//alifErr_setString(_alifExcValueError_, "negative shift count");
		return nullptr;
	}
	if (_alifLong_isZero(a)) {
		return alifLong_fromLong(0);
	}

	int64_t shiftBy{};
	if (alifLong_asInt64(_bb, &shiftBy) < 0) {
		//if (alifErr_exceptionMatches(_alifExcOverflowError_)) {
		//	alifErr_setString(_alifExcOverflowError_,
		//		"too many digits in integer");
		//}
		return nullptr;
	}
	return long_lShiftInt64(a, shiftBy);
}

static AlifObject* long_lShiftInt64(AlifLongObject* _a, int64_t _shiftby) { // 5439

	if (_alifLong_isZero(_a)) {
		return alifLong_fromLong(0);
	}
#if ALIF_SIZET_MAX <= INT64_MAX / ALIFLONG_SHIFT
	if (_shiftby > (int64_t)ALIF_SIZET_MAX * ALIFLONG_SHIFT) {
		//alifErr_setString(_alifExcOverflowError_,
		//	"too many digits in integer");
		return nullptr;
	}
#endif
	AlifSizeT wordshift = (AlifSizeT)(_shiftby / ALIFLONG_SHIFT);
	digit remshift = (digit)(_shiftby % ALIFLONG_SHIFT);
	return long_lShift1(_a, wordshift, remshift);
}

AlifObject* _alifLong_lShift(AlifObject* _a, int64_t _shiftby) { // 5473
	return long_lShiftInt64(ALIFLONG_CAST(_a), _shiftby);
}

static void v_complement(digit* _z, digit* _a, AlifSizeT _m) { // 5458
	AlifSizeT i{};
	digit carry = 1;
	for (i = 0; i < _m; ++i) {
		carry += _a[i] ^ ALIFLONG_MASK;
		_z[i] = carry & ALIFLONG_MASK;
		carry >>= ALIFLONG_SHIFT;
	}
}

static AlifObject* long_bitwise(AlifLongObject* _a,
	char _op,  /* '&', '|', '^' */ AlifLongObject* _b) { // 5473
	AlifIntT nega{}, negb{}, negz{};
	AlifSizeT sizeA{}, sizeB{}, sizeZ{}, i_{};
	AlifLongObject* z{};

	   /* If a is negative, replace it by its two's complement. */
	sizeA = alifLong_digitCount(_a);
	nega = _alifLong_isNegative(_a);
	if (nega) {
		z = alifLong_new(sizeA);
		if (z == nullptr) return nullptr;
		v_complement(z->longValue.digit, _a->longValue.digit, sizeA);
		_a = z;
	}
	else
		/* Keep reference count consistent. */
		ALIF_INCREF(_a);

	/* Same for b. */
	sizeB = alifLong_digitCount(_b);
	negb = _alifLong_isNegative(_b);
	if (negb) {
		z = alifLong_new(sizeB);
		if (z == nullptr) {
			ALIF_DECREF(_a);
			return nullptr;
		}
		v_complement(z->longValue.digit, _b->longValue.digit, sizeB);
		_b = z;
	}
	else ALIF_INCREF(_b);

	/* Swap a and b if necessary to ensure size_a >= size_b. */
	if (sizeA < sizeB) {
		z = _a; _a = _b; _b = z;
		sizeZ = sizeA; sizeA = sizeB; sizeB = sizeZ;
		negz = nega; nega = negb; negb = negz;
	}

	switch (_op) {
	case '^':
		negz = nega ^ negb;
		sizeZ = sizeA;
		break;
	case '&':
		negz = nega & negb;
		sizeZ = negb ? sizeA : sizeB;
		break;
	case '|':
		negz = nega | negb;
		sizeZ = negb ? sizeB : sizeA;
		break;
	default:
		ALIF_UNREACHABLE();
	}

	z = alifLong_new(sizeZ + negz);
	if (z == nullptr) {
		ALIF_DECREF(_a);
		ALIF_DECREF(_b);
		return nullptr;
	}

	/* Compute digits for overlap of a and b. */
	switch (_op) {
	case '&':
		for (i_ = 0; i_ < sizeB; ++i_)
			z->longValue.digit[i_] = _a->longValue.digit[i_] & _b->longValue.digit[i_];
		break;
	case '|':
		for (i_ = 0; i_ < sizeB; ++i_)
			z->longValue.digit[i_] = _a->longValue.digit[i_] | _b->longValue.digit[i_];
		break;
	case '^':
		for (i_ = 0; i_ < sizeB; ++i_)
			z->longValue.digit[i_] = _a->longValue.digit[i_] ^ _b->longValue.digit[i_];
		break;
	default:
		ALIF_UNREACHABLE();
	}

	/* Copy any remaining digits of a, inverting if necessary. */
	if (_op == '^' and negb)
		for (; i_ < sizeZ; ++i_)
			z->longValue.digit[i_] = _a->longValue.digit[i_] ^ ALIFLONG_MASK;
	else if (i_ < sizeZ)
		memcpy(&z->longValue.digit[i_], &_a->longValue.digit[i_],
			(sizeZ - i_) * sizeof(digit));

	/* Complement result if negative. */
	if (negz) {
		_alifLong_flipSign(z);
		z->longValue.digit[sizeZ] = ALIFLONG_MASK;
		v_complement(z->longValue.digit, z->longValue.digit, sizeZ + 1);
	}

	ALIF_DECREF(_a);
	ALIF_DECREF(_b);
	return (AlifObject*)maybe_smallLong(long_normalize(z));
}

static AlifObject* long_and(AlifObject* _a, AlifObject* _b) { // 5594
	CHECK_BINOP(_a, _b);
	AlifLongObject* x = (AlifLongObject*)_a;
	AlifLongObject* y = (AlifLongObject*)_b;
	if (alifLong_isCompact(x) and alifLong_isCompact(y)) {
		return (AlifObject*)_alifLong_fromSTwoDigits(MEDIUM_VALUE(x) & MEDIUM_VALUE(y));
	}
	return long_bitwise(x, '&', y);
}

static AlifObject* long_xor(AlifObject* _a, AlifObject* _b) { // 5606
	CHECK_BINOP(_a, _b);
	AlifLongObject* x = (AlifLongObject*)_a;
	AlifLongObject* y = (AlifLongObject*)_b;
	if (alifLong_isCompact(x) and alifLong_isCompact(y)) {
		return (AlifObject*)_alifLong_fromSTwoDigits(MEDIUM_VALUE(x) ^ MEDIUM_VALUE(y));
	}
	return long_bitwise(x, '^', y);
}

static AlifObject* long_or(AlifObject* _a, AlifObject* _b) { // 5618
	CHECK_BINOP(_a, _b);
	AlifLongObject* x = (AlifLongObject*)_a;
	AlifLongObject* y = (AlifLongObject*)_b;
	if (alifLong_isCompact(x) and alifLong_isCompact(y)) {
		return (AlifObject*)_alifLong_fromSTwoDigits(MEDIUM_VALUE(x) | MEDIUM_VALUE(y));
	}
	return long_bitwise(x, '|', y);
}


static AlifObject* long_long(AlifObject* _v) { // 5630
	if (ALIFLONG_CHECKEXACT(_v)) {
		return ALIF_NEWREF(_v);
	}
	else {
		return _alifLong_copy((AlifLongObject*)_v);
	}
}


static AlifObject* long_float(AlifObject* _v) { // 5848
	double result{};
	result = alifLong_asDouble(_v);
	if (result == -1.0 and alifErr_occurred()) return nullptr;
	return alifFloat_fromDouble(result);
}





static AlifObject* long_vectorCall(AlifObject* type, AlifObject* const* args,
	AlifUSizeT nargsf, AlifObject* kwnames) { // 6479
	AlifUSizeT nargs = ALIFVECTORCALL_NARGS(nargsf);
	if (kwnames != nullptr) {
		AlifThread* thread = alifThread_get();
		return alifObject_makeTpCall(thread, type, args, nargs, kwnames);
	}
	switch (nargs) {
	case 0:
		return _alifLong_getZero();
	case 1:
		return alifNumber_long(args[0]);
	//case 2:
	//	return long_newImpl(ALIFTYPE_CAST(type), args[0], args[1]);
	//default:
	//	return alifErr_format(_alifExcTypeError_,
	//		"int expected at most 2 arguments, got %zd",
	//		nargs);
	}
}



static AlifNumberMethods _longAsNumber_ = { // 6560
	.add_ = long_addMethod,  
	.subtract = long_subMethod,
	.multiply = long_mulMethod,  
	.remainder = long_mod,              
	.divmod = long_divmod,           
	.power = long_pow,              
	.negative = long_negMethod,   
	.positive = long_long,        
	.absolute = long_absMethod,
	.sqrt = long_sqrt,
	.bool_ = long_bool,    
	.invert = long_invert,
	.lshift = long_lShiftMethod,           
	.rshift = long_rShift,           
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
	.trueDivide = long_trueDivide,      
	.inplaceFloorDivide = 0,                     
	.inplaceTrueDivide = 0,                     
	.index = long_long,
};

AlifTypeObject _alifLongType_ = { // 6597
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "صحيح",                                   
	.basicSize = offsetof(AlifLongObject, longValue.digit),
	.itemSize = sizeof(digit),                              
	.dealloc = long_dealloc,                               
                                        
	.repr = longTo_decimalString,                     
	.asNumber = &_longAsNumber_,
                                        
	.hash = long_hash,                              
                                        
	.getAttro = alifObject_genericGetAttr,                    
                                        
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		ALIF_TPFLAGS_LONG_SUBCLASS |
		_ALIF_TPFLAGS_MATCH_SELF,

	.richCompare = long_richCompare,
                                       
	.free = alifMem_objFree,
	.vectorCall = long_vectorCall,
};



 // 6661
#define LONG_TO_INT(obj, value, type_name) \
    do { \
        AlifIntT flags = (ALIF_ASNATIVEBYTES_NATIVE_ENDIAN \
                     | ALIF_ASNATIVEBYTES_ALLOW_INDEX); \
        AlifSizeT bytes = alifLong_asNativeBytes(obj, value, sizeof(*value), flags); \
        if (bytes < 0) { \
            return -1; \
        } \
        if ((AlifUSizeT)bytes > sizeof(*value)) { \
            /*alifErr_setString(_alfiExcOverflowError_, \
                            "Alif int too large to convert to " type_name);*/ \
            return -1; \
        } \
        return 0; \
    } while (0)

AlifIntT alifLong_asInt64(AlifObject* _obj, int64_t* _value) { // 6682
	LONG_TO_INT(_obj, _value, "CPP int64_t");
}
