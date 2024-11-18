#pragma once




extern AlifTypeObject _alifWeakrefRefType_;
extern AlifTypeObject _alifWeakrefProxyType_;
extern AlifTypeObject _alifWeakrefCallableProxyType_;

 // 15
#define ALIFWEAKREF_CHECKREF(_op) ALIFOBJECT_TYPECHECK((_op), &_alifWeakrefRefType_)
#define ALIFWEAKREF_CHECKREFEXACT(_op) \
        ALIF_IS_TYPE((_op), &_alifWeakrefRefType_)
#define ALIFWEAKREF_CHECKPROXY(_op) \
        (ALIF_IS_TYPE((_op), &_alifWeakrefProxyType_) \
         or ALIF_IS_TYPE((_op), &_alifWeakrefCallableProxyType_))













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

#ifdef ALIF_GIL_DISABLED
	AlifMutex* weakRefsLock{};
#endif
};
