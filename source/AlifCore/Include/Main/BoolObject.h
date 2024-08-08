#pragma once

extern AlifInitObject _alifBoolType_;

#define ALIFBOOL_CHECK(x) ALIF_IS_TYPE((x), &_alifBoolType_)



extern AlifIntegerObject alifTrue;
extern AlifIntegerObject alifFalse;

#define ALIF_TRUE (AlifObject*)&alifTrue
#define ALIF_FALSE (AlifObject*)&alifFalse


AlifObject* alifBool_fromInteger(long boolean);

int alifObject_isTrue(AlifObject*);
