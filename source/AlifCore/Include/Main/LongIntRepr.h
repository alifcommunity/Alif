#pragma once



 // 42
#if ALIFLONG_BITS_IN_DIGIT == 30
typedef uint32_t digit;
typedef int32_t sdigit;
typedef uint64_t twodigits;
typedef int64_t stwodigits;
#define ALIFLONG_SHIFT    30
#define ALIFLONG_DECIMAL_SHIFT   9 /* max(e such that 10**e fits in a digit) */
#define ALIFLONG_DECIMAL_BASE    ((digit)1000000000) /* 10 ** DECIMAL_SHIFT */
#elif ALIFLONG_BITS_IN_DIGIT == 15
typedef unsigned short digit;
typedef short sdigit;
typedef unsigned long twodigits;
typedef long stwodigits;
#define ALIFLONG_SHIFT    15
#define ALIFLONG_DECIMAL_SHIFT   4 /* max(e such that 10**e fits in a digit) */
#define ALIFLONG_DECIMAL_BASE    ((digit)10000) /* 10 ** DECIMAL_SHIFT */
#else
#error "ALIFLONG_BITS_IN_DIGIT should be 15 or 30"
#endif
#define ALIFLONG_BASE     ((digit)1 << ALIFLONG_SHIFT)
#define ALIFLONG_MASK     ((digit)(ALIFLONG_BASE - 1))







class AlifLongValue { // 93
public:
	uintptr_t tag{};
	uint32_t digit[1]{};
};

class AlifLongObject { // 98
public:
	ALIFOBJECT_HEAD;
	AlifLongValue longValue{};
};



 // 117
#define ALIFLONG_SIGN_MASK 3
#define ALIFLONG_NON_SIZE_BITS 3

static inline AlifIntT alifLong_isCompact(const AlifLongObject* _op) { // 121
	return _op->longValue.tag < (2 << ALIFLONG_NON_SIZE_BITS);
}


static inline AlifSizeT alifLong_compactValue(const AlifLongObject* _op) { // 129
	AlifSizeT sign{};
	sign = 1 - (_op->longValue.tag & ALIFLONG_SIGN_MASK);
	return sign * (AlifSizeT)_op->longValue.digit[0];
}
