#pragma once

#include "AlifCore_DureRun.h"




#define ALIFLONG_SMALL_INTS ALIF_SINGLETON(smallInts)


extern unsigned char _alifLongDigitValue_[256]; // 118






 // 164
#define SIGN_MASK 3
#define SIGN_ZERO 1
#define SIGN_NEGATIVE 2
#define NON_SIZE_BITS 3


static inline bool _alifLong_isNegative(const AlifLongObject* _op) { // 212
	return (_op->longValue.tag & SIGN_MASK) == SIGN_NEGATIVE;
}


static inline AlifSizeT alifLong_digitCount(const AlifLongObject* _op) { // 223
	return _op->longValue.tag >> NON_SIZE_BITS;
}

static inline AlifIntT alifLong_nonCompactSign(const AlifLongObject* _op) { // 247
	return 1 - (_op->longValue.tag & SIGN_MASK);
}


#define TAG_FROM_SIGN_AND_SIZE(_sign, _size) ((1 - (_sign)) | ((_size) << NON_SIZE_BITS)) // 262


#define ALIFLONG_FALSE_TAG TAG_FROM_SIGN_AND_SIZE(0, 0) // 300
#define ALIFLONG_TRUE_TAG TAG_FROM_SIGN_AND_SIZE(1, 1) // 301
