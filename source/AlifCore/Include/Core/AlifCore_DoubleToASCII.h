#pragma once


#include "AlifCore_Math.h"


typedef uint32_t ULong; // 14


struct Bigint { // 16
	struct Bigint* next;
	int k, maxwds, sign, wds;
	ULong x[1];
};


 // 33
#define BIGINT_KMAX 7

#define BIGINT_POW5SIZE 8

#ifndef PRIVATE_MEM
#define PRIVATE_MEM 2304
#endif
#define BIGINT_PREALLOC_SIZE \
    ((PRIVATE_MEM+sizeof(double)-1)/sizeof(double))

class DToAState {
public:
	Bigint* p5s[BIGINT_POW5SIZE]{};
	struct Bigint* freeList[BIGINT_KMAX + 1]{};
	double preAllocated[BIGINT_PREALLOC_SIZE]{};
	double* preAllocatedNext{};
};
#define DTOA_STATE_INIT(INTERP) \
    { \
        .preAllocatedNext = (INTERP)->dtoa.preAllocated, \
    }




extern double _alif_dgStrToDouble(const char*, char**); // 62
extern char* _alif_dgDoubletoASCII(double, int, int, int*, int*, char**); // 63
extern void _alif_dgFreeDoubleToASCII(char*); // 65


extern AlifStatus _alifDtoa_init(AlifInterpreter*); // 68
