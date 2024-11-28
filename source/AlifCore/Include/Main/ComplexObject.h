#pragma once


extern AlifTypeObject _alifComplexType_;


#define ALIFCOMPLEX_CHECK(_op) ALIFOBJECT_TYPECHECK((_op), &_alifComplexType_) // 13
#define ALIFCOMPLEX_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifComplexType_)

/*-----------------------------------------------------------------------------------------------------*/

class AlifComplex { // 5
public:
	double real{};
	double imag{};
};


class AlifComplexObject { // 26
public:
	ALIFOBJECT_HEAD;
	AlifComplex cVal{};
};



AlifComplex alifComplex_asCComplex(AlifObject*); // 33
