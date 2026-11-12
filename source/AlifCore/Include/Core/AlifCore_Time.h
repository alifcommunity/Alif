#pragma once



#define _SIZEOF_ALIFTIME_T 8 // 65

enum AlifTimeRoundT { // 67
	AlifTime_Round_FLOOR = 0,
	AlifTime_Round_CEILING = 1,
	AlifTime_Round_HALF_EVEN = 2,
	AlifTime_Round_UP = 3,
	AlifTime_Round_TIMEOUT = AlifTime_Round_UP
};



ALIFAPI_FUNC(AlifObject*) _alifLong_fromTimeT(time_t); // 96


ALIFAPI_FUNC(time_t) _alifLong_asTimeT(AlifObject*); // 100

ALIFAPI_FUNC(AlifIntT) _alifTime_objectToTimeT(AlifObject*, time_t*, AlifTimeRoundT); // 104


extern AlifTimeT _alifTime_fromMicrosecondsClamp(AlifTimeT); // 147


ALIFAPI_FUNC(AlifIntT) _alifTime_fromSecondsObject(AlifTimeT*, AlifObject*, AlifTimeRoundT); // 157


ALIFAPI_FUNC(AlifTimeT) _alifTime_asMilliseconds(AlifTimeT, AlifTimeRoundT); // 170


#ifdef _WINDOWS
extern AlifTimeT _alifTime_as100Nanoseconds(AlifTimeT, AlifTimeRoundT); // 180
#endif


ALIFAPI_FUNC(AlifIntT) _alifTime_asTimEval(AlifTimeT, struct timeval*, AlifTimeRoundT); // 199


#if defined(HAVE_CLOCK_GETTIME) or defined(HAVE_KQUEUE)

ALIFAPI_FUNC(AlifIntT) _alifTime_asTimeSpec(AlifTimeT, struct timespec*); // 232

ALIFAPI_FUNC(void) _alifTime_asTimeSpecClamp(AlifTimeT, timespec*); // 237
#endif

extern AlifTimeT _alifTime_add(AlifTimeT, AlifTimeT); // 242

class AlifClockInfoT { // 245
public:
	const char* implementation{};
	AlifIntT monotonic{};
	AlifIntT adjustable{};
	double resolution{};
};



ALIFAPI_FUNC(AlifIntT) _alifTime_localtime(time_t, struct tm*); // 276



ALIFAPI_FUNC(AlifTimeT) _alifDeadline_get(AlifTimeT); // 305


class AlifTimeFraction { // 310
public:
	AlifTimeT numer{};
	AlifTimeT denom{};
};


extern AlifIntT alifTimeFraction_set(AlifTimeFraction*, AlifTimeT, AlifTimeT); // 318

extern AlifTimeT alifTimeFraction_mul(AlifTimeT, const AlifTimeFraction*); // 325
extern double alifTimeFraction_resolution(const AlifTimeFraction*); // 330
