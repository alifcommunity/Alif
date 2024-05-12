#pragma once

extern AlifInitObject typeBool;


extern AlifIntegerObject alifTrue;
extern AlifIntegerObject alifFalse;

#define ALIF_TRUE (AlifObject*)&alifTrue
#define ALIF_FALSE (AlifObject*)&alifFalse


AlifObject* alifBool_fromInteger(long boolean);

int alifObject_isTrue(AlifObject*);