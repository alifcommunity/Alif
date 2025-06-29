#pragma once


#include "AlifCore_CriticalSection.h"
#include "AlifCore_Lock.h"


 // 18
#define WEAKREF_LIST_LOCK(_obj) \
    _alifInterpreter_get()  \
        ->weakrefLocks[((uintptr_t)_obj) % NUM_WEAKREF_LIST_LOCKS]

#define LOCK_WEAKREFS(_obj) \
    alifMutex_lockFlags(&WEAKREF_LIST_LOCK(_obj), AlifLockFlags_::Alif_Lock_Dont_Detach)
#define UNLOCK_WEAKREFS(_obj) ALIFMUTEX_UNLOCK(&WEAKREF_LIST_LOCK(_obj))



static inline AlifObject* _alifWeakRef_getRef(AlifObject* ref_obj) { // 58
	AlifWeakReference* ref = ALIF_CAST(AlifWeakReference*, ref_obj);

	AlifObject* obj = (AlifObject*)alifAtomic_loadPtr(&ref->object);
	if (obj == ALIF_NONE) {
		// clear_weakref() was called
		return nullptr;
	}

	LOCK_WEAKREFS(obj);
	if (ref->object == ALIF_NONE) {
		// clear_weakref() was called
		UNLOCK_WEAKREFS(obj);
		return nullptr;
	}
	if (alif_tryIncRef(obj)) {
		UNLOCK_WEAKREFS(obj);
		return obj;
	}
	UNLOCK_WEAKREFS(obj);
	return nullptr;
}



extern AlifSizeT _alifWeakref_getWeakrefCount(AlifObject*); // 108
