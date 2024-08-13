#pragma once

class AlifFloatObject {
public:
	ALIFOBJECT_HEAD
	long double digits_{};
};

extern AlifInitObject _alifFloatType;

#define FLOAT_MAX 1.7976931348623157e308
#define FLOAT_MIN 2.2250738585072014e-308

long double alifFloat_getMax();
long double alifFloat_getMin();


AlifObject* alifFloat_fromString(AlifObject*);
AlifObject* alifFloat_fromDouble(long double);

long double alifFloat_asLongDouble(AlifObject*);