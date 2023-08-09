#pragma once
#include <cstdint>

class AlifClockInfoT {
public:
	const char* implementation;
	int monotonic;
	int adjustable;
	double resolution;
};

 enum AlifTimeRoundT {

	alifTimeRoundFloor = 0,

	 alifTimeRoundCeiling = 1,

	 alifTimeRoundHalfEven = 2,

	 alifTimeRoundUp = 3,

	 alifTimeRoundTimeout = alifTimeRoundUp
};

int64_t alifTime_add(int64_t t1, int64_t t2);

inline int _alifTime_add(int64_t* t1, int64_t t2);

int64_t alifTime_asMicroseconds(int64_t t, AlifTimeRoundT round);

int64_t alifTime_getPerfCounter();
