#pragma once



extern AlifTypeObject _alifCapsuleType_;

typedef void (*AlifCapsuleDestructor)(AlifObject*); // 23



AlifObject* alifCapsule_new(void*, const char*, AlifCapsuleDestructor); // 28
