#pragma once

#include "AlifCore_BytesObject.h"
#include "AlifCore_GlobalObjects.h"
#include "AlifCore_DureRun.h"



#define ALIF_LONG_DEFAULT_MAX_STR_DIGITS 4300

#define ALIF_LONG_MAX_STR_DIGITS_THRESHOLD 640 // 43

#define ALIFLONG_SMALL_INTS ALIF_SINGLETON(smallInts) // 58


static inline AlifObject* _alifLong_getZero() { // 68
	return (AlifObject*)&ALIFLONG_SMALL_INTS[ALIF_NSMALLNEGINTS];
}

static inline AlifObject* _alifLong_getOne(void) { // 73
	return (AlifObject*)&ALIFLONG_SMALL_INTS[ALIF_NSMALLNEGINTS + 1];
}



extern AlifObject* _alifLong_fromBytes(const char*, AlifSizeT, AlifIntT); // 91

extern AlifObject* _alifLong_rShift(AlifObject*, int64_t); // 107

extern AlifObject* _alifLong_lShift(AlifObject*, int64_t); // 111


AlifObject* _alifLong_add(AlifLongObject*, AlifLongObject*); // 113

extern unsigned char _alifLongDigitValue_[256]; // 118






 // 164
#define SIGN_MASK 3
#define SIGN_ZERO 1
#define SIGN_NEGATIVE 2
#define NON_SIZE_BITS 3

static inline AlifIntT _alifLong_isNonNegativeCompact(const AlifLongObject* _op) { // 192
	return _op->longValue.tag <= (1 << NON_SIZE_BITS);
}

static inline AlifIntT _alifLong_bothAreCompact(const AlifLongObject* _a,
	const AlifLongObject* _b) { // 198
	return (_a->longValue.tag | _b->longValue.tag) < (2 << NON_SIZE_BITS);
}


static inline bool _alifLong_isZero(const AlifLongObject* _op) { // 205
	return (_op->longValue.tag & SIGN_MASK) == SIGN_ZERO;
}

static inline bool _alifLong_isNegative(const AlifLongObject* op) { // 211
	return (op->longValue.tag & SIGN_MASK) == SIGN_NEGATIVE;
}

static inline bool _alifLong_isPositive(const AlifLongObject* op) { // 217
	return (op->longValue.tag & SIGN_MASK) == 0;
}

static inline AlifSizeT alifLong_digitCount(const AlifLongObject* _op) { // 223
	return (AlifSizeT)(_op->longValue.tag >> NON_SIZE_BITS);
}

static inline AlifSizeT _alifLong_signedDigitCount(const AlifLongObject* _op) { // 232
	AlifSizeT sign = 1 - (_op->longValue.tag & SIGN_MASK);
	return sign * (AlifSizeT)(_op->longValue.tag >> NON_SIZE_BITS);
}

static inline AlifIntT _alifLong_compactSign(const AlifLongObject* _op) { // 239
	return 1 - (_op->longValue.tag & SIGN_MASK);
}

static inline AlifIntT alifLong_nonCompactSign(const AlifLongObject* _op) { // 247
	return 1 - (_op->longValue.tag & SIGN_MASK);
}

static inline AlifIntT _alifLong_sameSign(const AlifLongObject* _a,
	const AlifLongObject* _b) { // 256
	return (_a->longValue.tag & SIGN_MASK) == (_b->longValue.tag & SIGN_MASK);
}


#define TAG_FROM_SIGN_AND_SIZE(_sign, _size) ((uintptr_t)(1 - (_sign)) | ((uintptr_t)(_size) << NON_SIZE_BITS)) // 262

static inline void _alifLong_setSignAndDigitCount(AlifLongObject* _op,
	AlifIntT _sign, AlifSizeT _size) { // 264
	_op->longValue.tag = TAG_FROM_SIGN_AND_SIZE(_sign, _size);
}

static inline void _alifLong_setDigitCount(AlifLongObject* _op, AlifSizeT _size) { // 273
	_op->longValue.tag = (((AlifUSizeT)_size) << NON_SIZE_BITS) | (_op->longValue.tag & SIGN_MASK);
}


#define NON_SIZE_MASK ~(uintptr_t)((1 << NON_SIZE_BITS) - 1) // 280

static inline void _alifLong_flipSign(AlifLongObject* op) { // 282
	AlifUIntT flippedSign = 2 - (op->longValue.tag & SIGN_MASK);
	op->longValue.tag &= NON_SIZE_MASK;
	op->longValue.tag |= flippedSign;
}


#define ALIFLONG_DIGIT_INIT(val) \
    { \
        .objBase = ALIFOBJECT_HEAD_INIT(&_alifLongType_), \
        .longValue  = { \
            .tag = TAG_FROM_SIGN_AND_SIZE( \
                (val) == 0 ? 0 : ((val) < 0 ? -1 : 1), \
                (val) == 0 ? 0 : 1), \
            .digit = { ((val) >= 0 ? (val) : -(val)) }, \
        } \
    }

#define ALIFLONG_FALSE_TAG TAG_FROM_SIGN_AND_SIZE(0, 0) // 300
#define ALIFLONG_TRUE_TAG TAG_FROM_SIGN_AND_SIZE(1, 1) // 301
