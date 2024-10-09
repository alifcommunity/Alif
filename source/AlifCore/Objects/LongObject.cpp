#include "alif.h"

#include "AlifCore_Long.h"



#define MEDIUM_VALUE(_x) ((stwodigits)alifLong_compactValue(_x)) // 23

#define IS_SMALL_INT(_iVal) (-ALIF_NSMALLNEGINTS <= (_iVal) and (_iVal) < ALIF_NSMALLPOSINTS) // 25


static AlifObject* get_smallInt(sdigit _iVal) { // 50
	return (AlifObject*)&ALIFLONG_SMALL_INTS[ALIF_NSMALLNEGINTS + _iVal];
}


 // 446
#define ALIF_ABS_LONG_MIN         (0-(unsigned long)LONG_MIN)
#define ALIF_ABS_SIZET_MIN      (0-(AlifUSizeT)ALIF_SIZET_MIN)



AlifLongObject* _alifLong_fromDigits(AlifIntT _negative, AlifSizeT _digitCount, digit* _digits) { // 172
	if (_digitCount == 0) {
		return (AlifLongObject*)_alifLong_getZero();
	}
	AlifLongObject* result = _alifLong_new(_digitCount);
	if (result == nullptr) {
		//alifErr_noMemory();
		return NULL;
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










unsigned char _alifLongDigitValue_[256] = {
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
