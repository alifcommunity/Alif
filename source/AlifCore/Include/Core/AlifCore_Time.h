#pragma once






AlifTimeT _alifTime_fromMicrosecondsClamp(AlifTimeT); // 147


#if defined(HAVE_CLOCK_GETTIME) or defined(HAVE_KQUEUE)
void _alifTime_asTimeSpecClamp(AlifTimeT, timespec*); // 237
#endif

extern AlifTimeT _alifTime_add(AlifTimeT, AlifTimeT);

class AlifClockInfoT { // 245
public:
	const char* implementation{};
	AlifIntT monotonic{};
	AlifIntT adjustable{};
	double resolution{};
};






AlifTimeT alifDeadline_get(AlifTimeT); // 305


class AlifTimeFraction { // 310
public:
	AlifTimeT numer{};
	AlifTimeT denom{};
};


extern AlifIntT alifTimeFraction_set(AlifTimeFraction*, AlifTimeT, AlifTimeT); // 318

extern AlifTimeT alifTimeFraction_mul(AlifTimeT, const AlifTimeFraction*); // 325
extern double alifTimeFraction_resolution(const AlifTimeFraction*); // 330
