#include "alif.h"

#include "AlifCore_Long.h"
#include "AlifCore_Object.h"



#define MEDIUM_VALUE(_x) ((stwodigits)alifLong_compactValue(_x)) // 23

#define IS_SMALL_INT(_iVal) (-ALIF_NSMALLNEGINTS <= (_iVal) and (_iVal) < ALIF_NSMALLPOSINTS) // 25


static AlifObject* get_smallInt(sdigit _iVal) { // 50
	return (AlifObject*)&ALIFLONG_SMALL_INTS[ALIF_NSMALLNEGINTS + _iVal];
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


AlifObject* alifLong_fromLong(long _iVal) { // 293
	AlifLongObject* v_{};
	unsigned long absIVal{}, t_{};
	AlifIntT ndigits{};

	/* Handle small and medium cases. */
	if (IS_SMALL_INT(_iVal)) {
		return get_smallInt((sdigit)_iVal);
	}
	if (-(long)ALIFLONG_MASK <= _iVal && _iVal <= (long)ALIFLONG_MASK) {
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




// 446
#define ALIF_ABS_LONG_MIN       (0-(unsigned long)LONG_MIN)
#define ALIF_ABS_SIZET_MIN      (0-(AlifUSizeT)ALIF_SIZET_MIN)



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
	//	"ALIF int too large to convert to CPP ssize_t");
	return -1;
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






AlifObject* alifLong_fromString(const char* _str, char** _pend, AlifIntT _base) { // 2995
	AlifIntT sign = 1, errorIfNonZero = 0;
	const char* orig_str = _str;
	AlifLongObject* z_ = nullptr;
	AlifObject* strObj{};
	AlifSizeT slen{};

	if ((_base != 0 and _base < 2) or _base > 36) {
		//alifErr_setString(_alifExcValueError_,
		//	"int() arg 2 must be >= 2 and <= 36");
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
	int ret = long_fromStringBase(&_str, _base, &z_);
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
	//	"invalid literal for int() with base %d: %.200R",
	//	_base, strObj);
	ALIF_DECREF(strObj);
	return nullptr;
}


AlifTypeObject _alifLongType_ = { // 6597
	.objBase = ALIFVAROBJECT_HEAD_INIT(&_alifTypeType_, 0),
	.name = "عدد_صحيح",                                   
	.basicSize = offsetof(AlifLongObject, longValue.digit),
	.itemSize = sizeof(digit),                              
	//.dealloc = long_dealloc,                               
                                        
	//.repr = long_toDecimalString,                     
	//.asNumber = &_longAsNumber_,
                                        
	//.hash = long_hash,                                  
                                        
	.getAttro = alifObject_genericGetAttr,                    
                                        
	.flags = ALIF_TPFLAGS_DEFAULT | ALIF_TPFLAGS_BASETYPE |
		ALIF_TPFLAGS_LONG_SUBCLASS |
		_ALIF_TPFLAGS_MATCH_SELF,                                                     
                                       
	.free = alifMem_objFree,                            
};
