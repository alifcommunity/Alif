#pragma once




extern AlifTimeT _alifTime_add(AlifTimeT, AlifTimeT);

class AlifClockInfoT { // 245
public:
	const char* implementation{};
	AlifIntT monotonic{};
	AlifIntT adjustable{};
	double resolution{};
};









class AlifTimeFraction { // 310
public:
	AlifTimeT numer{};
	AlifTimeT denom{};
};


extern AlifIntT alifTimeFraction_set(AlifTimeFraction*, AlifTimeT, AlifTimeT); // 318

extern AlifTimeT alifTimeFraction_mul(AlifTimeT, const AlifTimeFraction*); // 325
extern double alifTimeFraction_resolution(const AlifTimeFraction*); // 330
