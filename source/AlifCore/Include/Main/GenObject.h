#pragma once




typedef class AlifGenObject AlifGenObject; // 12

extern AlifTypeObject _alifGenType_;

#define ALIFGEN_CHECK(_op) ALIFOBJECT_TYPECHECK((_op), &_alifGenType_)
#define ALIFGEN_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifGenType_)






typedef class AlifCoroObject AlifCoroObject; // 27

extern AlifTypeObject _alifCoroType_;

#define ALIFCORO_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifCoroType_)





typedef class AlifAsyncGenObject AlifAsyncGenObject; // 38

extern AlifTypeObject _alifAsyncGenType_;

#define ALIFASYNCGEN_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifAsyncGenType_)
