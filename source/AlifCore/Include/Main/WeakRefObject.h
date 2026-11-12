#pragma once




ALIFAPI_DATA(AlifTypeObject) _alifWeakrefRefType_;
ALIFAPI_DATA(AlifTypeObject) _alifWeakrefProxyType_;
ALIFAPI_DATA(AlifTypeObject) _alifWeakrefCallableProxyType_;

 // 15
#define ALIFWEAKREF_CHECKREF(_op) ALIFOBJECT_TYPECHECK((_op), &_alifWeakrefRefType_)
#define ALIFWEAKREF_CHECKREFEXACT(_op) \
        ALIF_IS_TYPE((_op), &_alifWeakrefRefType_)
#define ALIFWEAKREF_CHECKPROXY(_op) \
        (ALIF_IS_TYPE((_op), &_alifWeakrefProxyType_) \
         or ALIF_IS_TYPE((_op), &_alifWeakrefCallableProxyType_))





ALIFAPI_FUNC(AlifObject*) alifWeakRef_newRef(AlifObject*, AlifObject*); // 26








/* ------------------------------------------------------------------------------------- */

class AlifWeakReference { // 8
public:
	ALIFOBJECT_HEAD{};

	AlifObject* object{};
	AlifObject* callback{};
	AlifHashT hash{};

	AlifWeakReference* prev{};
	AlifWeakReference* next{};
	VectorCallFunc vectorCall{};

	AlifMutex* weakRefsLock{};
};
