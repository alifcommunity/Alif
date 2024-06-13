#pragma once



extern AlifTypeObject _alifCapsuleType_;

typedef void (*AlifCapsuleDestructor)(AlifObject*);

AlifObject* alifCapsule_new(void*, const wchar_t*, AlifCapsuleDestructor);

void* alifCapsule_getPointer(AlifObject*, const wchar_t*);