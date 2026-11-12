#pragma once



ALIFAPI_DATA(AlifTypeObject) _alifCapsuleType_;

typedef void (*AlifCapsuleDestructor)(AlifObject*); // 23

#define ALIFCAPSULE_CHECKEXACT(_op) ALIF_IS_TYPE((_op), &_alifCapsuleType_) // 25

ALIFAPI_FUNC(AlifObject*) alifCapsule_new(void*, const char*, AlifCapsuleDestructor); // 28


ALIFAPI_FUNC(void*) alifCapsule_getPointer(AlifObject*, const char*); // 33
