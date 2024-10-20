#pragma once



extern AlifTypeObject _alifFloatType_; // 14

 // 16
#define ALIFFLOAT_CHECK(_op) ALIFOBJECT_TYPECHECK(_op, &_alifFloatType_)
#define ALIFFLOAT_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifFloatType_)

AlifObject* alifFloat_fromDouble(double); // 39







/*----------------------------------------------------------------------------------------------------------------------------------------------------------------*/

class AlifFloatObject { // 5
public:
	ALIFOBJECT_HEAD;
	double val{};
};


#define ALIFFLOAT_CAST(_op) (ALIF_CAST(AlifFloatObject*, _op)) // 10

static inline double _alifFloat_asDouble(AlifObject* _op) { // 15
	return ALIFFLOAT_CAST(_op)->val;
}
#define ALIFFLOAT_AS_DOUBLE(_op) _alifFloat_asDouble(ALIFOBJECT_CAST(_op))
