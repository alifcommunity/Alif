#pragma once





















































class TimeRuntimeState {
public:
#ifdef HAVE_TIMES
	int ticksPerSecondInitialized;
	long ticksPerSecond;
#else
	int notUsed;
#endif
};

#ifdef __clang__
struct timeval;
#endif




typedef int64_t alifTimeT;

#define ALIFTIME_MIN INT64_MIN

#define ALIFTIME_MAX INT64_MAX
#define SIZEOF ALIFTIMET 8

enum AlifTimeRoundT {


	AlifTime_Round_Floor = 0,


	AlifTime_Round_Ceiling = 1,


	AlifTime_Round_HalfEven = 2,






	AlifTime_Round_Up = 3,


	AlifTime_Round_TimeOut = AlifTime_Round_Up
} ;






































#define ALIFTIME_FROM_SECONDS(seconds) ((alifTimeT)(seconds) * (1000 * 1000 * 1000))



ALIFAPI_FUNC(alifTimeT) alifTime_fromNanoseconds(alifTimeT ns);





























ALIFAPI_FUNC(alifTimeT) alifTime_asMicroseconds(alifTimeT t, AlifTimeRoundT round);
































































extern alifTimeT alifTime_add(alifTimeT t1, alifTimeT t2);




extern alifTimeT alifTime_mulDiv(alifTimeT ticks,
	alifTimeT mul,
	alifTimeT div);


class AlifClockInfoT {
public:
	const char* implementation;
	int monotonic;
	int adjustable;
	double resolution;
} ;


























































ALIFAPI_FUNC(alifTimeT) alifTime_getPerfCounter(void);
